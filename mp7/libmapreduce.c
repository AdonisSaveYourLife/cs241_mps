/** @file libmapreduce.c */
/* 
 * CS 241
 * The University of Illinois
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>

#include "libmapreduce.h"
#include "libdictionary.h"

#define PROCESS_END 100
//#define DEBUG
#ifdef DEBUG
 	#define D(x) x
#else
 	#define D(x)
#endif


static const int BUFFER_SIZE = 2048;  /**< Size of the buffer used by read_from_fd(). */
int pipefd[200][2];
pid_t pids[200];
int num_values = 0;
pthread_t thread_id;


int n_remain_process;
pthread_mutex_t mutex_remain;
void set_remain(int value)
{
	pthread_mutex_lock(&mutex_remain);
	n_remain_process = value;
	pthread_mutex_unlock(&mutex_remain);
}

int get_remain()
{
	int value;
	pthread_mutex_lock(&mutex_remain);
	value = n_remain_process;
	pthread_mutex_unlock(&mutex_remain);
	return value;
}

void decrease_remain()
{
	pthread_mutex_lock(&mutex_remain);
	n_remain_process --;
	pthread_mutex_unlock(&mutex_remain);
}
/*
mapreduce_t *g_mr = NULL;
*/

void exit_error(const char* info, int line)
{
	printf("Exception at line %d",line);
	perror(info);
	exit(EXIT_FAILURE);
}

void print_debug(const char* info, int line)
{
#ifdef DEBUG
	printf("%s at line %d\n",info,line);
	exit(1);
#endif
}

/**
 * Adds the key-value pair to the mapreduce data structure.  This may
 * require a reduce() operation.
 *
 * @param key
 *    The key of the key-value pair.  The key has been malloc()'d by
 *    read_from_fd() and must be free()'d by you at some point.
 * @param value
 *    The value of the key-value pair.  The value has been malloc()'d
 *    by read_from_fd() and must be free()'d by you at some point.
 * @param mr
 *    The pass-through mapreduce data structure (from read_from_fd()).
 */
static void process_key_value(const char *key, const char *value, mapreduce_t *mr)
{
	const char* old_value = NULL;
	const char* new_value = NULL;
	if( (old_value = dictionary_get(&mr->kv,key)) == NULL)
	{
		if( dictionary_add(&mr->kv,key,value) != 0)
		{
			exit_error("key already exist",__LINE__);
		}
	}
	else
	{
		new_value = (*mr->myreduce)(old_value,value);
		if(new_value == NULL){
			exit_error("reduce returned NULL",__LINE__);
		}
		free((void*)value);

		if( dictionary_remove_free(&mr->kv,key) != 0 ){
			exit_error("key not exist",__LINE__);
		}

		if( dictionary_add(&mr->kv,key,new_value) != 0 ){
			exit_error("key already exist",__LINE__);
		}

	}
}


/**
 * Helper function.  Reads up to BUFFER_SIZE from a file descriptor into a
 * buffer and calls process_key_value() when for each and every key-value
 * pair that is read from the file descriptor.
 *
 * Each key-value must be in a "Key: Value" format, identical to MP1, and
 * each pair must be terminated by a newline ('\n').
 *
 * Each unique file descriptor must have a unique buffer and the buffer
 * must be of size (BUFFER_SIZE + 1).  Therefore, if you have two
 * unique file descriptors, you must have two buffers that each have
 * been malloc()'d to size (BUFFER_SIZE + 1).
 *
 * Note that read_from_fd() makes a read() call and will block if the
 * fd does not have data ready to be read.  This function is complete
 * and does not need to be modified as part of this MP.
 *
 * @param fd
 *    File descriptor to read from.
 * @param buffer
 *    A unique buffer associated with the fd.  This buffer may have
 *    a partial key-value pair between calls to read_from_fd() and
 *    must not be modified outside the context of read_from_fd().
 * @param mr
 *    Pass-through mapreduce_t structure (to process_key_value()).
 *
 * @retval 1
 *    Data was available and was read successfully.
 * @retval 0
 *    The file descriptor fd has been closed, no more data to read.
 * @retval -1
 *    The call to read() produced an error.
 */
static int read_from_fd(int fd, char *buffer, mapreduce_t *mr)
{
	/* Find the end of the string. */
	int offset = strlen(buffer);

	/* Read bytes from the underlying stream. */
	int bytes_read = read(fd, buffer + offset, BUFFER_SIZE - offset);
	if (bytes_read == 0)
		return 0;
	else if(bytes_read < 0)
	{
		fprintf(stderr, "error in read.\n");
		return -1;
	}

	buffer[offset + bytes_read] = '\0';

	D(printf("buffer: %s\n", buffer ));

	/* Loop through each "key: value\n" line from the fd. */
	char *line;
	while ((line = strstr(buffer, "\n")) != NULL)
	{
		*line = '\0';

		/* Find the key/value split. */
		char *split = strstr(buffer, ": ");
		if (split == NULL)
			continue;

		/* Allocate and assign memory */
		char *key = malloc((split - buffer + 1) * sizeof(char));
		char *value = malloc((strlen(split) - 2 + 1) * sizeof(char));

		strncpy(key, buffer, split - buffer);
		key[split - buffer] = '\0';

		strcpy(value, split + 2);

		/* Process the key/value. */
		process_key_value(key, value, mr);

		/* Shift the contents of the buffer to remove the space used by the processed line. */
		memmove(buffer, line + 1, BUFFER_SIZE - ((line + 1) - buffer));
		buffer[BUFFER_SIZE - ((line + 1) - buffer)] = '\0';
	}

	return 1;
}


/**
 * Initialize the mapreduce data structure, given a map and a reduce
 * function pointer.
 */
void mapreduce_init(mapreduce_t *mr, 
                    void (*mymap)(int, const char *), 
                    const char *(*myreduce)(const char *, const char *))
{	
	mr->mymap 	 = mymap;
	mr->myreduce = myreduce;
	dictionary_init(&mr->kv);
	pthread_mutex_init(&mutex_remain, NULL);
}

/** the worker thread for accepting reduce function **/
void* worker_thread(void* attr)
{
	mapreduce_t* mr = (mapreduce_t*)attr;
	if(num_values<=0) return NULL;
	// N buffers, each for a process
	char** p_buffer;
	int i;
	p_buffer = (char**)malloc(sizeof(char*)*num_values);
	for(i=0;i<num_values;i++){
		p_buffer[i] = (char*)malloc(sizeof(char)*(BUFFER_SIZE+1));
		p_buffer[i][0] = '\0';
	}
	int nfds = 0;
	for(i=0;i<num_values;i++){
		if(nfds < pipefd[i][0]) nfds = pipefd[i][0];
	}
	nfds ++;

	// select function
	int retval;
	int readret;
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    fd_set rfds;
    FD_ZERO(&rfds);
    for(i=0;i<num_values;i++) FD_SET(pipefd[i][0],&rfds);

    //print_debug("I am entering select",__LINE__);

    while( (retval = select(nfds, &rfds, NULL, NULL, &tv))!= -1 )
    {
    	//D(printf("retval:%d,n_remain_process:%d\n",retval,n_remain_process));
    	if(retval > 0)
    	{
    		/* deal with reduce */
    		for(i=0;i<num_values;i++)
    		{
    			if( FD_ISSET(pipefd[i][0],&rfds) )
    			{
    				readret = read_from_fd(pipefd[i][0], p_buffer[i], mr);
    				if(readret == PROCESS_END)
    				{
    					n_remain_process --;
    				}
    				else if( readret!=1 )
    				{
    					printf("pipeid:%d,pipe:%d\n",i,pipefd[i][0]);
    					exit_error("read_from_fd",__LINE__);
    				}
    			}
    		}
    	}

    	if(retval > 0) // reset
    	{
    		FD_ZERO(&rfds);
    		for(i=0;i<num_values;i++) FD_SET(pipefd[i][0],&rfds);
    	}

    	if(retval == 0) if(get_remain() == 0) break;
    	tv.tv_sec = 2;
    	tv.tv_usec = 0;
    }

    if(retval == -1){
    	exit_error("select error",__LINE__);
    }


	for(i=0;i<num_values;i++) free(p_buffer[i]);
	free(p_buffer);
	return NULL;
}


/**
 * Starts the map() processes for each value in the values array.
 * (See the MP description for full details.)
 */
void mapreduce_map_all(mapreduce_t *mr, const char **values)
{
	int i;
	pid_t newpid;

	for(i=0;values[i]!=NULL;i++)
	{
		num_values ++;
		if (pipe(pipefd[i]) == -1) {
			exit_error("pipe",__LINE__);
	    }
	    newpid=fork();
		if( newpid ==0)
		{
			/* child */
			(*mr->mymap)(pipefd[i][1], values[i]);
			exit(0);
		}
		else if(newpid > 0) pids[i] = newpid;
		else exit_error("error building child process",__LINE__);
	}
	D(printf("num_values: %d\n", num_values));
	set_remain(num_values);
	
	// create the worker thread.
	pthread_create(&thread_id,NULL,&worker_thread,mr);

}


/**
 * Blocks until all the reduce() operations have been completed.
 * (See the MP description for full details.)
 */
void mapreduce_reduce_all(mapreduce_t *mr)
{
	int i;
	int status;
	for(i=0;i<num_values;i++){
		waitpid(pids[i],&status,0);
		decrease_remain();
	}


	pthread_join(thread_id,NULL);

}


/**
 * Gets the current value for a key.
 * (See the MP description for full details.)
 */
const char *mapreduce_get_value(mapreduce_t *mr, const char *result_key)
{
	const char* value = dictionary_get(&mr->kv, result_key);
	return value;
}


/**
 * Destroys the mapreduce data structure.
 */
void mapreduce_destroy(mapreduce_t *mr)
{
	dictionary_destroy_free(&mr->kv);
	pthread_mutex_destroy(&mutex_remain);
}

