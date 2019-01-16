#ifndef __DOOLP_DOOLPNAMING_H
#define __DOOLP_DOOLPNAMING_H

#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpobject-doolpable.h>
#include <doolp/doolp-doolpexception.h>

#include <map>
using namespace std;

namespace Doolp
{
  class Naming : public Object
  {
#define DoolpObject_CurrentObject Naming
    DoolpObject_stdFeatures ( );
  protected:
    
  public:
    Naming() { }
    
    // Named Contexts : string -> FullContextId
    doolpfunc<FullContextId*,string&> getNamedContext;
    doolpfunc<bool,string&,FullContextId*> setNamedContext;
    
    // Named Objects : string(context name), string(object name) -> ObjectId
    doolpfunc<ObjectId,string&,string&> getNamedObject;
    doolpfunc<bool,string&,string&,ObjectId> setNamedObject;
    
    void setOptions () 
    {
      contextDependant = false;
      ttl = 0;
      forceCallsTOOwner = true;
    }
#undef DoolpObject_CurrentObject
  };
  
  class __Doolp_DoolpException(NoNamedContextFound,"Unable to find named context" );


}; // NameSpace Doolp
#endif // __DOOLP_DOOLPNAMING_H
