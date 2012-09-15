/** @file libwfg.c */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "libwfg.h"
#include "queue.h"

/**
 * Initializes the wait-for graph data structure.
 *
 * This function must always be called first, before any other libwfg
 * functions.
 *
 * @param wfg
 *   The wait-for graph data structure.
 */
void wfg_init(wfg_t *wfg)
{
	queue_init(&wfg->q_edge);
}

/****/
edge_t* fetch_thread_wait(queue_t * q, pthread_t t_id)
{
	int i;
	int size = queue_size(q);
	edge_t * item=NULL;
	for(i=0;i<size;i++)
	{
		item = queue_at(q,i);
		if(item->t_id == t_id && item->flag == Wait) return item;
	}
	return NULL;
}

/**
 * Adds a "wait" edge to the wait-for graph.
 *
 * This function adds a directed edge from a thread to a resource in the
 * wait-for graph.  If the thread is already waiting on any resource
 * (including the resource) requested, this function should fail.
 *
 * @param wfg
 *   The wait-for graph data structure.
 * @param t_id
 *   A unique identifier to a thread.  A caller to this function may
 *   want to use the thread ID used by the system, by using the
 *   pthread_self() function.
 * @param r_id
 *   A unique identifier to a resource.
 *
 * @return
 *   If successful, this function should return 0.  Otherwise, this
 *   function returns a non-zero value.
 */
int wfg_add_wait_edge(wfg_t *wfg, pthread_t t_id, unsigned int r_id)
{
	edge_t * ptr = fetch_thread_wait(&wfg->q_edge,t_id);
	if(ptr!=NULL) return 1;

	edge_t* new_edge = (edge_t*)malloc(sizeof(edge_t));
	new_edge -> t_id = t_id;
	new_edge -> r_id = r_id;
	new_edge -> flag = Wait;
	queue_enqueue(&wfg->q_edge,new_edge);

	return 0;
}

/****/
edge_t* fetch_resource_hold(queue_t* q, unsigned int r_id)
{
	int i;
	int size = queue_size(q);
	edge_t * item=NULL;
	for(i=0;i<size;i++){
		item = queue_at(q,i);
		if(item->r_id == r_id && item->flag == Hold) return item;
	}
	return NULL;
}
/**
 * Replaces a "wait" edge with a "hold" edge on a wait-for graph.
 *
 * This function replaces an edge directed from a thread to a resource (a
 * "wait" edge) with an edge directed from the resource to the thread (a
 * "hold" edge).  If the thread does not contain a directed edge to the
 * resource when this function is called, this function should fail.
 * This function should also fail if the resource is already "held"
 * by another thread.
 *
 * @param wfg
 *   The wait-for graph data structure.
 * @param t_id
 *   A unique identifier to a thread.  A caller to this function may
 *   want to use the thread ID used by the system, by using the
 *   pthread_self() function.
 * @param r_id
 *   A unique identifier to a resource.
 *
 * @return
 *   If successful, this function should return 0.  Otherwise, this
 *   function returns a non-zero value.
 */
int wfg_add_hold_edge(wfg_t *wfg, pthread_t t_id, unsigned int r_id)
{
	edge_t * item = fetch_thread_wait(&wfg->q_edge,t_id);
	if(item==NULL) return 1;
	if(item->r_id != r_id) return 2;
	if( fetch_resource_hold(&wfg->q_edge,r_id)!=NULL ) return 3;

	item -> flag = Hold;
	return 0;
}


/**
 * Removes an edge on the wait-for graph.
 *
 * If any edge exists between the thread and resource, this function
 * removes that edge (either a "hold" edge or a "wait" edge).
 *
 * @param wfg
 *   The wait-for graph data structure.
 * @param t_id
 *   A unique identifier to a thread.  A caller to this function may
 *   want to use the thread ID used by the system, by using the
 *   pthread_self() function.
 * @param r_id
 *   A unique identifier to a resource.
 *
 * @return
 * - 0, if an edge was removed
 * - non-zero, if no edge was removed
 */
int wfg_remove_edge(wfg_t *wfg, pthread_t t_id, unsigned int r_id)
{
	unsigned int i;
	edge_t * item = NULL;
	for(i=0;i<queue_size(&wfg->q_edge);i++){
		item = queue_at(&wfg->q_edge,i);
		if(item->t_id == t_id && item->r_id == r_id){
			item = queue_remove_at(&wfg->q_edge,i);
			free(item);
			return 0;
		}
	}
	return 1;
}


/**
 * Returns the numebr of threads and the identifiers of each thread that is
 * contained in a cycle on the wait-for graph.
 *
 * If the wait-for graph contains a cycle, this function allocates an array
 * of unsigned ints equal to the size of the cycle, populates the array with
 * the thread identifiers of the threads in the cycle, modifies the value
 * of <tt>cycle</tt> to point to this newly created array, and returns
 * the length of the array.
 *
 * For example, if a cycle exists such that:
 *    <tt>t(id=1) => r(id=100) => t(id=2) => r(id=101) => t(id=1) (cycle)</tt>
 * ...the array will be of length two and contain the elements: [1, 2].
 *
 * This function only returns a single cycle and may return the same cycle
 * each time the function is called until the cycle has been removed.  This
 * function can return ANY cycle, but it must contain only threads in the
 * cycle itself.
 *
 * It is up to the user of this library to free the memory allocated by this
 * function.
 *
 * If no cycle exists, this function must not allocate any memory and will
 * return 0.
 *
 *
 * @param wfg
 *   The wait-for graph data structure.
 * @param cycle
 *   A pointer to an (unsigned int *), used by the function to return the
 *   cycle in the wait-for graph if a cycle exists.
 *   
 * @return
 *   The number of threads in the identified cycle in the wait-for graph,
 *   or 0 if no cycle exists.
 */
int wfg_get_cycle(wfg_t *wfg, pthread_t** cycle)
{
	queue_t q_thread;
	queue_init(&q_thread);

	unsigned int i;
	int v_return=0;
	int cycle_flag=0;
	edge_t* begin = NULL;
	edge_t* next_hold  = NULL, *next_wait = NULL;
	for(i=0;i<queue_size(&wfg->q_edge);i++){
		begin = queue_at(&wfg->q_edge,i);
		if(begin->flag == Wait){
			// begin to find the cycle beginning from begin
			queue_destroy(&q_thread);
			queue_enqueue(&q_thread,begin);
			cycle_flag = 0;
			while(1){
				next_hold = fetch_resource_hold(&wfg->q_edge,begin->r_id);
				if(next_hold==NULL) break;
				next_wait = fetch_thread_wait(&wfg->q_edge,next_hold->t_id);
				if(next_wait==NULL) break;
				if(next_wait == queue_at(&q_thread,0)) {
					cycle_flag = 1;
					break;
				}
				queue_enqueue(&q_thread,next_wait);
				begin = next_wait;
			}
			if(cycle_flag == 1) break;
		}
	}
	if(cycle_flag == 1 && cycle!=NULL){
		pthread_t * cycle_node = (pthread_t*)malloc(sizeof(pthread_t)*queue_size(&q_thread));
		for(i=0;i<queue_size(&q_thread);i++)
		{
			cycle_node[i] = ((edge_t*)queue_at(&q_thread,i)) -> t_id;
		}
		*cycle = cycle_node;
		v_return = queue_size(&q_thread);
	}

	queue_destroy(&q_thread);
	return v_return;
}


/**
 * Prints all wait-for relations between threads in the wait-for graph.
 *
 * In a wait-for graph, a thread is "waiting for" another thread if a
 * thread has a "wait" edge on a resource that is being "held" by another
 * process.  This function prints all of the edges to stdout in a comma-
 * seperated list in the following format:
 *    <tt>[thread]=>[thread], [thread]=>[thread]</tt>
 *
 * If t(id=1) and t(id=2) are both waiting for r(id=100), but (r=100)
 * is held by t(id=3), the printout should be:
 *    <tt>1=>3, 2=>3</tt>
 *
 * When printing out the wait-for relations:
 * - this function may list the relations in any order,
 * - all relations must be printed on one line,
 * - all relations must be seperated by comma and a space (see above),
 * - a newline should be placed at the end of the list, and
 * - you may have an extra trailing ", " if it's easier for your program
 *   (not required, but should save you a bit of coding).
 *
 * @param wfg
 *   The wait-for graph data structure.
 */
void wfg_print_graph(wfg_t *wfg)
{
	unsigned int i;
	int first = 1;
	edge_t * item = NULL;
	edge_t * p_hold = NULL;
	for(i=0;i<queue_size(&wfg->q_edge);i++)
	{
		item = queue_at(&wfg->q_edge,i);
		if(item->flag == Wait){
			if( (p_hold = fetch_resource_hold(&wfg->q_edge,item->r_id)) != NULL)
			{
				if(first)
					first = 0;
				else printf(", ");
				printf("%lu=>%lu",(long unsigned int)item->t_id,(long unsigned int)p_hold->t_id);
			}
		}
	}
	printf("\n");
}


/**
 * Destroys the wait-for graph data structure.
 *
 * This function must always be called last, after all other libwfg functions.
 *
 * @param wfg
 *   The wait-for graph data structure.
 */
void wfg_destroy(wfg_t *wfg)
{
	void *item;
	while( (item=queue_dequeue(&wfg->q_edge))!=NULL ){
		free(item);
	}
	queue_destroy(&wfg->q_edge);
}

