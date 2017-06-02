#include <wait.h>
#include "helpfunc.h"
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <constants.h>

using namespace std;

namespace hf {

    // Get a date stamp
    my_string get_date(tm *timeinfo) {
        my_string date = timeinfo->tm_mday;
        char *tmp = (char *)"-";
        date += tmp;
        date += timeinfo->tm_mon;
        date += "-";
        date += "20";
        date += (timeinfo->tm_year - 100);

        return date;
    }

    // Get a time stamp
    my_string get_time(tm *timeinfo) {
        my_string time = timeinfo->tm_hour;
        time += ":";
        time += timeinfo->tm_min;
        time += ":";
        time += timeinfo->tm_sec;

        return time;
    }

    // Formats the given arguments and returns the string
    // (Used to create appropriate directories for the jobs)
    my_string format(my_string path, int l_id, my_string cmd_name, pid_t pid) {
        time_t now = time(0);

        tm *timeinfo;

        timeinfo = localtime(&now);

        my_string name = path; name += "/";

        my_string date = hf::get_date(timeinfo);

        my_string time = hf::get_time(timeinfo);

        cmd_name.remove('/');


        name += "sdi1200066_";
        name += l_id;
        name += "_";
        name += pid;
        name += "_";
        name += date;
        name += "_";
        name += time;
        name += "_";
        name += cmd_name;

        name += "/";

        return name;
    }

    // Returns an integer (defined in shared/include/Constants.h)
    // corresponding to the operation
    // If str is true, it will check for
    // the operation's full name (i.e. submit, status, etc)
    // as opposed to an encoded operation (0, 1, etc)
    int get_com(my_vector<my_string> com, bool str) {
        if (com.size() == 0) return C_UNKNOWN;
        if (str) {
            my_string tmp = com.at(0);
            if (tmp == "submit")        return C_SUB;
            if (tmp == "status")        return C_STAT;
            if (tmp == "status-all")    return C_STAT_ALL;
            if (tmp == "show-active")   return C_SHOW_ACT;
            if (tmp == "show-pools")    return C_SHOW_POOL;
            if (tmp == "show-finished") return C_SHOW_FIN;
            if (tmp == "suspend")       return C_SUSP;
            if (tmp == "resume")        return C_RES;
            if (tmp == "shutdown")      return C_SD;

            return C_UNKNOWN;

        } else {
            char op = com.at(0)[0];

            if (op == '0')              return C_SUB;
            if (op == '1')              return C_STAT;
            if (op == '2')              return C_STAT_ALL;
            if (op == '3')              return C_SHOW_ACT;
            if (op == '4')              return C_SHOW_POOL;
            if (op == '5')              return C_SHOW_FIN;
            if (op == '6')              return C_SUSP;
            if (op == '7')              return C_RES;
            if (op == '8')              return C_SD;
            if (op == 'j')              return C_JOB_DONE;
            if (op == 'p')              return C_POOL_DONE;

            return C_UNKNOWN;
        }
    }

    // Same as above, but checks according to
    // the PROT_P_* protocol
    int get_p_com(my_vector<my_string> com) {
        if (com.size() == 0) return C_UNKNOWN;

        try {
          my_string tmp = com.at(0);
          if (tmp == "0")                 return C_SUB;
          if (tmp == "1")                 return C_SUSP;
          if (tmp == "2")                 return C_RES;
          if (tmp == "3")                 return C_SD;
          if (tmp == "4")                 return C_DONE;

          return C_UNKNOWN;
        } catch (exception &e) {
          return C_UNKNOWN;
        }
    }
}
