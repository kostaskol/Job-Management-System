#ifndef JMS_POOL_H
#define JMS_POOL_H


#include "job.h"
#include "message.h"

class pool {
private:
    my_string _r_pipe;
    my_string _w_pipe;
    int _curr;
    int _max;

    void _submit(msg::message mes);

    int _get_status(int id);

    job *_get_status_all();

    job *_get_active();

    int _get_num_jobs();

    job *_get_finished();

    void _suspend(int id);

    void _resume(int id);

    void _shutdown();

    static void _listener(int sig);
public:
    pool(my_string r, my_string w, int max);
    pool();

    ~pool();

    int begin();

    bool operator==(const pool &other);

    friend std::ostream &operator<<(std::ostream &out, pool other);
};


#endif //JMS_POOL_H
