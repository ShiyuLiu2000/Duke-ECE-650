#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__

#include <stdio.h>

// global variables to keep track of program heap memory usage
unsigned long segment_size = 0;

struct _block_t {
  size_t size;
  struct _block_t * prev;
  struct _block_t * next;
};
typedef struct _block_t block_t;

extern block_t* head;
extern block_t* tail;

// First fit malloc/free
void * ff_malloc(size_t size);
void ff_free(void * ptr);

// Best fit malloc/free
void * bf_malloc(size_t size);
void bf_free(void * ptr);

// Helper functions to implement malloc and free
void * expandMemory(size_t size);
void updateBlock(block_t * cur, size_t size);
void addBlock(block_t * ptr);
void mergeBlocks(block_t * lhs, block_t * rhs);
void freeHelper(block_t * ptr);

// Helper functions for performance study report
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();

#endif
