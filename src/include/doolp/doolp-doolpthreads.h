// #include <doolp-config.h>

#ifdef __DOOLP_USE_DOOLPTHREADS
#ifndef __DOOLP_DOOLPTHREADS_H
#define __DOOLP_DOOLPTHREADS_H

#include <glog.h>

#define __DOOLPTHREADS_Log(...)

#include <pthread.h>
#include <semaphore.h>

#ifndef __DOOLPTHREADS_MAX
#define __DOOLPTHREADS_MAX 64
#endif

typedef enum doolpThreadStatus  { 
  DoolpThreadStatus_unborn,
  DoolpThreadStatus_free,
  DoolpThreadStatus_busy };

typedef struct doolpThread
{
  doolpThreadStatus status;
  sem_t sem;
  void * (*func) (void *);
  void * data;
  void (*destructor) ( void * __destruct_data );
  void * destruct_data;
  pthread_t realThread;
};

bool doolpThreads_init ( ); //void (*__doolpThreads_destructor) ( void * meta_data ) );

int doolpThreads_create ( pthread_t * thread, pthread_attr_t * attr, void * (*func) (void *), void * data, void (*__destructor) ( void * __destruct_data ), void * destruct_data );

void doolpThreads_log ();

#ifndef __DOOLP_DOOLPTHREADS_IN_CPP
extern void (*doolpThreads_destructor) ( void * meta_data );
// extern void * doolpThreads_metastruct;
#endif // __DOOLP_DOOLPTHREADS_IN_CPP

#endif // __DOOLP_DOOLPTHREADS_H
#endif //  __DOOLP_USE_DOOLPTHREADS
