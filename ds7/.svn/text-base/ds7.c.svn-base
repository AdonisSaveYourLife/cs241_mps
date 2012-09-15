#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>

void write_to_fd(int fd, const char *s)
{
	write(fd, s, strlen(s));
}

void *fun1(void *ptr)
{
	int fd = *((int *)ptr);

	sleep(2);  /* Simulate work being done... */
	write_to_fd(fd, "World");

	sleep(3);  /* Simulate work being done... */
	write_to_fd(fd, "CS241");

	close(fd);
	return NULL;
}

void *fun2(void *ptr)
{
	int fd = *((int *)ptr);

	/* No work needs to be done, send right away. */
	write_to_fd(fd, "Hello");

	sleep(3);  /* Simulate work being done... */
	write_to_fd(fd, "from ");

	close(fd);
	return NULL;
}

int main()
{
	char buffer[6];

	/* Create two pipes (1 /thread) */




	/* Launch the two threads, sending the write-end of the FD to the thread. */




	/* Use select() to perform I/O multiplexing. */




	/* Join the threads */




	return 0;
}

