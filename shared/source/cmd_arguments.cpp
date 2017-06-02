#include "cmd_arguments.h"
#include "constants.h"
#include <iostream>
#include <cstring>

using namespace std;

cmd_arguments::cmd_arguments(int mode) :
               _has_op(false), _has_r(false), _has_w(false),
              _has_path(false), _has_pool_size(false), _mode(mode) {}

cmd_arguments::~cmd_arguments() {}

bool cmd_arguments::parse(int argc, char *argv[]) {
    switch(_mode) {
        case A_CONS: {
            if (argc != 2 && argc != 7) {
                cout << "Invalid number of arguments. Run with --help for help" << endl;
                return false;
            }

            for (int i = 0; i < argc; i++) {
                my_string str = argv[i];
                if (str == "-o") {
                    if (++i != argc) {
                        _op_file = argv[i];
                        _has_op = true;
                        continue;
                    } else {
                        cerr << "Error parsing command line arguments. "
                                "Run with --help for help" << endl;
                        return false;
                    }
                } else if (str == "-w") {
                    if (++i != argc) {
                        _write_pipe = argv[i];
                        _has_r = true;
                        continue;
                    } else {
                        cerr << "Error parsing command line arguments. "
                                "Run with --help for help" << endl;
                        return false;
                    }
                } else if (str == "-r") {
                    if (++i != argc) {
                        _read_pipe = argv[i];
                        _has_w = true;
                        continue;
                    } else {
                        cerr << "Error parsing command line arguments. "
                                "Run with --help for help" << endl;
                    }
                } else if (str == "--help") {
                    _print_help();
                    return false;
                }
            }

            if (!(_has_op && _has_w && _has_r)) {
                cerr << "Not all arguments have been provided."
                        "Run with --help for help" << endl;
                return false;
            }
            return true;
        } case A_COORD: {
            if (argc != 2 && argc != 9) {
                cerr << "Invalid number of arguments."
                        "Run with --help for help" << endl;
                return false;
            }


            for (int i = 0; i < argc; i++) {
                my_string str = argv[i];
                if (str == "-l") {
                    if (++i != argc) {
                        _has_path = true;
                        _path = argv[i];
                        continue;
                    } else {
                        cerr << "Error parsing command line arguments. "
                                "Run with --help for help" << endl;
                        return false;
                    }
                } else if (str == "-n") {
                    if (++i != argc) {
                        _has_pool_size = true;
                        _pool_s = atoi(argv[i]);
                        continue;
                    } else {
                        cerr << "Error parsing command line arguments. "
                                "Run with --help for help" << endl;
                        return false;
                    }
                } else if (str == "-w") {
                    if (++i != argc) {
                        _has_w = true;
                        _write_pipe = argv[i];
                        continue;
                    } else {
                        cerr << "Error parsing command line arguments. "
                                "Run with --help for help" << endl;
                        return false;
                    }
                } else if (str == "-r") {
                    if (++i != argc) {
                        _has_r = true;
                        _read_pipe = argv[i];
                        continue;
                    } else {
                        cerr << "Error parsing command line arguments. "
                                "Run with --help for help" << endl;
                        return false;
                    }
                } else if (str == "--help") {
                    _print_help();
                    return false;
                }
            }

            if (!(_has_path && _has_r && _has_w && _has_pool_size)) {
                cerr << "Not all arguments have been provided. "
                        "Run with --help for help" << endl;
                return false;
            }
            return true;
        } default: {
            return false;
        }
    }
}

void cmd_arguments::_print_help() {
    switch (_mode) {
        case A_COORD: {
            cout << "Help for jms_coord:\n"
                    "\tLegal arguments:\n"
                    "\t\t-l : Specifies the path of the directory under which the jobs "
                    "stdout and stderr streams will be written\n"
                    "\t\t-n : Specifies the maximum jobs each pool is allowed to maintain\n"
                    "\t\t-r : Specifies the path of the read pipe\n"
                    "\t\t-w : Specifies the path of the write pipe" << endl;
            return;
        } case A_CONS: {
            cout << "Help for jms_console:\n"
                    "\tLegal arguments:\n"
                    "\t\t-o : Specifies operation file\n"
                    "\t\t-r : Specifies read pipe path\n"
                    "\t\t-w : Specifies write pipe path" << endl;
            return;
        } default: {
            cout << "Bad mode" << endl;
            return;
        }
    }
}

my_string cmd_arguments::get_op() { return _op_file; }

my_string cmd_arguments::get_r_pipe() { return _read_pipe; }

my_string cmd_arguments::get_w_pipe() { return _write_pipe; }

int cmd_arguments::get_p_size() { return _pool_s; }

my_string cmd_arguments::get_path() { return _path; }
