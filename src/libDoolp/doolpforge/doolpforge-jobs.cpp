#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpjob.h>
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpobjectstaticinfo.h>
#include <doolp/doolp-doolpobjectdynamicinfo.h>
#include <doolp/doolp-doolpobjectcache.h>
#include <doolp/doolp-doolpobjectbuffer.h>
#include <doolp/doolp-doolpcontext.h>
#include <doolp/doolp-doolpconnection.h>
#include <doolp/doolp-doolpexceptions.h>


void * DoolpForge_runJob ( void * arg )
{
  if ( arg == NULL )
    Bug ( "I have been given a null argument.\n" );
  __DOOLP_Log ( "arg at %p\n", arg );
  ((Doolp::Job*)arg)->forge->runJob
    ((Doolp::Job*)arg);
  return NULL;
}


#define DoolpJob__replyException(__job,__e)				\
  {									\
    try									\
      { Warn ( "Job from[%d.%d.%d] raised exception '%s'\n",		\
	       logDoolpFullContextId ( &(__job->fullContextId) ), (__e)->getMessage () ); \
	__job->preferedConn->startReply ( __job );			\
	__job->preferedConn->Write ( (__e) );				\
	__job->preferedConn->endMessage ();				\
      }									\
    catch ( Exception * e )				\
      {									\
	Warn ( "Could not reply job : got write exception '%s'\n", e->getMessage () ); \
	delete ( e );							\
      }									\
  }



bool Doolp::Forge::startJob (Doolp::FullContextId * fromContextId, 
			     Doolp::ObjectId objId, Doolp::ObjectNameId objNameId,
			     Doolp::ObjectSlotId slotId, 
			     Doolp::Connection * connection )
{
  __DOOLP_Log ( "New Job from context [%d:%d:%d] : objId=0x%x, slotId=0x%x\n",
		logDoolpFullContextId(fromContextId), objId, slotId );
  bool foundStaticSlot = false;

  Job * job = new Job ( this, objId, objNameId, slotId );
  job->preferedConn = connection;
  job->fullContextId = *fromContextId;
  job->fromAgentId = getAgentId ();
  job->toAgentId = job->fullContextId.agentId;
  
  if ( ( objId == 0 ) || ( objId == job->toAgentId ) )

    {
      AssertBug ( objNameId == getNameId(), "Invalid object NameId !\n" );
      Log ( "Deals with Forge\n" );
      job->obj = this;
    }
  else
    {
      Log ( "Trying to find object in cache...\n" );
      job->obj = getObjectCache()->get ( job->objId, &(job->fullContextId) );
      if ( job->obj != NULL ) 
	{ Log ( "Found object in cache !\n" ); }
    }
  if ( job->obj != NULL )
    {
      job->slot = job->obj->__getSlot ( slotId );
      if ( job->slot == NULL )
	{
	  NoObjectSlotFound e;
	  DoolpJob__replyException ( job, &e );
	  connection->forwardMessage ( NULL );
	  return true;
	}
    }
  else
    {
      job->slot = objectStaticInfo->getSlot ( objNameId, slotId );
      if ( job->slot == NULL )
	return forwardJob ( job );
      else
	foundStaticSlot = true;
    }

  AssertFatal ( job->slot != NULL, "Could not get slot for slotId '0x%x'\n", slotId );

  while ( ! connection->readMessageEnd () )
    {
      if ( connection->readParamSubSection () )
	{
	  job->callParams = job->slot->prepare ( connection, job );
	  AssertBug ( connection->readSubSectionEnd (), "Arguments read, but param subsection not finished.\n" );
	  continue;
	}
      Bug ( "Wrong section in newCall...\n" );
    }
  AssertBug ( job->callParams != NULL, "Could not run prepare !\n" );
  if ( foundStaticSlot )
    job->slot = NULL;

  job->status = JobStatus_Prepared;

  lockJobs ();
  jobs.push_back ( job );
  unlockJobs ();

#ifdef __DOOLPJOB_ALLOW_RUNINSAMETHREAD
  if ( options.forceJobsInSameThread )
    {
      if ( ! options.useWaitSpecificCall )
	{
	  Warn ( "Options configuration error : forceJobsInSameThread set but not useWaitSpecificCall !\n" );
	  Warn ( "Falling back to thread creation for this job.\n" );
	}
      else
	{
	  __DOOLP_Log ( "Forcing running in same thread\n" );
	  if ( jobs.size () > 1 )
	    {
	      Warn ( "Already running a job ! Maybe we should enqueue here !\n" );
	    }
	  // job->status = DoolpJobStatus_Enqueued;
	  //	      Bug ( "NOT IMPLEMENTED\n" );
	  //	      return true;
	    
	  pushContext ();
	  runJob ( job );
	  popContext ();
	  return true;
	}
    }
#endif //  __DOOLPJOB_ALLOW_RUNINSAMETHREAD
  pthread_t thread;
  newThread ( &thread, NULL, DoolpForge_runJob, (void *) job );
  return true;
}


bool Doolp::Forge::forwardJob ( Doolp::Job * job )
/*
 * Resolves an agentId to forward to.
 */
{
  Warn ( "Must find someone implementing objNameId=0x%x, slotId=0x%x !!!\n", job->objNameId, job->slotId );
  ObjectBuffer fakeObj( job->objId, job->objNameId );
  SlotFake fakeSlot( job->slotId, "Unkown slot name" );
  AgentId fwdAgentId = getObjectDynamicInfo()->getAgentIdForSlotId ( job->objNameId, job->slotId );
  if ( fwdAgentId != 0 )
    {
      Log ( "Forwarding Job to '%d'\n", fwdAgentId );
      try
	{
	  Call * subCall = NULL;
	  subCall = newCall ( &(job->fullContextId), &fakeSlot, &fakeObj, fwdAgentId );
	  job->preferedConn->forwardMessage ( subCall->preferedConn );
	  if ( subCall != NULL )
	    delete ( subCall );
	  removeJob ( job );
	  Log ( "Job forwarding done.\n" );
	  return true;
	}
      catch ( Exception * e )
	{
	  Warn ( "Forward Job : Exception '%s'\n", e->getMessage() );
	  delete (e);
	}
    }
  Log ( "Trashing job !\n" );
  job->preferedConn->forwardMessage ( NULL );
  NoObjectSlotFound e;
  DoolpJob__replyException ( job, &e );
  removeJob ( job );
  return true;
}

bool Doolp::Forge::runJob (Doolp::Job * job)
{
  Connection * conn = job->preferedConn;
  SlotVirtual::ReplyParamsVirtual * replyParams = NULL;

  __DOOLP_Log ( "******** Trying to run job %p (at [%d.%d.%d], for [%d.%d.%d] ******** \n", job,
		logDoolpFullContextId ( getFullContextId() ), logDoolpFullContextId ( &(job->fullContextId) ) );

  setContextFather ( &(job->fullContextId) );
  getContext()->setCurrentJob ( job );

  if ( job->obj == NULL )
    {
      job->status = JobStatus_getObject;
      AssertBug ( job->slot == NULL, "Object is not set, but Slot is set !!!\n" );
      try
	{
	  Log ( "Trying to getObject [%d.%d.%d]:%d\n", 
		logDoolpFullContextId ( &(job->fullContextId) ), job->objId );
	  job->obj = getObject ( &(job->fullContextId), job->objId );
	}
      catch ( Exception * e )
	{
	  DoolpJob__replyException ( job, e );
	  delete (e);
	  goto jobEnd;
	}
      if ( job->obj == NULL )
	{
	  NoObjectFound e;
	  DoolpJob__replyException ( job, &e );
	  goto jobEnd;
	}
      Log ( "Now that we have a DoolpObject, get slotId=%d\n", job->slotId );
      job->slot = job->obj->__getSlot ( job->slotId );
    }
  AssertBug ( job->slot != NULL, "No slot defined !\n" );

  job->status = JobStatus_Running;
  __DOOLP_Log ( "############### JOB RUNNING : '%s' (%p) ############\n", job->slot->getSlotName(), job );
  try
    {
      replyParams = job->slot->run ( job );
    }
  catch ( Exception * e )
    {
      DoolpJob__replyException ( job, e );
      delete (e);
      goto jobEnd;
    }
  __DOOLP_Log ( "############## FINISHED JOB : '%s' (%p) ############\n", job->slot->getSlotName(), job );

  AssertBug ( replyParams != NULL, "Slot returned a NULL replyParams !\n" );
  try
    {
      conn->startReply ( job );
      conn->startParamSubSection ( ); 
      replyParams->Write ( job, conn );
      conn->endSubSection ( );
      conn->endMessage ();
    }
  catch ( Exception * e )
    {
      Warn ( "Could not reply job : got write exception '%s'\n", e->getMessage () );
      delete ( e );
    }

 jobEnd:
  getContext()->log();

  job->status = JobStatus_Finished;
  notifyContextEnd ( &(job->fullContextId) );
  getContext()->setCurrentJob ( NULL );
  clearContextFather ( );

  removeJob ( job );

#ifdef __DOOLPJOB_ALLOW_RUNINSAMETHREAD
  if ( options.forceJobsInSameThread )
    {
      runEnqueuedJob ();
    }
#endif

  __DOOLP_Log ( "End of runJob\n" );
  return true;
}

bool Doolp::Forge::runEnqueuedJob ( )
{
  if ( jobs.size () == 0 )
    {
      __DOOLP_Log ( "No job to run.\n" );
      return true;
    }
  list<Job*>::iterator job;
  for ( job = jobs.begin () ; job != jobs.end () ; job++ )
    {
      if ( (*job)->status == JobStatus_Enqueued )
	{
	  (*job)->status = JobStatus_Prepared;
	  __DOOLP_Log ( "Running job at %p\n", (*job) );
	  runJob ( *job );
	  return true;
	}
    }
  __DOOLP_Log ( "No enqueued job to run.\n" );
  return true;
}


Doolp::Job * Doolp::Forge::findJob ( Doolp::FullContextId * fullContextId )
{
  lockJobs ();
  list<Job*>::iterator job;
  for ( job = jobs.begin () ; job != jobs.end () ; job ++)
    {
      AssertBug ( *job != NULL, " Job in list is set NULL !\n" );
      __DOOLP_Log ( "job at %p, fullContextId [%d,%d,%d]\n",
		    *job, logDoolpFullContextId ( &((*job)->fullContextId) ) );
      if ( fullContextIdCmp ( fullContextId, 
			      &((*job)->fullContextId) ) )
	
	{ Job * _job = *job; unlockJobs (); return _job; }
    }
  Warn ( "Could not find job for this context : [%d:%d:%d]\n",
	logDoolpFullContextId ( fullContextId ) );
  unlockJobs ();
  return NULL;
}

bool Doolp::Forge::removeJob ( Doolp::Job * job )
{
  __DOOLP_Log ( "Removing Job %p\n", job );
  lockJobs ();
  jobs.remove ( job );
  unlockJobs ();
  delete ( job );
  return true;
}

