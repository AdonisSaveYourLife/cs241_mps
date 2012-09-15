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
			{ i--; continue; }
		if (drm_lock(&m2) != 0)
			{ drm_unlock(&m1); i--; continue; }
		
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
			{ i--; continue; }
		if (drm_lock(&m1) != 0)
			{ drm_unlock(&m2); i--; continue; }
		
		x--;
		
		drm_unlock(&m1);
		drm_unlock(&m2);
	}

	return NULL;
}



int main()
{
	printf("Expected result: Program should print \"0\". (Progam shouldn't deadlock and all memory should be freed.)\n");
	printf("  ...this will take a few seconds to run on the csil-linux-ts# boxes. Maybe more on other boxes.\n");
	printf("  ...if it doesn't finish in 30 seconds, it has likely deadlocked.\n");
	drm_setmode(DEADLOCK_AVOIDANCE);

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

