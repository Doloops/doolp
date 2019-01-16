#include <doolp/doolp-doolpobjectcache.h>
#include <doolp/doolp-doolpcontext.h>
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpforgescheduler.h>
#include <doolp/doolp-doolpobject.h>
// #include <sigc++/sigc++.h>

/*
 * Totally reimplemented from scratch
 * Be carefull of DoolpObject::setTTL() (not really clean about this yet)
 * DoolpObjectCache uses DoolpForgeScheduler for cache cleaning.
 * TODO : Is the killObject() really fully implemented ?
 */

#define __DOOLP_OBJECTCACHE_REMOVAL_SANITYCHECKS__

Doolp::ObjectCache::Record::Record ( Doolp::ObjectCache * _cache, 
				   Doolp::Object* _obj, 
				   Doolp::FullContextId * _fcId )
{
  cache = _cache;
  obj = _obj;
  fcId = *_fcId;
  tLast = time ( NULL );
  tDead = 0;
  isFrom = NULL;
  resurrected = 0;
}

bool Doolp::ObjectCache::Record::runSchedItem ( time_t when )
{
  if ( obj == NULL )
    {
      Log ( "runSchedItem : object already deleted !\n" );
      return true;
    }
  Log ( "runSchedItem when=%d objId=0x%x\n",
	(int)when, obj->getObjectId() );
  if ( resurrected > 0 )
    {
      __DOOLP_Log ( "Object 0x%x [%d.%d.%d] has resurrected !\n",
		    obj->getObjectId(), logDoolpFullContextId ( &(fcId) ) );
      return false;
    }
  cache->deleteObjectCD ( this );
  return true;
}

Doolp::ObjectCache::ObjectCache ( Doolp::Forge * _forge )
{ 
  myForge = _forge; 
  //  pthread_mutex_init ( &__lock, NULL );
  myLock.setName ( "ObjectCache Lock" );
}

bool Doolp::ObjectCache::__add ( Doolp::Object * obj, 
				 Doolp::FullContextId * fcId )
{
  AssertBug ( fcId != NULL, "Invalid FullContextId Given !!\n" );
  AssertBug ( obj->getObjectId() != 0,
	      "I have been given an Object with objectId=0 !\n" );
  lock ();
  __DOOLP_Log ( "HEY : Adding obj 0x%x [%d.%d.%d] isCD=%d\n",
		obj->getObjectId(), logDoolpFullContextId ( fcId ),
		obj->isContextDependant () );
  getForge()->getContext()->setNewObject ( obj );
  if ( ! obj->isContextDependant () )
    {
      __DOOLP_Log ( "Adding Context-Independant Object\n" );
      objectsCI.put(obj->getObjectId() ,obj);
      unlock ();
      return true;
    }
  Record * rec = new Record ( this, obj, fcId );
  bool result = addObjectCD ( fcId, rec );
  unlock ();
#if 0
  __DOOLP_Log ( "********************** AFTER ADD ***********************************\n" );
  log ();
  __DOOLP_Log ( "*********************** /AFTER ADD **********************************\n" );
#endif
  return result;
}
bool Doolp::ObjectCache::addObjectCD ( Doolp::FullContextId * fcId, Record* rec )
{
  Record * fromRecord = objectsCD.get ( rec->obj->getObjectId(), fcId->agentId, fcId->contextId );
  if ( fromRecord != NULL )
    {
      Warn ( "Already have a record for objId=0x%x, agentId=%d, contextId=%d\n",
	     rec->obj->getObjectId(), fcId->agentId, fcId->contextId );
      return true;
    }
  objectsCD.put ( rec->obj->getObjectId(), fcId->agentId, fcId->contextId, rec );
  if ( rec->obj->getTTL() > 0 )
    {
      list<Record*> * lWd = objectsWillDie.get ( fcId->agentId, fcId->contextId );
      if ( lWd == NULL )
	{
	  lWd = new list<Record*>;
	  objectsWillDie.put ( fcId->agentId, fcId->contextId, lWd );
	}
      lWd->push_back ( rec );
    }
  return true;
}

bool Doolp::ObjectCache::removeObjectCD ( Doolp::FullContextId * fcId, Doolp::ObjectId objId )
{
  Record* rec = objectsCD.get ( objId, fcId->agentId, fcId->contextId );
  AssertBug ( rec != NULL, " Could not get record.\n" );
  AssertBug ( rec->obj != NULL, " Record has no object.\n" );
  if ( rec->resurrected > 0 )
    {
      __DOOLP_Log ( "Object 0x%x [%d.%d.%d] has resurrected !\n",
		    rec->obj->getObjectId(), logDoolpFullContextId ( &(rec->fcId) ) );
      return true;
    }
  if ( rec->isFrom == NULL )
    {
      __DOOLP_Log ( "Record has no isFrom, deleting object\n" );
      rec->obj->setForge ( NULL );
      delete ( rec->obj );
    }
  else
    {
      __DOOLP_Log ( "Object isFrom=[%d.%d.%d]\n", logDoolpFullContextId( &(rec->isFrom->fcId) ) );
      rec->isFrom->resurrected--;
      if ( rec->isFrom->resurrected == 0 )
	{
	  __DOOLP_Log ( "Cascading remove to isFrom\n" );
	  removeObjectCD ( &(rec->isFrom->fcId), rec->obj->getObjectId() );
	}
    }
  objectsCD.remove ( objId, fcId->agentId, fcId->contextId );
  return true;
}

bool Doolp::ObjectCache::remove ( Doolp::Object * obj, 
				  bool force )
{
  return remove ( obj, NULL, force );
  //  lock ();
  //  Warn ( "Not Implemented\n" );
  //  unlock ();
  //  return true;
}

bool Doolp::ObjectCache::remove ( Doolp::Object * obj, Doolp::FullContextId * fcId, bool force )
/*
 * if force = true, immediate removing of the object
 * elsewere, push to runShedItem
 */ 
{
  //  Warn ( "Not Implemented\n" );
  if ( fcId != NULL )
    {
      AssertBug ( !force, "When specifing a FullContextId, force shall be set to false\n" );
    }
  lock ();
  time_t now = time ( NULL );
  AssertBug ( obj->getObjectId() != 0, "Removing an object with a zero objectId ! (obj=%p)\n", obj );
  AssertBug ( obj->getForge() != NULL, "Removing an object not added to a forge ! (objId=0x%x, obj=%p)\n", 
	      obj->getObjectId(), obj );
  if ( ! obj->isContextDependant() )
    {
      // Remove it from CI
      // Note : force does not apply here (always a strict immediate...)
      if ( ! objectsCI.has ( obj->getObjectId() ) )
	{ unlock(); return false; }
      AssertBug ( objectsCI.get ( obj->getObjectId() ) == obj, 
		  "ObjectId and objet differ ! objId=0x%x, got %p, stored %p\n",
		  obj->getObjectId(), obj, objectsCI.get ( obj->getObjectId() ) );
      if ( ! force )
	{
	  Warn ( "Must force object deletion because object is Context-Independant\n" );
	}
      objectsCI.remove ( obj->getObjectId() );
      unlock ();
      return true;
    }
  // Little bit brute-force, shall refine this...
  ObjectId objId = obj->getObjectId();
  for ( CacheObjectsCD::iterator o = objectsCD.begin() ; o != objectsCD.end() ; o++ )
    {
      if ( o.value()->obj == obj )
	{
	  if ( fcId != NULL )
	    {
	      if ( ( o.second() == fcId->agentId )
		   && ( o.third() == fcId->contextId ) )
		continue;
	    }
	  Log ( "Removing [objId=0x%x, agentId=%d, contextId=%d, Record=%p]\n",
		objId, o.second(), o.third(), o.value() );
	  AssertBug ( o.value()->obj == obj, "Record object %p differs from given object %p\n",
		      o.value()->obj, obj );
	  AssertFatal ( o.value()->resurrected == 0, 
			"Object has been dead and is now resurrected.. What shall we do ? NOT IMPLEMENTED...\n" );
	  AssertFatal ( o.value()->isFrom == NULL,
			"Object is from another record.. What shall we do ? NOT IMPLEMENTED...\n" );
	  list<Record *> * l = objectsWillDie.get ( o.second(), o.third() );
	  bool found = false;
	  if ( l != NULL )
	    {
	      // Delete record in objectsWillDie
	      for ( list<Record*>::iterator r = l->begin() ;
		    r != l->end() ; r++ )
		{
		  if ( *r == o.value() )
		    {
		      if ( force )
			{
			  delete ( *r );
			}
		      else
			{
			  (*r)->tDead = now;
			  addDeadObject ( now + obj->getTTL(), *r );
			}
		      l->erase ( r );
		      found = true;
		      break;
		    }
		}
#ifdef __DOOLP_OBJECTCACHE_REMOVAL_SANITYCHECKS__
	      for ( list<Record*>::iterator r = l->begin() ;
		    r != l->end() ; r++ )
		{
		  if ( *r == o.value() 
		       || (*r)->obj == obj )
		    {
		      Bug ( "Multiple records of the same object within same context in objectsWillDie table : "
			    "objId=0x%x, obj=%p, Record=%p, ctxt[%d.%d]\n", o.first(), obj, o.value(),
			    o.second(), o.third () );
		    }
		}
#endif // __DOOLP_OBJECTCACHE_REMOVAL_SANITYCHECKS__
	      if ( l->size() == 0 )
		{
		  delete ( l );
		  objectsWillDie.remove ( o.second(), o.third() );
		}
	    }
	  if ( force ) 
	    {
	      if ( !found )
		{
		  // Here we assert Record is in ForgeScheduler : prevent it from removing this object twice !
		  o.value()->obj = NULL;
		}
	      objectsCD.remove ( o );
	    }
	  continue;
	}
    }
  unlock ();
  return true;
}

Doolp::Object * Doolp::ObjectCache::get ( Doolp::ObjectId objId,
				      Doolp::FullContextId * fcId )
{
  Object * obj;
  lock ();
  __DOOLP_Log ( "get 0x%x [%d.%d.%d]\n", objId,
		logDoolpFullContextId ( fcId ) );
  if ( ( obj = objectsCI.get(objId) ) != NULL )
    {
      AssertBug ( ! obj->isContextDependant (), "Object is now ContextDependant !\n" );
      __DOOLP_Log ( "Found CI\n" );
      unlock ();
      return obj;
    }
  Record* rec = objectsCD.get ( objId, fcId->agentId, fcId->contextId );
  if ( rec == NULL )
    {
      __DOOLP_Log ( "No object found\n" );
      unlock ();
      return NULL;
    }
  time_t now = time ( NULL );
  rec->tLast = now;
  FullContextId * fcId_current = getForge()->getFullContextId();
  if ( ( fcId_current->agentId == fcId->agentId )
       && ( fcId_current->contextId == fcId->contextId ) )
    {
      AssertFatal ( rec->tDead == 0, " Been asked an object by a dead context !!\n" );
      obj = rec->obj;
      __DOOLP_Log ( "Been asked from same context.\n" );
      unlock ();
      return obj;
    }
  rec->resurrected ++;
  Record* newRec = new Record ( this, rec->obj, getForge()->getFullContextId() );
  newRec->isFrom = rec;
  addObjectCD ( fcId_current, newRec );
  obj = rec->obj;
  __DOOLP_Log ( "Return obj with new Record for [%d.%d.%d]\n",
		logDoolpFullContextId(getForge()->getFullContextId()) );
  unlock ();
  return obj;  
}


bool Doolp::ObjectCache::notifyContextEnd ( Doolp::FullContextId * fcId )
{
  lock ();
  __DOOLP_Log ( "Notified context end of [%d.%d.%d]\n", 
		logDoolpFullContextId(fcId) );
  if ( objectsWillDie.get(fcId->agentId, fcId->contextId) == NULL )
    {
      __DOOLP_Log ( "Nothing to do.(no objects)\n" );
      unlock ();
      return true;
    }
  list<Record*> * lst = objectsWillDie.get ( fcId->agentId, fcId->contextId );
  time_t now = time ( NULL );
  while ( lst->size () > 0 )
    {
      Record* rec = lst->front (); lst->pop_front ();
      __DOOLP_Log ( "Pushing obj '0x%x' to deadObjects\n", rec->obj->getObjectId() );
      rec->tDead = now;
      addDeadObject ( now + rec->obj->getTTL(), rec );
    }
  __DOOLP_Log ( "notifyContext done.\n" );
  unlock ();
  return true;
}

bool Doolp::ObjectCache::deleteObjectCD ( Record * rec )
{
  lock();
  Log ( "Deleting Object Record=%p, obj=0x%x\n",
	rec, rec->obj->getObjectId() );
  AssertBug ( rec->obj->isContextDependant() == true, 
	      "Object 0x%x is not context dependant !\n",
	      rec->obj->getObjectId() );
  if ( rec->resurrected == 0 )
    removeObjectCD ( &(rec->fcId), rec->obj->getObjectId() );
  else
    {
      Warn ( "Object 0x%x has resurrected.\n",
	     rec->obj->getObjectId() );
    }
  unlock();
  return true;
}


bool Doolp::ObjectCache::addDeadObject ( time_t deathTime, Record * rec)
{
  myForge->getScheduler()->add ( deathTime, rec );
  return true;
};


bool Doolp::ObjectCache::clearCache ( )
{
  Warn ( "This is really deprecated !!\n" );
  Warn ( "Or not implemented ?\n" );
  return false;
#if 0
  lock ();
  time_t now = time ( NULL );
  CacheDeadObjects * dead; 

  while ( ( dead = deadObjects ) != NULL )
    {
      if ( dead->tDead > now )
	{
	  unlock ();
	  return true;
	}
      __DOOLP_Log ( "Deleting objects at time '%d'\n", (int)dead->tDead );
      AssertBug ( dead->objects != NULL, " There is a record, but no list\n" );
      Record * rec;
      while ( dead->objects->size () > 0 )
	{
	  rec = dead->objects->front(); dead->objects->pop_front();
	  __DOOLP_Log ( "At object 0x%x [%d.%d.%d]\n",
			rec->obj->getObjectId(), logDoolpFullContextId( &(rec->fcId) ) );
	  if ( rec->resurrected > 0 )
	    {
	      __DOOLP_Log ( "Object has resurrected=%d\n", rec->resurrected );
	      continue;
	    }
	  removeObjectCD ( &(rec->fcId), rec->obj->getObjectId() );
	}
      deadObjects = dead->next;
      delete ( dead->objects );
      free ( dead );
    }
  unlock ();
#endif
  return true;
}

bool Doolp::ObjectCache::log () 
{
  lock ();
  time_t now = time ( NULL );
  __DOOLP_Log ( "DoolpObjectCache : %u objectsCI, %u objectsCD\n", 
		objectsCI.size (), objectsCD.size () );
  for ( CacheObjectsCI::citerator oCI = objectsCI.begin();
	oCI != objectsCI.end() ; oCI++ )
    {
      __DOOLP_Log ( "objectCI objectId=0x%x\n", oCI->first );
      AssertBug ( oCI->second != NULL, "Object has no record !\n" );
      AssertBug ( ! (oCI->second->isContextDependant () ), "Object is not Context Independant !\n" );
    }
  for ( CacheObjectsCD::citerator oCD = objectsCD.cbegin () ;
	oCD != objectsCD.cend() ; oCD++ )
    {
      AssertBug ( oCD->second != NULL, "Object has no record !\n" );
      AssertBug ( oCD->second->obj != NULL, "Record has a NULL object !\n" );
      AssertBug ( oCD->second->obj->isContextDependant (), "Object is not Context Dependant !\n" );
      Record* rec = oCD->second;
      AssertBug ( oCD->first.key1 == rec->obj->getObjectId(), "objectId 0x%x has changed in record !\n",
		  rec->obj->getObjectId() );
      AssertBug ( oCD->first.key2 == rec->fcId.agentId, "Record has a wrong agentId\n" );
      AssertBug ( oCD->first.key3 == rec->fcId.contextId, "Record has a wrong contextId"
		  "(obj=0x%x, stored for %d, record is %d\n",
		  oCD->first.key1, oCD->first.key3, rec->fcId.contextId);
      __DOOLP_Log ( "objectCD objId=0x%x fcId=[%d.%d.%d] last=%ds ago dead=%d resurr=%d\n",
		    oCD->first.key1, oCD->first.key2, oCD->first.key3,
		    rec->fcId.stepping,
		    (int)(now - rec->tLast),
		    (int)(rec->tDead == 0 ? -1 : now - rec->tDead), (int)rec->resurrected );
    }
  for ( CacheObjectsWillDie::iterator wd = objectsWillDie.begin () ;
	wd != objectsWillDie.end () ; wd++ )
    {
      AssertBug ( wd.third() != NULL, " No value in objectsWillDie!\n" );
      for ( list<Record*>::iterator rec = wd.third()->begin () ;
	    rec != wd.third()->end () ; rec++ )
	{
	  AssertBug ( *rec != NULL, "Empty record.\n" );
	  __DOOLP_Log ( "WillDie : wd[%d.%d] fcId[%d.%d.%d] objId=0x%x last=%ds ttl=%d\n",
			wd.first(), wd.second(),
			logDoolpFullContextId ( &((*rec)->fcId)), (*rec)->obj->getObjectId(),
			(int)(*rec)->tLast, (int)(*rec)->obj->getTTL() );
	}
    }
  unlock ();
  return true;
}
