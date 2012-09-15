/** @file libdrm.c */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "libdrm.h"
#include "libwfg.h"
#include "queue.h"

#define EXITERROR(e) _exit_error_(__FUNCTION__,__LINE__,e)
#define DEBUG

#define DDEBUG
#ifdef DDEBUG
	#define D(x) x
#else
	#define D(x)
#endif

void _exit_error_(const char* func, unsigned int line, char* error)
{
#ifdef DEBUG
	printf("EXCEPTION in function %s at line %d: %s\n",func,line,error);
#endif
	exit(1);
}

enum drmmode_t runmode;
unsigned int ID = 0;
queue_t q_mutex;

wfg_t wfg_o; // the wait for graph.
pthread_mutex_t mutex_wfg;

pthread_t o_thread;

int add_wait_edge(unsigned int r_id)
{
	int ret;
	pthread_mutex_lock(&mutex_wfg);
	ret = wfg_add_wait_edge(&wfg_o,pthread_self(),r_id);
	pthread_mutex_unlock(&mutex_wfg);
	return ret;
}

int add_hold_edge(unsigned int r_id)
{
	int ret;
	pthread_mutex_lock(&mutex_wfg);
	ret = wfg_add_hold_edge(&wfg_o,pthread_self(),r_id);
	pthread_mutex_unlock(&mutex_wfg);
	return ret;
}

int remove_edge(unsigned int r_id)
{
	int ret;
	pthread_mutex_lock(&mutex_wfg);
	ret = wfg_remove_edge(&wfg_o,pthread_self(),r_id);
	pthread_mutex_unlock(&mutex_wfg);
	return ret;
}

void print_graph()
{
	int ret;
	pthread_t * cycle;
	pthread_mutex_lock(&mutex_wfg);
	if( (ret = wfg_get_cycle(&wfg_o,&cycle)) != 0){
		free(cycle);
		printf("Deadlock detected! Edges: ");
	}
	wfg_print_graph(&wfg_o);
	pthread_mutex_unlock(&mutex_wfg);
}

int exist_cycle()
{
	int ret;
	pthread_t * cycle;
	pthread_mutex_lock(&mutex_wfg);
	if( (ret = wfg_get_cycle(&wfg_o,&cycle)) != 0){
		free(cycle);
	}
	pthread_mutex_unlock(&mutex_wfg);
	return (ret==0) ? 0 : 1;
}

int get_cycle(pthread_t ** cycle)
{
	int ret;
	pthread_mutex_lock(&mutex_wfg);
	ret = wfg_get_cycle(&wfg_o,cycle);
	pthread_mutex_unlock(&mutex_wfg);
	return ret;
}

unsigned int getId()
{
	return (++ID);
}

void resetID()
{
	ID = 0;
}

/**
**	the thread function for detection cycles
**/

void* cycle_detect_thread(void* arg)
{
	pthread_t * cycle=NULL;
	int ret,i;
	while(1)
	{
		print_graph();
		ret = get_cycle(&cycle);
		if(ret!=0)
		{
			i = (float)rand()/RAND_MAX * ret;
			if(i==ret) i--;
			pthread_kill(cycle[i], SIGUSR1);
			free(cycle);
		}
		sleep(1);
	}
	return NULL;
}

/**
 * Set the mode of the deadlock resilient mutex.
 *
 * This function will only be called once, before any of the other libdrm
 * functions.  This functions sets the global state of all drm mutexes.
 *
 * @param mode
 *   One of the following deadlock resilient modes:
 *   - NO_DEADLOCK_CHECKING
 *   - DEADLOCK_PREVENTION
 *   - DEADLOCK_DETECTION
 *   - DEADLOCK_AVOIDANCE
 */
void drm_setmode(enum drmmode_t mode)
{
	runmode = mode;	
	resetID();
	queue_init(&q_mutex);
	
	pthread_mutex_init(&mutex_wfg,NULL);
	wfg_init(&wfg_o);
	srand(time(NULL));

	if(runmode == DEADLOCK_DETECTION)
	{
		pthread_create(&o_thread, NULL,&cycle_detect_thread, NULL);
	}

}

/**
 * Frees all memory used by libdrm.
 *
 * This function will be called last, after all mutexes have been destroyed.
 */
void drm_cleanup()
{
	queue_destroy(&q_mutex);
	pthread_mutex_destroy(&mutex_wfg);
	wfg_destroy(&wfg_o);

	if(runmode == DEADLOCK_DETECTION)
		pthread_cancel(o_thread);

}



/**
 * Initialize a deadlock resilient mutex.
 *
 * This function should function similarly to pthread_mutex_init().
 *
 * @param mutex
 *   A deadlock resilient mutex.
 */
void drm_init(drm_t *mutex)
{
	mutex->r_id = getId();
	mutex->t_id	= 0;
	mutex->lockflag = Unlock;
	pthread_mutex_init(&mutex->mu,NULL);
	queue_enqueue(&q_mutex,mutex);
}


/**
 * Lock a deadlock resilient mutex.
 *
 * This function should function similarly to pthread_mutex_lock(), except it
 * must also follow the specific rules for the deadlock resilient mode that
 * has been set by the user.
 *
 * When you call the underlying pthread_mutex_lock(), make sure to check its
 * return value.  If pthread_mutex_lock() fails, the lock was not acquired
 * and you must return a failure from your drm_lock() function (don't simply
 * retry again and again).
 *
 * @param mutex
 *   A deadlock resilient mutex.
 *
 * @return
 *   When the lock is successfully acquired, this function returns zero (0).
 *   Otherwise, returns non-zero.
 */
int drm_lock(drm_t *mutex)
{
	unsigned int i;
	int ret,ret1;
	drm_t* item = NULL;
	if(runmode == NO_DEADLOCK_CHECKING){
		return pthread_mutex_lock(&mutex->mu);
	}
	else if(runmode == DEADLOCK_PREVENTION)
	{
		for(i=0;i<queue_size(&q_mutex);i++){
			item = queue_at(&q_mutex,i);
			if(item->r_id > mutex->r_id && item->lockflag == Lock 
				&& item->t_id == pthread_self()){
				return 1;
			}
		}
		if( (ret = pthread_mutex_lock(&mutex->mu)) == 0 ){
			mutex->lockflag = Lock;
			mutex->t_id		= pthread_self();
		}
		return ret;
	}
	else if(runmode == DEADLOCK_AVOIDANCE)
	{

		if( add_wait_edge(mutex->r_id) !=0 ) EXITERROR(NULL);
		if( exist_cycle() )	{
			if( remove_edge(mutex->r_id) != 0) EXITERROR(NULL);
			return 1;
		}

		/*
		if( (ret1=add_hold_edge(mutex->r_id)) !=0 ){
			D(printf("%d\n",ret1));
			EXITERROR(NULL);
		}
		ret = exist_cycle();
		if( remove_edge(mutex->r_id) != 0) EXITERROR(NULL);
		if( ret ){
			pthread_mutex_unlock(&mutex_wfg);
			return 1;
		}

		if( add_wait_edge(mutex->r_id) != 0 ) EXITERROR(NULL);*/
		
		if( (ret = pthread_mutex_lock(&mutex->mu)) == 0)
		{
			if( (ret1=add_hold_edge(mutex->r_id)) != 0) {
				D(printf("%d\n",ret1));
				EXITERROR(NULL);
			}
			if(exist_cycle()){
				if( remove_edge(mutex->r_id) != 0) EXITERROR(NULL);
				pthread_mutex_unlock(&mutex->mu);
				return 1;
			}
		}
		else{
			if( remove_edge(mutex->r_id) != 0) EXITERROR(NULL);
		}

		return ret;

	}
	else if(runmode == DEADLOCK_DETECTION)
	{

		if( add_wait_edge(mutex->r_id) != 0 ) EXITERROR(NULL);
		if( (ret = pthread_mutex_lock(&mutex->mu)) == 0)
		{
			if(add_hold_edge(mutex->r_id) != 0) EXITERROR(NULL);
		}
		else{
			if( remove_edge(mutex->r_id) != 0) EXITERROR(NULL);
		}

		return ret;
	}

	return 1;
}


/**
 * Unlock a deadlock resilient mutex.
 *
 * This function should function similarly to pthread_mutex_unlock().
 *
 * @param mutex
 *   A deadlock resilient mutex.
 *
 * @return
 *   When the lock is successfully unlocked, this function returns zero (0).
 *   Otherwise, returns non-zero.
 */
int drm_unlock(drm_t *mutex)
{
	int ret;
	if(runmode == NO_DEADLOCK_CHECKING){
		return pthread_mutex_unlock(&mutex->mu);
	}else if(runmode == DEADLOCK_PREVENTION)
	{
		if(mutex->t_id != pthread_self()) EXITERROR("mutex not owned!");
		mutex->lockflag = Unlock;
		mutex->t_id		= 0;
		return pthread_mutex_unlock(&mutex->mu);	
	}
	else if(runmode == DEADLOCK_AVOIDANCE || runmode == DEADLOCK_DETECTION) 
	{
		if(remove_edge(mutex->r_id) != 0 ) EXITERROR(NULL);
		return pthread_mutex_unlock(&mutex->mu);
	}
	

	return 1;
}


/**
 * Destroy a deadlock resilient mutex.
 *
 * This function should function similarly to pthread_mutex_destory().
 *
 * @param mutex
 *   A deadlock resilient mutex.
 */
void drm_destroy(drm_t *mutex)
{
	unsigned int i;
	for(i=0;i<queue_size(&q_mutex);i++){
		if(queue_at(&q_mutex,i) == mutex){
			queue_remove_at(&q_mutex,i);
			break;
		}
	}
	mutex->r_id = 0;
	mutex->t_id = 0;
	mutex->lockflag = Unlock;
	pthread_mutex_destroy(&mutex->mu);

}

