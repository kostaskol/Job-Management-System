#include <cstring>
#include <stdexcept>
#include <iostream>
#include <cmath>
#include <my_vector.h>
#include <vector>
#include "mystring.h"

using namespace std;

my_string::my_string() : _len(1) {
    _str = new char[1];
    _str[0] = '\0';
    _len = 1;
}

my_string::my_string(char *str) {

	_len = (int) strlen(str) + 1;
	_str = new char[_len];
	strcpy(_str, str);
}

my_string::my_string(const char *str) {
	_len = (int) strlen(str) + 1;
	_str = new char[_len];
	strcpy(_str, str);
}

my_string::my_string(char c) {
    _str = new char[2];
    _len = 2;
    _str[0] = (char) c;
    _str[1] = '\0';
}

my_string::my_string(const my_string &other) {
	if (other._str != nullptr) {
		_len = (int) strlen(other._str) + 1;
		_str = new char[_len + 1];
		strcpy(_str, other._str);
        return;
	}

	_len = 1;
	_str = new char[1];
    _str[0] = '\0';
}

my_string::my_string(int num) {
    char tmp[20];
    sprintf(tmp, "%d", num);

    _str = new char[strlen(tmp) + 1];
    strcpy(_str, tmp);
    _len = strlen(_str) + 1;
}


my_string::~my_string() {
    delete[] _str;
    _str = nullptr;
}

const char *my_string::c_str() { return _str; }

size_t my_string::length() { return _len - 1; }

my_vector<my_string> my_string::split(char delim) {

	my_vector<my_string> list;
	/*char *tmp_str = new char[_len];
	strcpy(tmp_str, _str);
	my_string tmp;
	char *tok = strtok(tmp_str, delim);
	tmp = tok;
	list.push(tmp);

	while(true) {
		tok = strtok(NULL, delim);
        if (tok == NULL) {
            break;
        }
        tmp = tok;
        list.push(tmp);
    }

	delete[] tmp_str;*/

    my_string tmp;
    bool pushed = false;
    for (size_t i = 0; i < _len - 1; i++) {
        if (_str[i] != delim) {
            tmp += _str[i];
            pushed = false;
        } else {
            pushed = true;
            list.push(tmp);
            tmp = "";
        }
    }

    if (!pushed) {
        list.push(tmp);
    }

	return list;
}

my_string my_string::substr(int start, int length) {
	if ((start + length) >= (int) _len) {
		throw std::runtime_error("Bad length");
	}

    char *tmp = new char[length + 2];

    int j = 0;
    for (int i = start; i <= start + length; i++) {
        tmp[j++] = _str[i];
    }
    tmp[j] = '\0';

    my_string tmp_str = tmp;
    delete[] tmp;
    return tmp_str;

}

int my_string::to_int() { return atoi(_str); }

void my_string::remove(char c) {
    char *tmp = new char[strlen(_str) + 1];

    int j = 0;
    for (size_t i = 0; i < strlen(_str) + 1; i++) {
        if (_str[i] == c) continue;
        tmp[j++] = _str[i];
    }

    delete[] _str;
    _str = tmp;
    _len = strlen(tmp) + 1;
}

my_string &my_string::operator=(const my_string& other) {

    delete[] _str;

	if (other._str == nullptr) {
		_len = 0;
		_str = new char[1];
        _str[0] = '\0';
		return *this;
	}

	_len = (int) strlen(other._str) + 1;
	_str = new char[_len + 1];
	strcpy(_str, other._str);
	return *this;
}

my_string &my_string::operator=(const char *str) {
    delete[] _str;

	_len = strlen(str) + 1;
	_str = new char[_len];
	strcpy(_str, str);
	return *this;
}

my_string &my_string::operator+=(const my_string& other) {

    char *tmp = new char[_len + other._len + 2];

    strcpy(tmp, _str);

    delete[] _str;

    strcat(tmp, other._str);


    _str = new char[_len + other._len + 2];

    strcpy(_str, tmp);

    delete[] tmp;

    _len += other._len - 1;


	return *this;
}



my_string &my_string::operator+=(const char *str) {
    char *tmp = new char[strlen(_str) + strlen(str) + 2];


    strncpy(tmp, _str, strlen(_str) + 1);
    delete[] _str;
    strcat(tmp, str);

    _str = new char[strlen(tmp) + 1];

    strcpy(_str, tmp);

    delete[] tmp;

    _len += strlen(_str) + 1;

    return *this;
}


my_string &my_string::operator+=(const char c) {

    _len += 1;

    char *tmp = new char[_len + 1];

    for (int i = 0; i < (int) _len - 1; i++) {
        tmp[i] = _str[i];
    }

    delete[] _str;

    _str = tmp;

    _str[_len - 2] = (char) c;
    _str[_len - 1] = '\0';

	return *this;
}

my_string &my_string::operator+=(int num) {
//    char tmp[20];
//    char *tmp_str = new char[strlen(_str) + 1];
//    strcpy(tmp_str, _str);
//    delete[] _str;
//    _len = (size_t) sprintf(tmp, "%d", num);
//
//    _str = new char[_len + strlen(tmp_str) + 2];
//    strcpy(_str, tmp_str);
//    delete[] tmp_str;
//    strcat(_str, tmp);
//
//    return *this;
    my_string tmp(num);

    char *tmp_str = new char[_len + 1];
    strcpy(tmp_str, _str);
    delete[] _str;

    _str = new char[_len + tmp._len + 2];
    strcpy(_str, tmp_str);
    strcat(_str, tmp._str);

    delete[] tmp_str;

    _len = strlen(_str) + 1;

    return *this;
}

char my_string::operator[](int index) {
	if (index > (int) (_len - 1)) {
		throw std::runtime_error("Index out of range");
	}

	return _str[index];
}

bool my_string::operator==(const my_string& other) {
	if (_str == nullptr) return false;
	return (strcmp(_str, other._str) == 0);
}

bool my_string::operator==(const char *str) {
	if (str == nullptr) return false;
	if (_str == nullptr) return false;
	return (strcmp(_str, str) == 0);
}

bool my_string::operator!=(const my_string &other) {
  if (_str == nullptr) return false;
  return (strcmp(_str, other._str) != 0);
}

bool my_string::operator!=(const char *str) {
  if (_str == nullptr) return false;
  return (strcmp(_str, str) != 0);
}

std::ostream &operator<<(std::ostream &out, my_string str) {
	if (str._str == nullptr) {
		out << "null";
		return out;
	}

	out << str._str;
	return out;
}
