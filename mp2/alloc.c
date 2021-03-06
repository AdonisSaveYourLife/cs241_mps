/** @file alloc.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define true 1
#define false 0
typedef char bool;

#define K_ORDER 1
#define MIN_SBRK_SIZE K_ORDER*1024*16
#define MAX_SBRK_SIZE K_ORDER*1024*1024*64
#define MIN_BLOCK_SIZE K_ORDER*32
#define MIN_BLOCK_ORDER 5
#define MAX_BLOCK_ORDER 35
#define TOTAL_ORDER_NUM (MAX_BLOCK_ORDER - MIN_BLOCK_ORDER + 1)
#define MAX_ARRAY_LENGTH 40
#define MAX(A,B) ( (A>B)?(A):(B) )
#define MIN(A,B) ( (A>B)?(B):(A) )
#define MDIC(A) ((mem_dic*)A)


//#define DEBUG
#ifdef DEBUG
#  define D(x) x
#else
#  define D(x) 
#endif

//#define LOGOUTPUT
#ifdef LOGOUTPUT
#  define L(x) x
#else
#  define L(x) 
#endif

L(size_t actual_size = 0);

/* the free list queue to store the information of valid orders */

typedef struct _QUEUE_HEAD_{
	size_t queue_size;
	void** queue_ptr;
} QUEUE_HEAD;

QUEUE_HEAD *free_list_ptr = NULL;
/**************************************/

/*** record the maximal one time alloc made **********/
size_t param_one_time_sbrk = MIN_SBRK_SIZE;

/* The main heap*/
void* heap_ptr = NULL;
size_t total_size = 0;
// size_t total_available_size = 0;
typedef struct _mem_dic {
	size_t size;
	void*  area_head;
	void*  area_end;  // This is the ptr to the end of the area, which is outside the area already.
	bool occupy;
} mem_dic;

void *split_block(void* , size_t );


bool divided2(size_t small, size_t large)
{
	while (small < large)
	{
		small *= 2;
	}
	if (small==large) return 1;
	else return 0;
}

int size2order(size_t size)
{
	size_t i = K_ORDER;
	int order = 0;
	while(i<size){
		i *= 2;
		order ++;
	}
	if(i == size) return order;
	else {
		D( printf("Error in size2order(): size %zu is not order of 2",size) );
		exit(0);
	}
}

size_t order2size(int order)
{
	size_t size = K_ORDER;
	int i;
	for(i=0;i < order; i++) size *= 2;

	return size;
}

size_t find_one_time_sbrk_size(size_t request_size)
{
	if(request_size <= MIN_SBRK_SIZE) return MIN_SBRK_SIZE;
	if(request_size >= MAX_SBRK_SIZE) return request_size;
	size_t size = 0;

	if(request_size > param_one_time_sbrk){
		size = request_size;
		param_one_time_sbrk = MIN(param_one_time_sbrk*2,MAX_SBRK_SIZE);
		return size;
	}
	else{
		size = param_one_time_sbrk;
		param_one_time_sbrk = MAX(param_one_time_sbrk/2,MIN_SBRK_SIZE);
		return size;
	}

}

/***init the free list ****/
QUEUE_HEAD* free_list_init()
{
	size_t header_size = TOTAL_ORDER_NUM *( sizeof(QUEUE_HEAD) + MAX_ARRAY_LENGTH * sizeof(void*));
	QUEUE_HEAD* new_free_list = (QUEUE_HEAD*)sbrk( header_size );
	memset(new_free_list,0x00,header_size);

	int i;
	for(i=0;i<TOTAL_ORDER_NUM;i++)
	{
		new_free_list[i].queue_size = 0;
		new_free_list[i].queue_ptr = 
			(void**)(((void*)new_free_list) + TOTAL_ORDER_NUM*sizeof(QUEUE_HEAD) 
									+ i * MAX_ARRAY_LENGTH * sizeof(void*));
	}

	return new_free_list;
}

void free_list_delete(void* block_ptr, int order)
{
	if(order<MIN_BLOCK_ORDER || order > MAX_BLOCK_ORDER){
		D( printf("Error in free_list_delete(): order %d is out of scale\n", order) );
		return;
	}
	if(free_list_ptr==NULL){
		D( printf("Error in free_list_delete(): free_list is not initialize yet\n") );
		return;
	}

	int i;
	for (i = 0; i < MAX_ARRAY_LENGTH; ++i)
	{
		if(free_list_ptr[ order - MIN_BLOCK_ORDER ].queue_ptr[i] == block_ptr )
		{
			free_list_ptr[ order - MIN_BLOCK_ORDER ].queue_ptr[i] = NULL;
			free_list_ptr[ order - MIN_BLOCK_ORDER ].queue_size -= 1;
			return;
		}
	}

	L( printf("free_list_delete(): does not find the corresponding address %ld in order %d\n",
	 	(long)block_ptr,order) );
	return;
}

void free_list_add(void* block_ptr, int order)
{
	if(order<MIN_BLOCK_ORDER || order > MAX_BLOCK_ORDER){
		L( printf("Error in free_list_delete(): order %d is out of scale\n", order) );
		return;
	}
	if(free_list_ptr==NULL){
		D( printf("Error in free_list_delete(): free_list is not initialize yet\n") );
		exit(0);
	}

	if ( free_list_ptr[order - MIN_BLOCK_ORDER].queue_size >= MAX_ARRAY_LENGTH)
	{
		L( printf("free_list_add(): Queue of order %d is full\n",order ));
		return;
	}

	int i;
	void ** empty_space_ptr = NULL;
	for (i = 0; i < MAX_ARRAY_LENGTH; ++i)
	{
		if(free_list_ptr[ order - MIN_BLOCK_ORDER ].queue_ptr[i] == block_ptr){
			L( printf("free_list_add(): address %ld of order %d already exists!\n",
				(long)block_ptr,order ) );
			return;
		}
		else if ( !(free_list_ptr[ order - MIN_BLOCK_ORDER ].queue_ptr[i]) 
							&& !empty_space_ptr){
			empty_space_ptr = &(free_list_ptr[ order - MIN_BLOCK_ORDER ].queue_ptr[i]);
		}
	}
	// add to empty_space_ptr
	if(!empty_space_ptr){
		D( printf("Error in free_list_add(): queue is not full but no availabe space\n") );
		exit(0);
	}

	*empty_space_ptr = block_ptr;
	free_list_ptr[ order - MIN_BLOCK_ORDER ].queue_size ++;
	return;


}

// Find the feasible length of block that can store size of information
size_t find_new_alloc_size(size_t size)
{
	size_t actual_size = size + sizeof(mem_dic);
	size_t num = MIN_BLOCK_SIZE;
	while(num<actual_size){
		num *=2;	
	}

	D(printf("find_new_alloc_size: new size is %zu \n",size));
	D(printf("find_new_alloc_size: the block length should be %zu \n",num));
	return num;
}

void* search_smallest_address_array(void** array)
{
	int i;
	void* frontest_addr_ptr = NULL;
	for (i = 0; i < MAX_ARRAY_LENGTH; ++i)
	{
		if(array[i]){
			if(!frontest_addr_ptr || array[i] < frontest_addr_ptr)
				frontest_addr_ptr = array[i];
		}
	}
	return frontest_addr_ptr;
}

// Find the available block large enough to fit the size,
// the available block should be the smallest block which is larger than the size
// if returned NULL, the current heap does not have empty block larger than or equal to size_plus_dic
void* block_available(size_t size_plus_dic)
{
	if(heap_ptr==NULL || total_size ==0){
		D(printf("block_available: the heap is empty, no block of size %zu\n", size_plus_dic));
		return NULL;
	}

	// search for available block from the free_list
	int order = size2order(size_plus_dic);
	int i;
	if(order >= MIN_BLOCK_ORDER && order <= MAX_BLOCK_ORDER)
	{
		for (i = order - MIN_BLOCK_ORDER; i < TOTAL_ORDER_NUM; ++i)
		{
			if (free_list_ptr[i].queue_size > 0)
			{
				/* find the corresponding frontest block address from the void** list */
				return search_smallest_address_array(free_list_ptr[i].queue_ptr);
			}
		}
	}
	/************************************************/

	// if can not find in the free list, do nothing. 
	L(printf("block_available: no block in free list for size %zu, order %d\n", size_plus_dic,order));
	return NULL;
	/*
	void* p = heap_ptr;
	void* lowest_ptr = NULL;
	size_t lowest_size = 0;

	D(printf("block_available(): total size is %zu\n", total_size));
	while((unsigned long)(p - heap_ptr) < total_size){
		if( MDIC(p)->size >= size_plus_dic && MDIC(p)->occupy==false) {
			if (MDIC(p)->size < lowest_size || lowest_size == 0)
			{
				lowest_ptr = p;
				lowest_size = MDIC(p)->size;
			}
		}
		p += MDIC(p)->size;
	}

	if (lowest_size >0)
	{
		p = lowest_ptr;
		D(printf("block_available: find block at pos: %zu with size %zu for actual size %zu\n",
				(size_t)(p - heap_ptr), MDIC(p)->size, size_plus_dic));
		return p;
	}
	else{
		D(printf("block_available: no block for size %zu\n", size_plus_dic));
		return NULL;
	}
	*/
}

/**
* Initialize heap if necessary
* Allocate blocks 2^k begining from the size MAX(MIN_SBRK_SIZE, total_size) until it is 
enough to put size into it.
* If size is small, do further split.
* Else do not need to split, just put inside.
size should be pwer of 2.
**/

void* allocate_new_space(size_t size)
{
	void* extend_heap_ptr = NULL;
	size_t one_time_alloc = find_one_time_sbrk_size(size);
	 

	extend_heap_ptr = sbrk(one_time_alloc);
	MDIC(extend_heap_ptr) -> size = one_time_alloc;
	MDIC(extend_heap_ptr) -> occupy = false;
	free_list_add(extend_heap_ptr,size2order(MDIC(extend_heap_ptr)->size));
	MDIC(extend_heap_ptr) -> area_head = extend_heap_ptr;
	MDIC(extend_heap_ptr) -> area_end = extend_heap_ptr + one_time_alloc;

	total_size += one_time_alloc;

	if(heap_ptr == NULL) heap_ptr = extend_heap_ptr;

	L(printf("allocate_new_space(): sbrked %zu bytes at loc %zu, total_size: %zu\n",
		MDIC(extend_heap_ptr) -> size, (size_t)(extend_heap_ptr - heap_ptr) ,total_size));

	if (MDIC(extend_heap_ptr) -> size >= size)
	{
		/* Split if necessary, then return */
		if(size > MDIC(extend_heap_ptr)->size / 2) return extend_heap_ptr;
		return split_block(extend_heap_ptr,size);
	}
	else
	{
		D( printf("new area %zu is smaller than the size %zu needed\n",one_time_alloc,size ) );
		exit(0);
	}
}


/**
* split the block located at block_ptr to get blocks suitable to store size
small_block_size.
small_block_size must be smaller than the block of block_ptr.
small_block_size must be power of 2.
block_ptr size must be power of 2.
small_block_size must can be divided by the size of block_ptr
Initialize all generated blocks
**/

void* split_block(void* block_ptr, size_t small_block_size)
{
	/* error check */
	if(MDIC(block_ptr) -> size < small_block_size)
	{
		D(
		printf("split_block: the oritinal size: %zu is smaller than the splitted size %zu\n",
			MDIC(block_ptr) -> size, small_block_size)
		);
		return NULL;
	}
	else if (MDIC(block_ptr) -> size == small_block_size)
		return block_ptr;

	if (!divided2(small_block_size,MDIC(block_ptr)->size))
	{
		D(
		printf("Error in split_block(): the original %zu can not be divided by %zu \n", 
			 MDIC(block_ptr)->size, small_block_size)
		);
		return NULL;
	}
	/* end error check */
	// if occupied, it is possbile that the block is being realloc.
	if(MDIC(block_ptr) -> occupy){
		L(printf("split_block(): the block to be splitted is occupied, realloc is running\n"));
	}

	void* front_ptr = block_ptr;
	void* back_ptr = NULL;
	while(MDIC(front_ptr)->size > small_block_size)
	{
		if(!MDIC(front_ptr)->occupy)
			free_list_delete(front_ptr,size2order(MDIC(front_ptr)->size));
		MDIC(front_ptr)->size /= 2;
		if(!MDIC(front_ptr)->occupy)
			free_list_add(front_ptr,size2order(MDIC(front_ptr)->size));

		back_ptr = front_ptr + MDIC(front_ptr)->size;
		MDIC(back_ptr)->size = MDIC(front_ptr)->size;
		MDIC(back_ptr)->area_head = MDIC(front_ptr)->area_head;
		MDIC(back_ptr)->area_end = MDIC(front_ptr)->area_end;
		MDIC(back_ptr)->occupy = false;
		free_list_add(back_ptr,size2order(MDIC(back_ptr)->size));
	}

	if (MDIC(front_ptr)->size != small_block_size)
	{
		D( printf("Fatal Error in split_block(): size is splited too small!\n") );
		exit(0);
	}
	else return front_ptr;
}

// find the buddy address of the block_ptr
void* buddy_address(void* block_ptr)
{
	void* area_head = MDIC(block_ptr)->area_head;
	void* area_end = MDIC(block_ptr)->area_end;
	size_t size = MDIC(block_ptr)->size;
	int buddy_flag = ( (block_ptr - area_head) / size )%2;
	void* buddy_ptr = NULL;

	buddy_ptr = (buddy_flag==1) ? (block_ptr-size) : (block_ptr+size);
	if (buddy_ptr < area_head || buddy_ptr >= area_end)
	{
		D(printf("buddy_address(): buddy outside area \n"));
		return NULL;
	}
	else return buddy_ptr;

}

void* buddy_exist_not_occupied(void* block_ptr)
{
	
	void* buddy_ptr = buddy_address(block_ptr);

	if(buddy_ptr != NULL){
		if( MDIC(buddy_ptr)->size == MDIC(block_ptr)->size )
			if( MDIC(buddy_ptr)->occupy==false )
				return MIN(buddy_ptr,block_ptr);
	}
	else return NULL;
}

/**
 * Allocate space for array in memory
 * 
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 * 
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size)
{
	/* Note: This function is complete. You do not need to modify it. */
	void *ptr = malloc(num * size);
	
	if (ptr)
		memset(ptr, 0x00, num * size);

	return ptr;
}


/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size)
{
	if(size<=0) return NULL;

	if(free_list_ptr == NULL)
		free_list_ptr = free_list_init();

	L( actual_size += size);


	size_t size_plus_dic = find_new_alloc_size(size);
	
	void *block_ptr = block_available(size_plus_dic);

	if (block_ptr == NULL)
	{
		/* Allocate new space from sbrk() */
		block_ptr = allocate_new_space(size_plus_dic);
		
		D(printf("malloc(): find new space at loc %zu with length %zu, occupy:%d\n",
				(size_t)(block_ptr - heap_ptr),MDIC(block_ptr)->size,
				MDIC(block_ptr)->occupy ));

		// store the size_plus_dic into the block
		MDIC(block_ptr) -> occupy = true;
		free_list_delete(block_ptr , size2order( MDIC(block_ptr)->size )); // delete from free list.

		L( printf("size actual_size total_size: %zu %zu %zu\n",size,actual_size,total_size) );

		return block_ptr + sizeof(mem_dic);
	}
	else
	{
		if (MDIC(block_ptr)->size > size_plus_dic)
			block_ptr = split_block(block_ptr,size_plus_dic);

		if ( !block_ptr || MDIC(block_ptr)->size != size_plus_dic){
			D( printf("Error in malloc(): size after split cannot match\n") );
			return NULL;
		}
		/* Assign the size to the block, split the block if necessary */
		D( printf("malloc(): find old space at loc %zu with length %zu, occupy:%d\n",
			(size_t)(block_ptr - heap_ptr),MDIC(block_ptr)->size,
			MDIC(block_ptr)->occupy ) );

		MDIC(block_ptr) -> occupy = true;
		free_list_delete(block_ptr,size2order( MDIC(block_ptr)->size ));

		L( printf("size actual_size total_size: %zu %zu %zu\n",size,actual_size,total_size) );

		return block_ptr + sizeof(mem_dic);

	}
	return NULL;
}


/**
 * Deallocate space in memory
 * 
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr)
{
	// "If a null pointer is passed as argument, no action occurs."
	if (!ptr)
		return;

	void* block_ptr = ptr - sizeof(mem_dic);
	if ( MDIC(block_ptr)->occupy==false )
	{
		D( printf("error in free(): the pos in %ld is not used\n",block_ptr - heap_ptr) );
	}

	MDIC(block_ptr)->occupy = false;
	free_list_add(block_ptr, size2order( MDIC(block_ptr)->size ));

	void* merge_ptr = NULL;
	void* buddy_ptr = NULL;

	while( (merge_ptr = buddy_exist_not_occupied(block_ptr)) != NULL )
	{
		// delete from free_list block_ptr and its buddy
		buddy_ptr = buddy_address(block_ptr);
		free_list_delete(block_ptr, size2order( MDIC(block_ptr)->size ));
		free_list_delete(buddy_ptr, size2order( MDIC(buddy_ptr)->size ));

		// merge buddy_ptr and block_ptr
		MDIC(merge_ptr)->size *= 2;
		MDIC(merge_ptr)->occupy = false;
		free_list_add(merge_ptr,size2order( MDIC(merge_ptr)->size ));

		block_ptr = merge_ptr;
	}
	
	return;
}


/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *    
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size)
{
	 // "In case that ptr is NULL, the function behaves exactly as malloc()"
	if (!ptr)
		return malloc(size);

	 // "In case that the size is 0, the memory previously allocated in ptr
	 //  is deallocated as if a call to free() was made, and a NULL pointer
	 //  is returned."
	if (!size)
	{
		free(ptr);
		return NULL;
	}

	size_t size_plus_dic = find_new_alloc_size(size);
	void* block_ptr = ptr - sizeof(mem_dic);
	
	// void* back_ptr = NULL;
	if ( MDIC(block_ptr)->size >= size_plus_dic){
		/* if I do not add this code, it will be less space applied
		if( MDIC(block_ptr)->size >= 2 * size_plus_dic )
		{
			MDIC(block_ptr)->size /= 2;
			back_ptr = block_ptr + MDIC(block_ptr)->size;
			MDIC(back_ptr)->size = MDIC(block_ptr)->size;
			MDIC(back_ptr)->area_head = MDIC(block_ptr)->area_head;
			MDIC(back_ptr)->area_end = MDIC(block_ptr)->area_end;
			MDIC(back_ptr)->occupy = false;
			free_list_add( back_ptr,size2order(MDIC(back_ptr)->size) );
		}
		**/
		return block_ptr + sizeof(mem_dic);
	}

	void* new_ptr = malloc(size);
	memcpy( new_ptr, ptr, MDIC(block_ptr)->size - sizeof(mem_dic) );
	free(ptr);
	return new_ptr;
}
