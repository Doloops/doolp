#include <doolp/doolp-doolpconnection.h>
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpcontext.h>
#include <doolp/doolp-doolpexception.h>
#include <doolp/doolp-doolpobjectdynamicinfo.h> // For removeAgent()
#include <doolp/doolp-doolpobject-linkvirtual.h>

char * Doolp::ConnectionStatus_Str[] = 
  {
    "Unknown", 
    "Connecting", 
    "ToBeHandShaked",
    "Handshaking",
    "Handshaked",
    "ToBeAuthenticated",
    "Authenticated", 
    "Ready", 
    "Busy", 
    "Lost",
    "Disconnected",
    "QueryDisconnect",
    "QueryDisconnectOK",
    "ToBeDeleted"
  };


bool Doolp::Connection::__init ( Forge * _forge )
{ 
  AssertBug ( _forge != NULL, "Can not create Doolp::Connection with forge == NULL !\n" );

  myForge = _forge;
  status = Doolp::ConnectionStatus_Connecting; 
  readThread = 0;
  writeThread = 0;
  distAgentId = 0;
  subHandleLevel = 0;
  disableGetConnectionForAgentId = false;
  DOOLP_PTHREAD_MUTEX_INIT( readMutex );
  DOOLP_PTHREAD_MUTEX_INIT( writeMutex );
  sem_init ( &(handleThreadCanceledSemaphore), 0, 0 );
  return true;
}

bool Doolp::Connection::destroyMutexes ()
{
  pthread_mutex_unlock ( readMutex );
  pthread_mutex_unlock ( writeMutex );

  pthread_mutex_destroy ( readMutex );
  pthread_mutex_destroy ( writeMutex );
  free ( readMutex );
  free ( writeMutex );
  return true;
}

#define __DOOLPCONNECTION_LOCK_Log(...) // __DOOLP_Log (__VA_ARGS__)
/* 
 * Locking Mechanisms 
 */
bool Doolp::Connection::tryLockRead ()
{
  __DOOLPCONNECTION_LOCK_Log ( "Trying to tryLockRead 0x%x for thread 0x%x\n", this, pthread_self () );
  if ( pthread_mutex_trylock ( readMutex ) != 0 )
    return false;
  readThread = pthread_self ();
  __DOOLPCONNECTION_LOCK_Log ( "LockRead 0x%x for thread 0x%x\n", this, pthread_self () );
  return true;
}

bool Doolp::Connection::lockRead ( )
{
  __DOOLPCONNECTION_LOCK_Log ( "Trying to lockRead 0x%x for thread 0x%x\n", this, pthread_self () );
  pthread_mutex_lock ( readMutex ); readThread = pthread_self ();
  __DOOLPCONNECTION_LOCK_Log ( "LockRead 0x%x for thread 0x%x\n", this, pthread_self () );
  return true;
}

bool Doolp::Connection::unlockRead ()
{
  __DOOLPCONNECTION_LOCK_Log ( "UnlockRead 0x%x by thread 0x%x\n", this, pthread_self () );
  AssertBug ( isLockRead (), "Thread that locked (0x%x) is not thread that unlocks (0x%x)",
	      (int)readThread, (int)pthread_self () );
  pthread_mutex_unlock ( readMutex ); readThread = 0;
  return true;
}

bool Doolp::Connection::lockWrite ( )
{
  if ( writeThread != 0 )
    Warn ( "Connection already locked by 0x%x\n", (int)writeThread );
       
  __DOOLPCONNECTION_LOCK_Log ( "Trying to lockWrite 0x%x for thread 0x%x\n", this, pthread_self () );
  pthread_mutex_lock ( writeMutex ); 
  writeThread = pthread_self ();
  __DOOLPCONNECTION_LOCK_Log ( "LockWrite 0x%x for thread 0x%x\n", this, pthread_self () );
  return true;
}

bool Doolp::Connection::unlockWrite ()
{
  __DOOLPCONNECTION_LOCK_Log ( "UnlockWrite 0x%x by thread 0x%x\n", this, pthread_self () );
  AssertBug ( isLockWrite (),"Thread that locked (0x%x) is not thread that unlocks (0x%x)\n",
	      (int)writeThread, (int)pthread_self () ); 
  flushWrite ();
  writeThread = 0;
  pthread_mutex_unlock ( writeMutex ); 
  return true;
}


bool Doolp::Connection::isLockRead ()
{
  if ( readThread != pthread_self () )
    {
      Warn ( "0x%x not locked for Read by thread 0x%x ! (locked by 0x%x)\n",
	     (int)this, (int)pthread_self (), (int)readThread );
      return false;
    }
  return true;
}

bool Doolp::Connection::isLockWrite ()
{
  if ( writeThread != pthread_self () )
    {
      Warn ( "0x%x not locked for Write by thread 0x%x ! (locked by 0x%x)\n",
	     (int)this, (int)pthread_self (), (int)writeThread );
      return false;
    }
  return true;
}

/*
 * Generic way to write Objects.
 */
bool Doolp::Connection::Write ( Doolp::Object * obj )
{ 
  AssertBug ( currentCallContext != NULL, "Current Call Context is not set !\n" );
  return Write ( obj, currentCallContext->serializeFromStepping ); 
}


bool Doolp::Connection::Write ( ObjectLink& l ) { return l.serialize ( this ); }
bool Doolp::Connection::Read ( ObjectLink& l ) { return l.unserialize ( this ); }



/*
 * Handling thread mecanism
 */

void * Doolp::DoolpConnection_runHandleThread(void* arg)
{
  ((Doolp::Connection*)arg)->callRunHandleThread();
  return NULL;
}

void Doolp::Connection::callRunHandleThread()
{
  if ( status == Doolp::ConnectionStatus_ToBeHandShaked )
    {
      try
	{
	  __DOOLP_Log ( "handShaking this connection\n" );
	  if ( !  handShake () )
	    {
	      Error ( "Could not handShake connection from %s\n", name );
	      return ;
	    }
	  myForge->addConnection ( this );
	  status = Doolp::ConnectionStatus_ToBeAuthenticated;
  	}
      catch ( Doolp::Exception * e )
	{
	  Warn ( "Got Exception '%s'\n", e->getMessage () );
	  delete (e);
	  return;
	}
    }
  else
    {
      __DOOLP_Log ( "Connection already handshaked\n" );
    }
  AssertBug ( status == Doolp::ConnectionStatus_ToBeAuthenticated,
	      "HandShaking error : status=%d\n", status );
  
  // TODO : implement authentication.
  status = Doolp::ConnectionStatus_Ready;
  myForge->getContext()->setHandledConnection ( this );
  try
    {
      runHandleThread(NULL, NULL);
    }
  catch ( Doolp::Exception * e )
    {
      Warn ( "Got Exception '%s'\n", e->getMessage () );
      delete (e);
    }
  Log ( "End of Connection for '%d'\n", distAgentId );
  myForge->removeConnection ( this );
  myForge->getObjectDynamicInfo()->removeAgent ( distAgentId );
}

void Doolp::Connection::handle()
{
  myForge->newThread ( &handleThread, NULL, DoolpConnection_runHandleThread, this );
  pthread_detach ( handleThread );
}
 
void Doolp::Connection::cancelHandleThread ()
{
  status = Doolp::ConnectionStatus_QueryDisconnect;
  sem_wait ( &handleThreadCanceledSemaphore );
}
 
void Doolp::Connection::setHandleThreadCanceled ()
{
  status = Doolp::ConnectionStatus_QueryDisconnectOK;
  sem_post ( &handleThreadCanceledSemaphore );
}
