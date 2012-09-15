#
# MiniMP #1
# CS 241
#

CC = gcc
INC = -I.
FLAGS = -W -Wall -g

all: test1 test2 doc/html

doc/html: btree.c btree.h
	doxygen doc/Doxyfile

btree.o: btree.c btree.h
	$(CC) -c $(FLAGS) $(INC) $< -o $@
	
test1: btree.o test1.c
	$(CC) $(FLAGS) $(INC) $^ -o $@
	
test2: btree.o test2.c
	$(CC) $(FLAGS) $(INC) $^ -o $@

.PHONY : clean
clean:
	-rm -f *.o test1 test2