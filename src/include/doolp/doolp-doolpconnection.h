#ifndef __DOOLP_DOOLPCONNECTION_H
#define __DOOLP_DOOLPCONNECTION_H

#include <string.h>
#include <errno.h>

#include <pthread.h>
#include <semaphore.h>
#include <list>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <doolp/doolp-doolpclasses.h>
// #include <doolp/doolp-doolpobject.h>

using namespace std;

namespace Doolp
{
  typedef enum ConnectionStatus
    {
      ConnectionStatus_Unknown           = 0,
      ConnectionStatus_Connecting        = 1,
      ConnectionStatus_ToBeHandShaked    = 2,
      ConnectionStatus_Handshaking       = 3,
      ConnectionStatus_Handshaked        = 4,
      ConnectionStatus_ToBeAuthenticated = 5,
      ConnectionStatus_Authenticated     = 6,
      ConnectionStatus_Ready             = 7,
      ConnectionStatus_Busy              = 8,
      ConnectionStatus_Lost              = 9,
      ConnectionStatus_Disconnected      = 11,
      ConnectionStatus_QueryDisconnect   = 12,
      ConnectionStatus_QueryDisconnectOK = 13,
      ConnectionStatus_ToBeDeleted       = 14
    };
  extern char * ConnectionStatus_Str[];
  class Connection
  {
    friend void * DoolpConnection_runHandleThread(void*);
  private:
    pthread_t handleThread;
    pthread_mutex_t * readMutex, * writeMutex;
    pthread_t readThread, writeThread; // Threads that actually read or write lock Mutexes.
    
  protected:
    bool Connection::__init ( Forge * _forge );
    CallContext * currentCallContext; // To be filled in the implementations...
  public:
    char * name;
    ConnectionStatus status;
    AgentId distAgentId; // agentId at the other side of connection
    bool disableGetConnectionForAgentId;
    
    Forge * myForge;
    
    bool Connection::destroyMutexes ();
    virtual Connection::~Connection ( ) 
    { __DOOLP_Log ( "DoolpConnection Destructor for %p\n", this ); }
    
    ConnectionStatus getStatus () { return status; }
    
    virtual bool endConnection ( ) = 0;
    
    /* 
     * Locking Mechanims : default behaviour
     */
    virtual bool tryLockRead ();
    virtual bool lockRead ();
    virtual bool unlockRead ();
    virtual bool isLockRead ();
    
    virtual bool lockWrite ();
    virtual bool unlockWrite ();
    virtual bool isLockWrite ();
    
    /*
     * Connection Handling
     * handle() creates a new thread, that runs the runHandleThread() function
     */
  protected:
    pthread_t subHandleThread;
    unsigned int subHandleLevel;
    void callRunHandleThread();
  public:
    void handle();
    virtual bool handShake () = 0;
    
    virtual bool forwardMessage ( Connection * toConnection ) = 0;
    
    /*
     * runHandleThread() Implementation-dependant handling function
     * 0 : error, 1 : continue, 2 : quit because of specificCall
   */
    virtual bool runHandleThread (Call * specificCall, StreamVirtual * specificStream) = 0; 
    
    sem_t handleThreadCanceledSemaphore;
    void cancelHandleThread ();
    void setHandleThreadCanceled ();
    
    /*
     * flushWrite() virtual func
     */ 
    virtual bool flushWrite () = 0;
    
    /*
     * Reading and Writing prototypes
     */ 
    bool Write ( Object * obj );

    bool Write ( ObjectLink& l );
    bool Read ( ObjectLink& l );

    template <class T>
      bool Read ( T ** obj )
      { return Read ( (Object**) obj, false, false ); }
    template<typename T1,typename T2>
      bool Write ( pair<T1,T2> &p )
    {
      Write ( p.first );
      Write ( p.second );
      return true;
    }
    template<typename T1,typename T2>
      bool Read ( pair<T1,T2> *p )
    {
      T1 t1;
      T2 t2;
      Read ( &t1 );
      Read ( &t2 );
      *p = make_pair (t1, t2);
      return true;
    }
    template<typename T> 
      bool Write ( list<T> * l )
      {
	startList ( 0 );
	// T t = new T;
	if ( l != NULL )
	  {
	    typename list<T>::iterator t;
	    for ( t = l->begin () ; t != l->end() ; t++ )
	      {
		__DOOLP_Log ( "Writing op '%p'\n", &(*t) );
		Write ( *t );
		__DOOLP_Log ( "Written.\n" );
	      }
	    __DOOLP_Log ( "List finished.\n" );
	    
	  }
	endSubSection ( );
	return true;
      }
    template<typename T> 
      bool Read ( list<T> ** l )
      {
	*l = new list<T>;
	// int idx = 
	readList ();
	while ( ! readSubSectionEnd () )
	  {
	    T * t = new T;
	    Read ( t );
	    (*l)->push_back ( *t );
	  }
	return true;
      }
#define __DOOLP_VIRTUAL__ virtual
#define __DOOLP_VIRTUAL_IMPL__ = 0;
#include <doolp/doolp-doolpconnection-virtualfuncs.h>
  };
}; // NameSpace Doolp
#endif // __DOOLP_DOOLPCONNECTION_H
