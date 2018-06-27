#include "cconcurrent.h"

/*FUTURE*/
future * create_future(){
  future * f = malloc(sizeof(struct future));
  f->val=NULL;
  f->done=0;
  f->cancel=0;
  pthread_mutex_init( &(f->mutex), NULL);
  pthread_cond_init( &(f->cond), NULL);
  return f;
}

void destroy_future(future * f){
  if(!f)
    return;
  pthread_mutex_destroy( &(f->mutex));
  pthread_cond_destroy(&(f->cond));
  free(f);
}

char isDone(future * f){
  return f->done;
}

char isCancel(future * f){
  return f->cancel;
}

//TODO: add pthread_t to struct future for achieve a real cancel
void cancel_future(future * f){
  pthread_mutex_lock( &(f->mutex));
  f->cancel=1;
  pthread_mutex_unlock(&(f->mutex));
}

void complete_future(future * f, void * val){
  if(!f)
    return;
  pthread_mutex_lock(&(f->mutex));
 
  if( !(f->done) && !(f->cancel)){ 
    f->val=val;
    f->done=1;
    pthread_cond_broadcast( &(f->cond) );
  }

  pthread_mutex_unlock(&(f->mutex));
  
}

void* get_value_future(future *f, void* (*func)(void*)){
  if(!f)
    return NULL;
  pthread_mutex_lock(&(f->mutex));
  
  if(!(f->done))
    pthread_cond_wait( &(f->cond), &(f->mutex) );
  
  pthread_mutex_unlock(&(f->mutex));
  
  if(func != NULL)
    func(f->val);
 
  return f->val;
  
}

//////POOL///////

static void* unl(void * l){
  pthread_mutex_unlock((pthread_mutex_t*)l);
  return NULL;
}
static void* pthread_pool_loop(void* arg){
  pthread_pool* pool = (pthread_pool*)arg;

  while(1){
    pthread_cleanup_push(unl, &(pool->mutex));
    pthread_mutex_lock(&(pool->mutex));
    if(!(pool->nb_task))
      pthread_cond_wait(&(pool->cond), &(pool->mutex));

    void* (*current)(void*) = pool->task[0];
    void* currentArg = (void*)(pool->taskArg[0]);
    future* currentFuture = pool->completable[0];
    int i;
    
    for(i=0; i< MAX_TASK_ARRAY-1 ; i++){
      pool->task[i] = pool->task[i+1];
      pool->taskArg[i] = pool->taskArg[i+1];
      pool->completable[i] = pool->completable[i+1];
    }
    pool->task[MAX_TASK_ARRAY-1] = NULL;
    pool->taskArg[MAX_TASK_ARRAY-1] = NULL;
    pool->completable[MAX_TASK_ARRAY-1] = NULL;
    
    (pool->nb_task)--;
    pthread_mutex_unlock(&(pool->mutex)); 
    complete_future(currentFuture, current(currentArg));
    pthread_cleanup_pop(0);
    
    //complete_future(currentFuture, current(currentArg));
    pthread_testcancel();
  }
  return NULL;
}

pthread_pool* create_pthread_pool(int capacity){
  if(capacity<=0)
    return NULL;
  
  pthread_pool * p = malloc(sizeof(pthread_pool));
  p->pool = malloc(sizeof(pthread_t) * capacity );
  if(!(p->pool))
    return NULL;
  
  p->nb_task=0;
  p->pool_size=capacity;
  pthread_mutex_init(&(p->mutex), NULL);
  pthread_cond_init(&(p->cond), NULL);
  int i;
  for(i=0; i<MAX_TASK_ARRAY; i++){
    p->task[i] = NULL;
    p->taskArg[i] = NULL;
    p->completable[i] = NULL;
  }
  
  for(i=0; i<capacity; i++)
    pthread_create((p->pool)+i, NULL, pthread_pool_loop, p);
  
  return p;
}

future* submit_pthread_pool(pthread_pool* pool, void* (*func)(void*), void* arg){
  if(!pool || !func)
    return NULL;
  future* res = NULL;
  pthread_mutex_lock(&(pool->mutex));
  if(pool->nb_task < MAX_TASK_ARRAY){
    res = pool->completable[pool->nb_task] = create_future();
    pool->task[pool->nb_task] = func;
    pool->taskArg[pool->nb_task]=arg;
    (pool->nb_task)++; 
    pthread_cond_signal(&(pool->cond));
  }
  pthread_mutex_unlock(&(pool->mutex));
  return res;
}

void shutdown_pthread_pool(pthread_pool* pool){
  if(!pool)
    return;
  int i;
  for(i=0; i< pool->pool_size; i++)
    pthread_cancel(pool->pool[i]);
  
  for(i=0; i<pool->pool_size; i++)
    pthread_join(pool->pool[i], NULL);
 
  /*Memory clean*/
  free(pool->pool);

  for(i=0; i< MAX_TASK_ARRAY; i++)
    if(pool->completable[i])
      cancel_future(pool->completable[i]);

  pthread_mutex_destroy(&(pool->mutex));
  pthread_cond_destroy(&(pool->cond));
  
  free(pool);
}

