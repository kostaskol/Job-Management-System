CC = g++

CXXFLAGS = -g -Wall -std=c++0x -Ishared/include/ -Ijms-cons/include/ -Ijms-coord/include/

OUTP_CONS = jms-console-exec
OUTP_COORD = jms-coord-exec
OUTP_LONG = long

SOURCE_SHRD = shared/source/cmd_arguments.cpp\
				shared/source/filemanager.cpp\
				shared/source/helpfunc.cpp\
				shared/source/message.cpp\
				shared/source/mystring.cpp\
				shared/source/my_vector.cpp\
				shared/source/pipemanager.cpp\

SOURCE_SHRD_COORD = jms-coord/source/job.cpp\
				jms-coord/source/pool.cpp

SOURCE_COORD = jms-coord/source/coordinator.cpp\
				jms-coord/source/hash_table.cpp\
				jms-coord/source/job.cpp\
				jms-coord/source/main-coord.cpp\
				jms-coord/source/pool.cpp

SOURCE_CONS = jms-cons/source/console.cpp\
				jms-cons/source/main-cons.cpp\
				jms-cons/source/parser.cpp

#Objective files
OBJS_CONS = ${SOURCE_CONS:.cpp=.o} ${SOURCE_SHRD:.cpp=.o} ${SOURCE_SHRD_COORD:.cpp=.o}
OBJS_COORD = ${SOURCE_COORD:.cpp=.o} ${SOURCE_SHRD:.cpp=.o}
OBJS_ALL = ${OBJS_CONS} ${OBJS_COORD} ${OBJS_SHARED}

all : cons coord long

cons : ${OBJS_CONS}
	@${CC} ${CFLAGS} -o ${OUTP_CONS} ${OBJS_CONS}

coord : ${OBJS_COORD}
	@${CC} ${CFLAGS} -o ${OUTP_COORD} ${OBJS_COORD}

long : long_running.cpp
	${CC} ${CFLAGS} -o long long_running.cpp


#Objective dependencies

#Console
console.o : parser.h pipemanager.h message.h constants.h console.h

main-cons.o : cmd_arguments.h console.h

parser.o : parser.h filemanager.h


#Coordinator
coordinator.o : pipemanager.h helpfunc.h constants.h coordinator.h

hash_table.o : hash_table.h my_vector.h

job.o : job.h helpfunc.h constants.h

main-coord.o : cmd_arguments.h coordinator.h

pool.o : pool.h pipemanager.h helpfunc.h constants.h

#shared
cmd_arguments.o : cmd_arguments.h constants.h

filemanager.o : filemanager.h

helpfunc.o : helpfunc.h constants.h

message.o : helpfunc.h message.h constants.h

my_vector.o : my_string.h job.h pool.h pipemanager.h

mystring.o : mystring.h my_vector.h

pipemanager.o : pipemanager.h message.h



clean:
	@rm ${OUTP_COORD} ${OUTP_CONS} ${OUTP_LONG}

clean_all:
	@rm ${OUTP_COORD} ${OUTP_CONS} ${OBJS_COORD} ${OBJS_CONS} ${OUTP_LONG}
