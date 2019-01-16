#ifndef __DOOLP_DOOLPOBJECTDYNAMICINFO_H
#define __DOOLP_DOOLPOBJECTSTATICINFO_H

namespace Doolp
{
  class ObjectDynamicInfo
  {
  protected:
    class SlotProfile
    {
    public:
      strongdoublemap<ObjectNameId, ObjectSlotId, bool> slots;
      unsigned int slotNumber;
      
      SlotProfile(list<pair<ObjectNameId,ObjectSlotId> > *);
      bool isEqual (list<pair<ObjectNameId,ObjectSlotId> > *);
      list<AgentId> agents;
    };
    strongmap<AgentId,SlotProfile*> agentSlots;
    strongmap<ObjectSlotId,SlotProfile*> slotProfiles;
    ObjectStaticInfo * objectStaticInfo;
    inline ObjectStaticInfo * getObjectStaticInfo() const { return objectStaticInfo; }
    Forge * myForge;
    inline Forge * getForge() const { return myForge; }
    
  public:
    bool setImplementedSlots ( AgentId, list<pair<ObjectNameId,ObjectSlotId> > * );
    ObjectDynamicInfo ( Forge * _forge, ObjectStaticInfo * _staticInfo ) 
      { myForge = _forge; objectStaticInfo = _staticInfo; }
    
    AgentId getAgentIdForSlot ( SlotVirtual * slot,
				   Object * obj );
    AgentId getAgentIdForSlotId ( ObjectNameId objectNameId, 
				  ObjectSlotId slotId );
    bool removeAgent ( AgentId agentId );
  };

};// NameSpace Doolp

#endif // __DOOLP_DOOLPOBJECTSTATICINFO_H
