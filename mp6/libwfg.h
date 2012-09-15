#ifndef __LIBWFG_H__
#define __LIBWFG_H__

#include "queue.h"
#include <pthread.h>

typedef enum{Wait=0,Hold=1} _wt_;

typedef struct _edge_t_{
	pthread_t t_id;
	unsigned int r_id;
	_wt_		 flag;
} edge_t;

typedef struct _wfg_t
{
	queue_t q_edge;
} wfg_t;

void wfg_init(wfg_t *wfg);
int  wfg_add_wait_edge(wfg_t *wfg, pthread_t t_id, unsigned int r_id);
int  wfg_add_hold_edge(wfg_t *wfg, pthread_t t_id, unsigned int r_id);
int  wfg_remove_edge(wfg_t *wfg, pthread_t t_id, unsigned int r_id);
int  wfg_get_cycle(wfg_t *wfg, pthread_t** cycle);
void wfg_print_graph(wfg_t *wfg);
void wfg_destroy(wfg_t *wfg);

#endif
