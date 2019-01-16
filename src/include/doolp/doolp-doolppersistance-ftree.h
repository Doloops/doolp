#ifndef __DOOLP_DOOLPPERSISTANCE_FTREE_H
#define __DOOLP_DOOLPPERSISTANCE_FTREE_H

#include <doolp/doolp-doolppersistance.h>
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpforgeservices.h>

namespace Doolp
{
  class PersistanceFTree : public Persistance, public ForgeService
  {
#define DoolpObject_CurrentObject PersistanceFTree
    DoolpObject_stdFeatures ( );
    DoolpObject_Option ( forceObjectAs, Persistance );
    char * ftreeRoot;
  public:
    PersistanceFTree ( char * _ftreeRoot );
    bool start () { return true; }
    bool stop () { return true; }
    Object * getServiceAsObject () { return (Object*) this; }

    FullContextId * doolpfunclocal(getNewContext) (bool force);

    bool doolpfunclocal(commit) ( FullContextId * fullContextId, 
				  Stream<ObjectId> * deletedObjects, 
				  Stream<Object*> * objects );

    Object * doolpfunclocal(retrieveObject) ( FullContextId * fullContextId, ObjectId objId );

  protected:
    bool writeObject ( FullContextId * fullContextId, ObjectBuffer * objBuff );
#undef DoolpObject_CurrentObject
  };


}; // NameSpace Doolp
#endif // __DOOLP_DOOLPPERSISTANCE_FTREE_H
