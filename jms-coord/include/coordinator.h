#ifndef JMS_COORDINATOR_H
#define JMS_COORDINATOR_H


#include <mystring.h>
#include <message.h>
#include "job.h"
#include "pool.h"
#include "hash_table.h"
#include <cmd_arguments.h>
#include <pipemanager.h>

typedef my_vector<my_string> str_vec;

class coordinator {
private:
    int _max_jobs;
    my_string _r_pipe;
    my_string _w_pipe;
    my_string _dir_path;
    // Used to find a job's id from its pid
    hash_table<my_string> _jobs_pid_id_map;
    str_vec _susp_jobs;
    str_vec _fin_jobs;

    // Used to find a job's pid from its id
    hash_table<my_string> _jobs_id_pid_map;

    my_vector<pipe_manager> _coord_pipes;

    // Holds the starting time of job with id <n> at index <n>
    my_vector<int> _job_timer;

    // Holds the number of jobs per pool
    hash_table<int> _jobs_per_pool;
    int _next_job;
    int _num_pools;

    void _submit(my_vector<my_string> com, my_string *pid);

    void _get_status(my_string id);

    void _get_status_all(int time_lim = -1);

    void _get_active();

    void _get_pools();

    void _get_finished();

    void _suspend(my_string id);

    void _resume(my_string id);

    void _shutdown();

    void _job_death(my_string pid, my_string ppid);

    void _pool_death();

    int _get_next_pool();

    int _get_pool(int jid);
public:
    coordinator(cmd_arguments &args);
    ~coordinator();

    void start();
};


#endif //JMS_COORDINATOR_H
