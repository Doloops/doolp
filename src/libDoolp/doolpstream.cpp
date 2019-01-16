#include <doolp/doolp-doolpstream.h>
#include <doolp/doolp-doolpcallcontext.h>
#include <doolp/doolp-doolpconnection.h>

Doolp::StreamVirtual::StreamVirtual()
{
  myForge = NULL;
}

bool Doolp::StreamVirtual::bind ( Doolp::Connection * conn, Doolp::CallContext * callContext, 
				  Doolp::StreamIndex idx, Doolp::StreamBindOptions options )
{
  AssertBug ( conn->myForge != NULL, "Connection has no forge !\n" );
  if ( myForge == NULL )
    myForge = conn->myForge;
  else
    { 
      AssertBug ( myForge == conn->myForge, 
		  "DoolpStream binded within different DoolpForge (already binded to %p, now %p) !!"
		  "NOT IMPLEMENTED YET !!\n", myForge, conn->myForge ); 
    }
  __DOOLP_Log ( "bind to conn=%p, index=0x%x, options=0x%x\n",
		conn, idx, options );
  AssertFatal ( idx < CallContextMaxStreams, " DoolpStream Index out of bounds !\n" );
  bindedContexts[callContext] = idx;
  __DOOLP_Log ( "Inserting %p at index 0x%x\n", this, idx );
  callContext->doolpStreams[idx] = this;
  flush ( conn );
  return true;
}

bool Doolp::StreamVirtual::unbind ( Doolp::CallContext * callContext )
{
  __DOOLP_Log ( "unbind from context %p\n", callContext );
  if ( ! preUnbind ( callContext ) )
    Bug ( "preUnbind failed... This is not expected "
	  "(and not implemented, even thought about)\n" );
  
  bindedContexts.erase ( callContext );
  return true;
}

bool Doolp::StreamVirtual::unbind ( )
{
  return true;
}
