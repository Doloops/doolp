#ifndef __DOOLP_DOOLPOBJECTINDEXERBYRULE_H
#define __DOOLP_DOOLPOBJECTINDEXERBYRULE_H

#include <doolp/doolp-doolpobject-doolpable.h>
#include <doolp/doolp-doolpobject-slot.h>
#include <doolp/doolp-doolpobjectindexer.h>

namespace Doolp
{
  class ObjectIndexerByRule : public ObjectIndexer
  {
#define DoolpObject_CurrentObject ObjectIndexerByRule
    DoolpObject_stdFeatures ( );
  public:
    ObjectIndexerByRule ( ObjectId _start, ObjectId _step )
      {
	setForge ( NULL ); setOwner ( 0 ); setOptions ();
	start = _start;
	step = _step;
	__DOOLP_Log ( "New ObjectIndexer : start=%d, step=%d\n",
		      (ObjectId)start, (ObjectId)step );
      }

    doolpparam<ObjectId> start;
    doolpparam<ObjectId> step;

    //  doolpable ObjectId getStart ( );
    doolpfunc<ObjectId,bool> getStart;

    inline ObjectId getStart_local ( )
      { return (ObjectId) start; }

    //  doolpable ObjectId setNewRule ( ObjectId newStep );
    doolpfunc<ObjectId,ObjectId> setNewRule;
    ObjectId setNewRule_local ( ObjectId newStep );

    ObjectId getFreeObjectId();




    void setOptions () 
    {
      contextDependant = false;
      ttl = 0;
      forceCallsTOOwner = true;
    }
#undef DoolpObject_CurrentObject
  };

};
#endif // __DOOLP_DOOLPOBJECTINDEXERBYRULE_H
