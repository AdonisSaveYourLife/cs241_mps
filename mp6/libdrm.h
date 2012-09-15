#ifndef __LIBDRM_H__
#define __LIBDRM_H__

#include <pthread.h>
#include "queue.h"
#include "libwfg.h"

enum drmmode_t
{
	NO_DEADLOCK_CHECKING,
	DEADLOCK_PREVENTION,
	DEADLOCK_DETECTION,
	DEADLOCK_AVOIDANCE
};

typedef struct _drm_t
{
	unsigned int r_id; // the id of the mutex
	pthread_t t_id; // the id of the thread that locks the mutex.
	enum{Lock=1,Unlock=0} lockflag; // the flag showing whether the mutex is locked or not.
	pthread_mutex_t mu;
} drm_t;

void drm_setmode(enum drmmode_t mode);
void drm_init(drm_t *mutex);
int drm_lock(drm_t *mutex);
int drm_unlock(drm_t *mutex);
void drm_destroy(drm_t *mutex);
void drm_cleanup();

#endif
