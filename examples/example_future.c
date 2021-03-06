#include "../src/cconcurrent.h"
#include <unistd.h>

void* threadF(void * arg){
  future * tmp = (future *) arg;
  int i;
  
  for(i=0; i<4000000; i++);
  printf("PASS\n");
  complete_future(tmp, (void *)(i/2) );
  pthread_exit(NULL);
}

void * notify(void* data){
  printf("Data available: do something smart with %d\n", (int)data);
  return NULL;
}

int main(){
  
  future * f = create_future();
  pthread_t t;
  pthread_create(&t, NULL, threadF, (void *)f);
 
  /*get the result of threadF*/
  printf("DONE %d\n", (int)get_value_future(f, notify));
  /*Free the future*/
  destroy_future(f);

  return EXIT_SUCCESS;
}
