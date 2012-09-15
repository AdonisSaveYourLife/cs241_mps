#include <stdio.h>
#include <stdlib.h>

#define TOTAL_ALLOCS 50000
#define LOOP_SIZE 50000

#define D(x) x

int main()
{
	malloc(1);

	int i;
	D(printf("tester-2: need to allocate %zu bytes\n", TOTAL_ALLOCS * sizeof(int *)));
	int **arr = malloc(TOTAL_ALLOCS * sizeof(int *));

	if (arr == NULL)
	{
		printf("Memory failed to allocate!\n");
		return 1;
	}

	for (i = 0; i < LOOP_SIZE; i++)
	{
		arr[i] = malloc(sizeof(int));
		if (arr[i] == NULL)
		{
			printf("Memory failed to allocate!\n");
			return 1;
		}
		
		*(arr[i]) = i;
	}

	for (i = 0; i < LOOP_SIZE; i++)
	{
		if (*(arr[i]) != i)
		{
			printf("Memory failed to contain correct data after many allocations!\n");
			return 2;
		}
	}

	for (i = 0; i < LOOP_SIZE; i++)
		free(arr[i]);

	free(arr);
	printf("Memory was allocated, used, and freed!\n");	
	return 0;
}
