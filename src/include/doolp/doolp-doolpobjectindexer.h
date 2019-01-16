#ifndef __DOOLP_DOOLPOBJECTINDEXER_H
#define __DOOLP_DOOLPOBJECTINDEXER_H

#include <doolp/doolp-doolpids.h>
#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpobject-doolpable.h>


namespace Doolp
{
  class ObjectIndexer : public Object
  {
#define DoolpObject_CurrentObject ObjectIndexer
    DoolpObject_stdFeatures ( );

  public:
    ObjectIndexer () { __DOOLP_Log ( "New Indexer\n" ); }
    virtual ObjectId getFreeObjectId () { Bug ( "Should not been called !\n" ); return 0; }
    void setOptions ()
    {
      contextDependant = false;
      ttl = 0;
      forceCallsTOOwner = true;
    }
#undef DoolpObject_CurrentObject
  };

}; // NameSpace Doolp
#endif // __DOOLP_DOOLPOBJECTINDEXER_H
