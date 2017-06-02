#ifndef JMS_PARSER_H
#define JMS_PARSER_H

#include "filemanager.h"
#include "mystring.h"
#include "my_vector.h"

class parser {
private:
    my_string _line;
    file_manager _f_manager;
public:
    parser(my_string file);

    ~parser();

    /* Returns true if we've reached the end of the file */
    bool next_command(my_vector<my_string> *);
};


#endif //JMS_PARSER_H
