
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

typedef unsigned long long ull;
#define TIME_CUT 1000000009


#if defined(_OPENMP)

int me;
#pragma omp threadprivate(me)

#define CPU_TIME ({ struct timespec ts; (clock_gettime( CLOCK_REALTIME, &ts ), \
					 (ull)ts.tv_sec * 1000000000 +	\
					 (ull)ts.tv_nsec); })

#define CPU_TIME_th ({ struct  timespec ts; (clock_gettime( CLOCK_THREAD_CPUTIME_ID, &myts ), \
					     (ull)myts.tv_sec*1000000000 + \
					     (ull)myts.tv_nsec); })

#else

#define CPU_TIME ({ struct timespec ts; (clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts ), \
					 (ull)ts.tv_sec * 1000000000 +	\
					 (ull)ts.tv_nsec); })
#endif


#if defined(DEBUG)
#define TIMESTAP (CPU_TIME % TIME_CUT)
#define dbgout(...) printf( __VA_ARGS__ );
#else
#define TIMESTAP
#define dbgout(...) 
#endif


//
// =========================================================================
//
// define data structures
//

#define DONT_USE_TASKYIELD  0
#define USE_TASKYIELD  1

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
// declare data structures
//

int clashes;

//
// =========================================================================
//
// prototypes
//

llnode_t* get_head        ( llnode_t *);
int       walk            ( llnode_t *);
int       delete          ( llnode_t * );
int       find            ( llnode_t *, int, llnode_t **, llnode_t ** );
int       find_and_insert ( llnode_t *, int );

#if defined(_OPENMP)
int       find_and_insert_parallel ( llnode_t *, int, int );
#endif

//
// =========================================================================
// =========================================================================


// ······················································

llnode_t *get_head ( llnode_t *start )
/*
 * walk the list basck to find the list head
 * returns the head
 */
{
  while( start->prev != NULL )
    start = start->prev;
  
  return start;
}

// ······················································

int walk ( llnode_t *start )
/*
 * walk the list starting from the node start
 * as first, the list is walked back until the list head
 * if mode == 1, the list is then walked ahed printing
 * the first 100 nodes.
 */
{
  int n = 0;
  if ( start != NULL )
    {
      n = 1;
      int prev_value = start->data;
      printf("%9d [-]", start->data );
      start = start->next;
      while( start != NULL)
	{
	  if (++n < 100 )
	    printf( "%9d %s ",
		   start->data,
		   (start->data < prev_value? "[!]":"[ok]") );
	  else if ( n == 100)
	    printf( "..." );
	  prev_value = start->data;
	  start = start->next;
	}
    }
  printf("\n");
  return n;
}


// ······················································

int delete ( llnode_t *head )
/*
 * delete all the nodes
 * destroy every lock
 */
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

int find ( llnode_t *head, int value, llnode_t **prev, llnode_t **next )
{
  *prev = NULL, *next = NULL;
  
  if ( head == NULL )
    // The first node must exist in this simple
    // implementation.
    // To improve that, pass **head instead
    // of *head
    return -1;

  int       nsteps = 0;
  llnode_t *ptr = NULL;

  if ( head-> data > value )
    {
      // we need to walk back
      //
      ptr  = head->prev;
      *next = head;
      while ( (ptr != NULL) && (ptr->data > value) )
	{
	  *next = ptr;
	  ptr  = ptr->prev;
	  nsteps++;
	}
      *prev = ptr;
    }
  else
    {
      // we need to walk ahead
      //
      ptr  = head->next;
      *prev = head;
      while ( (ptr != NULL) && (ptr->data < value) )
	{
	  *prev = ptr;
	  ptr  = ptr->next;
	  nsteps++;
	}
      *next = ptr;
    }

  return nsteps;
}


int find_and_insert( llnode_t *head, int value )
{
  if ( head == NULL )
    // The first node must exist in this simple
    // implementation.
    // To improve that, pass **head instead
    // of *head
    return -1;

  llnode_t *prev = NULL, *next = NULL;

  find ( head, value, &prev, &next );
  
  llnode_t *new = (llnode_t*)malloc( sizeof(llnode_t) );
  if ( new == NULL )
    // signals a problem in mem alloc
    return -2;
  
  new->data = value;
  new->prev = prev;
  new->next = next;
  if( prev != NULL )
    prev->next = new;
  if( next != NULL )
    next->prev = new;

  return 0;
}



#if defined(_OPENMP)


// ······················································


int find_and_insert_parallel( llnode_t *head, int value, int use_taskyield )
{
  if ( head == NULL )
    return -1;

  llnode_t *prev = NULL, *next = NULL;

  dbgout("[ %llu ] > T %d process value %d\n", TIMESTAMP, me, value );
  
  find ( head, value, &prev, &next );
  
  dbgout("[ %llu ] T %d V %d found p: %d and n: %d\n", TIMESTAMP, me, value,
	 prev!=NULL?prev->data:-1, next!=NULL?next->data:-1);
  
  // to our best knowledge, ptr is the first node with data > value
  // and prev is the last node with data < value
  // then, we should create a new node between prev and ptr
  
  // acquire the lock of prev and next
  //
  if ( use_taskyield )
    {
      if ( prev != NULL )
	while ( omp_test_lock(&(prev->lock)) == 0 ) {
	 #pragma omp taskyield
	}
      prev->owner=me;
      if ( next != NULL )
	while ( omp_test_lock(&(next->lock)) == 0 ) {
	 #pragma omp taskyield
	}
    }
  else
    {
      if( prev != NULL )
	omp_set_lock(&(prev->lock));
      
      if( next != NULL )
	omp_set_lock(&(next->lock));
    }


  dbgout("[ %llu ] T %d V %d locked: (p: %d p>n: %d) (n: %d n<p: %d)\n",
	 TIMESTAMP, me, value,
	 (prev!=NULL?prev->data:-1),((prev!=NULL)&&(prev->next!=NULL)?(prev->next)->data:-1),
	 (next!=NULL?next->data:-1),((next!=NULL)&&(next->prev!=NULL)?(next->prev)->data:-1) );
  
  // meanwhile, did somebody already insert a node between prev and next?
  if( ( (prev != NULL) && (prev-> next != next) ) ||
      ( (next != NULL) && (next-> prev != prev) ) )
    {
      // yes, that happened
      // let's keep track of how many clashes
      // 
     #pragma omp atomic update
      clashes++;
      
      if( (prev != NULL) && (prev-> next != next) )
	{
	  // the next pointer has changed
	  // prev is not null, so that is our still valid point
	  // we'll walk ahead from there
	  //
	  dbgout("[ %llu ]\t>> T %d V %d next has changed: from %d to %d\n",
		 TIMESTAMP, me, value,
		 (next!=NULL?next->data:-1),(prev->next!=NULL?(prev->next)->data:-1) );

	  if (next != NULL)
	    // free the lock on the old next
	    omp_unset_lock(&(next->lock));

	  dbgout("[ %llu ]\t\t>>> T %d V %d restart from %d to walk ahead\n",
		 TIMESTAMP, me, value, prev->data);
	  
	  // search again, while always keeping prev locked
	  next = prev->next;
	  while(next)
	    {
	      dbgout("[ %llu ]\t\t\t>>> T %d V %d stepping into %d\n",
		     TIMESTAMP, me, value, next->data );
	      omp_set_lock(&(next->lock));
	      
	      if( next->data >= value )
		break;
	      omp_unset_lock(&(prev->lock));
	      prev = next;
	      next = next->next;
	    }	  
	}
      
      else if ( next->prev != prev )
	// note that next can not be NULL
	{
	  // the prev pointer has changed
	  // next is not null, so that is our still valid point
	  // we walk back from there
	  //
	  dbgout("[ %llu ]\t>> T %d V %d prev has changed: from %d to %d\n",
		 TIMESTAMP, me, value,
		 (prev!=NULL?prev->data:-1),(next->prev!=NULL?(next->prev)->data:-1) );

	  if (prev != NULL)
	    // free the lock on the old next
	    omp_unset_lock(&(prev->lock));

	  dbgout("[ %llu ]\t\t>> T %d V %d restart from %d to walk back\n",
		 TIMESTAMP, me, value, next->data);
      	
	  // search again, while always keeping prev locked
	 prev = next->prev;
	 while(prev)
	   {
	     dbgout("[ %llu ]\t\t\t>>> T %d V %d stepping into %d\n",
		    TIMESTAMP, me, value, prev->data);
	     omp_set_lock(&(prev->lock));
	     if( prev->data <= value )
	       break;
	     omp_unset_lock(&(next->lock));
	     next = prev;
	     prev = prev->prev;
	   }
	}
      else if ( next == NULL )
	{
	  printf("Some serious error occurred, a prev = next = NULL situation arose!\n");
	  return -3;
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
  new->next = next;
  omp_init_lock( &(new->lock) );
  if ( prev != NULL )
    prev->next = new;
  if ( next != NULL)
    next->prev = new;
  
  // release locks
  //
  if ( prev != NULL ) {
    omp_unset_lock(&(prev->lock));
    dbgout("[ %llu ]\tthread %d processing %d releases lock for %d\n",
	   TIMESTAMP, me, value, prev->data);}
					  
  if( next != NULL ) {
    omp_unset_lock(&(next->lock));
    dbgout("[ %llu ]\tthread %d processing %d releases lock for %d\n",
	   TIMESTAMP, me, value, next->data);}

  dbgout("T %d V %d has done\n", me, value);
  return 0;
}

#endif


// ······················································

int main ( int argc, char **argv )
{
  int N, mode;
  
  {
    int a = 1;
    N    = ( argc > 1 ? atoi(*(argv+a++)) : 1000000 ); 
   #if defined(_OPENMP)
    mode = ( argc > a ? atoi(*(argv+a++)) : DONT_USE_TASKYIELD );
   #endif
    int seed = ( argc > a ? atoi(*(argv+a++)) : 98765 );
    
    srand( seed );
  }


  llnode_t *head = (llnode_t*)malloc(sizeof(llnode_t));
  head->data = rand();
  head->prev = NULL;
  head->next = NULL;
 #if defined(_OPENMP)
  omp_init_lock( &(head->lock) );
 #endif
  
  ull timing = CPU_TIME;
  
 #if !defined(_OPENMP)
  
  int n = 1;
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
      printf("running with %d threads\n", omp_get_num_threads());
      int n = 1;

      while ( n < N )
	{
	  int new_value = rand();

	 #pragma omp task
	  find_and_insert_parallel( head, new_value, mode );
	  
	  n++;
	}
    }
  }

 #endif

  timing = CPU_TIME - timing;

  head = get_head( head );

  int actual_nodes = walk( head);
  if ( actual_nodes != N )
    printf("shame on me! %d nodes instaed of %d have been found!",
	   actual_nodes, N);
  
  delete ( head );

  char string[23] = {0};
 #if defined(_OPENMP)
  sprintf( string, " with %d clashes", clashes);  
 #endif
  printf("generation took %g seconds (wtime) %s\n", ((double)timing/1e9), string);
  
  
  return 0;
}
