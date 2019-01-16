#ifndef __DOOLP_DOOLPPUBSUB_GENERIC_H
#define __DOOLP_DOOLPPUBSUB_GENERIC_H

#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpobject-slot.h>
#include <doolp/doolp-doolpstream.h>

namespace Doolp
{
  /*
   * Generic Publish and Subscribe
   * This pseudo-object is provided by PubSubService
   * and shall be encapsulated in more specific objects
   */
  class PubSubGeneric : public Object
  {
#define DoolpObject_CurrentObject PubSubGeneric
    DoolpObject_stdFeatures ( );
  protected:
    PubSubGeneric() {}
  public:
    doolpfunc<bool,Stream<Object*>*,string&,string&> subscribe;

    void setOptions () 
    {
      contextDependant = false;
      ttl = 0;
      forceCallsTOOwner = true;
    }
#undef DoolpObject_CurrentObject
  };
  class __Doolp_DoolpException(PubSubCouldNotSubscribe,"Could not perform subscribe (invalid subscription ?)");
}; // NameSpace Doolp
#endif // __DOOLP_DOOLPPUBSUB_GENERIC_H
