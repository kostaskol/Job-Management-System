#ifndef JMS_MESSAGE_H
#define JMS_MESSAGE_H


#include <mystring.h>
#include "my_vector.h"

// Simple class that encodes - decodes
// my_vector - my_string objects
// according to the protocol provided
namespace msg {

	class message {
	private:
	    my_string _message;
	    my_vector<my_string> _list;
	    bool _using_list;
	    int _mode;
	public:
	    message(my_string mes, int mode);
	    message(my_vector<my_string> vec, int mode);

	    ~message();

	    bool encode();

	    bool decode();

	    my_vector<my_string> get_list();

	    my_string get_message();
	};

}


#endif //JMS_MESSAGE_H
