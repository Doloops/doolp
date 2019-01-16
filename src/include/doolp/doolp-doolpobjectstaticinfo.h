#ifndef __DOOLP_DOOLPOBJECTSTATICINFO_H
#define __DOOLP_DOOLPOBJECTSTATICINFO_H

#include <doolp/doolp-doolpids.h>
#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpobject-slotvirtual.h>

#include <strongmap.h>

namespace Doolp
{
  typedef Object * (*ObjectConstructor) (ObjectId id);
  typedef Exception * (*ExceptionConstructor) ();

  class ObjectStaticInfo
  {
  protected:
    map<ObjectNameId,ObjectInfo *> objects;
    strongmap<ObjectNameId,char *> objectsNames;
    strongmap<ObjectNameId,ObjectConstructor> objectsConstructors;
    strongdoublemap<ObjectNameId,ObjectSlotId,SlotVirtual*> objectsSlots;
    strongdoublemap<ObjectNameId,ObjectSlotId,bool> objectsSlotsImplements;
    strongmap<ExceptionId,ExceptionConstructor> exceptionsConstructors;
    Forge * forge;
  public:
    ObjectStaticInfo ( Forge * __forge ) { forge = __forge; }

    bool addLib ( char * fileName );
    bool addObject ( ObjectNameId objNameId, char * objName,
		     ObjectConstructor );
    bool assignObjectConstuctor ( ObjectNameId objNameId, ObjectConstructor );
    bool addSlot ( ObjectNameId objNameId, ObjectSlotId slotId, SlotVirtual * staticSlot );
    bool setImplementedSlot ( ObjectNameId objNameId, ObjectSlotId slotId );
    bool addParam ( ObjectNameId objNameId, ObjectParamId objParamId,
		    char * type, char * name );
    bool addException ( ExceptionId exceptionId, char * exceptionName,
			Exception * (*constructor) () );
    inline ObjectConstructor getObjectConstructor ( ObjectNameId nameId )
      { 
	return objectsConstructors.get(nameId); 
      }
    inline SlotVirtual * getSlot ( ObjectNameId objNameId, ObjectSlotId slotId )
      { return objectsSlots.get ( objNameId, slotId ); }
    inline ExceptionConstructor getExceptionConstructor ( ExceptionId exceptionId )
      {
	return exceptionsConstructors.get(exceptionId);
      }
    bool tellImplementedSlots ( AgentId toAgentId );
  };
};

#endif // __DOOLP_DOOLPOBJECTSTATICINFO_H
