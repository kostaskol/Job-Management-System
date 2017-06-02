#include "parser.h"
#include "filemanager.h"
#include <fcntl.h>
#include <iostream>
#include <my_vector.h>

using namespace std;

parser::parser(my_string file) : _f_manager(file) {}

parser::~parser() {}

bool parser::next_command(my_vector<my_string> *com) {
    my_string line;
    bool end = _f_manager.read_line(&line);
    if (end) return true;
    *com = line.split(' ');
    return false;
}
