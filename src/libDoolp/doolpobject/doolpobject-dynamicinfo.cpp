#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpobjectdynamicinfo.h>
#include <doolp/doolp-doolpcontext.h>

/*
 * Complex Mapping
 */

Doolp::ObjectDynamicInfo::SlotProfile::SlotProfile(list<pair<Doolp::ObjectNameId,Doolp::ObjectSlotId> > * slotList)
{
  list<pair<Doolp::ObjectNameId,Doolp::ObjectSlotId> >::iterator slot;
  for ( slot = slotList->begin() ; slot != slotList->end (); slot ++ )
    {
      slotNumber++;
      slots.put ( slot->first, slot->second, true );
    }
}
bool Doolp::ObjectDynamicInfo::SlotProfile::isEqual (list<pair<Doolp::ObjectNameId,Doolp::ObjectSlotId> > * slotList)
{
  if ( slotList->size() != slotNumber )
    {
      Warn ( "Wrong number of slots..\n" );
      return false;
    }
  list<pair<Doolp::ObjectNameId,Doolp::ObjectSlotId> >::iterator slot;
  for ( slot = slotList->begin() ; slot != slotList->end (); slot ++ )
    {
      if ( ! slots.get ( slot->first, slot->second ) )
	return false;
    }
  return true;
}


/*
 * Dynamic Information about objects, slots...
 */

list<Doolp::AgentId>* Doolp::Forge::doolpfunclocal(getAgentIdsForSlotId) ( Doolp::ObjectNameId objectNameId, 
									   Doolp::ObjectSlotId slotId )
{ 
  Warn ( "TO REIMPLEMENT ????\n" );
  return NULL;
  //  return getObjectDynamicInfo()->getAgentIdsForSlotId ( objectNameId, 
  //							slotId );
}

bool Doolp::Forge::doolpfunclocal(tellImplementedSlots) ( list<pair<Doolp::ObjectNameId,Doolp::ObjectSlotId> > * slots )
{
  AssertBug ( getContext()->isOnJob(), "Invalid use of tellImplementedSlots : shall be called by another agent !\n" );
  AgentId fromAgentId = getContext()->getJobCaller();
  return getObjectDynamicInfo()->setImplementedSlots ( fromAgentId, slots );
}

bool Doolp::ObjectDynamicInfo::setImplementedSlots ( Doolp::AgentId agentId, 
						     list<pair<Doolp::ObjectNameId,Doolp::ObjectSlotId> > * slotList )
{
  list<pair<Doolp::ObjectNameId,Doolp::ObjectSlotId> >::iterator slot;
  ObjectSlotId slotIdx = 0;
  for ( slot = slotList->begin() ; slot != slotList->end (); slot ++ )
    {
      Log ( "Agent '%d' tells implemented nameId='0x%x' slotId='0x%x'\n",
	    agentId, slot->first, slot->second );
      slotIdx += slot->second;
    }
  Log ( "Agent '%d' : slotProfile index is '0x%x'\n", agentId, slotIdx );
  if ( slotProfiles.has ( slotIdx ) )
    {
      SlotProfile * slotProfile = slotProfiles.get (slotIdx);
      AssertBug ( slotProfile->isEqual ( slotList ), "Not Implemented.\n" );
      agentSlots.put ( agentId, slotProfile );
      slotProfile->agents.push_back ( agentId );
    }
  else
    {
      SlotProfile * slotProfile = new SlotProfile ( slotList );
      slotProfiles.put ( slotIdx, slotProfile );
      agentSlots.put ( agentId, slotProfile );
      slotProfile->agents.push_back ( agentId );
    }
  return true;
}




Doolp::AgentId Doolp::ObjectDynamicInfo::getAgentIdForSlot ( Doolp::SlotVirtual * slot,
							 Doolp::Object * obj )
{
  Warn ( "TO IMPLEMENT !\n" );
  return 1;
}

Doolp::AgentId Doolp::ObjectDynamicInfo::getAgentIdForSlotId ( Doolp::ObjectNameId objectNameId, 
							   Doolp::ObjectSlotId slotId )
{
  strongmap<ObjectSlotId,SlotProfile*>::iterator slotProfileIter;
  for ( slotProfileIter = slotProfiles.begin () ; slotProfileIter != slotProfiles.end () ;
	slotProfileIter ++ )
    {
      SlotProfile * slotProfile = slotProfileIter->second;
      if ( slotProfile->slots.get ( objectNameId, slotId ) )
	{
	  Log ( "Found implement on profile '0x%x'\n", slotProfileIter->first );
	  Doolp::AgentId agentId = slotProfile->agents.front ();
	  slotProfile->agents.push_back ( agentId ); // Fuzzy round-robin...
	  slotProfile->agents.pop_front ();
	  return agentId;
	}
    }
  //  Warn ( "TO IMPLEMENT !\n" );
  return 0;
}


bool Doolp::ObjectDynamicInfo::removeAgent ( Doolp::AgentId agentId )
{
  __DOOLP_Log ( "Removing dynamic info from agentId %d\n", agentId );
  SlotProfile * slotProfile = agentSlots.get(agentId);
  if ( slotProfile == NULL )
    { 
      Log ( "AgentId %d has no SlotProfile !\n", agentId );
      return true;
    }
  slotProfile->agents.remove ( agentId );
  agentSlots.remove ( agentId );
  __DOOLP_Log ( "Removed everything about agentId %d\n", agentId );
  return true;
}
