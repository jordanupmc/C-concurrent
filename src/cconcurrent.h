#ifndef CCONCURRENT_H_INCLUDED
#define CCONCURRENT_H_INCLUDED
#define MAX_TASK_ARRAY 256
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

/*A basic Future/Promise struct */
typedef struct future{
  pthread_mutex_t mutex; /*mutex for internal usage*/
  pthread_cond_t cond;   /*condition for internal usage*/
  void * val; /*the value available after the computation*/
  char done; /*0 if the value is not available*/
  char cancel; /*0 if the future is not in cancel mode*/
} future;

/*allocate a future, use malloc ->  must use destroy_future*/
extern future* create_future();

/*free a future allocated with create_future*/
extern void destroy_future(future* f);

/*return 0 if the result is not available*/
extern char isDone(future * f);

/*return 0 if the future is not in cancel mode*/
extern char isCancel(future * f);

/*cancel the task associate to the future*/
/*TODO: add pthread_t to struct future for achieve a real cancel*/
extern void cancel_future(future * f);

/*put the result in the future*/
extern void complete_future(future * f, void * val);

/*get the result, wait until the result is available*/
/*func arguments is call when the result is available with the result as argument*/
/*if you don't want this behavior set NULL*/
extern void* get_value_future(future *f, void* (*func)(void*) );


/*PTHREAD_POOL*/
/*Inspired by the Java API Executor fixed thread pool*/
/*To simplify the code the queue of task/task arguments/future use a array with a size of MAX_TASK_ARRAY*/

typedef struct pthread_pool{
  pthread_t* pool;
  int pool_size;
  pthread_mutex_t mutex; /*mutex for internal usage*/
  pthread_cond_t cond;   /*condition for internal usage*/
  void* (*task[MAX_TASK_ARRAY])(void *); /*array of tasks to execute in FIFO order*/
  void* taskArg[MAX_TASK_ARRAY]; /*array of arguments associate to the task to execute in FIFO order*/
  future* completable[MAX_TASK_ARRAY]; /*array of future* associate to a task */
  int nb_task; /*the number of waiting task*/
} pthread_pool;


/*initialise a pool with capacity threads*/
extern pthread_pool* create_pthread_pool(int capacity);

/*Submit a task to a pthread_pool*/
/*If the number of task exceed MAX_TASK_ARRAY the task is ignore*/
/*You must keep in a variable the future* return by this function and don't forget to call destroy_future
for free the future*/
extern future* submit_pthread_pool(pthread_pool* pool, void* (*func)(void*), void* arg);

/*Free a pthread_pool allocated with create_pthread_pool function*/
/*Stop all the threads in the pool. And wait with a pthread_join that all threads really terminate*/
extern void shutdown_pthread_pool(pthread_pool* pool);
#endif
