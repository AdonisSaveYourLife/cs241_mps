#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>


void* print_msg(void* arg)
{
	int* value = arg;
	char buf[100];
	sprintf(buf,"sleep %d",*value);
	system(buf);
	printf("I am done\n");
	return NULL;
}

int main()
{
	pthread_t thread_list[3];
	int i;
	int values[3];
	values[0] = 1;
	values[2] = 10;
	values[3] = 20;
    //pthread_mutex_init(&mutex1, NULL);


    for(i=0;i<3;i++)
    {
    	pthread_create(&thread_list[i],NULL,print_msg,&values[i]);
    }
    
    for(i=0;i<3;i++)
    {
    	pthread_join(thread_list[i],NULL);
    }
    return 0;
}