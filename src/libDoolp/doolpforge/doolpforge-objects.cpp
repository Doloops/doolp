#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpcontext.h>
#include <doolp/doolp-doolpexceptions.h>

#include <doolp/doolp-doolpobjectstaticinfo.h>
#include <doolp/doolp-doolpobjectcache.h>
#include <doolp/doolp-doolpobjectindexer.h>
#include <doolp/doolp-doolpobjectbuffer.h>
#include <doolp/doolp-doolppersistance.h>

bool Doolp::Forge::addObject ( Doolp::Object * obj )
{
  __DOOLP_Log ( "New object %p\n", obj );
  obj->setObjectId ( getFreeObjectId () );
  AssertFatal ( obj->getObjectId() != 0,
		"I have been given a zero ObjectId\n" );
  obj->setForge ( getForge() );
  obj->setOwner ( getForge()->getAgentId () );
  obj->setOptions ();
  obj->__initSlots ();
  return getObjectCache()->add ( obj, getFullContextId() );
}

bool Doolp::Forge::removeObject ( Doolp::Object * obj )
{
  return getObjectCache()->remove ( obj, false );
}


Doolp::Object * Doolp::Forge::createEmptyObject ( Doolp::ObjectNameId nameId,
						  Doolp::FullContextId * fullContextId, 
						  Doolp::ObjectId objId )
  /*
   * If we find the correct obejct, build it
   * If not, and if forge option doolpObjectBuffer=true
   * Build a Doolp::ObjectBuffer
   */
{
  Object * obj;
  ObjectConstructor constructor = objectStaticInfo->getObjectConstructor ( nameId );
  if ( constructor == NULL )
    {	
      AssertFatal ( options.allowDoolpObjectBuffer, 
		    "No DoolpObjectInfo for this nameId 0x%x, and DoolpObjectBuffers not allowed\n", nameId );
      obj = new ObjectBuffer ( objId, nameId );
    }
  else
    obj = constructor ( objId );
  obj->setForge ( this );
  obj->setTTL ( options.jobObjectTTL );
  getObjectCache()->add ( obj, fullContextId );
  return obj;
}

Doolp::Object * Doolp::Forge::doolpfunclocal(getDistObject) ( Doolp::FullContextId * fullContextId,
							  Doolp::ObjectId objId )
{
  __DOOLP_Log ( "getObject 0x%x for context[%d:%d:%d]\n",
		objId, fullContextId->agentId, 
		fullContextId->contextId, 
		fullContextId->stepping );
  __DOOLP_Log ( "current Context is [%d:%d:%d]\n",
		getFullContextId()->agentId,
		getFullContextId()->contextId,
		getFullContextId()->stepping );
  Object * obj = objectCache->get ( objId, fullContextId );
  if ( obj == NULL )
    {
      __DOOLP_Log ( "No object objId=0x%x found in cache\n", objId);
      if ( fullContextId->agentId != getAgentId () )
	{
	  __DOOLP_Log ( "Trying to ask agentId : '%d'\n", fullContextId->agentId );
	  try
	    {
	      obj = getObject ( fullContextId, objId );
	    }
	  catch ( Exception * e ) // TODO : better handle this
	    {
	      Warn ( "Could not get object '0x%x' in context [%d.%d.%d] : exception '%s'\n",
		     objId, logDoolpFullContextId ( fullContextId ), e->getMessage () );
	      throw e;
	    }
	  if ( obj != NULL )
	    return obj;
	}
      __DOOLP_Log ( "Trying to ask for persistance..\n" );
      try 
	{
	  // TODO : take care of the validity of persistance inside of the fullContextId...
	  if ( getContext()->commitContext != NULL )
	    {
	      __DOOLP_Log ( "Persistance context is [%d.%d.%d]\n",
			    logDoolpFullContextId ( getContext()->commitContext ) );
	      return getObject( getContext()->commitContext, objId );
	    }
	  else
	    {
	      Log ( "Could not get object from persistance : no persistance context set.\n" );
	      // throw new NoObjectFound();
	      return getPersistance()->retrieveObject ( fullContextId, objId );
	    }
	}
      catch ( Exception * e )
	{
	  Warn ( "Could not get object '0x%x' in context [%d.%d.%d] : exception '%s'\n",
		 objId, logDoolpFullContextId ( fullContextId ), e->getMessage () );
	  throw e;
	}

    }
  return obj;
}

