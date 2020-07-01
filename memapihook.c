/* Prototypes for __malloc_hook, __free_hook */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>

#include "hashsep.h"


//#define printf(...)

pthread_mutex_t lock;


/* stats */

unsigned  int TotalAlloc =0, CurrentAlloc=0, TotalFree=0;

int CurrentSize;
//
//static 
HashTable H, T;

time_t StartTime ;
char *slotname[] ={
	"[0-4) bytes",
	"[4-8) bytes",
	"[8-16) bytes",
	"[16-32) bytes",
	"[32-64) bytes",
	"[64-128) bytes",
	"[128-256) bytes",
	"[256-512) bytes",
	"[512-1024) bytes",
	"[1024-2048) bytes",
	"[2048-4096) bytes",
	"4096plus bytes",
	"sizeofenum"
};


typedef enum _bucket{

	_0to3=0,
	_4to7,
	_8to15,
	_16to31,
	_32to63,
	_64to127,
	_128to255,
	_256to511,
	_512to1023,
	_1024to2047,
	_2048to4095,
	_4096plus,
	_sizeofenum

}bucket;


bucket memBuckets[_sizeofenum];

void InitLock (  )
{
  if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
    }

}

void DestroyLock ( )
{

   pthread_mutex_destroy(&lock);

}

void AcqLock ( )
{
   pthread_mutex_lock(&lock);
}


void RelLock ()
{
   pthread_mutex_unlock(&lock);
}

static
void printmemstat( )
{
  float  currentTotalMiB = 0.0;
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  char s[64];
  assert(strftime(s, sizeof(s), "%c", tm));
  printf("%s\n", s);

  fprintf(stderr, ">>>>>>>>>>>>>>>> ");

  fprintf(stderr, " %s ", s);

  fprintf(stderr, "  <<<<<<<<<<<<<< \n");

  /// unsigned  int TotalAlloc =0, CurrentAlloc=0, TotalFree=0;
  fprintf(stderr, "Overall stats:  \n");
  fprintf(stderr, " %u Current allocations  \n", TotalAlloc - TotalFree);
  fprintf(stderr, " %u Overall allocations since start \n", TotalAlloc);
  
  currentTotalMiB =  (float) (TotalAlloc - TotalFree) / (1024.0 * 1024.0) ;
  fprintf(stderr, " %0.2fMiB Current total allocated size\n\n\n", currentTotalMiB );


  for ( int i =0; i < _sizeofenum; i++ ){
     //fprintf(stderr, "memBuckets[%i] = %i\n", i, memBuckets[i]) ;
     fprintf(stderr, "%s %i \n", slotname[i], memBuckets[i]) ;
  }

  fprintf(stderr, "\n\n") ;

}

static
void showmemstat ( )
{
  static time_t lasttime = 0;
   volatile time_t currenttime;

  time(&currenttime);
  //fprintf(stderr, "lasttime=%u, currenttime=%u\n", lasttime, currenttime);
  if (currenttime - lasttime >= 5 ){
     //show memstats
       	printmemstat(); 
	ElementAgeReport (currenttime, T);
        lasttime = currenttime;
     }


}



bucket get_bucket ( size_t size )
{
  if (size < 0 ){
    printf(" --- size is less than zero \n");
  }

  if ( size <  4 ) return _0to3;
  if ( size <  8 ) return _4to7;
  if ( size <  16 ) return _8to15;
  if ( size <  32 ) return _16to31;
  if ( size <  64 ) return _32to63;
  if ( size <  128 ) return _64to127;	
  if ( size <  256 ) return _128to255;
  if ( size <  512 ) return _256to511;
  if ( size <  1024 ) return _512to1023;
  if ( size <  2048 ) return _1024to2047;
  if ( size <  4096 ) return _2048to4095;

  return _4096plus;
}

/*
 * Change the allocation count of the bucket corresponds to size
 */

void record_mem_events ( void * mem, size_t size, bool isalloc )
{

  ElementType tm = time(NULL);
  bucket slot = get_bucket(size);
  
  /* Allocate */
  isalloc ? (TotalAlloc += size)  : (TotalFree += size);
  /* Free */
  isalloc ? memBuckets[slot]++ : memBuckets[slot]--;

  printf("... record_mem_events before Hash update\n");
  /* Insert or Remove by Address key */
#if 1
  if (isalloc ){

    Insert(mem, tm,  H);
    Insert(tm, mem, T);  
  }else{

     Remove(mem, tm, H);
     Remove(tm, mem, T);
  }

}
#endif




/******************* hook code *****************/
/* Prototypes for our hooks.  */
//static void my_init_hook (void);

static void *my_malloc_hook (size_t, const void *);
static void my_free_hook (void*, const void *);
static void *my_realloc_hook (void*, size_t, const void *);

// -- tyoedef the base hook functions
typedef
 void (* old_malloc_hook ) (size_t, const void*);

typedef
 void (* old_realloc_hook ) (void*,  size_t, const void*);

typedef
 void (* old_free_hook) (void*, const void *);

old_malloc_hook  f_old_malloc_hook;
old_free_hook    f_old_free_hook;
old_realloc_hook  f_old_realloc_hook;


/* when the so is unloaded system should call this as cleaing process */
static void my_deinit(void) __attribute__((destructor));
void
my_deinit (void)
{
  /* replace back the systems hooks function */
  __malloc_hook = f_old_malloc_hook;
  __free_hook = f_old_free_hook;
  __realloc_hook = f_old_realloc_hook;

  DestroyLock();

  DestroyTable(H);
  DestroyTable(T);

}

// -- comment out when debugging in executable mode
static void my_init(void) __attribute__((constructor));
void
my_init (void)
{


  StartTime = time(NULL);
  //fprintf(stderr, "...Start Time =%u\n", StartTime);
  printf( "...Start Time =%u\n", StartTime);
  // -- Hash interface
  H = InitializeTable( CurrentSize = 0xffff );
  /* Time montonically increases (by second due to api use */
  T = InitializeTable( CurrentSize = 0xfffff );
  
  InitLock();
  /* save systems hooks function */
  f_old_malloc_hook =  __malloc_hook;
  f_old_free_hook   = __free_hook;
  f_old_realloc_hook = __realloc_hook;

  /* put our hook function */
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
  __realloc_hook = my_realloc_hook;

}

// -- Why recurse thru the stack when in .so mode ???
// -- not calling it
static
void* __attribute__ ((noinline))
MyAlloc(unsigned int size)
{
    void *p = NULL;

    p = malloc(size + sizeof(int));
    if ( p == NULL )
       return NULL;


    *(int *) p = size;

    p = (void *)( ((int *) p) + 1);
    printf ("MyAlloc: (%u) returns %p\n", (unsigned int) size, p);
    return p;
}


static void *
 __attribute__ ((noinline))
my_malloc_hook (size_t size, const void *caller)
{
  void *p;

  AcqLock();

  /* Restore all old hooks */
  __malloc_hook = f_old_malloc_hook;
  __realloc_hook = f_old_realloc_hook;
  __free_hook = f_old_free_hook;
  /* Call forward in normal path */

    p = MyAlloc(size);
    if ( p != NULL ) {
      record_mem_events (p, size, true);
    }
  
  showmemstat ( );
  /* Save  original memory hooks */
  f_old_malloc_hook = __malloc_hook;
  f_old_realloc_hook = __realloc_hook;
  f_old_free_hook = __free_hook;
  /* printf might call malloc, so protect it too. */
   printf ("my_malloc_hook:malloc (%u) returns %p\n\n", (unsigned int) size, p);
  /* Restore our own hooks */
  __malloc_hook = my_malloc_hook;
  __realloc_hook = my_realloc_hook;
  __free_hook = my_free_hook;

  RelLock();

  return p;
  //return (void *)( ((int *) p) + 1);

}


// -- Why recurse thru the stack when in .so mode ???
// -- not calling it
static
void*  __attribute__ ((noinline))
MyRealloc(void *oldp, unsigned int size)
{
  void* ptr = NULL;
  unsigned int mask = 1 <<31;

  ptr = oldp;
  ptr  = (void *)( ((int *) oldp) - 1);
  ptr = realloc(ptr, size + sizeof(int));

  // just use size
  *(int *)ptr =  size;

   printf ("MyRealloc (%i) oldp=%p returs %p\n", *(int* ) ptr, oldp, ptr);
   ptr  = (void *)( ((int *) ptr) + 1);
   return ptr;
}

static void *
__attribute__ ((noinline))
my_realloc_hook (void *addr, size_t size, const void *caller)
{
  void *ptr = NULL;

  AcqLock();

  /* Restore all old hooks */
  __malloc_hook = f_old_malloc_hook;
  __realloc_hook = f_old_realloc_hook;
  __free_hook = f_old_free_hook;

     /* if ptr is null it is equivalent to malloc(size) */
  if (addr == NULL ){
     ptr = malloc(size );

  }else if (size == 0 ){
    free(addr);
  }else {

     ptr = MyRealloc(addr, size );
  }
  if (ptr){

      record_mem_events (ptr,  size, true);
  }
  /* Save  original memory hooks */
  f_old_malloc_hook = __malloc_hook;
  f_old_realloc_hook = __realloc_hook;
  f_old_free_hook = __free_hook;

  printf ("my_realloc_hook (%u) returns %p \n\n", (unsigned int) size, ptr);
  showmemstat ( );

  /* Restore our own hooks */
  __malloc_hook = my_malloc_hook;
  __realloc_hook = my_realloc_hook;
  __free_hook = my_free_hook;

  RelLock();
  return ptr;
}

static
size_t  __attribute__ ((noinline))
MyFree(void *ptr)
{
    size_t size = 0;
    if (!ptr){
       printf("MyFree() should not be called with NULL ptr \n");
       exit(999);
    }

    /* normal alloc adjust ptr the length */

    printf("MyFree: ptr=%p as arg\n", ptr);  
    ptr = (void *)( ((int *) ptr) -1 );
    size = *(int*)ptr;
    printf("MyFree: freeing ptr=%p \n", ptr);  
    free(ptr);
    

    return size;
}

static void
__attribute__ ((noinline))
my_free_hook (void *ptr, const void *caller)
{
  size_t size = 0;
  AcqLock();
  
  /* Restore all old hooks */
  __malloc_hook = f_old_malloc_hook;
  __realloc_hook = f_old_realloc_hook;
  __free_hook = f_old_free_hook;

  if (!ptr){
    RelLock();
    return;
  }
  /* Actual memory free */
  size = MyFree (ptr);

 record_mem_events (ptr,  size, false);
  /* Save underlying hooks */
  f_old_malloc_hook = __malloc_hook;
  f_old_realloc_hook = __realloc_hook;
  f_old_free_hook = __free_hook;
  /* printf might call free, so protect it too. */
  printf ("freed pointer %p\n", ptr);
  /* Restore our own hooks */
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
  __realloc_hook = my_realloc_hook;

  RelLock();
}

// -- standalone executable 
//
//
int testrealloc() {
   char *str;
   unsigned len = 15;

   /* Initial memory allocation */
   str = (char *) malloc(15);
   printf("testrealloc: malloc(%u) str=%p\n", len, str);
   strcpy(str, "realloc test");
   printf("String = %s,  Address = %p\n", str, str);

   /* Reallocating memory */
   str = (char *) realloc(str, len + 32 );
   strcat(str, "strcat after realloc");
   printf("String = %s,  Address = %p\n", str, str);

   free(str);
   
   return(0);
}

#if 0
int main ()
{
  void *mem;
  printf ("main started -- only to debug ... \n");
  printf (" no of bucket =%u\n", sizeof(memBuckets)/sizeof(bucket )  );

  my_init ();

  testrealloc();

  int cnt = 0;
  while (cnt++ < 100 ){
    mem = malloc (24 + cnt );
    if ( mem ) {
       printf("mem addr=%p\n", mem );
       free(mem);
    }else{
       printf("mem alloc fail\n");
    }

    sleep(1);
}
  /// testrealloc();

  //my_deinit();
  printf ("main exiting -- only to debug ... \n");

}

#endif


