#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpcall.h>
#include <doolp/doolp-doolpobjectdynamicinfo.h>
#include <doolp/doolp-doolpobject-slotvirtual.h>


Doolp::AgentId Doolp::SlotVirtual::getAgentId ( Doolp::Object * obj )
{
  Log ( "getAgentId (this=%p, obj=%p)\n", this, obj );
  Log ( "slotId is '0x%x'\n", getSlotId () );
  //  Bug ( "TO IMPLEMENTED ?\n" );
  AgentId agentId = obj->getForge()->getObjectDynamicInfo()->getAgentIdForSlot(this,obj);
  Log ( "Gave agentId '%d'\n", agentId );
  return agentId;
}


Doolp::SlotVirtual::ReplyParamsVirtual * Doolp::SlotVirtual::getReply ( Doolp::Call * call )
{
  Forge * forge = call->obj->getForge();
  Log ( "(this=%p,obj=%p(0x%x),forge=%p) : getReply for call %p\n", 
	this, call->obj, call->obj->getObjectId(), forge, call );
  forge->waitCall ( call );
  Log ( "Got answer for call %p\n", call );
  forge->checkCall ( call ); // Check for Exceptions;
  Log ( "Call checked.\n" );
  ReplyParamsVirtual * replyParams = call->replyParams;
  AssertBug ( replyParams != NULL, "Call did not provide replyParams !\n" );
  forge->removeCall ( call );
  return replyParams;
}
