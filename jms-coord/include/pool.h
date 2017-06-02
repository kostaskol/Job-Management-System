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
    
    struct sigaction _action;

    void _submit(msg::message mes);

    void _suspend(int id);

    void _resume(int id);

    int _shutdown();
public:
    pool(my_string r, my_string w, int max);
    pool();

    ~pool();

    int begin();

    bool operator==(const pool &other);

    friend std::ostream &operator<<(std::ostream &out, pool other);
};


#endif //JMS_POOL_H
