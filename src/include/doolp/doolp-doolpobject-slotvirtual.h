#ifndef __DOOLP__DOOLPOBJECT_SLOTVIRTUAL_H
#define __DOOLP__DOOLPOBJECT_SLOTVIRTUAL_H

#include <doolp/doolp-doolpclasses.h>

namespace Doolp
{
  class SlotVirtual
  {
  public:
    class CallParamsVirtual
    {
    public:
      virtual ~CallParamsVirtual() { }
    };
    class ReplyParamsVirtual
    {
    public:
      virtual ~ReplyParamsVirtual() { }
      virtual bool Write ( Job * job, Connection * conn ) = 0;
    };
    virtual ~SlotVirtual() { }
    virtual CallParamsVirtual * prepare ( Connection * conn, Job * job ) = 0;
    virtual ReplyParamsVirtual * run ( Job * job ) = 0;
    virtual ReplyParamsVirtual * handleReply ( Connection * conn ) = 0;
    virtual ReplyParamsVirtual * getReply ( Call * call );
  
    virtual ObjectSlotId getSlotId() const = 0; // { Bug ( "Shall not be here\n" ); }
    virtual SlotVirtual * getStaticSlot() const = 0;
    virtual char * getSlotName() const = 0;
    virtual AgentId getAgentId ( Object * obj );
  };


  /*
   * Doolp::SlotFake is a fake slot for Job Forwarding.
   */
  class SlotFake : public SlotVirtual
  {
  protected:
    ObjectSlotId slotId;
    char * slotName;
  public:
    SlotFake ( ObjectSlotId _slotId, char * _slotName )
      { slotId = _slotId; slotName = _slotName; }
    CallParamsVirtual * prepare ( Connection * conn, Job * job )
      { Bug ( "NOT TO USE !\n" ); return NULL; }
    ReplyParamsVirtual * run ( Job * job )
      { Bug ( "NOT TO USE !\n" ); return NULL; }
    ReplyParamsVirtual * handleReply ( Connection * conn )
      { Bug ( "NOT TO USE !\n" ); return NULL; }
    ReplyParamsVirtual * getReply ( Call * call )
      { Bug ( "NOT TO USE !\n" ); return NULL; }
  
    ObjectSlotId getSlotId() const
    { return slotId; }
    SlotVirtual * getStaticSlot() const
      { Bug ( "NOT TO USE !\n" ); return NULL; }
    char * getSlotName() const
      { return slotName; }
    AgentId getAgentId ( Object * obj )
    { Bug ( "NOT TO USE !\n" ); return 0; }
  };


}; // NameSpace Doolp
#endif // __DOOLP__DOOLPOBJECT_SLOTVIRTUAL_H
