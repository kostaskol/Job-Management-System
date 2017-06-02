#include "filemanager.h"
#include <stdio.h>
#include <sys/types.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <cstdio>
#include <fstream>

using namespace std;

file_manager::file_manager(my_string path) : _path(path) {
        _file = new ifstream(_path.c_str());
	_open = true;
}

file_manager::~file_manager() {
	if (_open) {
		close_file();
	}
}

bool file_manager::is_open() { return _open; }

bool file_manager::read_line(my_string *line) { 
    char *buffer = new char[1024];
    if (!(_file->getline(buffer, 1023))) return true;
    *line = buffer;
    delete[] buffer;
    return false;
}

int file_manager::close_file() {
	if (_open) {
		_file->close();
	}

	return 1;
}
