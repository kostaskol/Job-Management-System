#include "job.h"
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include "helpfunc.h"
#include "constants.h"
#include <sys/stat.h>

using namespace std;

extern my_string outp_path;

job::job() {}

job::job(my_vector<my_string> com) : _command(com) {}

job::job(const job &other) {
    _command = other._command;
}

job::~job() {}

pid_t job::start() {
    char **args;

    pid_t pid = fork();

    if (pid == 0) {
        // Parse the vector and create the argument array
        // for execvp

        // _command holds a command of the format:
        // 0 jid command (arguments)
        // So we get a sublist from index 2 through the end of the vector
        // which is the actual command to be executed
        my_vector<my_string> cmd = _command.sublist(2, _command.size() - 2);
        if (cmd.size() > 1) {
            args = new char *[cmd.size()];
            for (size_t i = 0; i < cmd.size(); i++) {
                my_string tmp = cmd.at(i);

                args[i] = new char[tmp.length() + 1];
                strcpy(args[i], tmp.c_str());
            }
            args[cmd.size() - 1] = nullptr;
        } else {
            args = new char *[2];
            args[0] = new char[1];
            strcpy(args[0], cmd.at(0).c_str());
            args[1] = nullptr;
        }

        // Create the directory for this job
        my_string name = hf::format(outp_path, getpid(), cmd.at(0),
                                    _command.at(1).to_int());

        my_string std_out;
        std_out = _command.at(1);
        std_out += "_stdout";

        my_string std_err;
        std_err = _command.at(1);
        std_err += "_stderr";

	// Also create two files inside that directory (stdout, stderr)
        my_string stdout_outp = name; stdout_outp += std_out; stdout_outp += ".txt";
        my_string stderr_outp = name; stderr_outp += std_err; stderr_outp += ".txt";

        mkdir((name).c_str(), 0777);

	// Open two file descriptors (which will be inherited by the process
	// once execvp runs
        int fdout = open(stdout_outp.c_str(), O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);
        int fderr = open(stderr_outp.c_str(), O_CREAT | O_WRONLY , S_IWUSR | S_IRUSR);

        if (fdout == -1 || fderr == -1) {
            perror("Job: Open File");
        }

	// Replace the stdout and stderr respectively
        dup2(fdout, 1);
        dup2(fderr, 2);

        close(fdout);
        close(fderr);

        if (execvp(cmd.at(0).c_str(), args) < 0) {
            cerr << "Job: Error executing command " << cmd.at(0) << endl;
        }
        exit(0);
    } else {
        return pid;
    }
}

bool job::operator==(const job &other) {
    return true;
}

ostream &operator<<(ostream &out, job j) {
    return out;
}

job &job::operator=(const job& other) {
    _command = other._command;
    return *this;
}
