/* 
 * DoolpStream General API
 * From step-2.5
 */

#ifndef __DOOLP_DOOLPSTREAM_H_
#define __DOOLP_DOOLPSTREAM_H_

#include <doolp-config.h>
#include <doolp/doolp-doolpclasses.h>
// #include <doolp/doolp-doolpcall.h>
#include <doolp/doolp-doolpcontext.h>
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpexception.h>

#include <list>
#include <map>
using namespace std;

#include <semaphore.h>

namespace Doolp
{

  typedef unsigned int StreamIndex;
  typedef unsigned int StreamBindOptions;
#define StreamBindOption_Read  0x0001
#define StreamBindOption_Write 0x0002
#define StreamBindOption_RdWr  0x0003
  // ...

#define __DOOLPSTREAM_Log Log

  class StreamVirtual
  {
  protected:
    map<CallContext*,StreamIndex> bindedContexts;
    Forge * myForge; // This is a fake Forge computed over bind...
    // Generic Functions for all kinds of Streams
  
    // Binding/Unbinding functions.
    // The connection provided is a prefered connection for writing
    // But it can be recieved by any other conn.
  public:
    StreamVirtual ();
    bool bind ( Connection * conn, CallContext * callContext, StreamIndex idx, StreamBindOptions options );
    bool unbind ( CallContext * callContext );
    bool unbind ( ); // unbind everyone.
  protected:
    virtual bool preUnbind ( CallContext * callContext ) = 0;

    // Read mechanism
  public:
    virtual bool readFrom ( Connection * connection, CallContext * callContext ) = 0;
  
    // Flushing 
    virtual bool flush ( Connection * connection ) = 0; // Flushes to one connection.
    virtual bool flush ( ) = 0; // Flushes to every connection.
    virtual ~StreamVirtual ( )
    {
      __DOOLPSTREAM_Log ( "Destruction of %p\n", this );
    }
    virtual bool isFinished () = 0;
    virtual bool callFinish ( Call * call ) = 0;
  };

  template<typename T_obj>
    class Stream : public StreamVirtual
  {
  protected:
    unsigned int sendBuffSz;
    list<T_obj> sendBuff;
    list<T_obj> recvBuff;
    bool _isFinished, _isWaiting;
    sem_t semaphore;
    Call * callFinishedInEmergency;
    bool waitRecv ( )
    {
      if ( _isWaiting )
	Bug ( "Already waiting !!!\n" );
      _isWaiting = true;
      AssertBug ( myForge != NULL, "waitRecv() but not binded to any Forge !!\n" );
      myForge->getContext()->backgroundWork ( this );
      sem_wait ( &semaphore );
      _isWaiting = false;
      checkCallFinished ();
      return true;
    }
    inline void checkCallFinished ()
      {
	if ( callFinishedInEmergency != NULL )
	  {
	    __DOOLPSTREAM_Log ( "Has an emergency exit !\n" );
	    if ( callFinishedInEmergency->exception ) 
	      {
		__DOOLPSTREAM_Log ( "Has an emergency exception !\n" );
		throw callFinishedInEmergency->exception;
	      }
	  }
      }
  public:
    Stream ( )
      {
	_isWaiting = false;
	_isFinished = false;
	sem_init ( &semaphore, 0, 0 );
	sendBuffSz = 3;
	callFinishedInEmergency = NULL;
      }
    virtual bool readFrom ( Connection * connection, CallContext * callContext )
    {
      if ( _isFinished )
	Bug ( "This Stream is Finished and I still have data for it.\n" );
      if ( connection->readStreamEnd () )
	{ 
	  _isFinished = true ; 
	  sem_post ( &semaphore ) ; 
	  return true; 
	}
      T_obj t;
      connection->Read ( &t );
      recvBuff.push_back ( t );
      sem_post ( &semaphore );
      return true;
    }
    bool flush ( Connection * connection )
    { return flush(); }

    typedef typename list<T_obj>::iterator list_T_iterator;
    inline bool flush ( CallContext * callContext,
			StreamIndex idx )
      {
	__DOOLPSTREAM_Log ( "isFinished=%d, sendBuff=%d\n",
			    _isFinished, sendBuff.size() );
	if ( (!_isFinished) && (sendBuff.size () == 0) )
	  {
	    __DOOLPSTREAM_Log ( "Nothing to flush()\n" );
	    return true;
	  }
	Connection * conn = callContext->preferedConn;
	conn->setStream ( callContext, idx );
	while ( sendBuff.size () > 0 )
	  {
	    conn->Write ( sendBuff.front () );
	    sendBuff.pop_front ();
	  }
	if ( _isFinished )
	  {
	    __DOOLPSTREAM_Log ( "endStream()\n" );
	    conn->endStream ();
	  }
	conn->leaveStream ();
	return true;
      }

    bool flush ( )
    { 
      __DOOLPSTREAM_Log ( "Flushing : %d elements in queue.\n",
			  sendBuff.size () );
      map<CallContext *,StreamIndex>::iterator ctx;
      for ( ctx = bindedContexts.begin ();
	    ctx != bindedContexts.end () ;
	    ctx ++ )
	{
	  if ( ! flush ( ctx->first, ctx->second ) )
	    return false;
	}
      return true; 
    }
 
  protected:
  
    bool finish ( CallContext * callContext, StreamIndex idx )
    {
      Connection * conn = callContext->preferedConn;
      conn->setStream ( callContext, idx );
      conn->endStream ( );
      conn->leaveStream ( );
      return true;
    }
    bool callFinish ( Call * call )
    {
      // Call finished, so clean up !!
      // Be carefull when several calls are involved !!
      __DOOLPSTREAM_Log ( "stream : waiting=%d finished=%d\n", _isWaiting, _isFinished );
      __DOOLPSTREAM_Log ( "setting callFinishedInEmergency !\n" );
      if ( ! _isFinished )
	{
	  callFinishedInEmergency = call;
	  sem_post ( &semaphore );
	}
      return true;
    }

    bool preUnbind ( CallContext * callContext )
    {
      __DOOLPSTREAM_Log ( "preUnbinding %p (from callContext %p)\n",
			  this, callContext );
      // We should say him the stream is finished if it is not already said so...
      if ( _isFinished )
	return true;
      flush ();
      finish ( callContext, bindedContexts [ callContext ] );
      return true;
    }  
  public:
    bool push ( T_obj t )
    {
      checkCallFinished ();
      // TODO
      __DOOLPSTREAM_Log ( "before push ; buff : %d, sendBuffSz %d\n",
			  sendBuff.size (), sendBuffSz );
      sendBuff.push_back ( t );
      if ( sendBuff.size () >= sendBuffSz )
	flush ();
      return true;
    }
    bool push ( T_obj &t )
    {
      Bug ( "NOT IMPLEMENTED (and no idea to do so) !!!!\n" );
      return true;
    }
    bool canPop ()
    {
      checkCallFinished ();
      if ( recvBuff.size () > 0 )
	return true;
      return false;    
    }
    bool pop ( T_obj & t)
    {
      checkCallFinished ();
      if ( recvBuff.size () == 0 )
	{
	  __DOOLPSTREAM_Log ( "recvBuff is empty  : calling waitRecv()\n" );
	  waitRecv ();
	}
      __DOOLPSTREAM_Log ( "recvBuff.size() = %d\n", recvBuff.size () );
    
      t = recvBuff.front ( );
      recvBuff.pop_front ();
      return true;
    }
    /*
      T& pop ( )
      { 
      T t; pop ( t ); return t; 
      }
    */
    bool setFinished ()
    {
      if ( _isFinished )
	return true; 
      __DOOLPSTREAM_Log ( "STREAM FINISHED : %p\n", this );
      _isFinished = true;
      flush();
      return true;
    }
    bool isFinished ()
    {
      // TODO 
      while ( true )
	{
	  if ( recvBuff.size () != 0 )
	    {
	      return false;
	    }
	  if ( _isFinished )
	    return true;
	  waitRecv ();
	}
      return false;
    }
  };

}; // NameSpace Doolp
#endif // __DOOLP_DOOLPSTREAM_H_
