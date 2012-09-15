#
# CS 241
#

CC = gcc
INC = -I.
FLAGS = -W -Wall -g

all: simulator doc/html

doc/html: doc/Doxyfile libscheduler/libscheduler.c
	doxygen doc/Doxyfile

simulator: simulator.o libscheduler/libscheduler.o
	$(CC) $^ -o $@

libscheduler/libscheduler.o: libscheduler/libscheduler.c libscheduler/libscheduler.h
	$(CC) -c $(FLAGS) $(INC) $< -o $@

simulator.o: simulator.c libscheduler/libscheduler.h
	$(CC) -c $(FLAGS) $(INC) $< -o $@




.PHONY : clean
clean:
	rm -rf simulator queuetest *.o libscheduler/*.o libpriqueue/*.o doc/html
