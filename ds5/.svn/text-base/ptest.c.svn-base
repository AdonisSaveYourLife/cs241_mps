#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/**
 * Data structure to store the prime candidate and
 * the range for each thread.
 */
typedef struct _ptest_t
{
	long prime;
	long start, end;
} ptest_t;


/**
 * Data structure to store the result of a ptest_t.
 */
typedef struct _result_t
{
	long result;
} result_t;


/**
 * Tests if a value is prime by dividing the canidate prime by
 * every number in the inclusive range: [start, end].
 * 
 * @param ptr
 *    A pointer to the ptest_t data structure.
 *
 * @return
 *    A pointer to a result_t data structure storing
 *    either the number that the prime canidate is divisible
 *    by or 0 if the prime canidate is not divisible by
 *    any number in the range [start, end].
 */
void *prime_tester(void *ptr)
{
	ptest_t *ptest = (ptest_t *)ptr;
	result_t *result;



	return result;
}


/**
 * Returns the minimum of two numbers.
 */
long min(long a, long b) { return (a < b) ? a : b; }



int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("Usage: %s <prime value> <# threads>\n", argv[0]);
		printf("       %s 1234567 1\n", argv[0]);
		printf("       %s 1234567 6\n", argv[0]);
		return 1;
	}

	long num = atol(argv[1]); /**< Canidate prime number */
	int threads = atoi(argv[2]); /**< Number of threads to spwan. */



	long div; /**< The result of all result_t data structures. */

	if (div == 0)
		printf("%ld is a prime!\n", num);
	else
		printf("%ld is not a prime (divisible by %ld).\n", num, div);

	return 0;
}




