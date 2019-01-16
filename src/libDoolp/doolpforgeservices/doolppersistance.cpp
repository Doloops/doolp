#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpexceptions.h>
#include <doolp/doolp-doolppersistance.h>

Doolp::Persistance * Doolp::Forge::getPersistance ()
{
  if ( persistanceService == NULL )
    {
      try
	{
	  persistanceService = (Persistance*) getService_char ( "DoolpPersistance" );
	}
      catch ( Exception * e )
	{
	  Fatal ( "Could not get naming service : exception '%s'\n",
		  e->getMessage () );
	}
    }
  AssertBug ( persistanceService != NULL, "Should have had the service !\n" );
  return persistanceService;

}
bool Doolp::Forge::setPersistance ( Doolp::Persistance * _persistanceService )
{
  persistanceService = _persistanceService;
  return true;
}
