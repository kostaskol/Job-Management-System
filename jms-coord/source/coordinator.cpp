#include <pipemanager.h>
#include <sys/stat.h>
#include <helpfunc.h>
#include <constants.h>
#include <wait.h>
#include "coordinator.h"
#include <cstring>

#define WRITE 0
#define READ 1

using namespace std;

my_vector<pid_t> g_active_pools_pid;
hash_table<my_string> g_pool_pid_id_map;
str_vec g_active_pools;
my_string get_pool_w(int);
my_string get_pool_r(int);
void pool_listener(int sig);

// Installs - Uninstalls a SIGCHLD signal blocker
void install_blocker();
void uninstall_blocker();

coordinator::coordinator(cmd_arguments &args)
        : _max_jobs(args.get_p_size()), _r_pipe(args.get_r_pipe()),
        _w_pipe(args.get_w_pipe()), _dir_path(args.get_path()), _next_job(0),
        _num_pools(0) {
            // pipes[0] -> write
            _coord_pipes.push(pipe_manager(_w_pipe, O_WRONLY));

            //pipes[1] -> read
            _coord_pipes.push(pipe_manager(_r_pipe, O_RDONLY));
        }

coordinator::~coordinator() {}

void coordinator::start() {
    // Setup the SIGCHLD signal handler
    // and add a SIGCHLD blocker mask
    struct sigaction sig_handl;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    memset(&sig_handl, 0, sizeof(sig_handl));

    sig_handl.sa_handler = pool_listener;
    sig_handl.sa_flags = 0;
    sig_handl.sa_mask = mask;

    sigaction(SIGCHLD, &sig_handl, NULL);
    mkdir(_dir_path.c_str(), 0777);

    cout << "Coordinator started" << endl;
    my_string msg;

    // An initial handshake with the console is required
    // before beginning
    if (_coord_pipes.at(READ).read_msg(&msg) < 0) {
        perror("read fifo");
    }

    if (!(msg == "ack?")) {
        cout << "Coord: Unexpected handshake: " << msg << "...Quitting" << endl;
        return;
    }

    if (msg == "ack?")
        _coord_pipes.at(WRITE).write_msg("ack");

    cout << "Coord: Handshake successful. Starting operations" << endl;
    // We don't want to be interrupted by a pool's death while
    // reading from the pipe. This can cause slight information
    // synchronization problems (where the console is required to 
    // send a message for the coordinator to be informed of a 
    // pool's death)
    install_blocker();
    do {
        // Add all of the active pool and the coord pipe names
        // to a vector object, and use the select() syscall on them
        my_vector<pipe_manager> fd_vec;
        fd_vec.push(_coord_pipes.at(READ));
        for (size_t i = 0; i < g_active_pools.size(); i++) {
            fd_vec.push(pipe_manager(get_pool_w(g_active_pools.at(i).to_int()),
                          O_RDONLY));
        }

        int bytes;
        cout << "[Waiting]" << endl;
        bytes = pipe_manager::select_read(&msg, fd_vec);
        cout << "[Got command]" << endl;
        // We don't mind select to be interrupted due to a received signal
        if (bytes < 0 && errno != EINTR) {
            perror("Coord: Select");
            cout << "Coordinator exiting abnormally" << endl;
            exit(-1);
        }

        if (bytes == 0) continue;

        // Decode the received message and figure out the required
        // operation
          msg::message message(msg, PROT_REQ);
          message.decode();
          my_vector<my_string> vec = message.get_list();

          if (vec.size() == 0) continue;

          switch (hf::get_com(vec, false)) {
              case C_SUB: {
                my_string pid;
                  my_vector<my_string> cmd;
                  cmd.push("0");
                  cmd.push(_next_job);

                  for (size_t i = 1; i < vec.size(); i++) {
                      cmd.push(vec.at(i));
                  }

                  // submit the job to the correct pool
                  _submit(cmd, &pid);

                  // Add the jobs to the map
                  _jobs_pid_id_map.insert_key(pid, _next_job);
                  _jobs_id_pid_map.insert_key(_next_job, pid);

                  // Prepare the console response
                  my_vector<my_string> vec_resp;

                  vec_resp.push("0");
                  vec_resp.push(my_string(_next_job));
                  vec_resp.push(pid);

                  msg::message resp_msg(vec_resp, PROT_RESP);
                  resp_msg.encode();

                  _coord_pipes.at(WRITE).write_msg(resp_msg);

                  _next_job++;

                  break;
              }
              case C_STAT: {
                  try {
                      cout << "Coord: Status for jID " << vec.at(1) << endl;
                      _get_status(vec.at(1));
                  } catch (exception &e) {
                      cerr << "Coord: Malformed command" << endl;
                      return;
                  }
                  break;
              }
              case C_STAT_ALL: {
                  cout << "Coord: Status for all jobs";
                  my_string tmp = vec.at(1);
                  int time = atoi(tmp.c_str());
                  if (time == -1) cout << " (No time limit)" << endl;
                  else cout << " (Time limit: " << time << endl;
                  _get_status_all(time);
                  break;
              }
              case C_SHOW_ACT: {
                  cout << "Coord: Request to show active" << endl;
                  _get_active();
                  break;
              }
              case C_SHOW_POOL: {
                  cout << "Coord: Request to show pools" << endl;
                  _get_pools();
                  break;
              }
              case C_SHOW_FIN: {
                  cout << "Coord: Request to show finished" << endl;
                  _get_finished();
                  break;
              }
              case C_SUSP: {
                  try {
                      cout << "Coord: Request to suspend jID " << vec.at(1)
                           << endl;
                      _suspend(vec.at(1));
                  } catch(exception &e) {
                      cerr << "Coord: Malformed command" << endl;
                      return;
                  }
                  break;
              }
              case C_RES: {
                  try {
                      cout << "Coord: Request to resume jID " << vec.at(1)
                           << endl;
                      _resume(vec.at(1));
                  } catch (exception &e) {
                      cerr << "Coord: Malformed command" << endl;
                      return;
                  }
                  break;
              }
              case C_SD: {
                  cout << "Coord: Request to shutdown" << endl;
                  _shutdown();
                  return;
              }
              case C_JOB_DONE: {
                  _job_death(vec.at(1), vec.at(2));
                  break;
              }
              default: {
                ;
              }
        }
	// After we have performed the whole operation, we can check
	// for any waiting signals (SIGCHLD)
        uninstall_blocker();

    } while (true);
}

void coordinator::_submit(my_vector<my_string> vec, my_string *pid) {
    int next_pool = _get_next_pool();

    if (next_pool >= _num_pools) {
        // We need to create a new pool
        cout << "Coord: Creating new pool (" << next_pool << ")" << endl;

        my_string pool_r = get_pool_r(next_pool);
        my_string pool_w = get_pool_w(next_pool);
        pool p(pool_r, pool_w, _max_jobs);
        _num_pools++;
        pid_t pool_pid = p.begin();
        g_active_pools_pid.push(pool_pid);
        g_active_pools.push(my_string(next_pool));
        g_pool_pid_id_map.insert_key(pool_pid, next_pool);
        _jobs_per_pool.insert_key(next_pool, 0);
    }

    my_string pool_w = get_pool_r(next_pool);
    my_string pool_r = get_pool_w(next_pool);

    // Prepare to submit the job to the pool for
    // execution
    msg::message msg(vec, PROT_P_REQ);
    msg.encode();

    pipe_manager writer(pool_w, O_WRONLY);
    pipe_manager reader(pool_r, O_RDONLY);

    writer.write_msg(msg);

    my_string resp;

    // Need to do this loop because sometimes the pool
    // sends an empty string as response first, and then
    // sends the actual response (couldn't figure out why)
    do {
      if (reader.read_timeout(&resp, 6) == 0) {
        cout << "Coord: Pool didn't respond within 6 seconds" << endl;
        return;
      }
    } while (resp == "");

    _job_timer.push(time(NULL));

    // Increase the job counter for this pool
    // by one
    int tmp_j;
    _jobs_per_pool.get_key(next_pool, &tmp_j);
    _jobs_per_pool.set_key(next_pool, tmp_j + 1);

    // Get the pool's response (the job's pid)
    msg::message resp_msg(resp, PROT_P_RESP);
    resp_msg.decode();

    my_vector<my_string> resp_vec = resp_msg.get_list();

    *pid = resp_vec.at(1);

}

void coordinator::_get_status(my_string id) {
    str_vec resp_vec;
    if (id.to_int() >= _next_job || id.to_int() < 0) {
        resp_vec.push("1");
        resp_vec.push("3");
    } else {
        resp_vec.push("1");
        if (_fin_jobs.in(id)) {
            resp_vec.push("2");
        } else if (_susp_jobs.in(id)) {
            resp_vec.push("1");
        } else {
            int now = time(NULL);
            try {
                int t_run = now - _job_timer.at(id.to_int());
                resp_vec.push("0");
                resp_vec.push(t_run);
            } catch (exception &e) {
                cout << "Coord: Status: Desynchronised job timer vector"
                     << endl;
                my_vector<my_string> tmp_vec;
                tmp_vec.push("3");
                resp_vec = tmp_vec;
                return;
            }
        }
    }


    msg::message resp_msg(resp_vec, PROT_RESP);

    resp_msg.encode();

    _coord_pipes.at(WRITE).write_msg(resp_msg);
}

void coordinator::_get_status_all(int time_lim) {
  my_vector<my_string> resp_vec;
  resp_vec.push("2");
  for (int i = 0; i < _next_job; i++) {
    if (_fin_jobs.in(i)) {
      resp_vec.push("2");
    } else if (_susp_jobs.in(i)) {
      resp_vec.push("1");
    } else {
        int lim = -1;
        try {
          lim = time(NULL) - _job_timer.at(i);
        } catch (exception &e) {
          cout << "Desynchronised job timer" << endl;
        }
        if (time_lim == -1) {
          resp_vec.push("0");
          resp_vec.push(lim);
        } else if (lim <= time_lim) {
          resp_vec.push("0");
          resp_vec.push(lim);
        }
    }
  }

  // We now have a full vector of job statuses
  msg::message resp_msg(resp_vec, PROT_RESP);
  resp_msg.encode();

  _coord_pipes.at(WRITE).write_msg(resp_msg);

}

void coordinator::_get_active() {
    my_vector<my_string> resp_vec;
    resp_vec.push("3");
    for (int i = 0; i < _next_job; i++) {
        if (!_fin_jobs.in(i) && !_susp_jobs.in(i)) {
            resp_vec.push(i);
        }
    }

    // We now have a vector of all non - suspended and non - finished jobs
    msg::message resp_msg(resp_vec, PROT_RESP);
    resp_msg.encode();
    _coord_pipes.at(WRITE).write_msg(resp_msg);
}

void coordinator::_get_pools() {
  my_vector<my_string> resp_vec;
  resp_vec.push("4");
  for (size_t i = 0; i < g_active_pools_pid.size(); i++) {
    my_string pid = g_active_pools_pid.at(i);
    my_string id;
    g_pool_pid_id_map.get_key(pid, &id);
    int jobs;
    _jobs_per_pool.get_key(id, &jobs);
    my_string msg = pid;
    msg += "-"; msg += jobs;
    resp_vec.push(msg);
  }

  msg::message resp_msg(resp_vec, PROT_RESP);
  resp_msg.encode();

  _coord_pipes.at(WRITE).write_msg(resp_msg);
}

void coordinator::_get_finished() {
  str_vec resp_vec;
  resp_vec.push("5");
  for (size_t i = 0; i < _fin_jobs.size(); i++) {
    resp_vec.push(_fin_jobs.at(i));
  }

  msg::message resp_msg(resp_vec, PROT_RESP);
  resp_msg.encode();
  _coord_pipes.at(WRITE).write_msg(resp_msg);
}

void coordinator::_suspend(my_string id) {
  str_vec resp_vec;
  if (_fin_jobs.in(id)) {
    resp_vec.push("6");
    resp_vec.push("F");
  } else if (_susp_jobs.in(id)) {
    resp_vec.push("6");
    resp_vec.push("E");
  } else {
    str_vec req_vec;
    my_string jpid;
    resp_vec.push("6");
    bool ex = _jobs_id_pid_map.get_key(id, &jpid);
    if (!ex) {
      resp_vec.push("I");
    } else {
      req_vec.push("1");
      req_vec.push(jpid);

      msg::message req_msg(req_vec, PROT_P_REQ);
      req_msg.encode();

      int pool_num = _get_pool(id.to_int());
      my_string r = get_pool_w(pool_num);
      my_string w = get_pool_r(pool_num);
      pipe_manager read_pipe(r, O_RDONLY);
      pipe_manager write_pipe(w, O_WRONLY);
      int ret = write_pipe.write_timeout(req_msg, 3);
      if (ret == 0 || ret == -1) {
        resp_vec.push("X");
      } else {
        my_string p_resp;
        read_pipe.read_msg(&p_resp);
        msg::message resp_msg(p_resp, PROT_P_RESP);
        resp_msg.decode();
        if ((resp_msg.get_list().at(0) != "1")
            || (resp_msg.get_list().at(1) != "K")) {
              cout << "Coord: Unexpected response for suspend: "
                   << p_resp << endl;
              resp_vec.push("X");
        } else {
          resp_vec.push("K");
          _susp_jobs.push(id);
        }
      }
    }
  }

  msg::message resp_msg(resp_vec, PROT_RESP);
  resp_msg.encode();

  _coord_pipes.at(WRITE).write_msg(resp_msg);
}

void coordinator::_resume(my_string id) {
  str_vec req_vec;
  str_vec resp_vec;
  if (!_susp_jobs.in(id)) {
      resp_vec.push("7");
      resp_vec.push("S");
  } else {
    my_string jpid;
    resp_vec.push("7");
    bool ex = _jobs_id_pid_map.get_key(id, &jpid);
    if (!ex) {
      resp_vec.push("X");
    } else {
      req_vec.push("2");
      req_vec.push(jpid);

      msg::message req_msg(req_vec, PROT_P_REQ);
      req_msg.encode();

      int pool_num = _get_pool(id.to_int());
      my_string r = get_pool_w(pool_num);
      my_string w = get_pool_r(pool_num);
      pipe_manager read_pipe(r, O_RDONLY);
      pipe_manager write_pipe(w, O_WRONLY);
      write_pipe.write_msg(req_msg);

      my_string p_resp;
      read_pipe.read_msg(&p_resp);
      msg::message resp_msg(p_resp, PROT_P_RESP);
      resp_msg.decode();
      if ((resp_msg.get_list().at(0) != "2")
          || (resp_msg.get_list().at(1) != "K")) {
            cout << "Coord: Unexpected response for resume: "
                 << p_resp << endl;
            resp_vec.push("X");
      } else {
        resp_vec.push("K");
        _susp_jobs.remove(id);
      }
    }
  }

  msg::message resp_msg(resp_vec, PROT_RESP);
  resp_msg.encode();

  _coord_pipes.at(WRITE).write_msg(resp_msg);
}

void coordinator::_shutdown() {
  // TODO: Change protocol to return
  // statistics
  // e.g. Jobs served, jobs in progress
    for (size_t i = 0; i < g_active_pools_pid.size(); i++) {
      kill(g_active_pools_pid.at(i), SIGTERM);
      while (waitpid(-1, NULL, WNOHANG) > 0) {}
    }
    str_vec resp_vec;
    resp_vec.push("8");
    resp_vec.push(_next_job);
    int counter = 0;
    for (int i = 0; i < _next_job; i++) {
      if (!_fin_jobs.in(i) && !_susp_jobs.in(i))
        counter++;
    }
    resp_vec.push(counter);
    msg::message resp_msg(resp_vec, PROT_RESP);
    resp_msg.encode();
    _coord_pipes.at(WRITE).write_msg(resp_msg);
}

void coordinator::_job_death(my_string pid, my_string pool_pid) {
    my_string id;
    bool ex = _jobs_pid_id_map.get_key(pid, &id);
    _fin_jobs.push(my_string(id.to_int()));

    // Figure out pool number
    my_string pool_id;
    if (!g_pool_pid_id_map.get_key(pool_pid, &pool_id)) {
      cout << "Coord: Got job death from unknown pool: " << pool_pid << endl;
      return;
    }

    // Open a pipe for the pool id
    pipe_manager write(get_pool_r(pool_id.to_int()), O_WRONLY);

    my_vector<my_string> resp_vec;
    resp_vec.push("j");
    if (ex) {
      resp_vec.push("K");
    } else {
      resp_vec.push("X");
    }

    msg::message resp_msg(resp_vec, PROT_RESP);
    resp_msg.encode();

    cout << "Coord: Writing message to pool #" << pool_id << "(" << pool_pid << ")" << endl;
    int ret = write.write_timeout(resp_msg, 3);
    if (ret == 0) {
      cout << "Coord: Timedout while responding to pool" << endl;
    } else {
      cout << "Coord: Job with jID " << id << " just died" << endl;
    }
}

void install_blocker() {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);

  sigprocmask(SIG_BLOCK, &mask, NULL);
}

void uninstall_blocker() {
  // Checks for any pending signals
  sigset_t waiting;

  sigpending(&waiting);

  if (sigismember(&waiting, SIGCHLD)) {
    pool_listener(SIGCHLD);
  }
}

int coordinator::_get_next_pool() {
    return _next_job / _max_jobs;
}

int coordinator::_get_pool(int jid) {
    return (jid + 1) / _max_jobs;
}

// Return the read and write pipe name
// of the pool with the specified id

// This is the name of the pipe that the
// pool READS from (so we need to switch them)
my_string get_pool_r(int id) {
    my_string pool_r = "ppipes/"; pool_r += "pp_"; pool_r += id; pool_r += "_r";
    return pool_r;
}

my_string get_pool_w(int id) {
    my_string pool_w = "ppipes/"; pool_w += "pp_"; pool_w += id; pool_w += "_w";
    return pool_w;
}

void pool_listener(int sig) {
  pid_t p;
  while ((p = waitpid(-1, NULL, WNOHANG)) > 0) {
    cout << "Coord: Got pool death #" << p << "@";
    // hf::print_stamp();
    cout << endl;

    my_string pool_id;
    bool ex = g_pool_pid_id_map.get_key(p, &pool_id);

    if (!ex) {
      cout << "Unsynchronised pid-id map" << endl;
      return;
    }

    g_active_pools_pid.remove(p);
    g_active_pools.remove(pool_id);
    cout << "Coord: Pool with pID " << pool_id << " (" << p << ")"
         << " just died" << endl;
  }
}
