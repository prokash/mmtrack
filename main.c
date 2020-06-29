#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>

int testrealloc() {
   char *str;

   /* Initial memory allocation */
   str = (char *) malloc(15);
   strcpy(str, "realloc test");

   /* Reallocating memory */
   str = (char *) realloc(str, 55);
   strcat(str, " strcat realloc");
   printf("String = %s,  Address = %p\n", str, str);
   printf("Freeing addr=%p\n", str);

   free(str);

   return(0);
}


int main (int argc, char*argv[])
{
  void *mem;
  long cnt = 0;
  int maxcnt = 1;
  char *p;

  srand(time(0)); 

  printf ("main started -- only to debug ... \n");
  if (argc > 1 )
      maxcnt = strtol(argv[1], &p, 10);

  testrealloc();
  printf("Loop count=%d\n", maxcnt);

  while (cnt++ < maxcnt ){
    mem = malloc (24 + rand() % 4097);
    if ( mem ) {
       printf("mem addr=%p\n", mem );
       free(mem);
       mem = NULL;
       testrealloc();
    }else{
       printf("mem alloc fail\n");
    }

    sleep(1);
  }
  /*printf(" Memory stats abut to print.... \n\n"); */

  printf ("main exiting -- only to debug ... \n");

}


