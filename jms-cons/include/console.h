#ifndef JMS_CONS_CONSOLE_H
#define JMS_CONS_CONSOLE_H


#include <mystring.h>
#include <cmd_arguments.h>
#include <pipemanager.h>

typedef my_vector<my_string> str_vec;

class console {
private:
    my_string _op;
    my_string _r_pipe;
    my_string _w_pipe;

    my_vector<pipe_manager> pipes;

    void _submit(str_vec job);

    void _status(str_vec jid);

    void _status_all(str_vec time_q);

    void _show_active(str_vec act);

    void _show_pool(str_vec pool);

    void _show_finished(str_vec fin);

    void _suspend(str_vec jid);

    void _resume(str_vec jid);

    void _sd(str_vec );
public:
    console(cmd_arguments &args);
    ~console();
    void start();
};


#endif //JMS_CONS_CONSOLE_H
