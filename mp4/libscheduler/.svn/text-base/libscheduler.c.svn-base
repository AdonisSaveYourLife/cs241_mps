/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"

typedef int bool;
#define true 1
#define false 0
#define MIN(a,b) ((a) < (b) ? (a) : (b))

int comparer(const void *, const void *);



typedef struct _job_t_
{
  int job_number;
  int arrival_time;
  int running_time;
  int priority;
  int remaining_time;
  int first_time_on_core;
  int last_update_time_on_core;
} job_t;

typedef struct _core_t_
{
  int num_cores;
  job_t** job_in_cores;
} core_t;
core_t core;

float total_wait = 0.0;
int num_wait = 0;
void insert_wait(int time)
{
  total_wait += time;
  num_wait ++;
}

float total_response = 0.0;
int num_response = 0;
void insert_response(int time)
{
  total_response += time;
  num_response ++;
}

float total_turnaround = 0.0;
int num_turnaround = 0;
void insert_turnaround(int time)
{
  total_turnaround += time;
  num_turnaround ++;
}

scheme_t applied_schema;
priqueue_t* job_queue_ptr;
core_t core;


int now; // the time now.
void update_time(int time)
{
  now = time;
  job_t * job_ptr = NULL;
  // update the remaining time stored in the queue.
  int i;
  for(i=0;i<core.num_cores;++i)
  {
    job_ptr = core.job_in_cores[i];
    if(job_ptr != NULL){
      if(job_ptr->first_time_on_core==-1 && job_ptr->last_update_time_on_core != now){
        job_ptr->first_time_on_core = job_ptr -> last_update_time_on_core;
        insert_response(job_ptr->first_time_on_core - job_ptr->arrival_time);
      }

      job_ptr -> remaining_time -= 
                  now - job_ptr -> last_update_time_on_core;
      job_ptr -> last_update_time_on_core = now;

    }
  }
  
}

int get_now()
{
  return now;
}
/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements. 
*/

job_t* new_job()
{
  job_t* ptr = (job_t*)malloc(sizeof(job_t));
  return ptr;
}

void destroy_job(job_t* ptr)
{
  free(ptr);
}



void core_init(core_t *ptr, int num)
{
  ptr -> num_cores = num;
  ptr -> job_in_cores = (job_t**)malloc(sizeof(job_t*)*num);
  int i;
  for(i=0;i<num;i++) ptr->job_in_cores[i] = NULL;
}

void core_destroy(core_t *ptr)
{
  free(ptr->job_in_cores);
}

// delete job from core according to the job_number,
// at the same time, check whether the job is indeed in the core.
job_t* core_job_remove_on_id(int core_id, int job_number)
{
  if(core_id<0 || core_id >= core.num_cores){
    ERROROUT("out of range.");
    exit(1);
  }
  if(core.job_in_cores[core_id]->job_number != job_number){
    ERROROUT("the job is not in the corresponding core.");
    exit(1);
  }

  job_t* job_ptr = core.job_in_cores[core_id];
  job_ptr -> last_update_time_on_core = -1;
  // re insert back into the queue.
  core.job_in_cores[core_id] = NULL;
  priqueue_offer(job_queue_ptr,job_ptr);
  return job_ptr;
}

int core_available()
{
  int i;
  for(i=0;i<core.num_cores;i++) if(core.job_in_cores[i] == NULL) return i;

  return -1;
}

// return the job assigned into the core
job_t* core_assign_on_id(int index, job_t* job_ptr)
{
  if(core.job_in_cores[index] != NULL){
    ERROROUT("core already occupied.");
    exit(1);
  }
  core.job_in_cores[index] = job_ptr;

  job_ptr -> last_update_time_on_core = get_now();
  return job_ptr;
}


int core_preempt(job_t* job_ptr)
{
  if(core_available() != -1){
    ERROROUT("no need to preempt.");
    exit(1);
  }
  int comp_result = 0, new_result = 0;
  int index = -1, i;
  for(i=0;i<core.num_cores;i++) {
    new_result = comparer(job_ptr, core.job_in_cores[i]);
    if(new_result < comp_result){
      comp_result = new_result;
      index = i;
    }else if(new_result == comp_result){
      if(index != -1){
        index = (core.job_in_cores[index]->arrival_time >= core.job_in_cores[i]->arrival_time) ? index : i;
      }
    }
  }

  // insert job into core.
  if(index >= 0){
    core_job_remove_on_id(index,core.job_in_cores[index]->job_number);
    core_assign_on_id(index,job_ptr);
  }

  return index;
}


// return true if the scheme is preempt
bool scheme_is_preempt() 
{
  return (applied_schema == PSJF || applied_schema == PPRI);
}





int comparer(const void * job1_ptr, const void * job2_ptr)
{
  // typedef enum {FCFS = 0, SJF, PSJF, PRI, PPRI, RR}
  int result = 0;
  int timediff = (int)( ((job_t*)job1_ptr) -> arrival_time - ((job_t*)job2_ptr) -> arrival_time );
  if(applied_schema == FCFS)
  {
    return timediff;
  }else if(applied_schema == SJF)
  {
    result = (int)( ((job_t*)job1_ptr) -> running_time - ((job_t*)job2_ptr) -> running_time );
    return (result==0) ? timediff : result;
  }else if(applied_schema == PSJF)
  {
    result = (int)( ((job_t*)job1_ptr) -> remaining_time - ((job_t*)job2_ptr) -> remaining_time );
    return (result==0) ? timediff : result;
  }else if(applied_schema == PRI || applied_schema == PPRI)
  {
    result = (int)( ((job_t*)job1_ptr) -> priority - ((job_t*)job2_ptr) -> priority );
    return (result==0) ? timediff : result;
  }else if(applied_schema == RR)
  {
    return 0;
  }

  ERROROUT("Invald schema!");
  return 0;
}

/**
  Initalizes the scheduler.
 
  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
  applied_schema = scheme;
  job_queue_ptr = (priqueue_t*)malloc(sizeof(priqueue_t));
  priqueue_init(job_queue_ptr,&comparer);
  core_init(&core, cores);
  update_time(0);
}


/**
  Called when a new job arrives.
 
  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made. 
 
 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
	update_time(time);
  job_t* new_job_ptr = new_job();
  new_job_ptr -> job_number = job_number;
  new_job_ptr -> arrival_time = time;
  new_job_ptr -> running_time = running_time;
  new_job_ptr -> remaining_time = running_time;
  new_job_ptr -> priority = priority;
  new_job_ptr -> first_time_on_core = -1;
  new_job_ptr -> last_update_time_on_core = -1;

  int index;

  if( (index = core_available()) != -1 ){
    core_assign_on_id(index,new_job_ptr);
    return index;
  }
  else if(scheme_is_preempt()){
    index = core_preempt(new_job_ptr);
    if(index == -1) priqueue_offer(job_queue_ptr,new_job_ptr);
    return index;
  }else{
    priqueue_offer(job_queue_ptr,new_job_ptr);
    return -1;
  }
}


/**
  Called when a job has completed execution.
 
  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.
 
  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
	update_time(time);
  job_t * job_ptr = core_job_remove_on_id(core_id,job_number);
  priqueue_remove(job_queue_ptr,job_ptr);

  insert_wait(get_now() - job_ptr->arrival_time - job_ptr->running_time);
  insert_turnaround(get_now() - job_ptr->arrival_time);

  destroy_job(job_ptr);

  job_ptr = priqueue_poll(job_queue_ptr);
  if(job_ptr){
    core_assign_on_id(core_id,job_ptr);
    return job_ptr -> job_number;
  }

  return -1;
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.
 
  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator. 
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
	update_time(time);
  core_job_remove_on_id(core_id,core.job_in_cores[core_id]->job_number);

  job_t *job_ptr = priqueue_poll(job_queue_ptr);
  if(job_ptr){
    core_assign_on_id(core_id,job_ptr);
    return job_ptr -> job_number;
  }

  return -1;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
	return (num_wait == 0) ? 0.0 : (float)total_wait/num_wait;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	return (num_turnaround==0) ? 0.0 : (float)total_turnaround/num_turnaround;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	return (num_response == 0) ? 0.0 : (float)total_response/num_response;
}


/**
  Free any memory associated with your scheduler.
 
  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
  void *p = NULL;
  while( (p = priqueue_poll(job_queue_ptr)) != NULL ) destroy_job((job_t*)p);
  core_destroy(&core);
  priqueue_destroy(job_queue_ptr);
  free(job_queue_ptr);
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)  
  
  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{

}
