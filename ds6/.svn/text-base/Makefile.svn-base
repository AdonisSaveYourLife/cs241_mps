#
# CS 241
# The University of Illinois
#

CC = gcc
FLAGS = -W -Wall -I.

all: ds6

ds6: ds6.c queue.c
	$(CC) $(FLAGS) $^ -o $@

clean:
	-rm -f ds6