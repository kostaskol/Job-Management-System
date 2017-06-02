#include "pipemanager.h"
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

using namespace std;

pipe_manager::pipe_manager() : _n_pipe(-1) {}

pipe_manager::pipe_manager(const pipe_manager &other) {
    _path = other._path;
    _buff_size = other._buff_size;
    _mode = other._mode;
}

pipe_manager::pipe_manager(my_string path, int mode) :
                            _path(path), _buff_size(1024), _mode(mode) {

    // If the named pipe already exists, ignore the error
    if (mkfifo(_path.c_str(), 0666) < 0 && errno != EEXIST) {
        perror("Mkfifo");
    }
}

pipe_manager::~pipe_manager() {}

int pipe_manager::write_msg(const char *msg) {
    if (_mode == O_RDONLY) {
        cout << "Error: Writing to RDONLY pipe" << endl;
        exit(-1);
    }
    _n_pipe = open(_path.c_str(), _mode);
    int bytes = (int) write(_n_pipe, msg, strlen(msg));

    close(_n_pipe);
    return bytes;
}

int pipe_manager::write_msg(msg::message m) {
    if (_mode == O_RDONLY) {
        cout << "Error: Writing to RDONLY pipe" << endl;
        exit(-1);
    }
    int bytes = write_msg(m.get_message().c_str());
    return bytes;
}

int pipe_manager::read_msg(my_string *str) {
    if (_mode == O_WRONLY) {
        cout << "Error: Reading from WRONLY pipe" << endl;
        exit(-1);
    }
    *str = "";
    _n_pipe = open(_path.c_str(), _mode);

    if (_n_pipe < 0) {
        return -1;
    }


    char *buf = new char[_buff_size];
    int bytes = (int) read(_n_pipe, buf, (size_t) _buff_size);

    for (int i = 0; i < bytes; i++) {
        *str += (char) buf[i];
    }

    delete[] buf;
    close(_n_pipe);
    return bytes;
}

int pipe_manager::read_timeout(my_string *str, int timeout) {
  fd_set rfds;
  FD_ZERO(&rfds);

  _n_pipe = open(_path.c_str(), _mode | O_NONBLOCK);
  FD_SET(_n_pipe, &rfds);

  struct timeval to;
  memset(&to, 0, sizeof(to));

  to.tv_sec = timeout;

  int ret = select(_n_pipe + 1, &rfds, NULL, NULL, &to);

  if (ret == 0) {
    close(_n_pipe);
    return 0;
  }

  char *buffer = new char[201];
  int bytes = read(_n_pipe, buffer, _buff_size);

  *str = "";

  for (int i = 0; i < bytes; i++) {
    *str += buffer[i];
  }

  return bytes;
}

int pipe_manager::write_timeout(msg::message msg, int timeout) {
  fd_set wfds;
  FD_ZERO(&wfds);

  _n_pipe = open(_path.c_str(), _mode | O_NONBLOCK);
  FD_SET(_n_pipe, &wfds);

  struct timeval to;
  memset(&to, 0, sizeof(to));

  to.tv_sec = timeout;

  int ret = select(_n_pipe + 1, NULL, &wfds, NULL, &to);

  if (ret < 0) {
    perror("Select: Write");
    close (_n_pipe);
    return -1;
  }

  if (ret == 0) {
    // Timeout occurred
    close (_n_pipe);
    return ret;
  }

  if (ret) {
    int bytes = (int) write(_n_pipe, msg.get_message().c_str()
                        ,strlen(msg.get_message().c_str()));
    close(_n_pipe);
    return bytes;
  }
  close(_n_pipe);
  return ret;
}

int pipe_manager::select_read(my_string *str, my_vector<pipe_manager> p) {
    fd_set rfds;

    FD_ZERO(&rfds);

    int max;

    // Open all of the supplied named pipes with the O_NONBLOCK flag
    for (size_t i = 0; i < p.size(); i ++) {
        p.at(i)._n_pipe = open(p.at(i)._path.c_str(), p.at(i)._mode | O_NONBLOCK);
        FD_SET(p.at(i)._n_pipe, &rfds);
        max = p.at(i)._n_pipe;
    }

    max++;

    int ret = select(max, &rfds, nullptr, nullptr, nullptr);

    // Select error occurred
    if (ret == -1) {
      for (size_t i = 0; i < p.size(); i++) {

        close(p.at(i)._n_pipe);
      }
      return ret;
    }

    // We give precedence to pool messages (informing us about a
    // job's death) over the console messages
    int bytes = -3;
    // The first file descriptor is always the console one
    // so we skip that in the iteration.
    // We check the console file descriptor IFF no pools have
    // written a message
    for (size_t i = 0; i < p.size(); i++) {
        if(FD_ISSET(p.at(i)._n_pipe, &rfds)) {
          char *buffer = new char[200];
          bytes = (int) read(p.at(i)._n_pipe, buffer, (size_t) 200);

          if (bytes == 0) {
            delete[] buffer;
            continue;
          }

          *str = "";
          for (int i = 0; i < bytes; i++) {
              *str += (char) buffer[i];
          }

          delete[] buffer;
          // If we read a message, we close all of the other pipes
          // and return what we read
          for (size_t j = i; j < p. size(); j++) {
            close(p.at(j)._n_pipe);
          }
          return bytes;
        }
        close(p.at(i)._n_pipe);
    }
    // If no pool has written a message,
    // we check if the console has
    return bytes;
}



bool pipe_manager::operator==(const pipe_manager &other) {
    return _path == other._path && _mode == other._mode;
}

pipe_manager &pipe_manager::operator=(const pipe_manager &other) {
    _path = other._path;
    _buff_size = other._buff_size;
    _mode = other._mode;

    return *this;
}

ostream &operator<<(ostream &out, pipe_manager other) {
    out << "Path: " << other._path;
    return out;
}
