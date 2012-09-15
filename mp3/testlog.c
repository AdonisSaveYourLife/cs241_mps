/** @file testlog.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

int main()
{
    log_t l;
    log_init(&l);

    int i;
    log_append(&l,"Iddd am a genius");
    log_append(&l,"2");
    log_append(&l, "ab  1");
 	log_append(&l, "a   2");
	log_append(&l, "abc 3");
 	log_append(&l, "ab  4");
 	log_append(&l, "a   5");

 	printf("search: %s\n",log_search(&l,"a"));
 	printf("pop: %s\n",log_pop(&l));
 	printf("search: %s\n",log_search(&l,"a"));
 	printf("search: %s\n",log_search(&l,"ab"));
 	printf("pop: %s\n",log_pop(&l));
 	printf("search: %s\n",log_search(&l,"ab"));
 	printf("search: %s\n",log_search(&l,"abc"));

 	

 	for(i=0;i<1024*1024*100;++i){
 		log_append(&l,"I am a genius");
 	}

 	printf("size: %ul \n",log_size(&l));
 	printf("search: %s\n",log_search(&l,"I am"));


 	for(i=0;i<1024*1024;++i){
 		log_pop(&l);
 	}

 	printf("size: %u \n",log_size(&l));
 	printf("search: %s\n",log_search(&l,"I am"));

    log_destroy(&l);


    return 0;
}
