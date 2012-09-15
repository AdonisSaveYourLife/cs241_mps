#
# CS 241
# The University of Illinois
#

CC = gcc
INC = -I.
FLAGS = -W -Wall -lpthread

all: ptest

ptest: ptest.c
	$(CC) $(INC) $(FLAGS) $^ -o $@

clean:
	-rm -f ptest