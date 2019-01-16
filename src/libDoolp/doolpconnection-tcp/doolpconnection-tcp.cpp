#include <doolp/doolp-doolpconnection-tcp.h>
#include <doolp/doolp-doolpconnection-tcp-msg.h>
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpjob.h>
#include <doolp/doolp-doolpcall.h>
#include <doolp/doolp-doolpcallcontext.h>

DoolpConnectionTCP::DoolpConnectionTCP ()
{
  __DOOLP_Log ( "New empty(raw) DoolpConnectionTCP at %p\n",
		this);
}


DoolpConnectionTCP::DoolpConnectionTCP ( DoolpForge * _forge )
{
  __initTCP ( _forge );
  strncpy ( identifier.name,
	   "Doolp::TCP::[BIN]::1", 32 );
  identifier.version = 0;
  identifier.subversion = 1;
  identifier.patchlevel = 5;
  __DOOLP_Log ( "New Connection TCP at %p\n", this );
}

DoolpConnectionTCP::DoolpConnectionTCP (DoolpForge * _forge, Socket _mySocket)
{
  __initTCP ( _forge );
  strncpy ( identifier.name,
	   "Doolp::TCP::[BIN]::1", 32 );
  identifier.version = 0;
  identifier.subversion = 1;
  identifier.patchlevel = 5;
  mySocket = _mySocket;
  __DOOLP_Log ( "New Connection TCP at %p, socket=%d\n", this, mySocket );
}


void DoolpConnectionTCP::__initTCP ( DoolpForge * _forge )
{
  __init ( _forge );
#ifdef __DOOLPCONNECTIONTCP_WRITEBUFF
  writeBuffSz = 0;
#endif // __DOOLPCONNECTIONTCP_WRITEBUFF
  retryNb = 5;
  retryWait = 100;
  //  isSubHandled = false;
  nextBlockIndex = 0;
  blockHeaderIsRead = false;
  //  sem_init ( &handleThreadEnd, 0, 0 );
}

bool DoolpConnectionTCP::tryConnect ( char * host, int port )
  // TODO : implement DNS resolution of host name
{
	int res;
	struct sockaddr_in AdrServ;

	__DOOLP_Log ( "Connecting to : %s:%d\n", host, port );

	int __sockAddrSz__ = sizeof ( struct sockaddr );

	if (  (mySocket = socket(PF_INET, SOCK_STREAM, 0)) <= -1)
		{ Fatal ( "Unable to create client socket" ); }
	memset(&AdrServ,0,sizeof AdrServ);
	AdrServ.sin_port = htons(port);
	AdrServ.sin_family = PF_INET;
	inet_aton(host,&(AdrServ.sin_addr));

	if ( (res = connect(mySocket, (struct sockaddr *) &AdrServ, __sockAddrSz__ )) <= -1 )
		{ Fatal ("Unable to connect() : result %d, errno %d\n", res, errno ); }

	Info ( "New connection (%s:%d)\n", host, port );
	this->name = host; // Should maybe strcpy this...
	// this->port= port;

	// Here we enter a standardized bi-directionnal struct DoolpAgent building section.
	// Handshaking
	if ( ! handShake () )
	  {
	    Error ( "Could not handshake\n" );
	    return false;
	  }
	myForge->addConnection ( this );
	if ( myForge->getDefaultDistAgentId () == 0 )
	  myForge->setDefaultDistAgentId ( distAgentId );

	status = DoolpConnectionStatus_Ready;
	handle ();
	return true;
}

bool DoolpConnectionTCP::handShake ()
{
  status = DoolpConnectionStatus_Handshaking;

  // First, send Connection Identifier
  lockRead ();
  lockWrite ();
  rawWrite ( &identifier,
	     sizeof ( DoolpConnectionTCP_Identifier ) );
  flushWrite ();
  // Recieve Connection Identifier
  DoolpConnectionTCP_Identifier distIdentifier;
  rawRead ( &distIdentifier,
	    sizeof ( DoolpConnectionTCP_Identifier ) );
  distIdentifier.name[31] = 0;
  __DOOLP_Log ( "Recieved DoolpConnectionTCP Identifier : \"%s\"\n",
		distIdentifier.name );
  __DOOLP_Log ( "Version \"%u.%u.%u\".\n",
		distIdentifier.version,
		distIdentifier.subversion,
		distIdentifier.patchlevel );

  // Should perform some checking about this remote Identifer.


  // Then, send your own agentId (zero if not set)
  DoolpAgentId myAgentId = myForge->getAgentId ();
  rawWrite ( &myAgentId,
	     sizeof ( DoolpAgentId ) );
  flushWrite ();

  // Recieve distant agentId
  rawRead ( &distAgentId, sizeof ( DoolpAgentId ) );
  __DOOLP_Log ( "Recieved dist AgentId : %d\n", distAgentId );

  if ( myAgentId == 0 && distAgentId == 0 )
    Bug ( "myAgentId and distAgentId are both null, this shall not happen.\n" );
    
  //  DoolpAgent * distAgent = new DoolpAgent ();
  if ( distAgentId == 0 )
    {
      __DOOLPCONNECTIONTCP_Log ( "Distant agentId is null, shall index it.\n" );
      distAgentId = myForge->getFreeAgentId ();
      if ( distAgentId == 0 )
	{
	  Fatal ( "Could not index distant agent.\n" );
	}
      rawWrite ( &distAgentId, sizeof ( DoolpAgentId ) );
      flushWrite ();
    }
  if ( myAgentId == 0 )
    {
      rawRead ( &myAgentId, sizeof ( DoolpAgentId ) );
      __DOOLPCONNECTIONTCP_Log ( "Recieved agentId : %d\n",
		    myAgentId );
      myForge->setAgentId ( myAgentId );
    }

  // Finished Handshaking
  status = DoolpConnectionStatus_Handshaked;
  unlockWrite ();
  unlockRead ();
  __DOOLPCONNECTIONTCP_Log ( "New Connection (this %p): me=%d, dist=%d\n",
		this, myAgentId, distAgentId );
  return true;
}

bool DoolpConnectionTCP::checkIdentifier (  DoolpConnectionTCP_Identifier * distIdentifier )
{
  if ( strncmp ( identifier.name, distIdentifier->name, 32 ) != 0 )
    {
      Warn ( "Invalid identifier : '%s'\n", distIdentifier->name );
      return false;
    }
  return true;
}

DoolpConnectionTCP::~DoolpConnectionTCP ( )
{
  __DOOLPCONNECTIONTCP_Log ( "Destruction of %p : lockRead and lockWrite OK\n", this );
  close ( mySocket );
  mySocket = 0;
  destroyMutexes ();
  // dmalloc_log_unfreed ();
}


bool DoolpConnectionTCP::endConnection ( )
{
  status = DoolpConnectionStatus_Disconnected;
#ifndef __DOOLP_USE_DOOLPTHREADS
  pthread_cancel ( handleThread); //, SIGKILL );
#endif 
  usleep ( 100 );
  DoolpMsg_Header header;
  memset ( &header, 0, sizeof ( DoolpMsg_Header ) );
  header.callFlags = DoolpMsg_CallFlag_EndConnection;
  lockWrite ();
  rawWrite  ( &header, sizeof ( DoolpMsg_Header) );
  unlockWrite ();
  return true;
}

/* Read/Write */
ssize_t DoolpConnectionTCP::rawRead(void *buf, size_t nbyte)
{ 
	if ( ! isLockRead () ) 
	  { Bug ( "Not locked for read !\n" ); }

	ssize_t res = read ( mySocket, buf, nbyte );
	__DOOLPCONNECTIONTCP_Log ( "Sock %p:%d : Read %d/%d\n", this, mySocket, res, nbyte );
	if ( res > 0 ) goto readOk;
	if ( res < 0 ) goto readError;
	for ( unsigned int _try = 0 ; _try < retryNb ; _try++)
	{
		__DOOLP_Log ( "Trying to wait...(try %d/%d)\n", _try, retryNb );

		if ( ( res = waitRead () ) < 0 )
			goto readError;

		res = read ( mySocket, buf, nbyte );
		__DOOLPCONNECTIONTCP_Log ( "Read gave result %d\n", res );
		if ( res > 0 ) goto readOk;
		if ( res < 0 ) break;
		usleep ( retryWait );
	}
 readError:
	LogErrNo;
	status = DoolpConnectionStatus_Lost;
#ifdef __DOOLP_DOOLPCONNECTION_READERROR_IS_FATAL
	Fatal ( "Read error is set fatal, exit. connection=%p, socket=%d, res=%d.\n", 
		this, mySocket, res );
#endif
	return 0;
 readOk:
	__DOOLPCONNECTIONTCP_Log ( "Read Ok : read %d bytes on %d\n", 
		      res, nbyte );
#ifdef __DOOLPCONNECTION_DUMPREAD
	_dumpBuffer ( buf, res );
#endif

	return res;
}


ssize_t DoolpConnectionTCP::rawWrite (const void *buf, size_t nbyte)
{ 
	if ( ! isLockWrite () ) 
	  { Bug ( "Not locked for write !\n" ); }

#ifdef __DOOLPCONNECTIONTCP_WRITEBUFF
	ssize_t res = nbyte;
	if ( nbyte > __DOOLPCONNECTIONTCP_WRITEBUFF_SZ ) 
	  { Fatal ( "Buffer too little (%d) for msg size : %d\n", 
		    __DOOLPCONNECTIONTCP_WRITEBUFF_SZ, nbyte ); }
	if ( (writeBuffSz + nbyte) > __DOOLPCONNECTIONTCP_WRITEBUFF_SZ ) 
	  { /* writeBuff_flush();*/ 
	    Bug ( "Should pre-Flush Here\n"); 
	  }  
	memcpy ( writeBuff + writeBuffSz, buf, nbyte );
	writeBuffSz += nbyte;

#else // __DOOLPCONNECTIONTCP_WRITEBUFF
	ssize_t res = write ( mySocket, buf, nbyte );
#endif // __DOOLPCONNECTIONTCP_WRITEBUFF
        __DOOLPCONNECTIONTCP_Log ( "Sock %p:%d : Write %d/%d\n", this, mySocket, res, nbyte ); 
	if ( res <= 0 ) 
	{ 
		LogErrNo;
		status = DoolpConnectionStatus_Lost;
	}
	return res; 
}

bool DoolpConnectionTCP::flushWrite ( )
{
#ifdef __DOOLPCONNECTIONTCP_WRITEBUFF
  __DOOLPCONNECTIONTCP_Log ( "Flushing buffer (buffer sz %d/%d)\n", 
			     writeBuffSz, __DOOLPCONNECTIONTCP_WRITEBUFF_SZ );
#ifdef __DOOLPCONNECTIONTCP_DUMPWRITEBUFF
  _dumpBuffer(writeBuff, writeBuffSz );
#endif // __DOOLPCONNECTIONTCP_DUMPWRITEBUFF
  if ( writeBuffSz > 0 ) 
    { 
      ssize_t __res = write 
	( mySocket, writeBuff, writeBuffSz );	
      __DOOLPCONNECTIONTCP_Log ( "Written %d bytes.\n", __res );
      if ( __res != writeBuffSz ) 
	{ Error ( "%p could not write full buffer\n", this ); return false; } 
      writeBuffSz = 0;							
    }
#else // __DOOLPCONNECTIONTCP_WRITEBUFF
  __DOOLPCONNECTIONTCP_Log ( "FlushWrite : not implemented.\n" );
#endif // __DOOLPCONNECTIONTCP_WRITEBUFF
  return true;
}


/* Waiting for Read Mechanisms.. */
int DoolpConnectionTCP::waitRead ( int timeout_sec, int timeout_usec )
{
  fd_set fdSet;
  struct timeval TimeToWait;
  
  TimeToWait.tv_sec = timeout_sec;
  TimeToWait.tv_usec = timeout_usec;
  
  FD_ZERO( &fdSet );
  FD_SET( mySocket, &fdSet );
  __DOOLPCONNECTIONTCP_Log ( "Waiting connection read %p:%d\n", this, mySocket );
  int result = select( mySocket+1, &fdSet, NULL, NULL, &TimeToWait );
  if ( result == 0 ) return result;
  if ( FD_ISSET ( mySocket, &fdSet ) != 0 )
    {
      __DOOLPCONNECTIONTCP_Log ( "New data on socket %d\n", mySocket );
      return 1;
    }
  return result;
}

int DoolpConnectionTCP::waitRead ( )
{ return waitRead ( 2, 0 ); }





/*
 * waitSpecificCall Mechanism
 */
#if 0
bool DoolpConnectionTCP::waitSpecificCall ( DoolpCall * call )
{
  if ( handleThread == pthread_self () )
    {
      __DOOLPCONNECTIONTCP_Log ( " : in same thread than the handleThread ! \n" );
    }
  else
    {
      __DOOLPCONNECTIONTCP_Log ( "Trying to waitSpecificCall in a different thread than the handle one\n" );
      if ( isSubHandled )
	{
	  if ( subHandleThread != pthread_self() )
	    {
	      Warn ( "In DoolpConnectionTCP:%p : connection already handled by another waitSpecificCall\n", this );
	      Warn ( "Handled by thread 0x%x (here i am 0x%x)\n", (int)subHandleThread, (int)pthread_self() );
	      return false;
	    }
	  __DOOLPCONNECTIONTCP_Log ( "Connection already subHandled by this thread, nothing to do\n" );
	  subHandleLevel++;
	  __DOOLPCONNECTIONTCP_Log ( "subHandleLevel=%d\n", subHandleLevel );
	}
      else // ( ! isSubHandled )
	{
	  isSubHandled = true;
	  subHandleLevel = 1;
	  subHandleThread = pthread_self ();
	  __DOOLPCONNECTIONTCP_Log ( "Lock Reading.\n" );
	  lockRead ();
	  __DOOLPCONNECTIONTCP_Log ( "I have been lockRead.\n" );
	  if ( call->replied == true )
	    {
	      __DOOLPCONNECTIONTCP_Log ( "Call has been replied while I was lockRead ()\n" );
	      unlockRead ();
	      isSubHandled = false;
	      return true;
	    }
	}
    }
  __DOOLPCONNECTIONTCP_Log ( "Trying to runHandleThread ( call=%p );\n", call );
  bool res = runHandleThread ( call );
  if ( isSubHandled )
    {
      __DOOLPCONNECTIONTCP_Log ( "isSubHandled=true : have given result res=%d\n", res );
      subHandleLevel--;
      __DOOLPCONNECTIONTCP_Log ( "now subHandleLevel=%d\n", subHandleLevel );
      if ( subHandleLevel == 0 )
	{
	  __DOOLPCONNECTIONTCP_Log ( "Cleaning up all sub handling data\n" );
	  subHandleThread = 0;
	  isSubHandled = false;
	  unlockRead ();
	}
    }
  return res;
}
#endif
