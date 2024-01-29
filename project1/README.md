# My malloc & free

### 1. Design

When calling `sbrk` to allocate new heap memory to the program, I abstract the allocated part to be a block region of bytes. Each block contains its metadata (used by the block structure) and the storage memory bytes. 

I use a doubly linked list to store only the free block regions. My block structure contains the following fields:

- `size_t size`: number of bytes this block contains (not including metadata bytes)
- `block_t * prev`: pointer to the previous free block region
- `block_t * next`: pointer to the next free block region

#### 1.1 Implementation of `malloc`

I think the only difference between the implementation of`ff_malloc` and `bf_malloc` is the way of linked list traversal, so I implemented these `malloc` functions with the same helper functions and different list traversal method. 

Specifically, the helper functions are:

- `void * expandMemory(size_t size)`
  - When no free block region is larger than the required size, call sbrk() to create one, and return a pointer to the start of the newly allocated memory, like sbrk()
- `updateBlock(block_t * cur, size_t size)`
  - Given a free block region of size larger than we need, mark the first `size` byte to be occupied memory region, and update the free block region linked list with shrinked block.
  - We distinct two cases in this function.
    - If the block's available free size is only slightly larger than what we need (a.k.a the remaining free bytes are not enough for metadata header bytes), then we throw away those unused bytes, and delete the block entirely from the linked list.
    - If the block is much larger (a.k.a the remaining bytes can make a new free block), then we slice the block into two parts, use the first part to store data, and put back the shrinked free block region into list

After finding the suitable free block region to allocate memory, I use these two helper functions to actually implement malloc. If no suitable free block is found, then I create one and use it with `expandMemory`. Else, I update (delete or slice) the chosen free block region with `updateBlock`. 

The different algorithms for *First Fit* and *Best Fit* to find the suitable block are:

- First Fit: traverse the free block region linked list **up to the point** we find the first block that is large enough to use the memory.
- Best Fit: traverse the **whole** free block region linked list to find the suitable block with the smallest size to enhance data usage efficiency.

#### 1.2 Implementation of `free`

I think that `free` function would do the same thing no matter what fit policy it is, so I implemented a `freeHelper` function with:

- `void addBlock(block_t * ptr)`
  - Add a free block region into the doubly linked list.
- `void mergeBlocks(block_t * lhs, block_t * rhs)`
  - Merge two free block region in list if adjacent to each other.

Specifically, every time I free a block with `freeHelper`, I insert the new free block region into the linked list using `addBlock`, do two merges (both left and right side) with `mergeBlock` upon insertion.

### 2. Performance

#### 2.1 Performance result presentation

I run the tests with `-O3` optimization flag on a 2-processor, 4 GB base memory, ubuntu20 virtual machine on Duke VM server. 

To make data more intuitive, here's a table for comparison of results:

|                  | FF run time (s) | BF run time (s) | FF fragmentation | BF fragmentation |
| ---------------- | --------------- | --------------- | ---------------- | ---------------- |
| small rand alloc | 11.40           | 4.77            | 0.074            | 0.027            |
| equal size alloc | 15.09           | 15.14           | 0.450            | 0.450            |
| large rand alloc | 37.52           | 50.98           | 0.093            | 0.041            |

#### 2.2 Result analysis

##### 2.2.1 Analysis of execution time

With `large_range_rand_allocs`, the Best Fit policy executes with more time than First Fit, as is expected. This is because First Fit policy would just naively take the first available free block region no matter how big it is, yet Best Fit would sacrifice more time to traverse the whole list to find the best fit to improve storage efficiency.

With `equal_size_allocs`, Best Fit and First Fit take about the same time, because the program uses the same number of bytes (128) in all its malloc calls. This is reasonable because I designed my Best Fit to end as soon as it found the block of the exact same size of requirement without having to traverse the whole list, which in this specific case acts the same as First Fit policy.

With `small_range_rand_allocs`, Best Fit runs surprisingly faster than First Fit, which is not very reasonable. My expectation is that Best Fit should always run slower than First Fit, because it has more blocks to traverse and check. After looking things up, I assume that it's because my Best Fit policy somehow gets automatically optimized by the compiler, which used the real library function `malloc` behind the scene. 

##### 2.2.2 Analysis of fragmentation

Fragmentation is calculated by the amout of unallocated data segment space divided by total data segment space. Except for the special case of `equal_size_allocs` where Best Fit and First Fit reasonably makes no difference, in all other cases, Best Fit has much lower fragmentation than First Fit. This is to my expectation because Best Fit policy chooses to sacrifice execution time in exchange for a better memory usage efficiency, achieved by always `malloc`-ing at the smallest suitable free block region. Moreover, Best Fit policy used less data segment size than First Fit, because it efficiently used its freed memory and avoided naively `sbrk`-ing too much new heap memory.

### 3. Things I learned along the project

- The most important thing I learned from this project is to use abundant `printf`s to debug. Sometimes, with a lot of iterations, even GDB cannot really point out where the true problem lies. For example, I got a lot of segmentation faults (core dumped) with GDB flagging my slice block logic in `updateBlock` to be problematic, while the true problem that influenced this to dysfunction is because my `mergeBlock` is not good. I did wrong merging with free block regions, leading to unexpected behaviors in block updates. I could not have spotted this problem without printing a lot of information and check them carefully and patiently one after another.
- Always check `NULL` input corner cases. Strange `NULL`s that seem to have come out of nowhere have been the main cause of my segmentation faults.
- Debugging can be emotional, and it's totally OK. When feeling down, take a deep breath, go out and stretch a little, talk to friends, take a nap. Debugging needs a sharp brain and a lot of patience.
- When doing pointer arithmetics, remember to cast to the same reasonable type. For example, instead of adding `sizeof(block_t)` (number of bytes of metadata) directly to a pointer of type `block_t *`, I need to first cast the pointer into `char *` to make sure I'm really adding by Bytes.
- I first tried using `void * start_address = sbrk(0);` as a global variable to mark the beginning of the program, but the IDE complains at `sbrk` that "initializer element is not a compile-time constant". After looking it up, I realized that C global variables must be initialized with constant expressions, so the right way to achieve my goal is to set `start_address` as `NULL` at compile time, then set it to the correct value at runtime by calling `sbrk` at the beginning of main. However, I then realized that I cannot control what is in the main function (as this will be done by the auto-grader), so I proposed an easier and more intuitive way to keep track of the whole program heap memory.





```
#include "my_malloc.h"

#include <stdint.h>  // intptr_t
#include <stdio.h>   // printf
#include <stdlib.h>  // EXIT_SUCCESS
#include <unistd.h>  // sbrk

// set global head and tail pointers of doubly linked list
block_t * head = NULL;
block_t * tail = NULL;

// When no free block region is larger than the required size, call sbrk() to create one
// return a pointer to the start of the newly allocated memory, like sbrk()
void * expandMemory(size_t size) {
  block_t * newBlock = sbrk(size + sizeof(block_t));
  // if sbrk() fails, return NULL (perhaps no sufficient memory on heap)
  if (newBlock == (void *)-1) {
    return NULL;
  }
  // else, sbrk() successfully allocated a block memory
  segment_size += (size + sizeof(block_t));
  newBlock->size = size;
  newBlock->prev = NULL;
  newBlock->next = NULL;
  return newBlock;
}

// Given a free block region of size larger than we need, makr the first `size` byte to be occupied memory region, and update the free block region linked list with shrinked block
void updateBlock(block_t * cur, size_t size) {
  //segment_free_space_size -= size;
  printf("cr: %p\n", cur);
  block_t * prev_temp = cur->prev;
  printf("pt: %p\n", prev_temp);
  block_t * next_temp = cur->next;
  printf("nt: %p\n", next_temp);
  // if the fit is exactly the size we want, then we delete that specific free block region
  // <==> node deletion in doubly linked list
  if (cur->size <= size + sizeof(block_t)) {
    segment_free_space_size -= cur->size;
    if (cur == head && head == tail) {
      head = NULL;
      tail = NULL;
    }
    else if (cur == head) {
      head = next_temp;
      next_temp->prev = NULL;
    }
    else if (cur == tail) {
      tail = prev_temp;
      prev_temp->next = NULL;
    }
    else {
      printf("nt1: %p\n", next_temp);
      printf("pt1: %p\n", prev_temp);
      prev_temp->next = next_temp;
      printf("pt2: %p\n", prev_temp);
      printf("nt2: %p\n", next_temp);
      next_temp->prev = prev_temp;
      printf("pt3: %p\n", prev_temp);
      printf("nt3: %p\n", next_temp);
    }
  }
  // else the fit has larger space and has spare bytes after allocating the memory, we split the free block region into two parts, and mark the occupied region
  else {
    //cur = cur + sizeof(block_t) + size - sizeof(block_t);
    block_t * updatedCur = (block_t *)((char *)cur + size + sizeof(block_t));
    // updatedCur->next = NULL;
    // updatedCur->prev = NULL;
    updatedCur->next = next_temp;
    updatedCur->prev = prev_temp;
    updatedCur->size = cur->size - size - sizeof(block_t);
    segment_free_space_size -= (size + sizeof(block_t));
    if (cur == head && head == tail) {
      head = updatedCur;
      tail = updatedCur;
    }
    else if (cur == head) {
      head = updatedCur;
      next_temp->prev = updatedCur;
    }
    else if (cur == tail) {
      tail = updatedCur;
      prev_temp->next = updatedCur;
    }
    else {
      prev_temp->next = updatedCur;
      next_temp->prev = updatedCur;
    }
  }
  cur->prev = NULL;
  cur->next = NULL;
}

// First Fit malloc
void * ff_malloc(size_t size) {
  // malloc man page: if size == 0, return NULL
  if (size == 0) {
    return NULL;
  }
  // traverse the free block region linked list...
  block_t * cur = head;
  // ...until find the first block that is large enough
  while (cur != NULL && cur->size < size) {
    cur = cur->next;
  }
  // if there is no such free block region, call sbrk() to create one, and return it
  if (cur == NULL) {
    return (char *)expandMemory(size) + sizeof(block_t);
  }
  // else if we find the fit, update the free block region linked list
  else {
    updateBlock(cur, size);
    return (char *)cur + sizeof(block_t);
  }
}

// Add a free block region into the doubly linked list
void addBlock(block_t * ptr) {
  // linked list initialization
  if (head == NULL && tail == NULL) {
    head = ptr;
    tail = ptr;
    ptr->prev = NULL;
    ptr->next = NULL;
    return;
  }
  // if the new free region is before the head of linked list
  if (ptr < head) {
    block_t * head_temp = head;
    head = ptr;
    ptr->prev = NULL;
    ptr->next = head_temp;
    head_temp->prev = ptr;
  }
  // if the new free region is behind the tail of linked list
  else if (ptr > tail) {
    block_t * tail_temp = tail;
    tail = ptr;
    ptr->next = NULL;
    ptr->prev = tail_temp;
    tail_temp->next = ptr;
  }
  // if the new free region is for sure somewhere in between the linked list
  // <==> insert a node inside a doubly linked list
  else {
    // find the former one of two consecutive free block regions in list, such that we are to insert a new free block region in between
    block_t * next_temp = head->next;
    block_t * prev_temp = head;
    printf("nt154 begin: %p\n", next_temp);
    printf("pt154 begin: %p\n", prev_temp);
    printf("ntn154 begin: %p\n", next_temp->next);
    while (next_temp != tail && next_temp < ptr && next_temp < tail && next_temp > head) {
      prev_temp = next_temp;
      next_temp = next_temp->next;
      printf("nt154 w: %p\n", next_temp);
      printf("pt154 w: %p\n", prev_temp);
      printf("ntn154 w: %p\n", next_temp->next);
    }
    prev_temp->next = ptr;
    ptr->prev = prev_temp;
    next_temp->prev = ptr;
    ptr->next = next_temp;
  }
  segment_free_space_size += ptr->size;
}

// Merge two free block region in list if adjacent to each other
void mergeBlocks(block_t * lhs, block_t * rhs) {
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
  lhs->size += rhs->size + sizeof(block_t);
  rhs->next = NULL;
  rhs->prev = NULL;
}

// Free helper function for both ff_free and bf_free
void freeHelper(block_t * ptr) {
  if (ptr == NULL) {
    return;
  }
  block_t * ptrInList = (block_t *)((char *)ptr - sizeof(block_t));
  // ptrInList->size = ptr->size;
  // add a new free block region into the linked list
  addBlock(ptrInList);
  // merge possible free block regions
  if (ptrInList->next != NULL) {
    mergeBlocks(ptrInList, ptrInList->next);
  }
  if (ptrInList->prev != NULL) {
    mergeBlocks(ptrInList->prev, ptrInList);
  }
}

// First Fit free
void ff_free(void * ptr) {
  freeHelper(ptr);
}

// Best Fit malloc
void * bf_malloc(size_t size) {
  // malloc man page: if size == 0, return NULL
  if (size == 0) {
    return NULL;
  }
  // traverse the whole free block region linked list to find the best fit
  block_t * cur = head;
  block_t * best_fit = NULL;
  while (cur != NULL) {
    if (cur->size >= size) {
      if (best_fit != NULL) {
        if (cur->size < best_fit->size) {
          best_fit = cur;
        }
      }
      else {
        best_fit = cur;
      }
    }
    cur = cur->next;
  }
  // if there is no such free block region, call sbrk() to create one, and return it
  if (best_fit == NULL) {
    return (char *)expandMemory(size) + sizeof(block_t);
  }
  // else if we find the fit, update the free block region linked list
  else {
    updateBlock(best_fit, size);
    return (char *)best_fit + sizeof(block_t);
  }
}

// Best Fit free
void bf_free(void * ptr) {
  freeHelper(ptr);
}

// Get data segment size
unsigned long get_data_segment_size() {
  return segment_size;
}

// Get data segment free space size
unsigned long get_data_segment_free_space_size() {
  return segment_free_space_size;
}




```

