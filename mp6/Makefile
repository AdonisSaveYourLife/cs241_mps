#
# CS 241
# The University of Illinois
#

CC = gcc
FLAGS = -W -Wall -g -I. -lpthread

all: wfgtest examples doc/html


doc/html: libdrm.c libwfg.c
	doxygen doc/Doxyfile


wfgtest: libwfg.o queue.o wfgtest.c
	$(CC) $(FLAGS) $^ -o $@


examples: example-DA example-DD example-DP example-NDC

example-DA: libwfg.o libdrm.o queue.o example-DA.c
	$(CC) $(FLAGS) $^ -o $@

example-DD: libwfg.o libdrm.o queue.o example-DD.c
	$(CC) $(FLAGS) $^ -o $@

example-DP: libwfg.o libdrm.o queue.o example-DP.c
	$(CC) $(FLAGS) $^ -o $@

example-NDC: libwfg.o libdrm.o queue.o example-NDC.c
	$(CC) $(FLAGS) $^ -o $@
	

queue.o: queue.c queue.h
	$(CC) $(FLAGS) -c $< -o $@

libwfg.o: libwfg.c libwfg.h
	$(CC) $(FLAGS) -c $< -o $@

libdrm.o: libdrm.c libdrm.h libwfg.h
	$(CC) $(FLAGS) -c $< -o $@



clean:
	rm -rf *.o wfgtest example-DA example-DD example-DP example-NDC doc/html


