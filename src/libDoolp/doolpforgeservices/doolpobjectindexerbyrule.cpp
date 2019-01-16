#include <doolp/doolp-doolpobject.h>

#include <doolp/doolp-doolpobjectindexer.h>
#include <doolp/doolp-doolpobjectindexerbyrule.h>


// ----------------------------------------------------- //
// object index (providing ObjectId )


Doolp::ObjectId 
Doolp::ObjectIndexerByRule::setNewRule_local ( Doolp::ObjectId newStep )
{
  Doolp::ObjectId oldStart = start;
  AssertBug ( newStep < 0x00100000, "new step is too high !\n" );
  step = newStep;
  __DOOLP_Log ( "Setting new rules : start=%d, newStep=%d\n",
		(ObjectId) start, newStep );
  return oldStart;
}


Doolp::ObjectId Doolp::ObjectIndexerByRule::getFreeObjectId ()
{ 
  //  return nextObject++; 
  Doolp::ObjectId result = start;
  __DOOLP_Log ( "(start=%d, step=%d) (owner=%d)\n", (ObjectId) start, 
		(ObjectId) step, getOwnerAgentId () );
  start += step;
  return result;
}
