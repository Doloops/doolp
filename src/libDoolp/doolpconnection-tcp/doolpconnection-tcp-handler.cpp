#include <doolp/doolp-doolpconnection-tcp.h>
#include <doolp/doolp-doolpconnection-tcp-msg.h>

#include <doolp/doolp-doolpforge.h>

// This is the handler for TCP Connections
extern char * DoolpConnectionStatus_Str[];

bool DoolpConnectionTCP::runHandleThread ( DoolpCall * specificCall, DoolpStreamVirtual * specificStream )
{
  __DOOLP_Log ( "runHandleThread begins.\n" );
  __DOOLP_Log ( "status=%d\n", status );
  AssertBug ( status != DoolpConnectionStatus_Connecting, "Shall not be here with status=Connecting.\n" );
  if ( status == DoolpConnectionStatus_ToBeHandShaked )
    {
      __DOOLP_Log ( "handShaking this connection\n" );
      AssertBug ( specificCall == NULL, "Waiting specificCall in a non-handshaked connection ?\n" );
      if ( !  handShake () )
	{
	  Error ( "Could not handShake connection from %s\n", name );
	  return false;
	}
      myForge->addConnection ( this );
      status = DoolpConnectionStatus_ToBeAuthenticated;
    }
  else
    {
      __DOOLP_Log ( "Connection already handshaked\n" );
    }
  int i = 0;

  bool haveto_tryLockRead = true;
  if ( specificCall != NULL )
    haveto_tryLockRead = false;
  while (true)
    {
      switch ( status )
	{
	case DoolpConnectionStatus_QueryDisconnect:
	  status = DoolpConnectionStatus_QueryDisconnectOK;
	  goto endOfHandling;
	case DoolpConnectionStatus_Disconnected:
	case DoolpConnectionStatus_Lost:
	  Warn ( "Connection set Disconnected/lost (status=%d).\n", status ) ; 
	  goto endOfHandling;
	case DoolpConnectionStatus_Ready:
	  break;
	case DoolpConnectionStatus_ToBeAuthenticated:
	  status = DoolpConnectionStatus_Ready; // Try to pass through this awfull lack of implementation !!
	  break;
	default:
	  Bug ( "Invalid connection status : %d\n", status );
	  
	}
      
      int result = waitRead ( 0, 100 );
      i++;
      if( result < 0 && errno != 0 )
	{ 
	  Error ( "waitRead Error : connection finished ?\n" ); 
	  __DOOLP_Log ( "Error %d:%s\n", errno, strerror (errno) );
	  status = DoolpConnectionStatus_Lost;
	  continue;
	}
      if ( result == 0 )
	{
	  continue;
	}
      if ( status != DoolpConnectionStatus_Ready )
	continue;
      __DOOLP_Log ( "Operation cycle %d\n", i );

      // Ifever Connection has been locked by another thread, we should skip this msg
      if ( haveto_tryLockRead )
	{
	  if ( tryLockRead () == false ) 
	    {
	      __DOOLP_Log ( "%p:%d - already lockRead, I shall not handle this message\n", this, mySocket );
	      lockRead ();
	      unlockRead ();
	      continue;
	    }
	}
      if ( status == DoolpConnectionStatus_Disconnected )
	{ Warn ( "Connection set Disconnected.\n") ; return false; }
      __DOOLP_Log ( "Handling message\n" );
      switch ( handleMsg ( specificCall, specificStream ) )
	{
	case 0:
	  __DOOLP_Log ( "Input Error !\n" );
	  goto endOfHandling;
	case 1:
	  __DOOLP_Log ( "Read Ok.\n" );
	  break;
	case 2:
	  __DOOLP_Log ( "Recieved specificCall !\n" );
	  return true;
	}
      if ( haveto_tryLockRead )
	unlockRead ();
    }
 endOfHandling:
  __DOOLP_Log ( "Terminating connection %p to %d\n", this, distAgentId );
  __DOOLP_Log ( "Forge is %p\n", myForge );
  if ( myForge != NULL )
    myForge->removeAgent ( distAgentId );
  return false;
}


