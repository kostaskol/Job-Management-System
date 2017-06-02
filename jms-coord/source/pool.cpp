#include <pipemanager.h>
#include <fcntl.h>
#include <iostream>
#include <wait.h>
#include <cstring>
#include <helpfunc.h>
#include <constants.h>
#include "pool.h"

using namespace std;

my_string g_r_pipe;
my_string g_w_pipe;
int g_max;
int g_current;
int g_served;
bool g_done = false;
my_vector<my_string> g_job_pids;
my_vector<my_string> g_fin;

void listener(int);
void alrm(int);
void install_blocker(int);
void uninstall_blocker(int);
void killer(int);

// Each pool object exists in the same process as the coord process
// However, in the pool::submit function, a new process is started for each pool.
// This means that the object which holds all of the information about the current
// state of the executed - executing jobs exists in the parent process but the actual
// pool work is done in a seperate process
pool::pool(my_string r, my_string w, int max) : _r_pipe(r), _w_pipe(w) {
    g_current = 0;
    g_r_pipe = _r_pipe;
    g_w_pipe = _w_pipe;
    g_max = max;

}


pool::pool() {}

pool::~pool() {}

int pool::begin() {

    pid_t pid = fork();

    if (!pid) {
        // This is the process that does all the work

        // Install a SIGCHLD handler
        struct sigaction sig_handl;
        struct sigaction kill_handl;

        memset(&sig_handl, 0, sizeof(sig_handl));
        memset(&kill_handl, 0, sizeof(sig_handl));

        sig_handl.sa_handler = listener;
        sig_handl.sa_flags = 0;

        kill_handl.sa_handler = killer;
        sig_handl.sa_flags = 0;

        sigaction(SIGCHLD, &sig_handl, NULL);
        sigaction(SIGTERM, &kill_handl, NULL);

        cout << "Pool: Pool started" << endl;
        pipe_manager read_pipe(_r_pipe, O_RDONLY);
        pipe_manager write_pipe(_w_pipe, O_WRONLY);

	install_blocker(SIGCHLD);
        do {

            my_string com = "";

            bool timedout = false;
            bool intrptd = false;

            int bytes = read_pipe.read_timeout(&com, 1);

            if (bytes == -1 && errno != EINTR) {
                perror("Pool error: Read");
            }

            if (errno == EINTR) {
              errno = 0;
              continue;
            }

            bool got_msg = false;
            my_vector<my_string> vec;

	    // We check whether we have any waiting job deaths
	    // only if we timedout during read (otherwise we have an operation we 
	    // we are required to perform)
	    if (bytes == 0) {
              if (g_fin.size() != 0) {
                  bool stop = false;
                  for (size_t i = 0; i < g_fin.size(); i++) {
                      my_string msg;
                      my_vector<my_string> req_vec;
                      req_vec.push("j");
                      req_vec.push(g_fin.at(i));
                      req_vec.push(getpid());
                      msg::message req_msg(req_vec, PROT_REQ);
                      req_msg.encode();

                      int ret = write_pipe.write_msg(req_msg);

                      if (ret == -1) {
                        perror("Pool: Write msg Error");
                        break;
                      }

                      if (ret == 0) {
                        // If the select times out after 3",
                        // it *probably* means that the coordinator
			// has already sent us another operation
			// to perform
                        stop = true;
                        got_msg = false;
                        break;
                      } else {
                        my_string resp;

                        read_pipe.read_msg(&resp);

                        msg::message resp_msg(resp, PROT_RESP);

                        resp_msg.decode();
                        my_vector<my_string> resp_vec = resp_msg.get_list();
                        // If we get a different response,
                        // perhaps the coordinator has sent
                        // a new job or a suspension signal, so we break
                        // and handle that.
                        if (resp_vec.at(0) != "j") {
                              vec = resp_vec;
                              stop = true;
                              got_msg = true;
                              break;
                        }

                        // Only after we've gotten a successful return
                        // message do we remove the job from the finished ones
                        g_fin.remove_at(i);
                    }
                  }
                  if (!stop && g_fin.size() == 0) {
                    if (g_done) {
                      if (g_current <= 0) {
                        // The pool will exit and the coordinator
                        // will catch the SIGCHLD signal
                        cout << "Pool #" << getpid() << " exiting" << endl;
                        exit(0);
                      }
                    }
                  }
                  stop = false;
              }
	    }

            if (!got_msg) {
              msg::message message(com, PROT_P_REQ);

              message.decode();

              vec = message.get_list();
            }
            got_msg = false;

            // Protect the actual operations
            // section with a SIGCHLD signal blocker
            if (!timedout && !intrptd) {
                switch (hf::get_p_com(vec)) {
                    case C_SUB: {
                        g_done = ++g_served == g_max;
                        g_current++;
                        my_vector<my_string> lst;
                        lst.push("0");
                        job j(vec);
                        pid_t j_pid = j.start();

                        cout << "Pool: Started job " << vec.at(2) << " | "
                             << j_pid << endl;

                        g_job_pids.push(my_string((int) j_pid));

                        lst.push(my_string(j_pid));
                        msg::message msg(lst, PROT_P_RESP);
                        if (!msg.encode()) {
                            cerr << "Pool: Could not encode message. Exiting"
                                 << endl;
                            exit(-1);
                        }
                        write_pipe.write_msg(msg);
                        break;
                    }
                    case C_SUSP: {
                        cout << "Pool: Suspending job with pid: " << vec.at(1)
                             << endl;
                        my_vector<my_string> resp_vec;
                        resp_vec.push("1");
                        try {
                          my_string pid = vec.at(1);
                          kill(pid.to_int(), SIGSTOP);
                          resp_vec.push("K");
                        } catch(exception &e) {
                          cout << "Pool: Malformed command" << endl;
                          resp_vec.push("X");
                        }

                        msg::message resp_msg(resp_vec, PROT_P_RESP);
                        resp_msg.encode();

                        write_pipe.write_msg(resp_msg);
                        break;
                    }
                    case C_RES: {
                      cout << "Resume" << endl;
                      my_vector<my_string> resp_vec;
                      resp_vec.push("2");
                      try {
                        my_string pid = vec.at(1);
                        kill(pid.to_int(), SIGCONT);
                        resp_vec.push("K");
                      } catch(exception &e) {
                        cout << "Pool: Malformed command" << endl;
                        resp_vec.push("X");
                      }

                      msg::message resp_msg(resp_vec, PROT_P_RESP);
                      resp_msg.encode();

                      write_pipe.write_msg(resp_msg);
                      break;
                    }
                    default: {
                        // Maybe it was an unknown operation or maybe
                        // a signal interrupted the read() syscall 
                    }
                }
            }
            uninstall_blocker(SIGCHLD);

        } while (true);

    } else if (pid == -1) {
        cerr << "Pool: Child process creation failed. Exiting..." << endl;
        perror("Fork");
        return -1;
    } else {
        return pid;
    }

}

void listener(int sig) { 
    pid_t p;
    int status;
    while ((p = waitpid(-1, &status, WNOHANG)) > 0) {
        if (p != 0) {
            g_current--;
            g_fin.push(my_string(p));
        }
    }
}

void killer(int num) {
    for (size_t i = 0; i < g_job_pids.size(); i++) {
        if (!g_fin.in(g_job_pids.at(i))) {
	    // Send a SIGKILL to each non - finished child
	    // and immediately wait for that child
            kill(g_job_pids.at(i).to_int(), SIGKILL);
            wait(NULL);
        }
    }
    exit(0);
}

void install_blocker(int sig) {
    sigset_t mask;
    sigemptyset(&mask);

    sigaddset(&mask, sig);

    sigprocmask(SIG_BLOCK, &mask, NULL);
}

void uninstall_blocker(int sig) {
    sigset_t waiting;
    sigpending(&waiting);

    if (sigismember(&waiting, sig)) {
        listener(sig);
    }
}

bool pool::operator==(const pool &other) {
    return true;
}

ostream &operator<<(ostream &out, pool other) {
    out << "";
    return out;
}
