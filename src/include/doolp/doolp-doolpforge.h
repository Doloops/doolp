#ifndef __DOOLP_DOOLPFORGE_H
#define __DOOLP_DOOLPFORGE_H

#include <sys/types.h>
#include <pthread.h>
#include <time.h>
#include <string.h> // for memset() 
#include <errno.h>

using namespace std;
#include <list>
#include <map>

#include <doolp/doolp-doolpids.h>
#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpagent.h> // Directly depends on this header.
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpobject-slot.h>

namespace Doolp
{
  typedef struct ForgeOptions
  {
    bool usePushedContexts; // unused ?
    bool useWaitSpecificCall;
    bool forceJobsInSameThread;
    bool allowDoolpObjectBuffer;
    ObjectTTL jobObjectTTL; // object sent only for a given job
  };
  
  class Forge : public Object
  {
#define DoolpObject_CurrentObject Forge
  DoolpObject_stdFeatures ( );

 public:
  ForgeOptions options;
  Forge ();
  ~Forge ();
  inline Forge * getForge() { return this; }
  bool close ();

  /* 
   * Agent
   */
 protected:
  Agent myAgent; // Me as an agent
  list<Agent*> agents; // List of known agents
 public:
  AgentId getAgentId (); 
  Agent * getAgent ( AgentId agentId );
  bool addAgent ( Agent * agent );
  bool setAgentId ( AgentId _agentId );
  Agent * getMyAgent () { return &myAgent; }
 protected:
  AgentId nextAgentId;
 public:
  AgentId getFreeAgentId () // Protect this ! Nobody except god can do that !
    { return nextAgentId++; }
 protected:
  AgentId defaultDistAgentId;
 public:
  AgentId getDefaultDistAgentId () { return defaultDistAgentId; }
  void setDefaultDistAgentId ( AgentId agentId )
  { defaultDistAgentId = agentId; }

  bool removeAgent ( AgentId agentId );

  /*
   * Forge Scheduler
   */
 protected:
  ForgeScheduler * scheduler;
 public:
  inline ForgeScheduler * getScheduler() const { return scheduler; }
  /*
   * Context Management
   */
 protected:
  pthread_key_t contextThreadKey;
  ContextId nextContextId;
  Context * newContext();
  unsigned int runningThreads;
 public:

  void setCurrentContext ( Context * context ); // Must be set public because used in setSpecific

  FullContextId * getNewFullContextId();

  bool setNextContextId(ContextId id);

  Context * getContext (); 
  bool notifyContextEnd (Context * context);
  bool notifyContextEnd (FullContextId * fullContextId);
  FullContextId * getFullContextId ();
  ContextStepping getStepping () 
    { return getFullContextId()->stepping; }
  
#ifdef __DOOLPFORGE_ALLOW_PUSHED_CONTEXTS
  void pushContext ();
  bool popContext ();
#endif // __DOOLPFORGE_ALLOW_PUSHED_CONTEXTS

  void setContextFather ( FullContextId * father ); // Sets context inheritance
  void clearContextFather (); // Clears context inheritance...
  FullContextId * getContextFather ( );

  AgentId getAgentIdForThisJob ();

  void incrementStepping ();
 protected:
  bool newThread ( Context * newContext, 
		   pthread_t * pthread,
		   const pthread_attr_t *attr,
		   void *(*start_routine)(void*), void *arg);
 public:
  /*
   * Context enabled Thread creation.
   */
  bool newThread (pthread_t * pthread,
		  const pthread_attr_t *attr,
		  void *(*start_routine)(void*), 
		  void *arg);
  

  /*
   * Active Objects
   */
  /* Objects Informations */
 protected:
  ObjectStaticInfo * objectStaticInfo;
  ObjectDynamicInfo * objectDynamicInfo;
 public:
  inline ObjectStaticInfo * getObjectStaticInfo() { return objectStaticInfo; }
  inline ObjectDynamicInfo * getObjectDynamicInfo() { return objectDynamicInfo; }
  Exception * getException ( ExceptionId exceptionId );

 public:
  // Myself as an object;
  static const bool forceCallsTOOwner = false;
  inline AgentId getOwnerAgentId() { return getAgentId(); }

  /* ObjectCache Mechanism */
 protected:
  ObjectCache * objectCache;
 public:
  bool addObject ( Object * obj );
  //  template<typename T_Object>
    //    T_Object * __getObject ( ObjectId objId )
    //    { return dynamic_cast<T_Object> ( getObject( getFullContextId(), objId ) ); }
  bool removeObject ( Object * obj );

  ObjectCache * getObjectCache ( ) { return objectCache; } 
  doolpfunc<Object *,FullContextId *,ObjectId> getDistObject;
  Object * doolpfunclocal(getDistObject) ( FullContextId * fullContextId, 
						ObjectId objId );
  AgentId doolpfunchelper(chooseAgentId,getDistObject) ( FullContextId * fullContextId, ObjectId objId )
  { return fullContextId->agentId; }

  inline Object * getObject ( FullContextId * fullContextId, 
			      ObjectId objId )
    { return getDistObject ( fullContextId, objId ); }
  inline Object * getObject ( ObjectId objId )
    { return getObject ( getFullContextId(), objId ); }

  Object * createEmptyObject ( ObjectNameId nameId,
			       FullContextId * fullContextId,
			       ObjectId objId );

  /*
   * Remote Method Invocation : Slots
   */
 public:
  doolpfunc<list<AgentId>*,ObjectNameId,ObjectSlotId> getAgentIdsForSlotId;
  list<AgentId>* doolpfunclocal(getAgentIdsForSlotId) ( ObjectNameId objectNameId, ObjectSlotId slotId );
  //  AgentId doolpfunchelper(chooseAgentId,getAgentIdsForSlotId) ( ObjectNameId objectNameId, ObjectSlotId slotId ) { return toAgentId; }
  doolpfunc<bool,list<pair<ObjectNameId,ObjectSlotId> >* > tellImplementedSlots;
  bool doolpfunclocal(tellImplementedSlots) ( list<pair<ObjectNameId,ObjectSlotId> > * );

  /*
   * Active Calls
   */
 protected:
  list<Call*> calls;
#ifdef __DOOLPFORGE_CALLMUTEX
  pthread_mutex_t callMutex;
  inline void lockCalls()
    { if ( pthread_mutex_trylock ( &callMutex ) == 0 ) return; 
      Warn ( "callMutex already locked : waiting...\n" );
      pthread_mutex_lock ( &callMutex ); }
  inline void unlockCalls() { pthread_mutex_unlock ( &callMutex ); }
#else
  inline void lockCalls(); inline void unlockCalls ();
#endif
#ifdef __DOOLPFORGE_JOBMUTEX
  pthread_mutex_t jobMutex;
  inline void lockJobs()
    { if ( pthread_mutex_trylock ( &jobMutex ) == 0 ) return; 
      Warn ( "jobMutex already locked : waiting...\n" );
      pthread_mutex_lock ( &jobMutex ); }
  inline void unlockJobs() { pthread_mutex_unlock ( &jobMutex ); }
#else
  inline void lockJobs(); inline void unlockJobs ();
#endif

 public:
  bool setAsyncCall ( Call ** call );
 public:
  Call * newCall ( SlotVirtual * slot, Object * obj, AgentId toAgentId );
  Call * newCall ( FullContextId * fullContextId, 
			SlotVirtual * slot, 
			Object * obj, 
			AgentId toAgentId );

  bool addCall ( Call * call );
  bool waitCall ( Call * call );
  Call * findCall ( FullContextId * fullContextId );
  bool checkCall ( Call * call );
  bool finishedCall ( Call * call );
  bool removeCall ( Call * call );

  /*
   * Active Jobs
   */
 protected:
  list<Job*> jobs;
 public:
  bool startJob  (FullContextId * fromContextId, 
		  ObjectId objId, 
		  ObjectNameId objNameId,
		  ObjectSlotId slotId, 
		  Connection * connection );
  bool forwardJob ( Job *job );
  bool runJob (Job * run);

  bool runEnqueuedJob ( ); // Runs next enqueued job..
  Job * findJob ( FullContextId * fullContextId );
  bool removeJob ( Job * job );

  /*
   * Connection Handling
   */ 
 protected:
  list<Connection*> connections;
#ifdef __DOOLPFORGE_CONNECTIONMUTEX
  pthread_mutex_t connectionMutex;
  inline void lockConnections()
    { if ( pthread_mutex_trylock ( &connectionMutex ) == 0 ) return; 
      Warn ( "connectionMutex already locked : waiting...\n" );
      pthread_mutex_lock ( &connectionMutex ); }
  inline void unlockConnections() { pthread_mutex_unlock ( &connectionMutex ); }
#else
  inline void lockConnections(); inline void unlockConnections ();
#endif

 public:
  bool addConnection ( Connection * connection );
  bool removeConnection ( Connection * connection );
  bool tryConnect ( const char * connectionString );

  Connection * getConnectionForAgentId (AgentId agentId )
    { return getConnectionForAgentId ( agentId, false ); }
  Connection * getConnectionForAgentId (AgentId agentId, bool strict );
  
  Connection * getDefaultConnection ( )
    { return getConnectionForAgentId ( getDefaultDistAgentId () ); }
  
  /*
   * ForgeServices
   */
 protected:
  ForgeServices * services;
 public:
  inline ForgeServices * getServices() const { return services; }
  doolpfunc<AgentId,string&> getAgentIdForService;
  AgentId getAgentIdForService_char ( char * sc );
  AgentId doolpfunclocal(getAgentIdForService) ( string& serviceName );  

  doolpfunc<Object*,string&> getService;
  Object * getService_char ( char * sc );
  Object * doolpfunclocal(getService) ( string & serviceName );
  AgentId getService_chooseAgentId ( string & serviceName );

  bool probeServices ( AgentId agentId );
  doolpfunc<list<string> *,AgentId> tellServices;
  list<string> * doolpfunclocal(tellServices) ( AgentId agentId );
  AgentId tellServices_chooseAgentId ( AgentId agentId )
  { return agentId; }

  /* 
   * ObjectIndexer Service 
   */
 protected:
  ObjectIndexer * objectIndexer;
 public:
  ObjectId getFreeObjectId (); // { return nextObjectId++; }
  ObjectIndexer * getObjectIndexer ( );
  bool setObjectIndexer ( ObjectIndexer * indexer )
  { objectIndexer = indexer; return true; }
 public:

  /*
   * Naming Services
   */
 protected:
  Naming * namingService;
 public:
  Naming * getNaming ();
  bool setNaming ( Naming * _namingService );

  /*
   * Persistance Service
   */
 protected:
  Persistance * persistanceService;
 public:
  Persistance * getPersistance ();
  bool setPersistance ( Persistance * _persistanceService );

  /*
   * Persistance-based Methods
   */
 protected:
  Object * getNamedObject ( string & objectName );
 public:
  bool bindContext ( string &contextName );
  bool bindContext ( const char *contextName ) { string __name = contextName; return bindContext ( __name ); }
  bool commit ();
  template<class T_Object>
    T_Object * getObject ( string &objectName )
    { return dynamic_cast<T_Object> ( getNamedObject ( objectName ) ); }
  template<class T_Object>
    T_Object * getObject ( const char * objectName )
    { string __name = objectName; return getObject<T_Object> ( __name ); }
  /*
   * auth Mechanisms
   */
  doolpfunc<bool,char*,char*> authPlain;
  bool doolpfunclocal(authPlain) ( char * name, char * password );


  /*
   * Log, give Stats
   */
  void logStats ( );
#undef DoolpObject_CurrentObject
};

typedef struct
{
  Forge * forge;
  Context * context;
  void *(*start_routine)(void*);
  void * arg;
} ForgeSetSpecificStruct;

}; // NameSpace Doolp
#endif // __DOOLP_DOOLPFORGE_H
