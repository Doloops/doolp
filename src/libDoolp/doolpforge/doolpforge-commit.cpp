#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpnaming.h>
#include <doolp/doolp-doolppersistance.h>


bool Doolp::Forge::bindContext ( string &contextName )
{
  AssertBug ( getContext() != NULL, "Can not get current context !\n" );
  try
    {
      FullContextId * ctxt = getNaming()->getNamedContext ( contextName );
      getContext()->commitContext = ctxt;
    }
  catch ( NoNamedContextFound * e )
    {
      __DOOLP_Log ( "No context : trying to create one...\n" );
      FullContextId * ctxt = getPersistance()->getNewContext(true);
      getNaming()->setNamedContext ( contextName, ctxt );
      getContext()->commitContext = ctxt;
      return true;
    }
  catch ( Exception * e )
    {
      __DOOLP_Log ( "Recieved this exception 0x%x : '%s'\n",
		    e->getExceptionId (), e->getMessage () );
    }
  
  return true;
}
bool Doolp::Forge::commit ()
{
  AssertBug ( getContext() != NULL, "Can not get current context !\n" );
  AssertFatal ( getContext()->commitContext != NULL, "Context not binded to any named context !\n" );
  Log ( "-------------- COMMITING ---------------\n" );

  Stream<ObjectId> deletedObjectsStream;
  Stream<Object*> objectsStream;

  Context * currentContext = getContext();
  Call * commitCall = getPersistance()->commit.callAsync ( currentContext->commitContext, 
								&deletedObjectsStream, 
								&objectsStream );
  commitCall->serializeFromStepping = currentContext->lastCommitStepping;
  ContextChangeSet::iterator change;
  for ( change = currentContext->changeSet.begin () ;
	change != currentContext->changeSet.end () ;
	change++ )
    {
      __DOOLP_Log ( "Object 0x%x, (at %p), "
		    "Creation %d, Deletion %d, "
		    "firstModified %d, lastModified %d\n",
		    change->first,
		    change->second->obj,
		    change->second->creation,
		    change->second->deletion,
		    change->second->firstModified,
		    change->second->lastModified );
      if ( change->second->deletion == 0 )
	{
	  AssertBug ( change->second->obj != NULL, "Object not deleted but not in changeset !\n" );
	  if ( ! (change->second->obj->isContextDependant()) )
	    continue;
	  objectsStream.push ( (Object*)change->second->obj );
	}
      else if ( change->second->deletion > 0 )
	{
	  deletedObjectsStream.push ( change->first );
	}
    }
  deletedObjectsStream.setFinished ();
  objectsStream.setFinished ();
  bool res = getPersistance()->commit.getResult ( commitCall );
  Info ( "Commit %s\n", res ? "Success" : "Failed" );
  // TODO : what to do on Failure ?
  currentContext->changeSet.clear ();  
  currentContext->lastCommitStepping = currentContext->fullContextId.stepping;
  return res;
}
