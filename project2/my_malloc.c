#include "my_malloc.h"

#include <pthread.h>
#include <stdint.h> // intptr_t
#include <stdio.h>  // printf
#include <stdlib.h> // EXIT_SUCCESS
#include <unistd.h> // sbrk

// set global head and tail pointers of doubly linked list
block_t *head = NULL;
block_t *tail = NULL;

__thread block_t *nolockhead = NULL;
__thread block_t *nolocktail = NULL;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// lock version
void updateBlock_lock(block_t *cur, size_t size);
void addBlock_lock(block_t *ptr);
void mergeBlocks_lock(block_t *lhs, block_t *rhs);

// nolock version
void updateBlock_nolock(block_t *cur, size_t size);
void addBlock_nolock(block_t *ptr);
void mergeBlocks_nolock(block_t *lhs, block_t *rhs);

// When no free block region is larger than the required size, call sbrk() to
// create one return a pointer to the start of the newly allocated memory, like
// sbrk()
void *expandMemory(size_t size) {
  block_t *newBlock = sbrk(size + sizeof(block_t));
  // if sbrk() fails, return NULL (perhaps no sufficient memory on heap)
  if (newBlock == (void *)-1) {
    return NULL;
  }

  newBlock->size = size;
  newBlock->prev = NULL;
  newBlock->next = NULL;
  return newBlock;
}

// Given a free block region of size larger than we need, makr the first `size`
// byte to be occupied memory region, and update the free block region linked
// list with shrinked block
void updateBlock_lock(block_t *cur, size_t size) {
  block_t *prev_temp = cur->prev;
  block_t *next_temp = cur->next;
  // if the fit is exactly the size we want, or the remaining region is not
  // enough for a metadata header, then we delete that specific free block
  // region
  // <==> node deletion in doubly linked list
  if (cur->size <= size + sizeof(block_t)) {
    // segment_free_space_size -= cur->size;
    if (cur == head && head == tail) {
      head = NULL;
      tail = NULL;
    } else if (cur == head) {
      head = next_temp;
      next_temp->prev = NULL;
    } else if (cur == tail) {
      tail = prev_temp;
      prev_temp->next = NULL;
    } else {
      prev_temp->next = next_temp;
      next_temp->prev = prev_temp;
    }
  }
  // else the fit has larger space and has spare bytes after allocating the
  // memory, we split the free block region into two parts, and mark the
  // occupied region
  else {
    block_t *updatedCur = (block_t *)((char *)cur + size + sizeof(block_t));
    updatedCur->next = next_temp;
    updatedCur->prev = prev_temp;
    updatedCur->size = cur->size - size - sizeof(block_t);
    cur->size = size;
    if (cur == head && head == tail) {
      head = updatedCur;
      tail = updatedCur;
    } else if (cur == head) {
      head = updatedCur;
      next_temp->prev = updatedCur;
    } else if (cur == tail) {
      tail = updatedCur;
      prev_temp->next = updatedCur;
    } else {
      prev_temp->next = updatedCur;
      next_temp->prev = updatedCur;
    }
  }
  cur->prev = NULL;
  cur->next = NULL;
}

// Add a free block region into the doubly linked list
void addBlock_lock(block_t *ptr) {
  // linked list initialization
  if (head == NULL && tail == NULL) {
    head = ptr;
    tail = ptr;
    if (ptr != NULL) {
      ptr->prev = NULL;
    }
    if (ptr != NULL) {
      ptr->next = NULL;
    }
    return;
  }
  // if the new free region is before the head of linked list
  if (ptr < head) {
    block_t *head_temp = head;
    head = ptr;
    if (ptr != NULL) {
      ptr->prev = NULL;
    }
    if (ptr != NULL) {
      ptr->next = head_temp;
    }
    head_temp->prev = ptr;
  }
  // if the new free region is behind the tail of linked list
  else if (ptr > tail) {
    block_t *tail_temp = tail;
    tail = ptr;
    ptr->next = NULL;
    ptr->prev = tail_temp;
    tail_temp->next = ptr;
  }
  // if the new free region is for sure somewhere in between the linked list
  // <==> insert a node inside a doubly linked list
  else {
    // find the former one of two consecutive free block regions in list, such
    // that we are to insert a new free block region in between
    block_t *next_temp = head->next;
    block_t *prev_temp = head;
    while (next_temp != tail && next_temp < ptr && next_temp < tail &&
           next_temp > head) {
      prev_temp = next_temp;
      if (next_temp != NULL) {
        next_temp = next_temp->next;
      }
    }
    prev_temp->next = ptr;
    ptr->prev = prev_temp;
    next_temp->prev = ptr;
    ptr->next = next_temp;
  }
}

// Merge two free block region in list if adjacent to each other
void mergeBlocks_lock(block_t *lhs, block_t *rhs) {
  // do nothing if the two blocks are not adjacent
  if (((char *)lhs + sizeof(block_t) + lhs->size) != ((char *)rhs)) {
    return;
  }
  if (rhs == tail) {
    tail = lhs;
  }
  lhs->next = rhs->next;
  if (rhs->next != NULL) {
    rhs->next->prev = lhs;
  }
  // printf("rhs.size %lu\n", rhs->size);
  lhs->size += rhs->size + sizeof(block_t);
  // printf("lhs.size %lu\n", lhs->size);
  rhs->next = NULL;
  rhs->prev = NULL;
}

// Free helper function for both ff_free and bf_free
void freeHelper_lock(block_t *ptr) {
  if (ptr == NULL) {
    return;
  }
  block_t *ptrInList = (block_t *)((char *)ptr - sizeof(block_t));
  // add a new free block region into the linked list
  addBlock_lock(ptrInList);
  if (ptrInList != NULL && ptrInList->next != NULL) {
    mergeBlocks_lock(ptrInList, ptrInList->next);
  }
  if (ptrInList != NULL && ptrInList->prev != NULL) {
    mergeBlocks_lock(ptrInList->prev, ptrInList);
  }
}

// Best Fit malloc
void *ts_malloc_lock(size_t size) {
  // malloc man page: if size == 0, return NULL
  if (size == 0) {
    return NULL;
  }
  pthread_mutex_lock(&lock);
  // traverse the whole free block region linked list to find the best fit
  block_t *cur = head;
  block_t *best_fit = NULL;
  while (cur != NULL) {
    if (cur->size == size) {
      updateBlock_lock(cur, size);
      pthread_mutex_unlock(&lock);
      return (char *)cur + sizeof(block_t);
    }
    if (cur->size > size) {
      if (best_fit != NULL) {
        if (cur->size < best_fit->size) {
          best_fit = cur;
        }
      } else {
        best_fit = cur;
      }
    }
    cur = cur->next;
  }

  // if there is no such free block region, call sbrk() to create one, and
  // return it
  if (best_fit == NULL) {
    void *newBlock = NULL;
    newBlock = (char *)expandMemory(size) + sizeof(block_t);
    pthread_mutex_unlock(&lock);
    return newBlock;
  }
  // else if we find the fit, update the free block region linked list
  else {
    updateBlock_lock(best_fit, size);
    pthread_mutex_unlock(&lock);
    return (char *)best_fit + sizeof(block_t);
  }
  // pthread_mutex_unlock(&lock);
}

// Best Fit free
void ts_free_lock(void *ptr) {
  pthread_mutex_lock(&lock);
  freeHelper_lock(ptr);
  pthread_mutex_unlock(&lock);
}

// nolock verison
// Thread Safe malloc/free: non-locking version
// Given a free block region of size larger than we need, makr the first `size`
// byte to be occupied memory region, and update the free block region linked
// list with shrinked block
void updateBlock_nolock(block_t *cur, size_t size) {
  block_t *prev_temp = cur->prev;
  block_t *next_temp = cur->next;
  // if the fit is exactly the size we want, or the remaining region is not
  // enough for a metadata header, then we delete that specific free block
  // region
  // <==> node deletion in doubly linked list
  if (cur->size <= size + sizeof(block_t)) {
    // segment_free_space_size -= cur->size;
    if (cur == nolockhead && nolockhead == nolocktail) {
      nolockhead = NULL;
      nolocktail = NULL;
    } else if (cur == nolockhead) {
      nolockhead = next_temp;
      next_temp->prev = NULL;
    } else if (cur == nolocktail) {
      nolocktail = prev_temp;
      prev_temp->next = NULL;
    } else {
      prev_temp->next = next_temp;
      next_temp->prev = prev_temp;
    }
  }
  // else the fit has larger space and has spare bytes after allocating the
  // memory, we split the free block region into two parts, and mark the
  // occupied region
  else {
    block_t *updatedCur = (block_t *)((char *)cur + size + sizeof(block_t));
    updatedCur->next = next_temp;
    updatedCur->prev = prev_temp;
    updatedCur->size = cur->size - size - sizeof(block_t);
    cur->size = size;
    if (cur == nolockhead && nolockhead == nolocktail) {
      nolockhead = updatedCur;
      nolocktail = updatedCur;
    } else if (cur == nolockhead) {
      nolockhead = updatedCur;
      next_temp->prev = updatedCur;
    } else if (cur == nolocktail) {
      nolocktail = updatedCur;
      prev_temp->next = updatedCur;
    } else {
      prev_temp->next = updatedCur;
      next_temp->prev = updatedCur;
    }
  }
  cur->prev = NULL;
  cur->next = NULL;
}

// Add a free block region into the doubly linked list
void addBlock_nolock(block_t *ptr) {
  // linked list initialization
  if (nolockhead == NULL && nolocktail == NULL) {
    nolockhead = ptr;
    nolocktail = ptr;
    if (ptr != NULL) {
      ptr->prev = NULL;
    }
    if (ptr != NULL) {
      ptr->next = NULL;
    }
    return;
  }
  // if the new free region is before the head of linked list
  if (ptr < nolockhead) {
    block_t *head_temp = nolockhead;
    nolockhead = ptr;
    if (ptr != NULL) {
      ptr->prev = NULL;
    }
    if (ptr != NULL) {
      ptr->next = head_temp;
    }
    head_temp->prev = ptr;
  }
  // if the new free region is behind the tail of linked list
  else if (ptr > nolocktail) {
    block_t *tail_temp = nolocktail;
    nolocktail = ptr;
    ptr->next = NULL;
    ptr->prev = tail_temp;
    tail_temp->next = ptr;
  }
  // if the new free region is for sure somewhere in between the linked list
  // <==> insert a node inside a doubly linked list
  else {
    // find the former one of two consecutive free block regions in list, such
    // that we are to insert a new free block region in between
    block_t *next_temp = nolockhead->next;
    block_t *prev_temp = nolockhead;
    while (next_temp != nolocktail && next_temp < ptr &&
           next_temp < nolocktail && next_temp > nolockhead) {
      prev_temp = next_temp;
      if (next_temp != NULL) {
        next_temp = next_temp->next;
      }
    }
    prev_temp->next = ptr;
    ptr->prev = prev_temp;
    next_temp->prev = ptr;
    ptr->next = next_temp;
  }
}

// Merge two free block region in list if adjacent to each other
void mergeBlocks_nolock(block_t *lhs, block_t *rhs) {
  // do nothing if the two blocks are not adjacent
  if (((char *)lhs + sizeof(block_t) + lhs->size) != ((char *)rhs)) {
    return;
  }
  if (rhs == nolocktail) {
    nolocktail = lhs;
  }
  lhs->next = rhs->next;
  if (rhs->next != NULL) {
    rhs->next->prev = lhs;
  }
  // printf("rhs.size %lu\n", rhs->size);
  lhs->size += rhs->size + sizeof(block_t);
  // printf("lhs.size %lu\n", lhs->size);
  rhs->next = NULL;
  rhs->prev = NULL;
}

// Free helper function for both ff_free and bf_free
void freeHelper_nolock(block_t *ptr) {
  if (ptr == NULL) {
    return;
  }
  block_t *ptrInList = (block_t *)((char *)ptr - sizeof(block_t));
  // add a new free block region into the linked list
  addBlock_nolock(ptrInList);
  if (ptrInList != NULL && ptrInList->next != NULL) {
    mergeBlocks_nolock(ptrInList, ptrInList->next);
  }
  if (ptrInList != NULL && ptrInList->prev != NULL) {
    mergeBlocks_nolock(ptrInList->prev, ptrInList);
  }
}

// Best Fit malloc
void *ts_malloc_nolock(size_t size) {
  // malloc man page: if size == 0, return NULL
  if (size == 0) {
    return NULL;
  }
  // traverse the whole free block region linked list to find the best fit
  block_t *cur = nolockhead;
  block_t *best_fit = NULL;
  while (cur != NULL) {
    if (cur->size == size) {
      updateBlock_nolock(cur, size);
      return (char *)cur + sizeof(block_t);
    }
    if (cur->size > size) {
      if (best_fit != NULL) {
        if (cur->size < best_fit->size) {
          best_fit = cur;
        }
      } else {
        best_fit = cur;
      }
    }
    cur = cur->next;
  }

  // if there is no such free block region, call sbrk() to create one, and
  // return it
  if (best_fit == NULL) {
    pthread_mutex_lock(&lock);
    void *newBlock = NULL;
    newBlock = (char *)expandMemory(size) + sizeof(block_t);
    pthread_mutex_unlock(&lock);
    return newBlock;
  }
  // else if we find the fit, update the free block region linked list
  else {
    updateBlock_nolock(best_fit, size);
    return (char *)best_fit + sizeof(block_t);
  }
}

// Best Fit free
void ts_free_nolock(void *ptr) { freeHelper_nolock(ptr); }
