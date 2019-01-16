#ifndef __DOOLP_DOOLPOBJECTBUFFER_H
#define __DOOLP_DOOLPOBJECTBUFFER_H

#include <doolp/doolp-doolpobject.h>

/*
 * A generic way to read an object
 * without knowing anything about it.
 */


#include <map>
using namespace std;

/*
 * TODO : 
 * Enrich the DoolpForge::createEmptyObject to have DoolpObjectBuffer as fallback /ok
 * Enrich DoolpConnection to implement ReadRawSection and WriteRawSection
 * Enrich DoolpForge::getObject_local to retrieve persistence if object not in cache
 */

namespace Doolp
{
  class ObjectBufferParam
  {
  public:
    typedef enum ParamType
      {
	type_int,
	type_float,
	type_string
	// ADD MORE HERE....
      };
    ParamType type;
    void * buffer;
    int size;
  };
  class ObjectBuffer : public Object
  {
    friend class PersistanceFTree;
  public:
    map<ObjectParamId, ObjectBufferParam*> params;

    ObjectBuffer ( ObjectId _objectId, ObjectNameId _nameId ) 
      { 
	setForge (NULL);
	objectId = _objectId ; 
	nameId = _nameId; 
	setOptions ( ); 
      }
    ObjectNameId nameId;

    ObjectBufferParam * getParam ( ObjectParamId id )
      { return params[id]; }

    bool serialize ( Connection * connection );
    bool serialize ( Connection * conn, ObjectParamId paramId );
    bool unserialize ( Connection * connection );

    ObjectNameId getNameId ( )
    { return nameId; }
    bool isObjectBuffer () { return true; }

    static ObjectSlotMap * __slotmap;
    ObjectSlotMap * __getSlotMap ()
      { 
	// slotmap();
	if ( __slotmap == NULL )
	  {
	    Log ( "Creating Fake Dummy ObjectSlotMap for ObjectBuffer\n" );
	    __slotmap = new ObjectSlotMap();
	  }
	Warn ( "Fake Dummy ObjectSlotMap for ObjectBuffer\n" );
	return __slotmap; 
      }
    bool __initStatic (Forge *) {return true;}
    bool __initSlots () { return true; }
    bool __assignSlots () { return true; }
    bool __registerSlots (Forge * _forge) { return true; }
    SlotVirtual * __getSlot ( ObjectSlotId slotId )
      { Warn ( "getSlot for ObjectBuffer : returning NULL\n" ); return NULL; }
  };
};
#endif // __DOOLP_DOOLPOBJECTBUFFER_H
