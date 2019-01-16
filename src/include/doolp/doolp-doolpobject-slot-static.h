#ifndef __DOOLP_DOOLPOBJECT_SLOT_STATIC_H
#define __DOOLP_DOOLPOBJECT_SLOT_STATIC_H



#define DoolpObjectSlot__StaticValues(__line,T_obj,__slotId,__slotName,T_return,...) \
  unsigned int Doolp::FakeSlot<__line,T_obj>::Slot<T_return,__VA_ARGS__>::slotId = __slotId; \
  unsigned int Doolp::FakeSlot<__line,T_obj>::Slot<T_return,__VA_ARGS__>::offset = 0; \
  char * Doolp::FakeSlot<__line,T_obj>::Slot<T_return,__VA_ARGS__>::slotName = __slotName; \
  Doolp::FakeSlot<__line,T_obj>::Slot<T_return,__VA_ARGS__> __DOOLPSLOT##__##__slotId##__line##__##__LINE__; \
  Doolp::SlotVirtual * Doolp::FakeSlot<__line,T_obj>::Slot<T_return,__VA_ARGS__>::staticSlot = \
    &(__DOOLPSLOT##__##__slotId##__line##__##__LINE__);

#define DoolpObjectSlot__StaticSlot(__line,T_obj,T_return,...) \
  (Doolp::FakeSlot<__line,T_obj>::DoolpSlot<T_return,__VA_ARGS__>::staticSlot)

#define DoolpObjectSlot__PerClass(T_objTarget,__slotName) \
  void * __DOOLPSLOT__##T_objTarget##__##__slotName = NULL;
#define DoolpObjectSlot__setFunctions(T_objTarget,__slotName) \
  __DOOLPSLOT__##T_objTarget##__##__slotName = __slotName.getFunctions();
#define DoolpObjectSlot__getFunctions(T_objTarget,__slotName) \
  __slotName.setFunctions(__DOOLPSLOT__##T_objTarget##__##__slotName);


#endif // __DOOLP_DOOLPOBJECT_SLOT_STATIC_H
