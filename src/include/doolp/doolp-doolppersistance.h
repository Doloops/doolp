#ifndef __DOOLP_DOOLPPERSISTANCE_H
#define __DOOLP_DOOLPPERSISTANCE_H

#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpstream.h>
#include <doolp/doolp-doolpobject-doolpable.h>

namespace Doolp
{
  class Persistance : public Object
  {
#define DoolpObject_CurrentObject Persistance
    DoolpObject_stdFeatures ( );
  protected:
  
  public:
    Persistance ( ) { __DOOLP_Log ( "NEW DOOLPPERSISTANCE\n" ); setOptions(); }

    //    doolpable FullContextId * getNewContext ();
    //    virtual FullContextId * getNewContext_local () { Bug ( "NOT TO BE CALLED DIRECTLY !\n" ); }
    doolpfunc<FullContextId *, bool> getNewContext;
    doolpfunc<bool, FullContextId *, Stream<ObjectId>*,Stream<Object*> *> commit;
    doolpfunc<Object*,FullContextId*,ObjectId> retrieveObject;

    void setOptions () 
    {
      contextDependant = false;
      ttl = 0;
      forceCallsTOOwner = true;
    }
#undef DoolpObject_CurrentObject 
  };


} // NameSpace Doolp;

#endif // __DOOLP_DOOLPPERSISTANCE_H
