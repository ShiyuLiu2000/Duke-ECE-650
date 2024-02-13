# My thread-safe malloc & free
## 1. Design

The general idea is that I use an explicit lock for the locking version, and used thread-local storage for no-lock version. 

Specifically, since the two versions use different global head/tail, I split all the malloc/free helper functions in two.

```c
// lock version
void updateBlock_lock(block_t *cur, size_t size);
void addBlock_lock(block_t *ptr);
void mergeBlocks_lock(block_t *lhs, block_t *rhs);

// no-lock version
void updateBlock_nolock(block_t *cur, size_t size);
void addBlock_nolock(block_t *ptr);
void mergeBlocks_nolock(block_t *lhs, block_t *rhs);
```

### 1.1 lock version

I used an explicit mutex lock in the lock version. The implementation abstraction is:

```c
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_lock(&lock);
// malloc & free function body here
pthread_mutex_unlock(&lock);
```

Basically I made all the function body part of malloc and free as critical code section. I use a mutex lock at the beginning of malloc and free, and unlock before every return inside the function body.

It is worth noting that we should not wrap a second lock around `sbrk` in lock version, else it would end up with dead-lock and the execution never ends.

### 1.2 no-lock version

I used thread-local storage for the no-lock version. The idea is that every thread uses its own linked list, so I created thread-specific new head and new tail as:

```c
__thread block_t *nolockhead = NULL;
__thread block_t *nolocktail = NULL;
```

Then I copy and paste all the malloc/free helper functions from project1, modified the head and tail used inside the functions, and wrapped a lock around `sbrk` function since it's not thread-safe.

## 2. Performance Analysis

I run the tests with -ggdb3 debug flag on a 2-processor, 4 GB base memory, ubuntu20 virtual machine on Duke VM server.

### 2.1 Results 

To be more accurate, I conducted the measurements 7 times for lock and no-lock respectively. The results are in images below.
<img width="909" alt="lock" src="https://github.com/ShiyuLiu2000/Duke-ECE-650/assets/131769951/743246c7-f0e1-4385-8d58-d46bf15a9624">
The above image is the results for lock version.
<img width="913" alt="no lock" src="https://github.com/ShiyuLiu2000/Duke-ECE-650/assets/131769951/e73732e2-83d5-46e3-bf6d-813125e085ab">
The above image is the results for no-lock version.

### 2.2 Analysis

#### 2.2.1 Execution time

I calculate the average execution time of lock version to be 0.12957s, while the average execution time of no-lock version is 0.09686s. Using lock version as a baseline, the no-lock version shows a 25.24% improvement in execution time. 

I account this improvement to the removal of overhead of mutex locks around the whole function body of malloc and free. Since the no-lock version has thread-local linked list storage, there is no need to take care of the global list race conditions, thus the paralllelism can be more flexible, resulting in a shorter execution time.

#### 2.2.2 Data segment size

I calculate the average data segment size of lock version to be 43,637,274 Bytes, while that of no-lock version is 42,901,507 Bytes. They are roughly the same, because they are both using best-fit allocation policy. The slight decrease in no-lock version might be due to the fact that memory is more fragmented and used in better efficiency with separate thread-local linked lists.
