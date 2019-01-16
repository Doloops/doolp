#if 0 //  __DOOLPFORGE_OBJECTS_CHANGESET_DEPRECATED__

#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpcontext.h>
#include <doolp/doolp-doolpobject.h>
#include <set>

using namespace std;

void DoolpForge::notifyAlteredParam ( DoolpContext * context, 
				      DoolpObject * obj )
//  DoolpObjectParamId paramId )
{
  __DOOLP_Log ( "##ALTER step=%d, objId=%d\n",
		context->fullContextId.stepping,
		obj->objectId );
  context->changeSet.perSteppingChangeSet->insert ( make_pair ( context->fullContextId.stepping,
				       obj ));

}


bool DoolpForge::serializeObjectsFromStepping ( DoolpConnection * conn, DoolpContextStepping fromStepping )
{
  DoolpContextStepping curStepping = getStepping ();
  DoolpContext * context = getContext ();
  DoolpContextPerSteppingChangeSet * perSteppingChangeSet =
    context->changeSet.perSteppingChangeSet;
  __DOOLP_Log ( "from Stepping %d : current is %d\n", fromStepping, curStepping );
  set<DoolpObject*> serialized;
  for ( DoolpContextStepping step = fromStepping ; 
	step <= curStepping ; step ++ )
    {
      __DOOLP_Log ( "Stepping %d : Count = %d / Size = %d\n",
		    step,
		    perSteppingChangeSet->count ( step),
		    perSteppingChangeSet->size () );
      if ( perSteppingChangeSet->count ( step) == 0 )
	continue;
      DoolpContextPerSteppingChangeSet::iterator start;
      for ( start = perSteppingChangeSet->find (step) ; 
	    start != perSteppingChangeSet->end () ;
	    start ++ )
	{
	  __DOOLP_Log ( "step %d, obj %p\n", start->first, start->second);
	  if ( start->first != step )
	    Bug ( "Hey, the find() list is not staying in the the same stepping !!!!\n" );
	  
	  __DOOLP_Log ( "Serializing object with objId=0x%.8x\n",
			start->second->objectId );
	  if ( start->second->getForge() == NULL )
	    Bug ( "Hey, this object has a null DoolpForge !\n" );
	  if ( start->second->objectId == 0 )
	    Bug ( "Hey, this object has a zero DoolpObjectId !\n" );

	  if ( serialized.find ( start->second ) != serialized.end () )
	    {
	      __DOOLP_Log ( "Already serialized\n" );
	      continue;
	    }
	  serialized.insert ( start->second );
	  if ( start->second->serializeFromStepping ( conn, 
						      fromStepping )
	       != true )
	    Bug ( "Could not serializeFromStepping objId Ox%.8x.\n",
		  start->second->objectId );
 	}
    }
  //  Bug ( "not impl\n" );
  return true;
}

#endif // __DOOLPFORGE_OBJECTS_CHANGESET_DEPRECATED__
