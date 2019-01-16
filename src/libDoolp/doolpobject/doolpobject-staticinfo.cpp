#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpobjectstaticinfo.h>

#include <dlfcn.h>

/*
 * Static Information about Objects
 */ 

bool Doolp::ObjectStaticInfo::addLib ( char * fileName )
{
  void * libHandler;
  libHandler = dlopen ( fileName, RTLD_NOW );
  if ( libHandler == NULL )
    {
      Error ( "Could not open lib '%s'\n", fileName );
      LogErrNo;
      Bug ( "Could not open this lib.\n" );
      return false;
    }
  bool (*_register) (Doolp::Forge *);
  _register = (bool (*) (Doolp::Forge *) )
    dlsym ( libHandler, "registerDoolpObjects" );
  if ( _register == NULL )
    {
      Error ( "Lib '%s' : Could not get the registerDoolpObjects symbol.\n",
	      fileName );
      return false;
    }
  __DOOLP_Log ( "getInfo at %p\n", _register );
  return _register ( forge );
}

bool Doolp::ObjectStaticInfo::addObject ( Doolp::ObjectNameId objNameId, char * objName,
					  Doolp::ObjectConstructor constructor )
{
  Log ( "New Object '0x%x' : '%s' (constructor at %p)\n",
	objNameId, objName, constructor );
  if ( objectsNames.has ( objNameId ) )
    {
      Fatal ( "Multiple objects : objNameId 0x%x\n", objNameId );
    }
  objectsNames.put ( objNameId, objName );
  if ( objectsConstructors.has ( objNameId ) )
    {
      Warn ( "Already have a constructor for objNameId 0x%x\n", objNameId );
    }
  else
    {
      objectsConstructors.put ( objNameId, constructor );
    }
  return true;
}

bool Doolp::ObjectStaticInfo::assignObjectConstuctor ( Doolp::ObjectNameId objNameId,
						       Doolp::ObjectConstructor constructor )
{
  if ( objectsConstructors.has ( objNameId ) )
    {
      Warn ( "Already have a constructor for objNameId 0x%x\n", objNameId );
      objectsConstructors.remove ( objNameId );
    }
  objectsConstructors.put ( objNameId, constructor );
  return true;
}


bool Doolp::ObjectStaticInfo::addSlot ( Doolp::ObjectNameId objNameId, 
					Doolp::ObjectSlotId slotId, 
					Doolp::SlotVirtual * staticSlot )
{
  Log ( "In object with nameId '0x%x' : adding slot '0x%x', staticSlot at %p\n",
	objNameId, slotId, staticSlot );
  objectsSlots.put ( objNameId, slotId, staticSlot );
  return true;
}

bool Doolp::ObjectStaticInfo::setImplementedSlot ( Doolp::ObjectNameId objNameId, Doolp::ObjectSlotId slotId )
// Cleanup, checks....
{
  objectsSlotsImplements.put ( objNameId, slotId, true );
  return true;
}

bool Doolp::ObjectStaticInfo::addException ( Doolp::ExceptionId exceptionId, char * exceptionName,
					     Doolp::Exception * (*constructor) () )
{
  Log ( "New Exception '0x%x' : '%s'\n",
	exceptionId, exceptionName );
  exceptionsConstructors.put ( exceptionId, constructor );
  return true;
}

/*
 * Tell Implemented Slots
 */

bool Doolp::ObjectStaticInfo::tellImplementedSlots ( Doolp::AgentId toAgentId )
{
  Log ( "Telling Implemented Slots to agentId %d\n", toAgentId );
  Doolp::SlotAttributes attrs;
  attrs.destAgentId = toAgentId;
  list<pair<Doolp::ObjectNameId,Doolp::ObjectSlotId> > slotList;
  strongdoublemap<Doolp::ObjectNameId,Doolp::ObjectSlotId,Doolp::SlotVirtual*>::iterator slot;
  for ( slot = objectsSlots.begin () ; slot != objectsSlots.end () ; slot++ )
    {
      if ( slot.first() == forge->getNameId() )
	continue;
      if ( ! objectsSlotsImplements.get ( slot.first(), slot.second() ) )
	continue;
      slotList.push_back ( make_pair ( slot.first (), slot.second () ) );
    }
  forge->tellImplementedSlots ( attrs, &slotList );
  return true;
}
