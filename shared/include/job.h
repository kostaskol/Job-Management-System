#ifndef JMS_JOB_H
#define JMS_JOB_H


#include <signal.h>
#include "mystring.h"


class job {
private:
    my_vector<my_string> _command;

public:
    job();
    job(my_vector<my_string> com);
    job(const job &other);

    ~job();

    void init(my_vector<my_string> com);


    pid_t start();

    job &operator=(const job &other);

    bool operator==(const job &other);

    friend std::ostream &operator<<(std::ostream &out, job j);
};


#endif //JMS_JOB_H
