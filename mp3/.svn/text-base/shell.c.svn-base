/** @file shell.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "log.h"

/**
 * Starting point for shell.
 */

#define _BUF_SIZE_ 1024
typedef int bool;
#define true 1
#define false 0
pid_t pid;
char* cwd;
char* lineBuf;
size_t lineSize;
ssize_t readSize;

log_t log_ob;

void insert_into_log(const char* buf){
	char* p = (char*)malloc( sizeof(char) * (strlen(buf)+5) );
	strcpy(p,buf);
	log_append(&log_ob,p);
}

bool non_built_in_command_process(char* query_ptr)
{
 	insert_into_log(query_ptr);
 	int ret = system(query_ptr);
 	int status;
 	wait(&status);
 	if(ret==-1) return false;
 	else return true;
}

void final_free_memory(){
	if(lineBuf!=NULL)
    	free(lineBuf);
    char* p;
    while( (p=log_pop(&log_ob))!=NULL )
    	free(p);
	log_destroy(&log_ob);
}



int main()
{
    log_init(&log_ob);
    pid = getpid();
    char buf[_BUF_SIZE_];
  	char* match_ptr=NULL;
  	char* query_ptr=NULL;
    cwd = getcwd(buf,_BUF_SIZE_);

    printf("(pid=%d)%s$ ", pid, cwd);

    lineBuf = NULL;
    lineSize = 0;

    while(match_ptr || ((readSize=getline(&lineBuf,&lineSize,stdin))!=-1)){
    	//printf("(pid=%d)%s$ length:%ld\n", pid, cwd,read);
    	// delete the '\n' in the end
    	if(match_ptr){
    		query_ptr = match_ptr;
    		match_ptr = NULL;
    	}
    	else{
    		query_ptr = lineBuf;
    		if(query_ptr[readSize-1]=='\n'){
    			query_ptr[readSize-1]='\0';
    			readSize --;
    		}
    	}

    	// cd xxx
    	if( strncmp(query_ptr,"cd ",strlen("cd ")) == 0 && strlen(query_ptr)>3 ){
    		insert_into_log(query_ptr);
    		if( chdir(query_ptr+3)==-1 ){
    			if(errno==ENOENT){
    				printf("%s: No such file or directory\n",query_ptr+3);
    			}
    			else{
    				printf("Error: %s\n",strerror(errno));
    				final_free_memory();
    				exit(1);
    			}
    		}
    		else{
    			cwd = getcwd(buf,_BUF_SIZE_);
    		}
    		printf("(pid=%d)%s$ ", pid, cwd);
    		continue;
    	}

    	// !#
    	if(strncmp(query_ptr,"!#",2)==0){
    		if(strcmp(query_ptr,"!#") == 0){
    			log_print(&log_ob);
    		}
    		printf("(pid=%d)%s$ ", pid, cwd);
    		continue;
    	}

    	// !Query
    	if(query_ptr[0]=='!'){
    		if(strlen(query_ptr)>1){
    			if(  (match_ptr=log_search(&log_ob,query_ptr+1))!=NULL ){
    				printf("%s matches %s\n", query_ptr+1, match_ptr);
    				continue;
    			}
    			else{ // not found result
    				printf("No Match\n");
    				printf("(pid=%d)%s$ ", pid, cwd);
    				continue;
    			}
    		}
    		else{
    			printf("(pid=%d)%s$ ", pid, cwd);
    			continue;
    		}
    	}

    	// terminate
    	if(strcmp(query_ptr,"exit") == 0){
    		insert_into_log(query_ptr);
    		break;
    	}

    	if(non_built_in_command_process(query_ptr)){
    		printf("(pid=%d)%s$ ", pid, cwd);
    		continue;
    	}
    	else{
    		printf("Error: %s\n",strerror(errno));
    		final_free_memory();
    		exit(1);
    	}
    }

    if(readSize==-1){
    	printf("Error running system()");
    	final_free_memory();
    	exit(1);
    }

    final_free_memory();
    return 0;
}

