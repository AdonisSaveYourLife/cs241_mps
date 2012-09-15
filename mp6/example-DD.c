#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include "libdrm.h"

int x = 0, N = 100000;
drm_t m1, m2;

void release(int sig)
{
	printf("Thread(id=%lu) caught signal SIGUSR1 and will now release its resouces and exit.\n", pthread_self());
	drm_unlock(&m1);
	drm_unlock(&m2);
	pthread_exit(NULL);
}

void *count_up(void *ptr)
{
	signal(SIGUSR1, release);

	int i;
	for (i=0; i < N; i++)
	{
		drm_lock(&m1);
		drm_lock(&m2);
		x++;
		drm_unlock(&m2);
		drm_unlock(&m1);
	}

	return NULL;
}

void *count_down(void *ptr)
{
	signal(SIGUSR1, release);

	int i;
	for (i=0; i < N; i++)
	{
		drm_lock(&m2);
		drm_lock(&m1);
		x--;
		drm_unlock(&m1);
		drm_unlock(&m2);
	}

	return NULL;
}


int main()
{
	printf("Expected result: Program should print \"Deadlock detected! Edges: 1 -> 2, 2 -> 1\", where 1 and 2 will be specific thread_ids.\n");
	printf("... after that, the program should output a non-zero number (usually a number close to 100000 or -100000).\n");
	printf("... as you run your program multiple times, the number should vary between positive and negative.\n");
	printf("    ...remember, the thread you send the signal to should be random, so you will be switching between killing the\n");
	printf("       count_up() and the count_down() threads.\n");
	printf("\n");
	printf("... this program should exit within a second or two and exit. If it takes longer than 5 seconds, it likely deadlocked.\n");
	printf("... due to how valgrind handles signals, the tester may not work under valgrind.\n");
	printf("\n");

	drm_setmode(DEADLOCK_DETECTION);

	drm_init(&m1);
	drm_init(&m2);

	pthread_t t1, t2;
	pthread_create(&t1, NULL, count_up, NULL);
	pthread_create(&t2, NULL, count_down, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	drm_destroy(&m1);
	drm_destroy(&m2);
	drm_cleanup();

	printf("%d\n", x);
	return 0;

}

