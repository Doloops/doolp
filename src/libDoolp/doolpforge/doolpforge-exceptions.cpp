#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpexception.h>
#include <doolp/doolp-doolpobjectstaticinfo.h>

Doolp::Exception * Doolp::Forge::getException ( Doolp::ExceptionId exceptionId )
{
  ExceptionConstructor constructor = objectStaticInfo->getExceptionConstructor ( exceptionId );
  AssertBug ( constructor != NULL, "ExceptionId=0x%x unknown\n",
	      exceptionId );
  return constructor ();
}
