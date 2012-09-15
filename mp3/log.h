/** @file log.h */

#ifndef __LOG_H_
#define __LOG_H_

#define _LOG_SIZE_ALLOC_ 1024

/** The log data structure. */
typedef struct _log_t
{
	char** log_ptr;
	size_t size;
	size_t log_length;
} log_t;

void log_init(log_t* l);
void log_destroy(log_t* l);

void log_append(log_t* l, char *item);
char *log_pop(log_t* l);
char *log_at(log_t* l, unsigned int idx);
unsigned int log_size(log_t* l);
char *log_search(log_t* l, const char *prefix);

void log_print(log_t* l);

#endif
