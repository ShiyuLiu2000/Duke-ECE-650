#include <stdlib.h>
#include <stdio.h>
#include "my_malloc.h"

#ifdef FF
#define MALLOC(sz) ff_malloc(sz)
#define FREE(p)    ff_free(p)
#endif
#ifdef BF
#define MALLOC(sz) bf_malloc(sz)
#define FREE(p)    bf_free(p)
#endif

void printList() {
  // printf("Start printing list\n");
    block_t *current = head;
    if (current == NULL) {
      printf("No node here\n");
    }else {
      printf("Start printing list\n");
      printf("--------------------\n");
    }
    int i = 0;
    while (current != NULL) {
        printf("Node %d: Address: %p,  Size: %lu, Prev: %p, Next: %p\n",
                i,
               (void*)current, 
               current->size, 
               (void*)current->prev, 
               (void*)current->next);

        current = current->next;  // Move to the next node
        i++;
    }
    printf("\n");
    if (head != NULL) {
      printf("Head Address: %p,  Size: %lu, Prev: %p, Next: %p\n",
                (void*)head, 
                head->size, 
                (void*)head->prev, 
                (void*)head->next);
    }

    printf("--------------------\n");
    if (tail != NULL) {
      printf("Tail Address: %p,  Size: %lu, Prev: %p, Next: %p\n",
                (void*)tail, 
                tail->size, 
                (void*)tail->prev, 
                (void*)tail->next);
    }

    printf("--------------------\n");
    printf("\n");
}


int main(int argc, char *argv[])
{
  const unsigned NUM_ITEMS = 10;
  int i;
  int size;
  int sum = 0;
  int expected_sum = 0;
  int *array[NUM_ITEMS];

  printf("find size 4\n");
  size = 4;
  expected_sum += size * size;
  array[0] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[0][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[0][i];
  } //for i
  printList();

  printf("find size 16\n");
  size = 16;
  expected_sum += size * size;
  array[1] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[1][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[1][i];
  } //for i
  printList();


  printf("find size 8\n");
  size = 8;
  expected_sum += size * size;
  array[2] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[2][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[2][i];
  } //for i
  printList();

  printf("find size 32\n");
  size = 32;
  expected_sum += size * size;
  array[3] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[3][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[3][i];
  } //for i
  printList();

  printf("free arr0\n");
  FREE(array[0]);
  printList();
  // printList();
  // printList();
  printf("free arr2\n");
  FREE(array[2]);
  printList();
  // printList();
  // printList();


  printf("find size 7\n");
  size = 7;
  expected_sum += size * size;
  array[4] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[4][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[4][i];
  } //for i
  printList();

  printf("find size 1024\n");
  size = 256;
  expected_sum += size * size;
  array[5] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[5][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[5][i];
  } //for i
  printList();

  printf("free arr5\n");
  FREE(array[5]);
  printList();
  // printList();
  // printList();
  printf("free arr1\n");
  FREE(array[1]);
  printList();
  printf("free arr3\n");
  FREE(array[3]);
  printList();

  printf("find size 23\n");
  size = 23;
  expected_sum += size * size;
  array[6] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[6][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[6][i];
  } //for i
  printList();

  printf("find size 4\n");
  size = 4;
  expected_sum += size * size;
  array[7] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[7][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[7][i];
  } //for i
  printList();

  printf("free arr4\n");
  FREE(array[4]);
  printList();

  printf("find size 10\n");
  size = 10;
  expected_sum += size * size;
  array[8] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[8][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[8][i];
  } //for i
  printList();

  printf("find size 32\n");
  size = 32;
  expected_sum += size * size;
  array[9] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[9][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[9][i];
  } //for i
  printList();

  printf("free arr6\n");
  FREE(array[6]);
  printList();
  printf("free arr7\n");
  FREE(array[7]);
  printList();
  printf("free arr8\n");
  FREE(array[8]);
  printList();
  printf("free arr9\n");
  FREE(array[9]);
  printList();

  if (sum == expected_sum) {
    printf("Calculated expected value of %d\n", sum);
    printf("Test passed\n");
  } else {
    printf("Expected sum=%d but calculated %d\n", expected_sum, sum);
    printf("Test failed\n");
  } //else

  return 0;
}