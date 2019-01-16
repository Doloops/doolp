#include <doolp/doolp-doolpclasses.h>

#include <doolp/doolp-doolpconnection-tcp.h>
#include <doolp/doolp-doolpconnection-tcp-msg.h>

#include <doolp/doolp-doolpcall.h>
#include <doolp/doolp-doolpjob.h>
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpstream.h>

bool DoolpConnectionTCP::isMessageForCall ( DoolpMsg_Header * head, DoolpCall * call )
{
  if ( !(head->callFlags & DoolpMsg_CallFlag_ModeReply ) )
    return false;
  return fullContextIdCmp ( &(head->fullContextId),
			    &(call->fullContextId) );
}

unsigned int DoolpConnectionTCP::handleMsg ( DoolpCall * specificCall, DoolpStreamVirtual * specificStream )
{
  if ( ! rawRead ( &msgh, sizeof ( DoolpMsg_Header ) ) )
    {
      Warn ( "Unable to read Message Header\n" );
      return 0;
    }
  logDoolpMsg_Header ( &msgh );

  if ( specificCall != NULL )
    {
#ifdef __DOOLPCONNECTIONTCP_ALLOW_WAITSPECIFICCALL
      __DOOLP_Log ( "Was waiting for call : %p\n", specificCall );
      if ( isMessageForCall ( &msgh, specificCall ) )
	{
	  __DOOLP_Log ( "This message is for the call I expect.\n" );
	  if ( ! handleMsgReply ( ) )
	    return 0;
	  return 2; // specificCall ok
	}
      __DOOLP_Log ( "This message is not for the call I expect.\n" );
#else // __DOOLPCONNECTIONTCP_ALLOW_WAITSPECIFICCALL
      Bug ( "Functionnality not selected : __DOOLPCONNECTIONTCP_ALLOW_WAITSPECIFICCALL\n" );
#endif // __DOOLPCONNECTIONTCP_ALLOW_WAITSPECIFICCALL
    }
 
  if ( ! handleMsg ( ) ) 
    { 
      unlockRead ();
      return 0;
    }
  return 1;
}

bool DoolpConnectionTCP::handleMsg ( )
{
  if ( msgh.callFlags == DoolpMsg_CallFlag_EndConnection )
    {
      Warn ( "Remote host %s has ended connection.\n",
	     name );
      return false;
    }
  if ( msgh.toAgentId == 0 )
    {
      return handleMsgBroadcast ( );
    }
  if ( msgh.toAgentId != myForge->getAgentId () )
    {
      return handleMsgForward ( );
    }
  if ( msgh.callFlags & DoolpMsg_CallFlag_ModeReply )
    {
      return handleMsgReply ( );
    }
  if ( msgh.callFlags & DoolpMsg_CallFlag_NewCall )
    {
      return handleMsgNewCall ( );
    }
  if ( msgh.callFlags & DoolpMsg_CallFlag_ModeCall )
    {
      return handleMsgCall ( );
    }
  Bug ( "Unexpected header : callFlags = %d\n", msgh.callFlags );
  dumpDoolpMsg_Header ( &msgh );
  
  return false;

}

bool DoolpConnectionTCP::readStreamContents ( DoolpCallContext * callContext, DoolpStreamIndex idx )
{
  AssertBug ( idx < DoolpCallContextMaxStreams, "Stream index out of bounds !\n" );
  DoolpStreamVirtual * stm = callContext->doolpStreams[idx];
  if ( stm == NULL ) Bug ( "Could not find stream\n" );
  
  while ( ! readSubSectionEnd () )
    {
      __DOOLP_Log ( "iter\n" );
      stm->readFrom ( this, callContext );
    }
  //  Bug ( "test" );
  return true;
}

bool DoolpConnectionTCP::handleMsgCall ( )
{
  __DOOLP_Log ( "HANDLE MSG CALL !!!! \n" );
  // Find the ad hoc DoolpCall
  __DOOLP_Log ( "Call : [%d,%d,%d]\n",
		logDoolpFullContextId ( &(msgh.fullContextId) ) );
  DoolpJob * job = myForge->findJob ( &(msgh.fullContextId) );
  if ( job == NULL )
    Bug ( "Could not find job.\n" );
  __DOOLP_Log ( "Continuation of job %p..\n", job );
  
  // Parse each section
  while ( ! readMessageEnd () )
    {
      if ( readDoolpStreamSubSection () ) // bh.blockType == DoolpMsg_BlockType_DoolpStream )
	{
	  // blockHeaderIsRead = false; // Reload a new blockHeader.

	  __DOOLP_Log ( "New stream data for stream index : %d\n",
			bh.blockIndex );
	  readStreamContents ( job, bh.blockIndex );
	}
    }
  return true;
}



bool DoolpConnectionTCP::handleMsgNewCall ( )
{
  DoolpMsg_BlockHeader h;
  readBlockHeader ( &h );
  if ( DoolpMsg_getBlockRawSize (&h) 
       != sizeof ( DoolpMsg_NewCall_Header ) )

    Bug ( "Could not get NewCall_Header\n" );
      
  DoolpMsg_NewCall_Header newCall_Header;
  rawRead ( &newCall_Header, sizeof ( DoolpMsg_NewCall_Header ) );
  __DOOLPCONNECTIONTCP_Log ( "New Job : objId=%d, rpcId=%p.\n", 
		newCall_Header.objId, newCall_Header.rpcId );
  Bug ( "NOT IMPLEMENTED : startJob() changed signature, must handle this...\n" );
  /*
  myForge->startJob ( &(msgh.fullContextId), 
		      newCall_Header.objId, 
		      newCall_Header.rpcId, this );
  */
  return true;


}
bool DoolpConnectionTCP::handleMsgReply ( )
{
  DoolpCall * call = myForge->findCall ( &( msgh.fullContextId ) );
  if ( call == NULL )
    {
      Bug ( "Could not find call !\n" );
      return false;
    }
  __DOOLPCONNECTIONTCP_Log ( "Found call %p\n", call );
  while ( ! readMessageEnd () )
    {
      if ( readParamSubSection () )
	{
	  __DOOLP_Log ( "Reading handleReply Params\n" );
#warning TO REIMPLEMENT	  
	  Bug ( "TO (RE)IMPLEMENT : check DoolpConnectionXML !\n" );

	  /*
	    bool (*handleReply) ( DoolpCall * call, DoolpConnection * conn );
	    handleReply = call->rpc->handleReply;
	    bool res = handleReply ( call, this );
	    if ( ! res )
	    { Bug ( "Could not handle reply for call %p\n", call ); }
	  */
	  myForge->finishedCall ( call );

	  continue;
	}
      if ( readDoolpObjectSubSection () )
	{
	  __DOOLP_Log ( "Reading sent-back objects\n" );
	  DoolpObject * obj;
	  Read ( &obj, true ); // Try to resolve.
	  continue;
	}
      if ( readDoolpStreamSubSection () )
	{
	  __DOOLP_Log ( "Reading DoolpStream\n" );
	  __DOOLP_Log ( "New stream data for stream index : %d\n",
			bh.blockIndex );
	  readStreamContents ( call, bh.blockIndex );
	  continue;
	}
      Bug ( "Fuzzy things on the phone... mode=%d, type=%d, index=%d\n",
	    bh.blockMode, bh.blockType, bh.blockIndex );
    }
  return true;
}

bool DoolpConnectionTCP::handleMsgForward ( )
{
  DoolpConnection * to = 
    myForge->getConnectionForAgentId ( msgh.toAgentId, true );
  if ( to == NULL || to->distAgentId != msgh.toAgentId )
    {
      Bug ( "fw impossible : no connection to agentId %d - deleting msg ??\n", 
	     msgh.toAgentId );
      to = NULL;
    }
  else
    {
      __DOOLPCONNECTIONTCP_Log ( "Forwarding to agentId %d : conn %p\n", 
				 to->distAgentId, to );
    }
  forwardMessage ( (DoolpConnectionTCP*) to );
  return true;
}


bool DoolpConnectionTCP::handleMsgBroadcast (  )
{
  Bug ( "Not Implemented !\n" );
  return false;
}
