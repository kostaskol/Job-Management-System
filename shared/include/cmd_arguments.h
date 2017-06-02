#ifndef JMS_CMD_ARGUMENTS_H
#define JMS_CMD_ARGUMENTS_H


#include "mystring.h"

class cmd_arguments {
private:
    /* Arguments for jms_console */
    bool _has_op;
    bool _has_r;
    bool _has_w;

    my_string _op_file;
    my_string _read_pipe;
    my_string _write_pipe;

    /* Arguments for jms_coord */
    bool _has_path;
    bool _has_pool_size;

    my_string _path;
    int _pool_s;

    /* General */
    int _mode;

    void _print_help();
public:

    cmd_arguments(int mode);

    ~cmd_arguments();

    bool parse(int argc, char *argv[]);

    my_string get_op();

    my_string get_r_pipe();

    my_string get_w_pipe();

    int get_p_size();

    my_string get_path();



};


#endif //JMS_CMD_ARGUMENTS_H
