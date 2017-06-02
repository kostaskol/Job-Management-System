
#include "constants.h"
#include "cmd_arguments.h"
#include "coordinator.h"
#include <sys/stat.h>

using namespace std;

my_string outp_path;

typedef my_vector<my_string> str_vec;

int main (int argc, char *argv[]) {


   cmd_arguments args(A_COORD);

   // The cmd_arguments class handles the error
   // messages, so if an error occurs,
   // we simply exit
   if (!args.parse(argc, argv)) exit(-1);

   mkdir("./ppipes", 0777);

   coordinator coord(args);

   outp_path = args.get_path();

   // All of the work happens in the coordinator class
   coord.start();

   cout << "Coord: Exiting" << endl;

	 return 0;
}
