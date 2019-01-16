#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpforgescheduler.h>
#include <doolp/doolp-doolpcontext.h>
#include <doolp/doolp-doolpconnection.h>
#include <doolp/doolp-doolpcall.h>
#include <doolp/doolp-doolpjob.h>

#include <doolp/doolp-doolpforgeservices.h>
#include <doolp/doolp-doolpobjectstaticinfo.h>
#include <doolp/doolp-doolpobjectdynamicinfo.h>
#include <doolp/doolp-doolpobjectcache.h>

#ifdef __DOOLP_USE_DOOLPTHREADS
#include <doolp/doolp-doolpthreads.h>
#endif


void contextDestructor (void * data );


#ifdef __DOOLP_USE_DOOLPTHREADS
void _doolpforge_contextdestruct ( void * meta_struct );
#endif 

// DoolpObjectsInfo * getDoolpForgeObjectsInfo ( );

Doolp::Forge::Forge ( ) { Forge (0); }

Doolp::Forge::Forge ( Doolp::ObjectId _agentId )
{ 
  /*
   * Raw Initing.
   */
  memset ( &(options), 0, sizeof ( ForgeOptions ) );
  memset ( &(myAgent), 0, sizeof ( Agent) );
  objectIndexer = NULL;
  namingService = NULL;
  persistanceService = NULL;
  if ( pthread_key_create (&contextThreadKey, contextDestructor) != 0 )
    {
      Fatal ( "pthread_key_create : could not create contextThreadKey\n" );
    }
  Log ( "pthread_key_create : Key=0x%x\n", contextThreadKey );

  /*
   * Miscelaneous services initing
   */ 
  scheduler = new ForgeScheduler();
  services = new ForgeServices ( this );
  objectStaticInfo = new ObjectStaticInfo ( this );
  objectDynamicInfo = new ObjectDynamicInfo ( this, objectStaticInfo );
  objectCache = new ObjectCache (this);

  /*
   * Forge as a Object
   */
  setAgentId ( _agentId );
  setForge ( (Forge*)this );
  setOwner ( _agentId );
  objectId = _agentId;
  __initStatic (this);

  Info ( "New Forge with agentId=%d\n", getAgentId() ); 
#ifdef __DOOLPFORGE_CALLMUTEX
  //  DOOLP_PTHREAD_MUTEX_INIT ( &callMutex );
  //  callMutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_init ( &callMutex, NULL );
#endif
#ifdef __DOOLPFORGE_JOBMUTEX
  pthread_mutex_init ( &jobMutex, NULL );
#endif 
#ifdef __DOOLPFORGE_CONNECTIONMUTEX
  pthread_mutex_init ( &connectionMutex, NULL );
#endif 

  /*
   * Default Value for various stuff.
   */

  nextContextId = 1;
  nextAgentId = 10; // TO BE DEPRECATED !
  defaultDistAgentId = 0;
  runningThreads = 0;
  options.jobObjectTTL = 10;
  options.allowDoolpObjectBuffer = false;

  Context * currentContext = newContext ( );
  setCurrentContext ( currentContext );


  __DOOLP_Log ( "Thread 0x%x : set context %p\n", (int)pthread_self(), currentContext );

#ifdef  __DOOLP_USE_DOOLPTHREADS
#warning Initing DoolpThreads
  Warn ( "Inting DoolpThreads !\n" );
  doolpThreads_init ( ); // _doolpforge_contextdestruct );
#endif

  // Start of active Services.
  getScheduler()->start();
}

Doolp::Forge::~Forge ( )
{
  __DOOLP_Log ( "Destroying DoolpForge (agentId %d)\n", 
		getAgentId () );
#ifdef __DOOLPFORGE_CALLMUTEX
  pthread_mutex_destroy ( &callMutex );
#endif // __DOOLPFORGE_CALLMUTEX
#ifdef __DOOLPFORGE_JOBMUTEX
  pthread_mutex_destroy ( &jobMutex );
#endif // __DOOLPFORGE_JOBMUTEX
#ifdef __DOOLPFORGE_CONNECTIONMUTEX
  pthread_mutex_destroy ( &connectionMutex );
#endif // __DOOLPFORGE_CONNECTIONMUTEX

  //  pthread_mutex_destroy ( connectionMutex );
  //  delete ( connectionMutex );
}

bool Doolp::Forge::close ( )
{
  __DOOLP_Log ( "runningThreads='%d'\n", runningThreads );

  lockConnections ();

  __DOOLP_Log ( "terminating connections.\n" );
  list<Connection*>::iterator conn;
  for ( conn = connections.begin ();
	conn != connections.end ();
	conn ++ )
    {
      (*conn)->endConnection ();
    }
  __DOOLP_Log ( "endConnection terminated.\n" );
  
  while ( connections.size () > 0 )
    {
      delete ( connections.front() );
      connections.pop_front ();
    }
  
  unlockConnections ();

  __DOOLP_Log ( "close terminated.\n" );
  return true;
}

/*
 * DoolpForge as a DoolpObject
 */
bool Doolp::Forge::serialize ( Doolp::Connection * connection )		
{
  Bug ( "Why am I here ?\n" );
  return false;
}
bool Doolp::Forge::serialize ( Doolp::Connection * conn,			
			     Doolp::ObjectParamId paramId )	
{
  Bug ( "Why am I here ?\n" );
  return false;
}
bool Doolp::Forge::unserialize ( Doolp::Connection * connection )
{
  Bug ( "Why am I here ?\n" );
  return false;
}



bool Doolp::Forge::setAgentId ( Doolp::AgentId _agentId )
{ 
  if ( _agentId == 0 )
    {
      Warn ( "Can not set agentId to 0  !\n" );
      return false;
    }
  myAgent.agentId = _agentId; 
  // Here we should change all the Doolp::FullContextId previously created (at least !)
  // First, update the current context
  __DOOLP_Log ( "getContext() = %p\n", getContext () );
  if ( getContext() )
    getContext()->fullContextId.agentId = myAgent.agentId;
  return true; 
}

Doolp::AgentId Doolp::Forge::getAgentId ( )
{
  return myAgent.agentId;
}


#define boolStr(_v) _v ? "true" : "false"
extern char * Doolp::ConnectionStatus_Str[];
char * DoolpJobStatus_Str[] =
  {
    "Unknown", "Preparing", "Prepared",
    "Enqueued", "getObject", "Running",
    "Finished"
  };

void Doolp::Forge::logStats ( )
{
  __DOOLP_Log ( "Stats for agentId=%d\n", getAgentId() );

  __DOOLP_Log ( "Agents : %d\n", agents.size () );
  list<Agent *>::iterator agent;
  for ( agent = agents.begin () ; agent != agents.end () ; agent++ )
    {
      __DOOLP_Log ( "\tAgent %d\n", (*agent)->agentId );
    }

  lockConnections ();
  __DOOLP_Log ( "Connections : %d\n", connections.size () );
  list<Connection *>::iterator conn;
  for ( conn = connections.begin () ; conn != connections.end () ; conn ++ )
    {
      __DOOLP_Log ( "\tConnection to %d : status %s\n",
		    (*conn)->distAgentId,
		    ConnectionStatus_Str[(*conn)->getStatus ()] );
    }
  unlockConnections ();

  lockCalls ();
  __DOOLP_Log ( "Calls : %d\n", calls.size () );
  list<Call*>::iterator _call;
  for ( _call = calls.begin () ; _call != calls.end () ; _call ++ )
    {
      Call * call = *_call;
      __DOOLP_Log ( "\tCall context=[%d,%d,%d], "
		    "from=%d, to=%d, "
		    "slotId=0x%x, replied=%s, async=%s\n",
		    logDoolpFullContextId ( &(call->fullContextId ) ),
		    call->fromAgentId, call->toAgentId,
		    call->slotId,
		    boolStr ( call->replied ),
		    boolStr ( call->async ) );
      AssertBug ( call->slot != NULL, "Call has no slot defined !\n" );
      __DOOLP_Log ( "\tCall has slotName '%s'\n",
		    call->slot->getSlotName() );
    }
  unlockCalls ();

  lockJobs ();
  __DOOLP_Log ( "Jobs : %d\n", jobs.size () );
  list<Job*>::iterator _job;
  for ( _job = jobs.begin () ; _job != jobs.end () ; _job ++ )
    {
      Job * job = *_job;
      __DOOLP_Log ( "\tJob context=[%d,%d,%d], "
		    "from=%d, to=%d, "
		    "slotId=0x%x, slotName='%s', objId=%d, status=%s\n",
		    logDoolpFullContextId ( &(job->fullContextId ) ),
 		    job->fromAgentId, job->toAgentId,
		    job->slotId,
		    job->slot != NULL ? job->slot->getSlotName() : "Unkwown",
		    job->objId,
		    DoolpJobStatus_Str[job->status]);
    }
  unlockJobs ();
  //  objectCache->log ();
}
