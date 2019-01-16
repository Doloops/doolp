#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpconnection.h>

/*
 * AuthPlain
 */

bool Doolp::Forge::authPlain_local ( char * name, char * password )
{
  __DOOLP_Log ( "authPlain : name=%s, password=%s\n",
		name, password );
  return true;
}
