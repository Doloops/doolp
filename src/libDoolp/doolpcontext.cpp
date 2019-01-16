#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpids.h>
#include <doolp/doolp-doolpcontext.h>
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpjob.h>


// Test

Doolp::Context::Context ( Forge * forge, 
			  AgentId agentId,
			  ContextId contextId )
{
  fullContextId.agentId = agentId;
  fullContextId.contextId = contextId;
  fullContextId.stepping = 1;
  this->forge = forge;

  father.agentId = 0;
  father.contextId = 0;
  father.stepping = 0;
#ifdef __DOOLPFORGE_ALLOW_PUSHED_CONTEXTS
  pushedContext = NULL;
#endif // __DOOLPFORGE_ALLOW_PUSHED_CONTEXTS
  handledConnection = NULL;
  // context->asyncCalls.clear ( );
  currentJob = NULL;

  commitContext = NULL;
  lastCommitStepping = 0;

  __DOOLP_Log ( "New Context [%d:%d:%d]\n", 
		fullContextId.agentId,
		fullContextId.contextId,
		fullContextId.stepping );
}

Doolp::Context::~Context ()
{
  __DOOLP_Log ( "Destroying Context [%d:%d:%d]\n", 
		fullContextId.agentId,
		fullContextId.contextId,
		fullContextId.stepping );

  __DOOLP_Log ( "End of context %p : [%d:%d:%d] (father [%d:%d:%d])\n",
		this,
		logDoolpFullContextId ( &(fullContextId ) ),
		logDoolpFullContextId ( &(father ) ) );
#ifdef __DOOLPFORGE_ALLOW_PUSHED_CONTEXTS
  if ( pushedContext != NULL )
    {
      Warn ( "Carefull, this thread has remaining pushed contexts !!\n" );
    }
#endif // __DOOLPFORGE_ALLOW_PUSHED_CONTEXTS

}

bool Doolp::Context::backgroundWork ( Doolp::Call * specificCall )
{
  Log ( "Background work until call %p\n", specificCall );
  if ( handledConnection != NULL )
    return handledConnection->runHandleThread ( specificCall, NULL );
  return false;
}
bool Doolp::Context::backgroundWork ( Doolp::StreamVirtual * specificStream )
{
  Log ( "Background work until stream %p\n", specificStream );
  if ( handledConnection != NULL  )
    {
      if ( handledConnection->status != ConnectionStatus_Ready )
	return false;
      return handledConnection->runHandleThread ( NULL, specificStream );
    }
  return true;
}

/*
 * Note : in the current job, toAgentId is the agentId to reply to.
 * So it is the one calling us.
 */
Doolp::AgentId Doolp::Context::getJobCaller ()
{ 
  if ( currentJob == NULL ) return 0;
  return currentJob->toAgentId; 
}


void Doolp::Context::setNewObject ( Doolp::Object * obj )
{
  Doolp::ContextChange * change = new Doolp::ContextChange();
  memset ( change, 0, sizeof ( Doolp::ContextChange ) );
  change->obj = obj;
  change->creation = fullContextId.stepping;
  changeSet.insert ( make_pair ( obj->getObjectId(), change ) ) ;
}

void Doolp::Context::setDeleteObject ( Doolp::ObjectId objId )
{
  Doolp::ContextChange * change = changeSet[objId];
  if ( change == NULL ) 
    {
      change = new Doolp::ContextChange ();
      memset ( change, 0, sizeof ( Doolp::ContextChange ) );
      changeSet[objId] = change;
    }
  change->obj = NULL;
  change->deletion = fullContextId.stepping;
}

void Doolp::Context::setModifiedObject ( Doolp::Object * obj )
{
  Doolp::ContextChange * change = changeSet[obj->getObjectId()];
  if ( change == NULL ) 
    {
      change = new Doolp::ContextChange ();
      memset ( change, 0, sizeof ( Doolp::ContextChange ) );
      changeSet[obj->getObjectId()] = change;
      change->obj = obj;
      change->firstModified = change->lastModified = fullContextId.stepping;
      return;
    }
  if ( change->firstModified == 0 )
    change->firstModified = fullContextId.stepping;
  change->lastModified = fullContextId.stepping;
}

void Doolp::Context::log ( )
{
  __DOOLP_Log ( "Doolp::Context [%d,%d,%d]\n",
		logDoolpFullContextId ( &fullContextId ) );
  
  Doolp::ContextChangeSet::iterator change;
  for ( change = changeSet.begin () ;
	change != changeSet.end () ;
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
      if ( change->second->obj != NULL )
	change->second->obj->logChangeSet ();
    }


  __DOOLP_Log ( "**end of Doolp::Context log**\n" );
}
