#warning DEPRECATED : TO BE REMOVED !

#if 0

#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolprpc.h>
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpcontext.h>
#include <doolp/doolp-doolpexceptions.h>


extern "C" 
{
  DoolpObjectInfo * getDoolpForgeStdRPC ( );
}

bool DoolpForge::registerStdRPCs ()
{
  DoolpObjectInfo * forgeInfo = getDoolpForgeStdRPC ();
  for ( unsigned int rpc = 0 ; forgeInfo->rpcTable[rpc] != NULL ; rpc ++ )
    if ( ! objectStaticInfo->add ( forgeInfo->rpcTable[rpc] ) )
      {
	Bug ( "Could not register std rpc !\n" );
	return false;
      }
  return true;
}
#endif 
/*
bool DoolpForge::tellImplementedRPCs_local ( DoolpAgentId toAgentId, list<DoolpRPCId> * rpcs )
{
  AssertFatal ( getContext()->isOnJob(), " Not called from inside a job!!!\n" );
  AssertFatal ( rpcs != NULL, " Could not get rpc list\n" );
  __DOOLP_Log ( "AgentId %d, tell implement %d rpcs\n",
		getContext()->getJobCaller (),
		rpcs->size () );
  return getObjectDynamicInfo()->tellImplementedRPCs ( getContext()->getJobCaller(), rpcs );
}
*/
/*
bool DoolpForge::tellImplementedRPCs ( DoolpAgentId toAgentId )
{
  __DOOLP_Log ( "telling implemented RPCs to agentId '%d'\n", toAgentId );
  tellImplementedRPCs ( toAgentId, getObjectStaticInfo()->getImplementedRPCs() );
  return true;
}
*/
 /*
DoolpAgentId DoolpForge::tellImplementedRPCs_chooseAgentId ( DoolpAgentId toAgentId, list<DoolpRPCId> * rpcs )
{
  __DOOLP_Log ( "telling implemented RPCs to agentId '%d'\n", toAgentId );
  return toAgentId;
}
 */
// RPC Registering and resolving.
  /*
list<DoolpAgentId>* DoolpForge::getAgentIdsForRPCId_local (DoolpRPCId rpcId)
{
  AssertFatal ( getContext()->isOnJob(), " Not called from inside a job!!!\n" );
  list<DoolpAgentId>* agents = getObjectDynamicInfo()->getAgentIdsForRPCId ( rpcId );
  if ( agents == NULL )
    {
      Warn ( "Nobody to implement %x (asked from agentId %d) !\n",
	     rpcId, getContext()->getJobCaller ());
      throw new DoolpCallNoAgentFound();
      //      return NULL;
    }
  return agents;
}


  */
