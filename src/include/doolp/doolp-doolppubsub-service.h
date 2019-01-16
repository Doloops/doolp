#ifndef __DOOLP_DOOLPPUBSUB_SERVICE_H
#define __DOOLP_DOOLPPUBSUB_SERVICE_H

#include <doolp/doolp-doolpforgeservices.h>
#include <doolp/doolp-doolppubsub-generic.h>
#include <strongmap.h>

namespace Doolp
{
  class PubSubService : public ForgeService
  {
    friend class PubSubGeneric;
  protected:
    PubSubGeneric * pubSubGeneric;
    Forge * myForge;
    inline Forge * getForge() { return myForge; }

  public:
    PubSubService ( Forge * _forge, 
		    PubSubGeneric * _pubSubGeneric );
    
    bool start () { return true; }
    bool stop () { return true; }
  
    Object * getServiceAsObject ()
      { return pubSubGeneric; }
  };
}; // NameSpace Doolp







#endif // __DOOLP_DOOLPPUBSUB_SERVICE_H
