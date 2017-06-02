#ifndef PIPEMANAGER_H_
#define PIPEMANAGER_H_

#include <unistd.h>
#include <fcntl.h>
#include "mystring.h"
#include "message.h"

class pipe_manager {
private:
	my_string _path;
	int _buff_size;
	int _n_pipe;
	int _mode;
public:
	pipe_manager();
	pipe_manager(my_string path, int mode);
	pipe_manager(const pipe_manager &other);

	virtual ~pipe_manager();

	void create();

	int write_msg(const char *msg);
	int write_msg(msg::message m);

	int read_msg(my_string *str);

	int read_timeout(my_string *str, int timeout);

	int write_timeout(msg::message msg, int timeout);

	static int select_read(my_string *str, my_vector<pipe_manager> p);

	my_string get_path() { return _path; }

	int get_fd() { return _n_pipe; }

	int disconnect();

	int flush();

	pipe_manager &operator=(const pipe_manager &other);

	bool operator==(const pipe_manager &other);

	friend std::ostream &operator<<(std::ostream &out, pipe_manager other);
};

#endif /* PIPEMANAGER_H_ */
