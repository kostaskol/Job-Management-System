#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_

#include "mystring.h"
#include <fstream>

class file_manager {
private:
	std::ifstream *_file;
	my_string _path;
	bool _open;
public:
	file_manager(my_string path);
	file_manager(const char *path);
	virtual ~file_manager();

	bool is_open();

	bool read_line(my_string *line);

	int close_file();

};

#endif /* FILEMANAGER_H_ */
