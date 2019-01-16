#include <doolp/doolp-doolpconnection-tcp.h>
#include <doolp/doolp-doolpconnection-tcp-msg.h>
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpjob.h>
#include <doolp/doolp-doolpcall.h>
#include <doolp/doolp-doolpcallcontext.h>

/*
 * ******************************
 * Message-Oriented Connection
 * ******************************
 */

bool DoolpConnectionTCP::startNewCall (
				       DoolpCall * call
				       )
{
  startMessage ( call,
		 DoolpMsg_CallFlag_NewCall );

  DoolpMsg_NewCall_Header newCall_h;
  newCall_h.objId = call->objId;
  newCall_h.slotId = call->slotId;

  DoolpMsg_BlockHeader bh;
  DoolpMsg_clearBlock ( &bh );
  DoolpMsg_setBlockRawSize ( &bh, sizeof ( DoolpMsg_NewCall_Header ) );
  bh.blockType = DoolpMsg_BlockType_NewCall_Header;
  writeBlockHeader ( &bh );
  rawWrite ( &newCall_h, sizeof ( DoolpMsg_NewCall_Header ) );
  __DOOLPCONNECTIONTCP_Log ( "New call : rpcId=%p, objId=%d\n", call->rpc->rpcId, call->objId );
  return true;
}


bool DoolpConnectionTCP::startMessage ( DoolpMsg_Header * header )
{
  lockWrite ();
  logDoolpMsg_Header ( header );
  rawWrite ( header, sizeof ( DoolpMsg_Header ) );
  nextBlockIndex = 0;
  return true;
}

bool DoolpConnectionTCP::startMessage ( DoolpCallContext * callContext,
					DoolpMsg_CallFlags extraFlags)
{
  DoolpMsg_Header h;
  h.callFlags = ( (  callContext->type == DoolpCallContextType_Job ) 
		  ? DoolpMsg_CallFlag_ModeReply 
		  : DoolpMsg_CallFlag_ModeCall )
                | extraFlags;
  h.fromAgentId = callContext->fromAgentId; 
  h.toAgentId   = callContext->toAgentId;
  h.fullContextId = callContext->fullContextId;
  return startMessage ( &h );
}

bool DoolpConnectionTCP::startReply (DoolpJob * job)
{ return startMessage ( job, 0 ); }

bool DoolpConnectionTCP::endMessage   ( )
{
  DoolpMsg_BlockHeader endHeader;
  DoolpMsg_clearBlock ( &endHeader );
  DoolpMsg_setBlockMode ( &endHeader, DoolpMsg_BlockMode_EndBlocks );
  writeBlockHeader ( &endHeader );
  unlockWrite (); // unlockWrite will flushWrite
  return true;
}

bool DoolpConnectionTCP::readMessageEnd ( )
{
  __DOOLPCONNECTIONTCP_Log ( "Trying to read Message End\n" );
  __DOOLPCONNECTIONTCP_Log ( "blockHeaderIsRead='%d'\n", blockHeaderIsRead );
  __tryReadBlockHeader ; 
  if ( ! DoolpMsg_isBlockEnd ( &bh ) )
    {
      return false;
    }
  __DOOLPCONNECTIONTCP_Log ( "Read Message End.\n" );
  blockHeaderIsRead = false;
  return true;
}

bool DoolpConnectionTCP::readMsgType ( unsigned int type )
{
  __tryReadBlockHeader;
  if ( DoolpMsg_getBlockType ( &bh ) != type ) // DoolpMsg_BlockType_Call_Params )
    {
      // Warn ( "Did not manage to read Param Subsection\n" );
      return false;
    }
  if ( bh.blockSize == 0 )
    blockHeaderIsRead = false; // This is finished with this block.
  return true;

}


// SubSections Management


bool DoolpConnectionTCP::startSubSection ( unsigned int type )
{ 
  return sendZeroSizeBlock ( DoolpMsg_BlockMode_SubSection, type ); 
}
bool DoolpConnectionTCP::endSubSection ( )
{ return sendZeroSizeBlock ( DoolpMsg_BlockMode_SubSectionEnd, 0 ); }

bool DoolpConnectionTCP::startParamSubSection ( )
{ return startSubSection ( DoolpMsg_BlockType_Call_Params ); }

bool DoolpConnectionTCP::readParamSubSection ()
{ return readMsgType ( DoolpMsg_BlockType_Call_Params ); }

bool DoolpConnectionTCP::readDoolpObjectSubSection ()
{ return readMsgType ( DoolpMsg_BlockType_DoolpObject ); }

bool DoolpConnectionTCP::readDoolpStreamEnd ()
{ return readMsgType ( DoolpMsg_BlockType_DoolpStreamEnd ) ; }

bool DoolpConnectionTCP::readSubSectionEnd ()
{ 
  __DOOLPCONNECTIONTCP_Log ( "Trying to read SubSection End\n" );
  __tryReadBlockHeader;
  if ( DoolpMsg_getBlockMode ( &bh ) != DoolpMsg_BlockMode_SubSectionEnd )
    {
      // Warn ( "Did not manage to read SubSection End\n" );
      return false;
    }
  __DOOLPCONNECTIONTCP_Log ( "Read SubSection End.\n" ); 
  blockHeaderIsRead = false; 
  return true;
}

bool DoolpConnectionTCP::setDoolpStream ( DoolpCallContext * callContext, 
					  DoolpStreamIndex idx )
{
  // TODO : Recode this !!!
  if ( false ) // isLockWrite () )
    {
      Warn ( "Already lockWrite : do not send header...\n" );
      doolpStream_hasSentHeader = false;
    }
  else
    {
      startMessage ( callContext, 0 ); // startMessage DOES the lockWrite ()
      doolpStream_hasSentHeader = true;
    }
  setNextBlockIndex ( idx );
  sendZeroSizeBlock ( DoolpMsg_BlockMode_SubSection,
		      DoolpMsg_BlockType_DoolpStream);
  return true;
}

bool DoolpConnectionTCP::readDoolpStreamSubSection () 
{ 
  return readMsgType ( DoolpMsg_BlockType_DoolpStream ); 
}

bool DoolpConnectionTCP::readDoolpStream ( DoolpCallContext * callContext, DoolpStreamVirtual * stream, 
					   DoolpStreamIndex idx )
{
  Bug ( "NOT IMPLEMENTED\n" );
  // See DoolpConnectionXML::readDoolpStream.
  return false;
}

bool DoolpConnectionTCP::leaveDoolpStream ( )
{
  // End of doolpStream
  sendZeroSizeBlock ( DoolpMsg_BlockMode_SubSectionEnd,
		      DoolpMsg_BlockType_DoolpStream);
  endMessage (); // endMessage DOES the unlockWrite
  return true;
}
bool DoolpConnectionTCP::endDoolpStream ( )
{
  sendZeroSizeBlock ( 0,
		      DoolpMsg_BlockType_DoolpStreamEnd);
  return true;
}

bool DoolpConnectionTCP::readDoolpExceptionSubSection () 
{
  Bug ( "NoT ImPlEmEnTeD !!\n" );
  return true;
}


bool DoolpConnectionTCP::startList ( unsigned int listIndex )
{ 
  setNextBlockIndex ( listIndex ); 
  return sendZeroSizeBlock ( DoolpMsg_BlockMode_SubSection, DoolpMsg_BlockType_List ); 
}

unsigned int DoolpConnectionTCP::readList ( )
{
  DoolpMsg_BlockHeader bh;
  readBlockHeader ( &bh );
  if ( bh.blockType != DoolpMsg_BlockType_List )
    Fatal ( "Wanted to read a list, but this is not.\n" );
  blockHeaderIsRead = false;
  return bh.blockIndex;
}

// BlockIndex Management

unsigned int DoolpConnectionTCP::getNextBlockIndex ( )
{ __tryReadBlockHeader; return bh.blockIndex; }

bool DoolpConnectionTCP::setNextBlockIndex ( unsigned int __nextBlockIndex )
{
  if ( nextBlockIndex )
    Bug ( "A blockIndex has already been set.\n" );
  nextBlockIndex = __nextBlockIndex;
  return true;
}
