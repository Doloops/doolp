#ifndef __DOOLP_DOOLPEXCEPTIONS_H
#define __DOOLP_DOOLPEXCEPTIONS_H


#include <doolp/doolp-doolpexception.h>
namespace Doolp
{
  class __Doolp_DoolpException(CallNoAgentFound,"Unable to find agent for this call !" );
  class __Doolp_DoolpException(NoForgeServiceFound,"No service found");
  class __Doolp_DoolpException(NoObjectFound,"No object found");
  class __Doolp_DoolpException(NoObjectSlotFound,"No object slot found");
  class __Doolp_DoolpException(ConnectionCouldNotWrite,"Could not write to connection");
}; // NameSpace Doolp

#endif // __DOOLP_DOOLPEXCEPTIONS_H
