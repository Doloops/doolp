#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpobjectindexer.h>
#include <doolp/doolp-doolpexceptions.h>

Doolp::ObjectId Doolp::Forge::getFreeObjectId ()
{
  AssertFatal ( getObjectIndexer() != NULL, " Could not get an object indexer\n" );
  return getObjectIndexer()->getFreeObjectId ();
}
Doolp::ObjectIndexer * Doolp::Forge::getObjectIndexer ( )
{
  if ( objectIndexer == NULL )
    {
      try
	{
	  objectIndexer = (ObjectIndexer*) getService_char ( "DoolpObjectIndexer" );
	}
      catch ( Exception * e )
	{
	  Fatal ( "Could not get indexer service : exception '%s'\n",
		  e->getMessage () );
	}
    }
  AssertBug ( objectIndexer != NULL, "Should have had the service !\n" );
  return objectIndexer;
}

