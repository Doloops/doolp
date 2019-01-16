#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpcall.h>
#include <doolp/doolp-doolpconnection.h>
#include <doolp/doolp-doolpcontext.h>
#include <doolp/doolp-doolpstream.h>
#include <doolp/doolp-doolpexceptions.h>
#include <doolp/doolp-doolpnaming.h> // TEMP

Doolp::Call * Doolp::Forge::newCall ( Doolp::SlotVirtual * slot, Doolp::Object * obj, Doolp::AgentId toAgentId )
{
  AssertBug ( getContext() != NULL, "Could not get current context !\n" );
  return newCall ( getFullContextId(), slot, obj, toAgentId );
}

Doolp::Call * Doolp::Forge::newCall ( Doolp::FullContextId * fullContextId, 
				      Doolp::SlotVirtual * slot, 
				      Doolp::Object * obj, 
				      Doolp::AgentId toAgentId )
{
  Log ( "New newCall() : slot=%p, obj=%p, toAgentId=%d\n",
	slot, obj, toAgentId );
  Warn ( "Check : is this Really Implemented ?\n" );
  Call * call = new Call();
  Connection * conn = getConnectionForAgentId ( toAgentId );
  if ( conn == NULL )
    throw new CallNoAgentFound ();
  incrementStepping ( );
  call->fullContextId = *fullContextId;
  call->slot = slot;
  call->preferedConn = conn;
  call->objId = obj->getObjectId();
  call->obj = obj;
  call->fromAgentId = getAgentId ();
  call->toAgentId = toAgentId;
  
  __DOOLP_Log ( "New Call : %p [%d:%d:%d]\n", call,
		logDoolpFullContextId ( &(call->fullContextId) ) );
  if ( fullContextId->agentId == getAgentId () )
    {
      lockCalls ();
      calls.push_back ( call ); // Dangerous ?$
      unlockCalls ();
    }
  else
    { Warn ( "Forwarded call : not added to call stack\n" ); }
  conn->startNewCall ( call );
  return call;
}

bool Doolp::Forge::addCall ( Doolp::Call * call )
{
  Warn ( "Nothing to be done here (DEPRECATED ?)\n" );
  //  __DOOLP_Log ( "Currently %d Calls\n", calls.size () );
  //  __DOOLP_Log ( "Adding Call : %p [%d:%d:%d]\n", call,
  //  		    logDoolpFullContextId ( &(call->fullContextId) ) );
  //  __DOOLP_Log ( "Head at %p\n", *(calls.begin () ) );
  return true;
}
bool Doolp::Forge::waitCall ( Doolp::Call * call )
{
  __DOOLP_Log ( "Waiting Call : %p [%d:%d:%d]\n", call,
		logDoolpFullContextId ( &(call->fullContextId) ) );
  if ( call->async )
    {
      __DOOLP_Log ( "Call is set asynchronous\n" );
    }
  else
    {
#ifdef __DOOLPCONNECTION_ALLOW_WAITSPECIFICCALL    
      if ( options.useWaitSpecificCall )
	{
	  __DOOLP_Log ( "Trying to WaitSpecific\n" );
	  call->waitSpecific = true;
	  if ( getContext()->backgroundWork ( call ) )
	    return true;
	  __DOOLP_Log ( "getContext()::backgroundWork : waitSpecific Failed.\n" );
	  call->waitSpecific = false;
	}
      else
	{
	  __DOOLP_Log ( "waitSpecific option not set.\n" );
	}
#else
      __DOOLP_Log ( "waitSpecificCall not selected.\n" );
#endif // __DOOLPCONNECTION_ALLOW_WAITSPECIFICCALL
    }
#ifdef __DOOLPCALL_USE_SEM
  sem_wait ( &(call->semaphore) );
  if ( call->replied )
    {
      _dumpBuffer ( call->result, call->resultSize );

      return true;
    }
  Warn ( "Call %p sent sem but not replied !\n", call );
#endif // __DOOLPCALL_USE_SEM
  for ( unsigned int __i = 0 ; __i < 0xffffffff ; __i++ ) 
   { 
      __DOOLP_Log ( "Call %p : attempt wait %d\n", call, __i );
      if ( call->replied == true ) break;		   
      if ( __i == 100 ) Bug ( "Wait %p timeout\n", call );
      sleep ( 1 );
    }
  return true;
}


bool Doolp::Forge::checkCall ( Doolp::Call * call )
{
  if ( call->exception != NULL )
    {
      __DOOLP_Log ( "Call %p : recieved exception : '%s'\n", 
		    call, call->exception->getMessage() );
      Doolp::Exception * e = call->exception;
      removeCall ( call );
      e->throwMyself ();
    }
  return true;
}

bool Doolp::Forge::finishedCall ( Doolp::Call * call )
{
  __DOOLP_Log ( "Finished Call : %p [%d:%d:%d]\n", call,
		logDoolpFullContextId ( &(call->fullContextId) ) );
  call->replied = true;
  for ( int nb = 0 ; nb < CallContextMaxStreams ; nb ++ )
    {
      if ( call->doolpStreams[nb] == NULL ) continue;
      __DOOLP_Log ( "Break-finishing doolpStream %p (idx=%d)\n",
		    call->doolpStreams[nb], nb );
      call->doolpStreams[nb]->callFinish ( call );
    }

#ifdef __DOOLPCALL_USE_SEM
  sem_post ( &(call->semaphore) );
#else
  usleep ( 20 );
#endif
  return true;
}

Doolp::Call * Doolp::Forge::findCall ( Doolp::FullContextId * fullContextId )
{
  lockCalls ();
  list<Call*>::iterator call;
  for ( call = calls.begin () ; call != calls.end () ; call ++)
    {
      if ( fullContextIdCmp ( fullContextId, 
			      &( (*call)->fullContextId) ) )
	{
	  if ( (*call)->replied )
	    {
	      Warn ( "Call [%d,%d,%d] already replied !!\n",
		     logDoolpFullContextId ( fullContextId ) );
	      unlockCalls ();
	      return NULL;
	    }
	  unlockCalls ();
	  return (*call);
	}
    }
  Warn ( "Could not find call for this context : [%d:%d:%d]\n",
	 logDoolpFullContextId ( fullContextId ) );
  unlockCalls ();
  return NULL;

}

bool Doolp::Forge::removeCall ( Doolp::Call * call )
{
  __DOOLP_Log ( "Removing call %p\n", call );
  lockCalls ();
  calls.remove ( call );
  delete ( call );
  unlockCalls ();
  return true;
}
