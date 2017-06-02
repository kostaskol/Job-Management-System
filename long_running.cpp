#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>

using namespace std;

void handler(int sig) {
	if (sig == SIGSTOP) {
		cout << "Got SIGSTOP" << endl;
	}
}

int main (int argc, char **argv) {
	signal(SIGSTOP, handler);
	int time_to_sleep;
	if (argc == 1) time_to_sleep = 5;
	else time_to_sleep = atoi(argv[1]);
	cout << "Sleeping for " << time_to_sleep << " seconds" << endl;
	sleep(time_to_sleep);
	cout << "Slept for " << time_to_sleep << " seconds. Exiting..." << endl;

}
