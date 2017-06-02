#ifndef JMS_HELPFUNC_H_H
#define JMS_HELPFUNC_H_H

#include "mystring.h"

namespace hf {

    my_string get_date(tm *info);

    my_string get_time(tm *info);

    my_string format(my_string path, int l_id, my_string cmd_name, pid_t pid);

    int get_com(my_vector<my_string> com, bool str);

    int get_p_com(my_vector<my_string> com);

    void print_stamp();
}
#endif //JMS_HELPFUNC_H_H
