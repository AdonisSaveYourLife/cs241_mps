/** @file parmake.c */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "parser.h"
#include "queue.h"
#include "rule.h"

#define EXITERROR(e) _error_out_(__FUNCTION__,__LINE__,e)

#define PRINTERROR(e) fprintf(stderr,"In %s at line %d: %s\n",__FUNCTION__,__LINE__,e)

//#define debug
#ifdef debug
	#define DEBUGOUT(e) fprintf(stdout,"DEBUG: In %s at line %d: %s\n",__FUNCTION__,__LINE__,e)
	#define D(x) x
#else
	#define DEBUGOUT(e) 
	#define D(x)
#endif

#define _buf_len_ 200

typedef enum { False=0, True=1} boolean;
int max_threads = 0;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond   = PTHREAD_COND_INITIALIZER;
queue_t* new_rules;

queue_t* all_targets;

queue_t* ready_rules;

void _error_out_(const char* func,  int line, const char* error)
{
	fprintf(stderr,"In %s at line %d: %s\n", func,line,error);
	exit(1);
}

void new_targets(char* targets)
{

	rule_t* newrule = (rule_t*)malloc(sizeof(rule_t));
	rule_init(newrule);

	newrule->target = (char*)malloc(sizeof(char)*_buf_len_);
	strcpy(newrule->target,targets);

	// insert into rules queue.
	queue_enqueue(new_rules,newrule);

	// insert into all_targets
	queue_enqueue(all_targets,newrule->target);
}

void new_dependency(char *target, char *dependency )
{
	int i;
	rule_t * ptr = NULL;
	char * depbuf = (char*)malloc(sizeof(char)*_buf_len_);
	strcpy(depbuf,dependency);
	for(i=0;i<queue_size(new_rules);i++){
		ptr = (rule_t*)queue_at(new_rules,i);
		if(strcmp(ptr->target,target)==0){
			// insert dependencies into the target.
			queue_enqueue(ptr->deps,depbuf);
			return;
		}
	}
	EXITERROR("no target found!!!!");
	return;
}

void new_command(char *target, char *command)
{
	char* command_buf = (char*)malloc(_buf_len_);
	strcpy(command_buf,command);
	rule_t * ptr = NULL;
	int i;
	for(i=0;i<queue_size(new_rules);i++){
		ptr = (rule_t*)queue_at(new_rules,i);
		if(strcmp(ptr->target,target)==0){
			// insert dependencies into the target.
			queue_enqueue(ptr->commands,command_buf);
			return;
		}
	}

	EXITERROR("no target found!!!!");
	return;
}

void free_rule(rule_t* p_rule)
{
	char* ptr=NULL;
	while( (ptr = (char*)queue_dequeue(p_rule->deps)) != NULL){
		free(ptr);
	}
	while( (ptr = (char*)queue_dequeue(p_rule->commands)) != NULL){
		free(ptr);
	}

	if(p_rule->target != NULL) free(p_rule->target);

	rule_destroy(p_rule);
	free(p_rule);
}

void free_queue(queue_t * ptr)
{
	queue_destroy(ptr);
	free(ptr);
}

void free_rule_queue(queue_t * p_queue)
{
	rule_t * ptr = NULL;
	while( (ptr=(rule_t*)queue_dequeue(p_queue)) !=NULL ){
		free_rule(ptr);
	}
	queue_destroy(p_queue);
	free(p_queue);
}

boolean is_rule(char* item)
{
	int i;
	for(i=0;i<queue_size(all_targets);i++)
	{
		if(strcmp(item, queue_at(all_targets,i))==0 )
			return True;
	}
	return False;
}

boolean rule_is_ready(char* item)
{
	int i;
	boolean value = False;
	pthread_mutex_lock(&mutex2);
	for (i = 0; i < queue_size(ready_rules); ++i)
	{
		if( strcmp(item,((rule_t*)queue_at(ready_rules,i)) -> target)==0 )
		{
				value = True;
				break;
		}
	}
	pthread_mutex_unlock(&mutex2);
	return value;

}

boolean deps_are_ready(queue_t *deps)
{
	if(queue_size(deps) == 0) return True;

	int i;
	char errorbuf[100];
	char* item = NULL;
	int size = queue_size(deps);
	for(i=0;i<size;i++)
	{
		item = queue_at(deps,i);
		if(is_rule(item)){
			if(rule_is_ready(item)==False)
				return False;
		}
		else{
			if(access(item,R_OK)==-1){
				sprintf(errorbuf,"File %s does not exist.",item);
				EXITERROR(errorbuf);
			}
		}
	}
	return True;

}

rule_t* fetch_rules_torun()
{
	int i;
	int size = queue_size(new_rules);
	rule_t * rule_ptr=NULL;
	int selected_pos = -1;
	if(size==0) return NULL;
	for(i=0;i<size;i++){
		// no dependencies
		rule_ptr = queue_at(new_rules,i);
		if(queue_size(rule_ptr->deps) == 0){
			selected_pos = i;
			break; 
		}
		else if(deps_are_ready(rule_ptr->deps)){
			selected_pos = i;
			break;
		}
	}
	if(selected_pos == -1) {
		return NULL;
	}
	else{
		D(printf("fetched: %s\n",((rule_t*)queue_at(new_rules,selected_pos))->target ));
		return queue_remove_at(new_rules,selected_pos);
	}
}

void run_the_rule(rule_t* rule_ptr)
{
	int i,ret;
	char* command;
	D(char errorbuf[100]);
	queue_t * commands = rule_ptr -> commands;
	D(printf("to run: %s\n",rule_ptr->target));
	for(i=0;i<queue_size(commands);i++)
	{
		command = queue_at(commands,i);
		D(printf("%s\n",command));
		ret = system(command);
		if(ret != 0){
			D(sprintf(errorbuf,"Error running command: %s",command));
			D(PRINTERROR(errorbuf));
			exit(1);
		}
	}
}

/*
	If the rule has dependents and all dependents are not rules but files on the disk
		return True
	Else return false.
*/
boolean all_deps_are_files(rule_t* rule_ptr)
{
	if(queue_size(rule_ptr->deps) == 0) return False;

	int i;
	for(i=0;i<queue_size(rule_ptr->deps);i++)
	{
		if(is_rule(queue_at(rule_ptr->deps,i)))
			return False;
	}

	return True;
}

/* if modification time of file 1 is later than file 2, return 1
	else return -1
*/
double modicomp(char* file1, char* file2)
{
	struct stat s1, s2;

	if (stat(file1, &s1) == -1) {
		printf("stat error for file %s",file1);
        EXITERROR(NULL);
    }
    if (stat(file2, &s2) == -1) {
        printf("stat error for file %s",file2);
        EXITERROR(NULL);
    }


    return difftime(s1.st_mtime,s2.st_mtime);

}


void* rule_server(void* ptr)
{
	rule_t* rule_ptr = NULL;
	int remain_size = 0;
	int i;

	do{
		pthread_mutex_lock(&mutex1);
		remain_size = queue_size(new_rules);
		while((rule_ptr = fetch_rules_torun()) == NULL){
			remain_size = queue_size(new_rules);
			if( remain_size > 0)
				pthread_cond_wait(&cond,&mutex1);
			else{
				break;
			}
		}
		pthread_mutex_unlock(&mutex1);
		if(remain_size == 0) break;
		
		
		// deal with rule_ptr
		if(all_deps_are_files(rule_ptr))
		{
			if(access(rule_ptr->target,F_OK) ==-1 )
				run_the_rule(rule_ptr);
			else{
				for(i=0;i<queue_size(rule_ptr->deps);i++)
					if( modicomp(queue_at(rule_ptr->deps,i),rule_ptr->target) >= 0){
						run_the_rule(rule_ptr);
						break;
					}
			}
		}
		else{
			run_the_rule(rule_ptr);
		}

		


		// need to do more judge.
		pthread_mutex_lock(&mutex2);
		queue_enqueue(ready_rules,rule_ptr);
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex2);

			
	}while(remain_size > 0);

	return NULL;

}

/**
 * Entry point to parmake.
 */
int main(int argc, char **argv)
{
	//char errorbuf[200];

	int opt,i,next;
	int threads = 1;
	char* makefile = NULL;
	char filebuf[100];
	int n_targets = 0;
	char** targets = NULL;

	while ((opt = getopt(argc, argv, "j:f:")) != -1) {
        switch (opt) {
        case 'j':
            threads = atoi(optarg);
            break;
        case 'f':
            makefile = optarg;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-j threads] [-f makefile] [targets]\n",
                    argv[0]);
            return 0;
        }
    }
    // read targets.
    if(optind < argc)
    {
    	n_targets = argc - optind;
    	next = optind;
    	targets = (char**)malloc(sizeof(char*)*n_targets+sizeof(char*));
    	for(i=0;i<n_targets;i++){
    		targets[i] = malloc(50);
    		strcpy(targets[i],argv[next]);
    		next ++;
    	}
    	targets[n_targets] = NULL;
    }

    // check access of make file
    if(makefile != NULL){
    	if(access(makefile,R_OK) == -1){
    		PRINTERROR("make file does not exist.");
    		return -1;
    	}
    }else{
    	if(access("makefile",R_OK) == -1 && access("Makefile",R_OK)== -1){
    		PRINTERROR("no makefile or Makefile");
    		return -1;
    	}else if(access("makefile",R_OK) == 0){
    		strcpy(filebuf,"makefile");
    		makefile = filebuf;
    	}
    	else if(access("Makefile",R_OK) == 0){
    		strcpy(filebuf,"Makefile");
    		makefile = filebuf;
    	}
    }
    // initialize the global values.
    new_rules   = (queue_t*)malloc(sizeof(queue_t));
    ready_rules = (queue_t*)malloc(sizeof(queue_t));
    all_targets = (queue_t*)malloc(sizeof(queue_t));
    queue_init(new_rules);
    queue_init(ready_rules);
    queue_init(all_targets);
    max_threads = threads;
    pthread_t* thread_list = (pthread_t*)malloc(sizeof(pthread_t)*max_threads);
    //pthread_mutex_init(&mutex1, NULL);

    // parse the makefile.
    parser_parse_makefile(makefile,targets,&new_targets,&new_dependency,&new_command);

    for(i=0;i<max_threads;i++)
    {
    	pthread_create(&thread_list[i],NULL,rule_server,NULL);
    }
    
    for(i=0;i<max_threads;i++)
    {
    	pthread_join(thread_list[i],NULL);
    }

    if(n_targets != 0){
    	for(i=0;targets[i]!=NULL;i++) free(targets[i]);
    	free(targets);
    }

    //pthread_mutex_destroy(&mutex1, NULL);
    free_rule_queue(new_rules);
    free_rule_queue(ready_rules);
    free_queue(all_targets);
    free(thread_list);
	return 0; 
}
