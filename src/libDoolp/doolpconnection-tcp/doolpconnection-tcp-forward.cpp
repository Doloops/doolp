#include <doolp/doolp-doolpconnection-tcp.h>
#include <doolp/doolp-doolpconnection-tcp-msg.h>


bool DoolpConnectionTCP::forwardMessage ( DoolpConnection * __to )
{
  DoolpConnectionTCP * to = (DoolpConnectionTCP*) __to;
  __DOOLPCONNECTIONTCP_Log ( "FW : fwd message to %p\n", to );
  if ( to == NULL )
    __DOOLPCONNECTIONTCP_Log ( "FW : fwd to null connection : Deleting message\n" );
#define fwMaxSz 2048
  char buffer[fwMaxSz];
  int bufferIdx = 0;
#define _FWpushBuff(_what, _sz) \
  memcpy ( buffer + bufferIdx, _what, _sz ); \
  bufferIdx += _sz
  _FWpushBuff ( &msgh, sizeof ( DoolpMsg_Header ) );
  DoolpMsg_BlockHeader h;
  int sz;
  while ( true )
    {
      readBlockHeader ( &h );
      _FWpushBuff ( &h, sizeof ( DoolpMsg_BlockHeader ) );
      if ( DoolpMsg_isBlockEnd ( &h ) )
	{
	  break;
	}
      sz = h.blockSize;
      __DOOLPCONNECTIONTCP_Log ( "FW : %d bytes\n", sz );
      if ( sz > fwMaxSz )
	Bug ( "fw : window width too small (%d) to handle %d bytes\n",
	      fwMaxSz, sz );
      if ( sz > 0 )
	{
	  rawRead ( buffer + bufferIdx, sz );
	  bufferIdx += sz;
	}
      else
	__DOOLPCONNECTIONTCP_Log ( "FW : empty block\n" );
    }
  __DOOLPCONNECTIONTCP_Log ( "FW : total msg length is %d\n", bufferIdx );
#ifdef __DOOLPCONNECTIONTCP_DUMPBUFFERS_FW
  FILE * fwFd = fopen ( "FW.log", "a+" );
  __DOOLPCONNECTIONTCP_LogMsg ( fwFd, buffer, bufferIdx );
  fclose ( fwFd );
#endif
  if ( to )
    {
      to->lockWrite ();
      ssize_t __res = write ( to->mySocket, buffer, bufferIdx );	
      __DOOLPCONNECTIONTCP_Log ( "FW : Written %d bytes.\n", __res );
      __res = 0;
      to->unlockWrite ();
    }
  return true;
#undef fwMaxSz
}
