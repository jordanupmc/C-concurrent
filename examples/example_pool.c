#include "../src/cconcurrent.h"
#include <unistd.h>

void * task1(void* arg){
  int x = (int) arg;
  printf("pool task1 number: %d\n", x);
  return x;
}
void * task2(void* arg){
  int x = (int) arg;
  printf("pool task2 number: %d\n", x);
  return x;
}

int main(){
 
  /*init a pool of 5 threads*/
  pthread_pool* p = create_pthread_pool(5);
  int x;
  future* res[20];

  /*Submit 20 tasks*/
  for(x=0; x<20; x++)
    if(!(x%2))
      res[x]=submit_pthread_pool(p, task1, (void*)x);
    else
      res[x]=submit_pthread_pool(p, task2, (void*)x);

  /*Wait the terminaison of all tasks*/
  for(x=0; x<20; x++)
    get_value_future(res[x], NULL);
  
  /*Free the entire pool*/
  shutdown_pthread_pool(p);
 
  /*Free all the future*/
  for(x=0; x<20; x++)
     destroy_future(res[x]);

  return EXIT_SUCCESS;
}
