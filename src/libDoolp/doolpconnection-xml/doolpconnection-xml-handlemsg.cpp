#include <doolp/doolp-doolpconnection-xml.h>
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpcall.h>

#include <doolp/doolp-doolpexceptions.h>

#undef __DOOLPXML_Log
#define __DOOLPXML_Log __DOOLP_Log

#define __checkEventName(__key) \
  AssertBug ( parser->isEventName ( __key ), "Not on the right event : Excepted '%s', Had '%s'.\n", \
	      (__key).getKeyword(), parser->getEventName () );


bool Doolp::ConnectionXML::runHandleThread ( Doolp::Call * specificCall, Doolp::StreamVirtual * specificStream )
{
  AssertBug ( status == ConnectionStatus_Ready, "Shall be here with status==Ready.\n" );
  __DOOLPXML_Log ( "********** Run Handle Thread ****************\n" );

  while ( true )
    {
      try
	{
	  parser->parse();
	  if ( parser->getEventsNumber() == 0 )
	    {
	      Warn ( "Could not parse anything !\n" );
	      continue;
	    }
	  unsigned int res =  handleMsg ( specificCall, specificStream );
	  switch ( res )
	    {
	    case 0:
	      Warn ( "Error.\n" );
	      return false;
	    case 1:
	      continue;
	    case 2:
	      return true;
	    }
	}
      catch ( DMLX::ParserException * e )
	{
	  Warn ( "Recieved exception from parser : '%s'\n", e->getMessage () );
	  status = ConnectionStatus_Lost;
	  return false;
	}
    }
}


unsigned int Doolp::ConnectionXML::handleMsg ( Doolp::Call * specificCall, Doolp::StreamVirtual * specificStream )
{
  AssertBug ( parser != NULL, "No parser defined.\n" );
  __DOOLPXML_Log ( "*********** Reading message ****************\n" );
  //  Assert ( specificCall != NULL, "Not yet implemented !"  )
  try
    {
      contextId = NULL;
      AssertBug ( parser->getEventsNumber() > 0, "No Event to handle !\n" );
      
      if ( parser->isEventName ( keywords.ping ) )
	{
	  lockWrite ();
	  writer->writeMarkup ( keywords.ping_reply );
	  writer->writeMarkupEnd ();
	  unlockWrite ();
	  parser->popEventAndEnd ();
	  return 1;
	}
      else if ( parser->isEventName ( keywords.ping_reply ) )
	{
	  timeb reply;
	  ftime ( &reply );
	  int usec = ( ( reply.time - pingTime.time ) * 1000 + ( reply.millitm - pingTime.millitm ) );
	  int sec = usec / 1000;
	  usec = usec % 1000;
	  Info ( "Ping : took %d.%03d seconds.\n", 
		 sec, usec );
	  parser->popEventAndEnd ();
	  return 1;
	}
      //      AssertBug ( parser->isEventName ( keywords.DoolpMessage ), 
      //		  "Unexpected event : '%s'\n", parser->getEvent()->name->getKeyword() );
      unsigned int type = 0; 
      // Type : 1 = newCall, 2 = Reply, 3 = InCall
      
      if ( parser->isEventName ( keywords.Call ) )
        type = 1;
      else if ( parser->isEventName ( keywords.Reply ) )
	type = 2;
      else if ( parser->isEventName ( keywords.InCall ) )
	type = 3;
      else Bug ( "Unexpected event : '%s'\n", parser->getEventName () );
      
      //      char * type = parser->getAttr ( keywords.type );
      AgentId fromAgentId = parser->getAttrInt ( keywords.fromAgentId );
      AgentId toAgentId = parser->getAttrInt ( keywords.toAgentId );
      
      if ( toAgentId != myForge->getAgentId () )
	{
	  Connection * to = 
	    myForge->getConnectionForAgentId ( toAgentId, true );
	  AssertFatal ( toAgentId != distAgentId, "Message target is from agent connected by this !\n" );
	  
	  if ( to != NULL && ( to->distAgentId == myForge->getAgentId () ) )
	    {
	      Warn ( "no use to forward to myself !\n" );
	      to = NULL;
	    }
	  if ( to == NULL )
	    {
	      Warn ( "fw impossible : no connection to agentId %d - deleting msg.\n", 
		     toAgentId );
	      parser->popEvent ();
	      Read ( &contextId );
	      AssertFatal ( contextId != NULL, "Could not read FullContextId !\n" );
	      __DOOLPXML_Log ( "To Trash : Message type='%d', from='%d', to='%d', context=[%d,%d,%d]\n",
			       type, fromAgentId, toAgentId,
			       logDoolpFullContextId ( contextId ) );
	      if ( type == 1 )
		{
		  __DOOLPXML_Log ( "Trashing a NewCall message : sending an exception\n" );
		  lockWrite ();
		  writer->writeMarkup ( keywords.Reply );
		  writer->writeAttrInt ( keywords.fromAgentId, myForge->getAgentId() );
		  writer->writeAttrInt ( keywords.toAgentId, fromAgentId );
		  Write ( contextId );
		  Exception * e = new CallNoAgentFound ();
		  Write ( e );
		  delete ( e );
		  endMessage (); // Performs unlockWrite () !
		}
	      free ( contextId );
	      contextId = NULL;
	      
	    }
	  forwardMessage ( (ConnectionXML*) to );
	  return true;
	}
      
      parser->popEvent ();
      Read ( &contextId );
      
      __DOOLPXML_Log ( "Message type='%d', from='%d', to='%d', context=[%d,%d,%d]\n",
		       type, fromAgentId, toAgentId,
		       logDoolpFullContextId ( contextId ) );
      if ( type == 1 ) // Call
	{
	  __checkEventName ( keywords.CallParams );
	  __DOOLPXML_Log ( "New Call\n" );
	  ObjectId objId = parser->getAttrInt ( keywords.objectId );
	  ObjectNameId objNameId = parser->getAttrInt ( keywords.objectNameId );
	  ObjectSlotId slotId = parser->getAttrInt ( keywords.slotId );
	  parser->popEventAndEnd ();
	  __DOOLPXML_Log ( "Starting Job slotId=0x%x, objId=0x%x\n",
			   slotId, objId );
	  myForge->startJob ( contextId, objId, objNameId, slotId, this );
	  goto readContinue;
	}
      else if ( type == 2 ) // Reply
	{
	  __DOOLPXML_Log ( "Reply\n" );
	  Call * call = myForge->findCall ( contextId );
	  if ( call == NULL )
	    {
	      Warn ( "Could not find call [%d,%d,%d] : trash\n",
		     logDoolpFullContextId ( contextId ) );
	      forwardMessage ( NULL );
	      goto readContinue;
	    }
	  
	  if ( specificCall != NULL )
	    {
	      __DOOLPXML_Log ( "specificCall(%p) = [%d,%d,%d] (call=%p)",
			       specificCall, logDoolpFullContextId ( &(specificCall->fullContextId) ),
			       call );
	    }
	  else
	    {
	      __DOOLPXML_Log ( "no specificCall given !\n" );
	    }
	  while ( ! readMessageEnd () )
	    {
	      __DOOLPXML_Log ( "Reading event : '%s'\n", parser->getEvent()->name->getKeyword() );
	      if ( readParamSubSection () )
		{
		  AssertBug ( call->slot != NULL, "No slot defined for this call : %p\n", call );
		  call->replyParams = call->slot->handleReply ( this );
		  AssertFatal ( call->replyParams != NULL, "Slot gave no replyParams !\n" );
		  AssertFatal ( readSubSectionEnd (), "Could not read subsection end : event is %s\n",
				parser->getEvent()->name->getKeyword() );
		}
	      else if ( readObjectSubSection () )
		{
		  __DOOLPXML_Log ( "Reading sent-back objects\n" );
		  Object * obj;
		  Read ( &obj, true, true );
		}
	      else if ( readExceptionSubSection () )
		{
		  __DOOLPXML_Log ( "Reading exception\n" );
		  Read ( &(call->exception) );
		}
	    }
	  myForge->finishedCall ( call );
	  
	  if ( call == specificCall )
	    goto readSpecific;
	  goto readContinue;
	}
      else if ( type == 3 ) // InCall (streams)
	{
	  StreamVirtual * stream = NULL;
	  if ( parser->isEventName ( keywords._Stream ) )
	    {
	      StreamIndex idx = parser->getAttrInt ( keywords.index );
	      CallContext * callContext = NULL;
	      if ( toAgentId == contextId->agentId )
		{
		  callContext = (CallContext*) myForge->findCall ( contextId ); // This is a call
		}
	      else 
		{
		  callContext = (CallContext*) myForge->findJob ( contextId ); // This is a job
		}
	      if ( callContext == NULL )
		{ 
		  Error ( "Could not get callContext for ContextId [%d.%d.%d]\n",
			  logDoolpFullContextId ( contextId ) );
		  return 0;
		}
	      stream = callContext->getStream ( idx );
	      AssertBug ( stream != NULL, "Could not find stream idx %d for ContextId [%d.%d.%d]\n",
			  idx, logDoolpFullContextId ( contextId ) );
	      readStream ( callContext, stream, idx );
	    }
	  if ( ! readMessageEnd () )
	    Bug ( "Unexpected event : '%s'\n", parser->getEvent()->name->getKeyword() );
	  if ( specificStream == stream )
	    {
	      Log ( "Found specificStream %p : quitting\n", stream );
	      goto readSpecific;
	    }
	  goto readContinue;
	}
      return 0;
    }
  catch ( DMLX::ParserException * e )
    {
      Warn ( "Recieved exception from DMLX::Parser : '%s'\n", e->getMessage () );
      throw e;
    }
 readSpecific:
  free ( contextId );
  contextId = NULL;
  return 2;
  
 readContinue:
  free ( contextId );
  contextId = NULL;
  return 1;
  /*
 readError:
  return 0;
  */
}
