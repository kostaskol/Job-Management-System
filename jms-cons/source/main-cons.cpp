#include <iostream>
#include <cmd_arguments.h>
#include <console.h>
#include <constants.h>

using namespace std;

my_string outp_path;
int curr_job;

int main(int argc, char **argv) {

    cmd_arguments args(A_CONS);

    if (!args.parse(argc, argv)) {
        return 0;
    }

    my_string read_pipe = args.get_r_pipe();
    my_string write_pipe = args.get_w_pipe();
    my_string operations = args.get_op();

    console cons(args);

    cons.start();
}
