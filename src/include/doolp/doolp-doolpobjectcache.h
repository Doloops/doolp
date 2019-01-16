#ifndef __DOOLP_DOOLPOBJECTCACHE_H
#define __DOOLP_DOOLPOBJECTCACHE_H

#include <time.h>
#include <pthread.h>

#include <doolp/doolp-doolpids.h>
#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpforgescheduler.h>
#include <doolp/doolp-doolplock.h>

#include <strongmap.h>
#include <map>
#include <functional>
using namespace std;


/*
 * DoolpObjectCache structure
 * 
 * Store Context Independant Objects : simple map (objectsCI)
 * Store Context Dependant : 3 layers
 * 1 map ( objectId, agentId, contextId ) -> rec for active objects (objectsCD)
 * 1 map ( agentId, contextId) -> list<rec> for objects that will die (getTTL > 0) (willDie)
 * 1 map ( tHeaven ) -> list<rec> for objects that shall be destroyed at time tHeaven (deadObjects)
 * 
 * objectsCD knows 5 main phases 
 * 1. add(obj) : 
 * * insert a Record in objectsCD
 * * and add it in willDie
 * 
 * 2. notifyContextEnd() : 
 * * set rec->tDead
 * * move record from willDie to deadObjects
 * 
 * 3a. get(obj) when context is not dead (also resurrection !!)
 * * nothing to do ? Check coherence...
 *
 * 3b. get(obj) when context is dead (resurrection) : 
 * * ( check rec->fcId != getForge()->getFullContextId() -> incoherence, except for DoolpPersistance... ? )
 * * Create a new Record * newRec with local getContext(), put it in objectsCD and willDie
 * * newRec->isFrom = rec; rec->resurrected++;
 * * update all the tDead for the whole isFrom chain 
 * * return rec->obj
 *
 * 4a. remove(obj) when resurrected==0 and isFrom==NULL
 * * remove it from objectsCD, and from willDie or deadObjects (depending on the value of tLast)
 * 
 * 4b. remove(ojb) when isFrom!=NULL
 * * isFrom->resurrected--;
 * * remove rec from objectsCD, willDie or deadObjects, but do not delete object !
 *
 * 4c. remove(ojb) when resurrected>0
 * 
 * 5a. killObject() : call removeObjectCD (context-dependant) (see 5b.)
 * 
 * 5b. removeObjectCD()
 * * if resurrected > 0 : resurrection, nothing to do... leave it alone
 * * if isFrom == NULL, delete the object.
 * * if isFrom != NULL :
 * * * decrement the isFrom->resurrected
 * * * if isFrom->resurrected == 0, cascade the removeObjectCd() to the isFrom
 * * remove the record.
 *
 */

namespace Doolp
{
  class ObjectCache
  {
  protected:
    class Record : public ForgeScheduler::SchedItem
    {
    public:
      Object * obj;
      ObjectCache * cache; // Necessary for runSchedItem.
      FullContextId fcId;
      time_t tLast; // Useless ?
      time_t tDead; // Time the context of the record is dead (if tDead = 0 context is not dead)
      Record * isFrom; // Pointer to resurrected object
      unsigned int resurrected; // Number of objects retrieved from this dead object
      Record ( ObjectCache * _cache, Object*, FullContextId *);
      bool runSchedItem ( time_t when );
    };

    Forge * myForge;
    inline Forge * getForge() { return myForge; }

    /* ContextIndependant Cache */
    typedef strongmap<ObjectId, Object*, less<ObjectId> > CacheObjectsCI;
    CacheObjectsCI objectsCI;

    /* ContextDependant Cache */
    typedef strongtriplemap<ObjectId, AgentId, ContextId, Record*, 
      less<ObjectId>, less<AgentId>, less<ContextId> >
      CacheObjectsCD;
    CacheObjectsCD objectsCD;

    /* Contexts with objects that will die some day */
    typedef strongdoublemap<AgentId, ContextId, list<Record*> *, 
      less<AgentId>, less<ContextId> > CacheObjectsWillDie;
    CacheObjectsWillDie objectsWillDie;
  
    bool addObjectCD ( FullContextId * fcId, Record* rec );
    bool removeObjectCD ( FullContextId * fcId, ObjectId objId );
    bool addDeadObject ( time_t deathTime, Record * rec);
    bool deleteObjectCD ( Record * rec );

    Lock myLock;
    inline void lock() { myLock.lock(); Warn ( "ObjectCache : locked.\n" ); }
    inline void unlock() { myLock.unlock(); Warn ( "ObjectCache : unlocked.\n" ); }

    /*
    pthread_mutex_t __lock;
    void lock ()
    {
      if ( pthread_mutex_trylock ( &__lock ) != 0 )
	{
	  Warn ( "ObjectCache : trylock failed, calling lock()\n" );
	  pthread_mutex_lock ( &__lock );
	}
      Warn ( "ObjectCache : locked.\n" );
    }
    void unlock ()
    { pthread_mutex_unlock ( &__lock ); Warn ( "ObjectCache : unlocked.\n" ); }
    */
  public:
    ObjectCache ( Forge * _forge );
    template<class T_obj>
      bool add ( T_obj * obj, FullContextId * fcId )
      { return __add ( dynamic_cast<Object*> (obj), fcId ); }
  protected:
    bool __add ( Object * obj, FullContextId * fcId );
  public:
    bool notifyContextEnd ( FullContextId * fullContextId );

    bool remove ( Object * obj, bool force );

    bool remove ( Object * obj, 
		  FullContextId * fullContextId, bool force );

    Object * get ( ObjectId objId,
		   FullContextId * fullContextId );

    bool clearCache ( ); // Only clears timed out dead-contexted objects

    bool log ( );
};

}; // NameSpace Doolp
#endif // __DOOLP_DOOLPOBJECTCACHE_H



#if 0
// This is a notes about using the less<> in std::map
struct DoolpCacheContextKeyCompareLess : public binary_function<DoolpCacheContextKey,DoolpCacheContextKey, bool>
{
  bool operator() ( const DoolpCacheContextKey & h1, const DoolpCacheContextKey & h2 ) const
  {
    if ( h1.agentId < h2.agentId )
      return true;
    else if ( h1.agentId > h2.agentId )
      return false;
    return ( h1.contextId < h2.contextId );
  }
};

#endif 
