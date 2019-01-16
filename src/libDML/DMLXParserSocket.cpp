#include <DMLXParser.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

DMLX::ParserSocket::ParserSocket ( int _sock )
{
  sock = _sock;
  dumpsock = -1;
  waitTime = 0; waits = 0;
}

DMLX::ParserSocket::~ParserSocket ()
{
  // TODO : disconnect socket.
}

unsigned int DMLX::ParserSocket::fillBuffer ( void * buff, int max_size )
{
  int attempt = 0;
  while ( true )
    {
      fd_set fdSet;
      struct timeval TimeToWait;
      
      TimeToWait.tv_sec = 2;
      TimeToWait.tv_usec = 0;
      
      FD_ZERO( &fdSet );
      FD_SET( sock, &fdSet );
      Log ( "calling select()\n" );
      int result = select( sock+1, &fdSet, NULL, NULL, &TimeToWait );
      Log ( "select() returned %d\n", result );
      if ( result == -1 )
	{
	  Warn ( "select() returned an error :\n" );
	  //	  LogErrNo();
	  Log ( "Error : %d:%s\n", errno, strerror (errno) );
	  throw new DMLX::ParserException ( "Could not fill buffer : res=-1 ! End of connection ?\n" );
	}
      if ( ( result > 0 )
	   && ( FD_ISSET ( sock, &fdSet ) != 0 ) )
	{
	  int res = read ( sock, buff, max_size );
	  waits++;
	  Log ( "attempt %d : read %d of %d (TTW %d:%d)\n", attempt, res, max_size, (int)TimeToWait.tv_sec, (int)TimeToWait.tv_usec );
	  waitTime += (2*1000000) - ( TimeToWait.tv_sec * 1000000 + TimeToWait.tv_usec );
	  Log ( "Waited : %d:%d (total %d:%d, mean %d:%d)\n",
		TimeToWait.tv_sec == 2 ? 0 : 1 - (int)TimeToWait.tv_sec, TimeToWait.tv_sec == 2 ? 0 : 1000000 - (int)TimeToWait.tv_usec,
		waitTime / 1000000, waitTime % 1000000,
		( waitTime / waits ) / 1000000, ( waitTime / waits ) % 1000000 );
	  if ( res > 0 )
	    {
	      if ( dumpsock != -1 )
		{
		  write ( dumpsock, buff, res );
		  fsync ( dumpsock );
		}
	      return res;
	    }
	  if ( res == -1 )
	    throw new DMLX::ParserException ( "Could not fill buffer : res=-1 ! End of connection ?\n" );
	}
      attempt++;
      if ( attempt == 100 )
	throw new DMLX::ParserException ( "Could not fill buffer : attempts max reached ! End of connection ?\n" );
    }
  return 0;
}
bool DMLX::ParserSocket::canFill ()
{
  return true; // TODO : Clean this !
}

bool DMLX::ParserSocket::dumpToSocket ( int _dumpsock )
{ dumpsock = _dumpsock; return true; }
