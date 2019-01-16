#ifndef __DOOLP_CONFIG_H
#define __DOOLP_CONFIG_H

// Values to be modified
#include <stdlib.h>
#include <stdio.h>

#ifdef __DOOLP_USE_DMALLOC
#include <dmalloc.h>
#endif

// Glog & Log facilities
#ifdef LOG
#define __GLOG_SHOW_LOG
#define DOOLP_LOG
#endif

// General
#define __DOOLPCONNECTIONTCP_DUMPBUFFERS_FW
#define __DOOLP_USE_DOOLPTHREADS

// DoolpCall
#define __DOOLPCALL_USE_SEM

// DoolpJob
#define __DOOLPJOB_ALLOW_RUNINSAMETHREAD // This is dangerous, because of sub-calls

// DoolpConnection

#define __DOOLPCONNECTION_ALLOW_WAITSPECIFICCALL

// DoolpConnectionTCP
#define __DOOLPCONNECTIONTCP_WRITEBUFF
#define __DOOLPCONNECTIONTCP_WRITEBUFF_SZ 2048
#undef __DOOLPCONNECTIONTCP_EXTRALOG // Log actions (read, write) when they meet success...
#define __DOOLPCONNECTIONTCP_ALLOW_WAITSPECIFICCALL

// DoolpConnectionXML
// #define __DOOLPCONNECTIONXML_READSPACE_SZ 4096
// #define __DOOLPCONNECTIONXML_READBUFFER_SZ 256

// DoolpForge
#define __DOOLPFORGE_ALLOW_PUSHED_CONTEXTS // Prerequisite to __DOOLPJOB_FORCE_RUN_IN_SAME_THREAD
#define __DOOLPFORGE_CALLMUTEX // This is an unnecessary security.
#define __DOOLPFORGE_JOBMUTEX
#define __DOOLPFORGE_CONNECTIONMUTEX
/*
 * Impact of these values
 */


#ifdef DOOLP_LOG
#define __DOOLP_Log Log
#else
#define __DOOLP_Log(...)
#endif 

#ifdef __DOOLPCONNECTIONTCP_EXTRALOG
#define __DOOLPCONNECTIONTCP_Log __DOOLP_Log
#else // __DOOLPCONNECTIONTCP_Log
#define __DOOLPCONNECTIONTCP_Log(...)
#endif // __DOOLPCONNECTIONTCP_Log


#define DOOLP_PTHREAD_MUTEX_INIT(__mutex)	\
  __mutex = (pthread_mutex_t *)			\
    malloc ( sizeof ( pthread_mutex_t ) );	\
  pthread_mutex_init ( __mutex, NULL ) 

#ifdef __DOOLP_DUMPBUFFERS
#define _dumpBuffer(_buff, _sz )		    \
  printf ( "**Dumping %d bytes\n", _sz );	    \
  if ( _buff == NULL )				    \
    Bug ( "Dont want to dump a null buffer\n" );    \
  for ( int _f = 0 ; _f <= _sz/4 ; _f++ )	    \
    {						    \
      if ( _f % 4 == 0 ) printf ( "\n" );			\
      printf ( "%d:%p\t", _f * 4, ((void**)_buff)[_f] );	\
    }								\
  if ( _sz % 16 != 1 ) printf ( "\n" );
#else 
#define _dumpBuffer(_buff,_sz) 
#endif // __DOOLP_Log


#endif // __DOOLP_CONFIG_H
