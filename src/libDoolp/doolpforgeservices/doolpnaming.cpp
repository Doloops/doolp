#include <doolp/doolp-doolpnaming.h>
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpexceptions.h>

Doolp::Naming * Doolp::Forge::getNaming ()
{
  if ( namingService == NULL )
    {
      try
	{
	  namingService = (Naming*) getService_char ( "DoolpNaming" );
	}
      catch ( Exception * e )
	{
	  Fatal ( "Could not get naming service : exception '%s'\n",
		  e->getMessage () );
	}
    }
  AssertBug ( namingService != NULL, "Should have had the service !\n" );
  return namingService;
}

bool Doolp::Forge::setNaming ( Doolp::Naming * _namingService )
{ 
  namingService = _namingService; 
  return true; 
}
