#ifndef __DOOLP__DOOLPOBJECT_SLOT_H
#define __DOOLP__DOOLPOBJECT_SLOT_H

#include <doolp/doolp-doolpids.h>
#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpcall.h>
#include <doolp/doolp-doolpjob.h>
#include <doolp/doolp-doolpconnection.h>
#include <doolp/doolp-doolpobject-slotvirtual.h>
#include <doolp/doolp-doolpobject-slot-callparams.h>

namespace Doolp
{

  class SlotAttributes
  {
  public:
    AgentId destAgentId;
    CallTimeOut callTimeOut;
    SlotAttributes() { destAgentId = 0; callTimeOut = 0; }
    ~SlotAttributes() { }
  };
  
  
#define __DOOLP__DOOLPOBJECT_SLOT_H__INSIDE
  template<unsigned int lineId, class T_obj>
    class FakeSlot
  {
  public:
    /************************************ MAX ARGS = 7 *************************************/
#define __DoolpSlot__Name DoolpSlot7
#define __DoolpSlot__MaxArgs
#define __DoolpSlot__AllArgs						\
    __DoolpSlot__Type(T_arg1) __DoolpSlot__Value(arg1) __DoolpSlot__Sep	\
      __DoolpSlot__Type(T_arg2) __DoolpSlot__Value(arg2) __DoolpSlot__Sep \
      __DoolpSlot__Type(T_arg3) __DoolpSlot__Value(arg3) __DoolpSlot__Sep \
      __DoolpSlot__Type(T_arg4) __DoolpSlot__Value(arg4) __DoolpSlot__Sep \
      __DoolpSlot__Type(T_arg5) __DoolpSlot__Value(arg5) __DoolpSlot__Sep \
      __DoolpSlot__Type(T_arg6) __DoolpSlot__Value(arg6) __DoolpSlot__Sep \
      __DoolpSlot__Type(T_arg7) __DoolpSlot__Value(arg7) 
#include <doolp/doolp-doolpobject-slot-iter.h>
#undef __DoolpSlot__Name
#undef __DoolpSlot__MaxArgs
#undef __DoolpSlot__AllArgs
    
    /************************************ 0 ARGS *************************************/
#define __DoolpSlot__Name DoolpSlot0
#define __DoolpSlot__AllArgs
#define __DoolpSlot__FillTemplateArgs void,void,void,void,void,void,void
#include <doolp/doolp-doolpobject-slot-iter.h>
#undef __DoolpSlot__Name
#undef __DoolpSlot__AllArgs
#undef __DoolpSlot__FillTemplateArgs
    
    /************************************ 1 ARGS *************************************/
#define __DoolpSlot__Name DoolpSlot1
#define __DoolpSlot__AllArgs				\
    __DoolpSlot__Type(T_arg1) __DoolpSlot__Value(arg1) 
#define __DoolpSlot__FillTemplateArgs void,void,void,void,void,void
#include <doolp/doolp-doolpobject-slot-iter.h>
#undef __DoolpSlot__Name
#undef __DoolpSlot__AllArgs
#undef __DoolpSlot__FillTemplateArgs
    
    /************************************ 2 ARGS *************************************/
#define __DoolpSlot__Name DoolpSlot2
#define __DoolpSlot__AllArgs						\
    __DoolpSlot__Type(T_arg1) __DoolpSlot__Value(arg1) __DoolpSlot__Sep	\
      __DoolpSlot__Type(T_arg2) __DoolpSlot__Value(arg2) 
#define __DoolpSlot__FillTemplateArgs void,void,void,void,void
#include <doolp/doolp-doolpobject-slot-iter.h>
#undef __DoolpSlot__Name
#undef __DoolpSlot__AllArgs
#undef __DoolpSlot__FillTemplateArgs
    
    /************************************ 3 ARGS *************************************/
#define __DoolpSlot__Name DoolpSlot3
#define __DoolpSlot__AllArgs						\
    __DoolpSlot__Type(T_arg1) __DoolpSlot__Value(arg1) __DoolpSlot__Sep	\
      __DoolpSlot__Type(T_arg2) __DoolpSlot__Value(arg2) __DoolpSlot__Sep \
      __DoolpSlot__Type(T_arg3) __DoolpSlot__Value(arg3) 
#define __DoolpSlot__FillTemplateArgs void,void,void,void
#include <doolp/doolp-doolpobject-slot-iter.h>
#undef __DoolpSlot__Name
#undef __DoolpSlot__AllArgs
#undef __DoolpSlot__FillTemplateArgs
    
};
  
#define doolpfunc Doolp::FakeSlot<__LINE__,DoolpObject_CurrentObject>::Slot
#define doolpfunchelper(__slot,__helper) __slot##_##__helper
#define doolpfunclocal(__slot) __slot##_local
  
#undef __DOOLP__DOOLPOBJECT_SLOT_H__INSIDE

  
}; // NameSpace Doolp
#endif // __DOOLP__DOOLPSLOT_H
