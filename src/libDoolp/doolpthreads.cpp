#include <doolp-config.h>

#include <doolp/doolp-doolpthreads.h>

#ifdef __DOOLP_USE_DOOLPTHREADS

#warning Using exerimental DoolpThreads

#ifndef DOOLP_PTHREAD_MUTEX_INIT__
#define DOOLP_PTHREAD_MUTEX_INIT(__mutex) \
  pthread_mutex_init ( &__mutex, NULL ) 
#endif

#define __DOOLPTHREADS_Log Log

doolpThread doolpThreads[__DOOLPTHREADS_MAX];
pthread_mutex_t doolpThreadMutex;
 

void (*doolpThreads_destructor) ( void * meta_data ) = NULL;
// void * doolpThreads_metastruct;

int doolpThreads_created = 0;
int doolpThreads_running = 0;
bool doolpThreads_inited = false;

bool doolpThreads_init ( ) // void (*__doolpThreads_destructor) ( void * meta_data ) )
{
  if ( doolpThreads_inited )
    { Warn ( "DoolpThreads already inited !\n" ); return true; }
  doolpThreads_inited = true;
  DOOLP_PTHREAD_MUTEX_INIT ( doolpThreadMutex );
  Warn ( "Initing DoolpThreads.\n" );
  __DOOLPTHREADS_Log ( "Initing doolpThreads, builtin %d threads\n", __DOOLPTHREADS_MAX );
  //  doolpThreads_destructor = __doolpThreads_destructor;
  for ( int th = 0 ; th < __DOOLPTHREADS_MAX ; th ++ )
    {
      doolpThreads[th].status = DoolpThreadStatus_unborn;
      sem_init ( &(doolpThreads[th].sem), 0, 0 );
    }
  return true;
}

void * __dpTh_create ( void * data )
{
  __DOOLPTHREADS_Log ( "entering __dpTh_create\n" );
  int idx = (int) data;
  int res;
  while ( true ) 
    {
      __DOOLPTHREADS_Log ( "for idx %d, func=%p, data=%p, destructor=%p, meta_data=%p\n", 
			   idx, doolpThreads[idx].func, 
			   doolpThreads[idx].data, 
			   doolpThreads[idx].destructor,
			   doolpThreads[idx].destruct_data );      

      AssertBug ( doolpThreads[idx].func != NULL, "I have been given a null function, I can't run it.\n" );
      res = pthread_mutex_unlock ( &doolpThreadMutex );
      AssertBug ( res == 0, " Could not lock doolpThreads\n" );

      __DOOLPTHREADS_Log ( "** (idx=%d) start run func=%p\n", idx, doolpThreads[idx].func );
      doolpThreads[idx].func ( doolpThreads[idx].data );
      __DOOLPTHREADS_Log ( "** (idx=%d) end run func=%p\n", idx, doolpThreads[idx].func );

      doolpThreads[idx].func = NULL;
      __DOOLPTHREADS_Log ( "** (idx=%d) destroying context destructor=%p destruct_data=%p\n",
			   idx, doolpThreads[idx].destructor, doolpThreads[idx].destruct_data );
      if ( doolpThreads[idx].destructor != NULL )
	doolpThreads[idx].destructor ( doolpThreads[idx].destruct_data ); 
      //      if ( doolpThreads[idx].func != NULL ) continue; // Destructor has refilled me with another function to run...
      __DOOLPTHREADS_Log ( "** (idx=%d) destroyed context destructor=%p destruct_data=%p\n",
			   idx, doolpThreads[idx].destructor, doolpThreads[idx].destruct_data );
      doolpThreads_running --;
      doolpThreads[idx].status = DoolpThreadStatus_free;

      __DOOLPTHREADS_Log ( "** (idx=%d) finished, waiting for new job\n", idx );
      doolpThreads_log ();
      while ( doolpThreads[idx].func == NULL )
	{
	  sem_wait ( &(doolpThreads[idx].sem) );
	  __DOOLPTHREADS_Log ( "** (idx=%d) : recieved sem\n", idx );
	  if ( doolpThreads[idx].func == NULL )
	    {
	      Warn ( "Recieved sem, but func is NULL\n" );
	    }
	}
    }
}


int doolpThreads_create ( pthread_t * thread, pthread_attr_t * attr, void * (*func) (void *), void * data, void (*__destructor) ( void * __destruct_data ), void * destruct_data )
{
  AssertBug ( doolpThreads_inited, "DoolpThreads not inited !\n" );
  __DOOLPTHREADS_Log ( "Locking doolpThreadMutex\n" );
  int res = pthread_mutex_lock ( &doolpThreadMutex );
  AssertBug ( res == 0, " Could not lock doolpThreads\n" );
  /*
  if ( res != 0 ) 
    { 
      Error ( "Lock : res %d\n", res ); 
      LogErrNo; 
    }
    */
  __DOOLPTHREADS_Log ( "Finding free slot in doolpThreads (func=%p, data=%p, destructor=%p, destruct_data=%p)\n",
		       func, data, __destructor, destruct_data );
  doolpThreadStatus status;
  for ( int idx = 0 ; idx <  __DOOLPTHREADS_MAX ; idx ++ )
    {
      if ( doolpThreads[idx].status == DoolpThreadStatus_busy ) continue;
      __DOOLPTHREADS_Log ( "Running at idx %d\n", idx );
      doolpThreads[idx].func = func;
      doolpThreads[idx].data = data;
      doolpThreads[idx].destructor = __destructor;
      doolpThreads[idx].destruct_data = destruct_data;
      status = doolpThreads[idx].status; // Original status before beeing set busy
      doolpThreads_running ++;
      doolpThreads[idx].status = DoolpThreadStatus_busy;
      // if ( res != 0 ) { Error ( "Unlock : res %d\n", res ); LogErrNo; }
      if ( status == DoolpThreadStatus_unborn )
	{
	  __DOOLPTHREADS_Log ( "Creating new slot '%d'\n", idx );
	  doolpThreads_created ++;
	  res = pthread_create ( &(doolpThreads[idx].realThread), attr, __dpTh_create, (void *) idx );
	  __DOOLPTHREADS_Log ( "pthread creation gave result %d\n", res );
	  AssertBug ( res == 0, "Could not create another doolpThread\n" );
	  return res;
	}
      else if ( status == DoolpThreadStatus_free )
	{
	  __DOOLPTHREADS_Log ( "Re-using slot '%d'\n", idx );
	  sem_post ( &(doolpThreads[idx].sem) );
	  return 0;
	}
      Bug ( "Should not be here !!!!\n" );
    }
  Bug ( "Not enough doolpThreads\n" );
  return -1;
}

void doolpThreads_log ()
{
  __DOOLPTHREADS_Log ( "Created %d, Running %d\n", doolpThreads_created, doolpThreads_running );
}

#endif  // __DOOLP_USE_DOOLPTHREADS
