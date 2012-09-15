/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"

node_t*   new_node        ()
{
  node_t * p = (node_t*)malloc(sizeof(node_t));
  p->obj = NULL;
  p->next = NULL;
  return p;
}

void*   get_obj_node    (node_t* node_ptr)
{
  if(node_ptr==NULL){
    ERROROUT("node is empty");
    exit(1);
  }
  return node_ptr->obj;
}

void*    destroy_node     (node_t* node_ptr)
{
  void* p = node_ptr->obj;
  free(node_ptr);
  return p;
}

void*    insert_value_node (node_t* node_ptr, void* obj_ptr)
{
  void* p = node_ptr->obj;
  node_ptr -> obj = obj_ptr;
  return p;
}

void*   insert_next_node   (node_t* node_ptr, node_t* inserter)
{
  node_t * p = node_ptr -> next;
  node_ptr -> next = inserter;
  return (void*)p;
}
/**
  Initializes the priqueue_t data structure.
  
  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
  q->size = 0;
  q->head = NULL;
  q->comp = comparer;
}


/**
  Inserts the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
	node_t* node_ptr = new_node();
  insert_value_node(node_ptr,ptr);
  int i;
  if(q->size ==0){
    q->head = node_ptr;
    q->size ++;
    return 0;
  }
  
  if(q->comp(node_ptr->obj,q->head->obj) < 0)
  {
    insert_next_node(node_ptr,q->head);
    q->head = node_ptr;
    q->size ++;
    return 0;
  }
  
  node_t* iterator = q->head;
  for (i = 0; i < q->size; ++i)
  {
    if(iterator->next == NULL || q->comp(node_ptr->obj,iterator->next->obj)<0){
      insert_next_node(node_ptr,iterator->next);
      iterator->next = node_ptr;
      q->size ++;
      return i+1;
    }
    else iterator = iterator -> next;
  }

  ERROROUT("insert failed.");
  exit(1);
  return -1;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
	if(q->size == 0) return NULL;
  return q->head->obj;
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
	if(q->size == 0) return NULL;

  node_t * node_ptr = q->head;
  q->head = node_ptr->next;
  q->size --;
  return destroy_node(node_ptr);
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
	if(index < 0){
    ERROROUT("index should be positive.");
    return NULL;
  }
  if(index >= q->size) return NULL;
  if(index == 0) return q->head->obj;

  int i;
  node_t * iterator = q->head;
  for(i=1;i<= index;++i)
    iterator = iterator -> next;
  return iterator -> obj;

}


/**
  Removes all instances of ptr from the queue. 
  
  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
  int number_removed = 0;

  while(q->size >0 && ptr == get_obj_node(q->head)) {
    priqueue_poll(q);
    number_removed ++; 
  }

  node_t *iterator, *node_ptr;
  if(q->size > 0)
  {
    iterator = q->head;
    while(iterator->next != NULL ){
      if(get_obj_node(iterator->next) == ptr){
        node_ptr = iterator -> next;
        iterator -> next = node_ptr -> next;
        q->size --;
        destroy_node(node_ptr);
        number_removed ++;
        continue;
      }
      iterator = iterator -> next;
    }
  }

  return number_removed;
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	if(index < 0){
    ERROROUT("index should be positive.");
    return NULL;
  }
  if(index >= q->size) return NULL;
  if(index == 0) return priqueue_poll(q);

  int i;
  node_t *iterator = q->head, *node_ptr;
  for(i=1;i<= index -1 ;++i)
    iterator = iterator -> next;

  node_ptr = iterator -> next;
  iterator -> next = node_ptr -> next;
  q-> size --;
  return destroy_node(node_ptr);
  
}


/**
  Returns the number of elements in the queue.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->size;
}


/**
  Destroys and frees all the memory associated with q.
  
  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
  while(q->size >0) priqueue_poll(q);
}
