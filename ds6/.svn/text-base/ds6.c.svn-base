/** @file ds6.c */
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"


/** Enum of types of nodes in the wait-for graph. */
typedef enum
{
	THREAD,   /**< Thread node */
	RESOURCE  /**< Resource node */
} node_type_t;


/** A node in our wait-for graph. */
typedef struct _node_t
{
	int id;  /**< ID of the node, given by the user in _add_wait_edge() and _add_hold_edge() */
	node_type_t node_type;  /**< Type of the node (either THREAD or RESOURCE) */
	struct _node_t *edge;   /**< Out-edge of our current node (always THREAD -> RESOURCE or RESOURCE -> THREAD) */
} node_t;


/** The wait-for graph data structure. */
typedef struct _wfg_t
{
	queue_t queue;  /**< Queue of nodes */
} wfg_t;


/**
 * [Private Helper Function]
 * Searches the wait-for graph for a node of a given type and id.
 *
 * @return
 *   The node matching the given type and id, or NULL if not found.
 */
static node_t *wfg_search(wfg_t *wfg, int id, node_type_t node_type)
{
	unsigned int i;
	for (i = 0; i < queue_size( &(wfg->queue) ); i++)
	{
		node_t *node = (node_t *)queue_at( &(wfg->queue), i );

		if (node->id == id && node->node_type == node_type)
			return node;
	}

	return NULL;
}


/** Options for wfg_node_op() */
typedef enum {
	ADD_EDGE_FROM_THREAD_TO_RESOURCE,
	ADD_EDGE_FROM_RESOURCE_TO_THREAD,
	REMOVE_EDGE
} node_option_t;


/** 
 * [Private Helper Function]
 * Preforms all node operations.
 *
 * @return
 *   Returns to match return value needed of callees.
 */
static int wfg_node_op(wfg_t *wfg, int t_id, int r_id, node_option_t option)
{
	// Check if our t_id already exists.
	node_t *thread_node = wfg_search( wfg, t_id, THREAD );

	if (!thread_node)
	{
		thread_node = malloc( sizeof(node_t) );
		thread_node->id = t_id;
		thread_node->edge = NULL;
		thread_node->node_type = THREAD;
		queue_enqueue( &(wfg->queue), thread_node );
	}

	// Check if our r_id already exists.
	node_t *resource_node = wfg_search( wfg, r_id, RESOURCE );

	if (!resource_node)
	{
		resource_node = malloc( sizeof(node_t) );
		resource_node->id = r_id;
		resource_node->edge = NULL;
		resource_node->node_type = RESOURCE;
		queue_enqueue( &(wfg->queue), resource_node );
	}

	// Do our operation...
	if (option == ADD_EDGE_FROM_THREAD_TO_RESOURCE)
	{
		if (thread_node->edge == NULL)
		{
			thread_node->edge = resource_node;
			return 0;
		}
		else
			return 1;
	}
	else if (option == ADD_EDGE_FROM_RESOURCE_TO_THREAD)
	{
		/* Check that we have a "wait" edge
		   AND check that the resource isn't already being held... */
		if ( thread_node->edge != NULL && thread_node->edge == resource_node
			 && resource_node->edge == NULL )
		{
			thread_node->edge = NULL;
			resource_node->edge = thread_node;
			return 0;
		}
		else
			return 1;
	}
	else if (option == REMOVE_EDGE)
	{
		int ret = 1;

		if (thread_node->edge != NULL && thread_node->edge == resource_node)
		{
			thread_node->edge = NULL;
			ret = 0;
		}

		if (resource_node->edge != NULL && resource_node->edge == thread_node)
		{
			resource_node->edge = NULL;
			ret = 0;
		}

		return ret;
	}

	// Should never happen...
	printf("Unknown op, bad call.");
	exit(1);
	return 0;
}


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
	queue_init(&(wfg->queue));
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
int wfg_add_wait_edge(wfg_t *wfg, int t_id, int r_id)
{
	return wfg_node_op(wfg, t_id, r_id, ADD_EDGE_FROM_THREAD_TO_RESOURCE);
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
int wfg_add_hold_edge(wfg_t *wfg, int t_id, int r_id)
{
	return wfg_node_op(wfg, t_id, r_id, ADD_EDGE_FROM_RESOURCE_TO_THREAD);
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
int wfg_remove_edge(wfg_t *wfg, int t_id, int r_id)
{
	return wfg_node_op(wfg, t_id, r_id, REMOVE_EDGE);
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
int wfg_get_cycle(wfg_t *wfg, unsigned int **cycle)
{
	return 0;
}


int main()
{
	wfg_t wfg;
	unsigned int *cycle;
	int cycle_ct;

	wfg_init(&wfg);


	/* t(id=2) ==(waiting for)==> r(id=100) */
	wfg_add_wait_edge(&wfg, 2, 100);

	/* t(id=2) <==(held by)== r(id=100) */
	wfg_add_hold_edge(&wfg, 2, 100);

	/* No cycle should exist at this point. */
	cycle_ct = wfg_get_cycle(&wfg, &cycle);
	if (cycle_ct > 0) { printf("Error! No cycle should exist.\n"); return 1; }


	/* EXISTING: t(id=2) <==(held by)== r(id=100)
	        NEW: t(id=1) ==(waiting for)==> r(id=200) */
	wfg_add_wait_edge(&wfg, 1, 200);

	/* EXISTING: t(id=2) <==(held by)== r(id=100)
	     UPDATE: t(id=1) <==(held by)== r(id=200) */
	wfg_add_hold_edge(&wfg, 1, 200);

	/* No cycle should exist at this point. */
	cycle_ct = wfg_get_cycle(&wfg, &cycle);
	if (cycle_ct > 0) { printf("Error! No cycle should exist.\n"); return 1; }


	/* EXISTING: t(id=2) <==(held by)== r(id=100)
	   EXISTING: t(id=1) <==(held by)== r(id=200)
	        NEW: t(id=2) ==(waiting for)==> r(id=200) */
	wfg_add_wait_edge(&wfg, 2, 200);

	/* EXISTING: t(id=2) <==(held by)== r(id=100)
	   EXISTING: t(id=1) <==(held by)== r(id=200)
	   EXISTING: t(id=2) ==(waiting for)==> r(id=200)
	        NEW: t(id=1) ==(waiting for)==> r(id=100) */
	wfg_add_wait_edge(&wfg, 1, 100);

	/* Should have a cycle:
	        t(id=1) ==(waiting for)==> r(id=100)
			r(id=100) ==(held by)==> t(id=2)
			t(id=2) ==(waiting for)==> r(id=200)
			r(id=200) ==(held by)==> t(id=1)
			                             CYCLE! */
	cycle_ct = wfg_get_cycle(&wfg, &cycle);
	if (cycle_ct == 0) { printf("Error! Cycle should exist.\n"); return 1; }

	int i;
	printf("Cycle found!\n");
	for (i = 0; i < cycle_ct; i++)
		printf("  cycle[%d]: %d\n", i, cycle[i]);


	return 0;
}
