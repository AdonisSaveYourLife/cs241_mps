/** @file part1.c */

/*
 * Machine Problem #1
 * CS 241
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mp1-functions.h"

/**
 * (Edit this function to print out the ten "Illinois" lines in mp1-functions.c in order.)
 */
int main()
{
	int value;
	int* p;
	int** q;
	first_step(81);
	value = 132;
	second_step(&value);

	value = 8942;
	p = &value;
	q = &p;
	double_step(q);


	strange_step((int*) 0);

	char temp[4] = {0,0,0,0};
	empty_step((void*) temp );

	temp[3] = 'u';
	two_step(temp,temp);


	three_step((char*)0,(char*)2,(char*)4);

	temp[1] = 0;
	temp[2] = 8;
	temp[3] = 16;
	step_step_step(temp,temp,temp);

	char aa = 1;
	it_may_be_odd(&aa,(int)aa);

	temp[0] = 1;
	temp[1] = 2;
	the_end(temp,temp);
	return 0;
}
