#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpobjectcache.h>
#include <doolp/doolp-doolpcontext.h>
#include <doolp/doolp-doolpjob.h>


#ifdef __DOOLP_USE_DOOLPTHREADS
#include <doolp/doolp-doolpthreads.h> // Only if __DOOLP_USE_DOOLPTHREADS is set.
#endif 

void contextDestructor ( void * data )
{
  // Here we should remove the context from the forge..
  Doolp::Context * context = (Doolp::Context*) data;
  context->getForge()->notifyContextEnd ( context );

  // delete ( context->changeSet.perSteppingChangeSet );
  delete ( context );
}


#ifdef __DOOLP_USE_DOOLPTHREADS
void _doolpforge_contextdestruct ( void * meta_data  ) // for use with doolpthreads
{
  contextDestructor ( (void*) (((Doolp::Forge*) meta_data)->getContext () ) );
  //  ((DoolpForge*) meta_data)->runEnqueuedJob ();
}
#endif
void Doolp::Forge::setCurrentContext ( Doolp::Context * context ) // Shall be renamed setContext ? 
{
  __DOOLP_Log ( "thread 0x%x : setting context to [%d,%d,%d]\n",
		(int)pthread_self (), logDoolpFullContextId ( &( context->fullContextId ) ) );
  int res = pthread_setspecific ( contextThreadKey, context );
  if ( res != 0 )
    {
      Error ( "Could not set context %p for this thread : 0x%x (res=%d)\n", context, (int)pthread_self (), res );
      LogErrNo;
    }
}


Doolp::Context * Doolp::Forge::getContext () // Warning : to be re-implemented while multithread capabilities
{
  Context * context;
  if ( ( context = (Context*) pthread_getspecific ( contextThreadKey ) ) != NULL )
    {
      return context;
    }
  Warn ( "Could not find current context for this thread : 0x%x\n", (int)pthread_self() );
  return NULL;
}

Doolp::FullContextId * Doolp::Forge::getFullContextId ()
{ return &(getContext()->fullContextId); }

Doolp::FullContextId * Doolp::Forge::getNewFullContextId()
{ 
  FullContextId * newC = new FullContextId (); 
  newC->agentId = getAgentId (); 
  newC->contextId = nextContextId++;
  newC->stepping = 1;
  return newC; 
}

bool Doolp::Forge::setNextContextId(Doolp::ContextId id) 
{ 
  AssertFatal ( nextContextId > id, "Can not go backwards !\n" ); 
  nextContextId = id; 
  return true; 
}


Doolp::Context * Doolp::Forge::newContext() 
{ 
  Context * context = new Context ( this, getAgentId (),
					      nextContextId ++ );
  return context;
}



#ifdef __DOOLPFORGE_ALLOW_PUSHED_CONTEXTS
void Doolp::Forge::pushContext ()
{
  AssertBug ( getContext() != NULL, "Could not get current context !!!\n" );
  Context * oldContext = getContext ();
  Context * context = newContext ();
  context->pushedContext = oldContext;
  context->handledConnection = oldContext->handledConnection;
  setCurrentContext ( context );

  __DOOLP_Log ( "Pushed context : %p. Now context is %p\n",
		oldContext, context );
}
bool Doolp::Forge::popContext ()
{
  Context * context = getContext ();
  if ( context->pushedContext == NULL )
    {
      Bug ( "No pushed Context !\n" );
      return false;
    }
  setCurrentContext ( context->pushedContext );
  __DOOLP_Log ( "Now context is %p (was %p)\n", 
		context->pushedContext,
		context );
  // Should destroy context here...
  return true;
}

#endif // __DOOLPFORGE_ALLOW_PUSHED_CONTEXTS

bool Doolp::Forge::notifyContextEnd (Doolp::Context * context)
// This is a pure notification, no real modification
{
  runningThreads--;
  return notifyContextEnd ( &(context->fullContextId) );
}
bool Doolp::Forge::notifyContextEnd (Doolp::FullContextId * id )
{
  getObjectCache()->notifyContextEnd ( id );
  return true;
}

void Doolp::Forge::setContextFather ( Doolp::FullContextId * father )
{
  getContext()->father = *father;
}
void Doolp::Forge::clearContextFather ( )
{
  FullContextId blank = { 0, 0, 0 };
  setContextFather ( &blank );
}
Doolp::FullContextId * Doolp::Forge::getContextFather ( )
{
  return &(getContext()->father);
}

Doolp::AgentId Doolp::Forge::getAgentIdForThisJob ()
{
  Context * context = getContext ();
  AssertBug ( context != NULL, "No current context !\n" );
  if ( context->getCurrentJob() == NULL )
    {
      __DOOLP_Log ( "Not running a job !\n" );
      return 0;
    }
  return context->getCurrentJob ()->toAgentId;
}

void Doolp::Forge::incrementStepping () 
  { getContext()->fullContextId.stepping ++; }


void * doolpForgeSetSpecific ( void * _args )
{
  AssertBug ( _args != NULL, " No args given\n" );
  Doolp::ForgeSetSpecificStruct * args = (Doolp::ForgeSetSpecificStruct *) _args;
  __DOOLP_Log ( "_args=%p : forge=%p, context=%p, start_routine=%p, args=%p\n",
		args, args->forge, args->context, args->start_routine, args->arg );
  Doolp::Forge * mainForge = args->forge;
  Doolp::Context * context = args->context;
  void * result;
  mainForge->setCurrentContext ( context );
  __DOOLP_Log ( "Thread 0x%x has context [%d,%d,%d]\n",
		(int) pthread_self(), logDoolpFullContextId (context->getFullContextId()) );
  result = args->start_routine (args->arg);
  args->forge->notifyContextEnd ( context );
  free ( _args );
  return result;
}

bool Doolp::Forge::newThread ( Doolp::Context * newContext, 
			       pthread_t * pthread,
			       const pthread_attr_t *attr,
			       void *(*start_routine)(void*), void *arg)
{
  Doolp::ForgeSetSpecificStruct * args = new Doolp::ForgeSetSpecificStruct ();
  args->forge = this;
  args->context = newContext;
  args->start_routine = start_routine;
  args->arg = arg;
  __DOOLP_Log ( "Starting newThread with context %p\n", newContext );
  // Here we should add this thread to a list of threads inside of forge...
  runningThreads++;
#undef __DOOLP_USE_DOOLPTHREADS
#ifdef  __DOOLP_USE_DOOLPTHREADS
  //  int res = doolpThreads_create ( pthread, NULL, doolpForgeSetSpecific, args, _doolpforge_contextdestruct, this );
  int res = doolpThreads_create ( pthread, NULL, doolpForgeSetSpecific, args, NULL, NULL );
  if ( res != 0 )
    {
      Error ( "Could not create thread.\n" );
      return false;
    }

  return true;
#else
  int res = pthread_create ( pthread, attr, /* attr,*/ doolpForgeSetSpecific, args );
  if ( res != 0 )
    { 
      Error ( "Could not pthread_create\n" );
      LogErrNo; 
      Bug ( "This is fatal.\n" );
    }
  Log ( "pthread created at '0x%x'\n", (int)*pthread );
  res = pthread_detach ( *pthread );
  if ( res != 0 )
    { 
      Error ( "Could not pthread_detach\n" );
      LogErrNo; 
      Bug ( "This is fatal.\n" );
    }

  return true;
#endif
}

bool Doolp::Forge::newThread ( pthread_t * pthread,
			       const pthread_attr_t *attr,
			       void *(*start_routine)(void*), void *arg)
{
  Context * context = newContext (  );
  __DOOLP_Log ( "New Thread with context %p\n", context );
  return newThread ( context, pthread, attr, start_routine, arg );
}


