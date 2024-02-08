#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> // EXIT_SUCCESS
#include <unistd.h> // sbrk

struct _block_t {
  size_t size;
  struct _block_t *prev;
  struct _block_t *next;
};
typedef struct _block_t block_t;

// extern block_t *head;
// extern block_t *tail;

// extern __thread block_t *nolockhead;
// extern __thread block_t *nolocktail;

// Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);

// Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

#endif
