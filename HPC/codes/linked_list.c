
#if defined(__STDC__)
#  if (__STDC_VERSION__ >= 199901L)
#     define _XOPEN_SOURCE 700
#  endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <omp.h>


// =========================================================================
//
//  define useful quantities
//

#if defined(_OPENMP)

int me;
#pragma omp threadprivate(me)

#define CPU_TIME ({ struct timespec ts; (clock_gettime( CLOCK_REALTIME, &ts ), \
					(double)ts.tv_sec +		\
					 (double)ts.tv_nsec * 1e-9); })

#define CPU_TIME_th ({ struct  timespec ts; (clock_gettime( CLOCK_THREAD_CPUTIME_ID, &myts ), \
					     (double)myts.tv_sec +	\
					     (double)myts.tv_nsec * 1e-9); })

#else

#define CPU_TIME ({ struct timespec ts; (clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts ), \
					(double)ts.tv_sec +		\
					 (double)ts.tv_nsec * 1e-9); })
#endif

//
// =========================================================================
//
// define data structures
//

#define SPARSE  0

typedef struct llnode
{
  int data;
 #if defined(_OPENMP)
  omp_lock_t lock;
 #endif
  
  struct llnode *next;
  struct llnode *prev;  
} llnode_t;

//
// =========================================================================
//
// prototypes
//

int walk            ( llnode_t * );
int delete          ( llnode_t * );
int find_and_insert ( llnode_t *, int );

#if defined(_OPENMP)
int find_and_insert_parallel ( llnode_t *, int );
int find_and_insert_parallel_alternative ( llnode_t *, int );
#endif

//
// =========================================================================
// =========================================================================


// ······················································

int walk ( llnode_t *head )
{
  while ( head != NULL )
    {
      printf("%9d ", head->data );
      head = head->next;
    }
  printf("\n");
  return 0;
}


// ······················································

int delete ( llnode_t *head )
{
  while ( head != NULL )
    {
      llnode_t *prev = head;
      head = head->next;
     #if defined(_OPENMP)
      omp_destroy_lock( &(prev->lock) );
     #endif
      free( prev );
    }
  return 0;
}


// ······················································

int find_and_insert( llnode_t *head, int value )
{
  if ( head == NULL )
    return -1;
  
  llnode_t *ptr  = head->next;
  llnode_t *prev = head;
  while ( (ptr != NULL) && (ptr->data < value) )
    {
      prev = ptr;
      ptr  = ptr->next;
    }

  llnode_t *new = (llnode_t*)malloc( sizeof(llnode_t) );
  if ( new == NULL )
    return -2;
  
  new->data = value;
  new->prev = prev;
  new->next = ptr;
  prev->next = new;

  return 0;
}



#if defined(_OPENMP)


// ······················································

int find_and_insert_parallel( llnode_t *head, int value )
{
  if ( head == NULL )
    return -1;

  llnode_t *ptr  = head->next;
  llnode_t *prev = head;

  while ( (ptr != NULL) && (ptr->data < value) )
    {
      prev = ptr;
      ptr  = ptr->next;
    }

  // to our best knowledge, ptr is the first node with data > value
  // and prev is the last node with data < value
  // then, we should create a new node between prev and ptr
  
  // acquire the lock of prev and ptr
  //
  omp_set_lock(&(prev->lock));
  if( ptr != NULL ) {
    omp_set_lock(&(ptr->lock)); }
    
  // meanwhile, did somebody already insert a node between prev and ptr?
  if( prev-> next != ptr )
    {
      // yes, that happenend
      if (ptr != NULL) {
	// free the lock on ptr
	omp_unset_lock(&(ptr->lock)); }

      // search again, while keeping prev locked
      ptr = prev->next;
      while(ptr)
	{
	  omp_set_lock(&(ptr->lock));
	  if( ptr->data >= value )
	    break;
	  omp_unset_lock(&(prev->lock));
	  prev = ptr;
	  ptr  = ptr->next;
	}
    }

  //
  // insertion code
  //

  llnode_t *new = (llnode_t*)malloc( sizeof(llnode_t) );
  if ( new == NULL )
    return -2;
  
  new->data = value;
  new->prev = prev;
  new->next = ptr;
  omp_init_lock( &(new->lock) );
  prev->next = new;

  // release locks
  //
  omp_unset_lock(&(prev->lock));
  if( ptr != NULL ) {
    omp_unset_lock(&(ptr->lock));}

  return 0;
}


// ······················································

int find_and_insert_parallel_alternative( llnode_t *head, int value )
{
  if ( head == NULL )
    return -1;
  
  llnode_t *ptr  = head->next;
  llnode_t *prev = head;
  
  while ( (ptr != NULL) && (ptr->data < value) )
    {
      prev = ptr;
      ptr  = ptr->next;
    }
    
  // to our best knowledge, ptr is the first node with data > value
  // and prev is the last node with data < value
  // then, we should create a new node between prev and ptr

  // acquire the lock of prev and ptr
  //
  while ( omp_test_lock(&(prev->lock)) == 0 ) {
    #pragma omp taskyield
  } 
  // omp_set_lock(&(prev->lock));  --> note: at this point the prev->lock has been taken
  if( ptr != NULL )
    omp_set_lock(&(ptr->lock));

  // meanwhile, did somebody already insert a node between prev and ptr?
  if( prev-> next != ptr )
    {
      // yes, that happenend
      if (ptr!= NULL )
	// free the lock on ptr
	omp_unset_lock(&(ptr->lock));

      // search again, while keeping prev locked
      ptr = prev->next;
      while(ptr)
	{
	  omp_set_lock(&(ptr->lock));
	  if( ptr->data >= value )
	    break;
	  omp_unset_lock(&(prev->lock));
	  prev = ptr;
	  ptr  = ptr->next;
	}
    }

  //
  // insertion code
  //

  llnode_t *new = (llnode_t*)malloc( sizeof(llnode_t) );
  if ( new == NULL )
    return -2;
  
  new->data = value;
  new->prev = prev;
  new->next = ptr;
  omp_init_lock( &(new->lock) );
  prev->next = new;

  // release locks
  //
  omp_unset_lock(&(prev->lock));
  if( ptr != NULL )
    omp_unset_lock(&(ptr->lock));

  return 0;
}


#endif


// ······················································

int main ( int argc, char **argv )
{
  int N    = ( argc > 1 ? atoi(*(argv+1)) : 1000000 );
  int mode = ( argc > 2 ? atoi(*(argv+2)) : SPARSE );
  int seed = ( argc > 3 ? atoi(*(argv+3)) : 98765 );

  srand( seed );


  llnode_t *head = (llnode_t*)malloc(sizeof(llnode_t));
  head->data = rand();
  head->prev = NULL;
  head->next = NULL;
 #if defined(_OPENMP)
  omp_init_lock( &(head->lock) );
 #endif
  
  double timing = CPU_TIME;
  
 #if !defined(_OPENMP)
  
  int n = 0;
  while ( n < N )
    {
      int new_value = rand();
      int ret = find_and_insert( head, new_value );
      if ( ret < 0 )
	{
	  printf("I've got a problem inserting node %d\n", n);
	  delete( head );
	}
      n++;
    }

 #else

  #pragma omp parallel
  {
    me = omp_get_thread_num();
    
    #pragma omp single
    {
      int n = 0;

      while ( n < N )
	{
	  int new_value = rand();

	  if ( mode == 0 ) {
	   #pragma omp task
	    find_and_insert_parallel( head, new_value ); }
	  else {
	   #pragma omp task untied
	    find_and_insert_parallel_alternative( head, new_value ); }
	  
	  n++;
	}
    }
    
  }

 #endif

  timing = CPU_TIME - timing;

  if ( N < 100)
    walk( head );

  delete ( head );

  printf("generation took %g seconds\n", timing);
  
  return 0;
}
