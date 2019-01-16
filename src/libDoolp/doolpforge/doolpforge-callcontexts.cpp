#include <doolp/doolp-doolpcallcontext.h>
#include <doolp/doolp-doolpstream.h>


unsigned int Doolp::CallContext::addStream ( Doolp::StreamVirtual * stream )
{
  for ( int i = 0 ; i < CallContextMaxStreams ; i++ )
    if ( doolpStreams[i] == NULL )
      {
	doolpStreams[i] = stream;
	Log ( "Binding stream !\n" );
	stream->bind ( preferedConn, this, i, StreamBindOption_RdWr );
	return i;
      }
  Bug ( "Too much streams ! max streams = %d\n", CallContextMaxStreams );
  return 0;
}
