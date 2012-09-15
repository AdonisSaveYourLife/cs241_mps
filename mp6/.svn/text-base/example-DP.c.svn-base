#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "libdrm.h"

int x = 0, N = 100000;
drm_t m1, m2;

void *count_up(void *ptr)
{
	int i;
	for (i=0; i < N; i++)
	{
		if (drm_lock(&m1) != 0)
		{
			printf("deadlock prevented!\n");
			exit(1);
		}
		if (drm_lock(&m2) != 0)
		{
			printf("deadlock prevented!\n");
			exit(1);
		}

		x++;

		drm_unlock(&m2);
		drm_unlock(&m1);
	}

	return NULL;
}

void *count_down(void *ptr)
{
	int i;
	for (i=0; i < N; i++)
	{
		if (drm_lock(&m2) != 0)
		{
			printf("deadlock prevented!\n");
			exit(1);
		}
		if (drm_lock(&m1) != 0)
		{
			printf("deadlock prevented!\n");
			exit(1);
		}

		x--;

		drm_unlock(&m1);
		drm_unlock(&m2);
	}

	return NULL;
}


int main()
{
	printf("Expected result: Program should print \"deadlock prevented!\" and exit.\n");
	printf("... valgrind won't run clean if exit() is called without calls to _destroy(). This is fine.\n");
	printf("... this program should print right away and exit. If it takes longer than a second, it likely deadlocked.\n");

	drm_setmode(DEADLOCK_PREVENTION);

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

