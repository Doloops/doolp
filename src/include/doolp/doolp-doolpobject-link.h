#ifndef __DOOLP_DOOLPOBJECT_LINK_H
#define __DOOLP_DOOLPOBJECT_LINK_H

#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpobject-linkvirtual.h>
#include <doolp/doolp-doolpconnection.h>
#include <strongmap.h>

namespace Doolp
{
  template<unsigned int id_field,class T_obj>
    class ObjectFakeLink
  {
  public:
    template<class T_linkedObj>
      class ObjectUniqueLink : public ObjectLink
    {
      protected:
      typedef T_linkedObj * T_pointer;
      /*
       * Static values
       */
      static ObjectParamId linkId;
      static unsigned int offset;
      static bool hasReverse;
      static unsigned int * reverseOffsetPtr; // Pointer to the runtime-computed offset of the reverse link
    
      /*
       * Dynamic values
       */
      ObjectId objectId;
      Object * object;

      unsigned int * getOffsetPtr() { return &offset; }
      inline T_obj * getBaseObject () const { return (T_obj *) ((unsigned int) this - offset); }	

      bool addLink ( const ObjectId id, const Object * obj, bool force = false )
      {
	if ( ! force )
	  AssertFatal ( ( objectId == 0 ) && ( object == NULL ), 
			"Could not addLink : already have an object '0x%x'\n", objectId );
	objectId = id; object = (Object*)obj;
	return true;
      }
      ObjectLink * getReverse()
      { AssertBug ( hasReverse, "Link has no reverse !\n" );
	AssertBug ( object != NULL, "Distant object not set !\n" );
	return ( (ObjectLink*) (((unsigned int)object) + *(reverseOffsetPtr) ) ); }

      public:
      ObjectUniqueLink() { objectId = 0; object = NULL; }
      ~ObjectUniqueLink() { Log ( "Destructor for ObjectUniqueLink '0x%x'\n", linkId ); cleanLinks (); }
      inline void setOffset ( T_obj * o )	   { offset = (unsigned int) this - (unsigned int) o; }
      inline void setReverse ( ObjectLink * ol )   { hasReverse = true; reverseOffsetPtr = ol->getOffsetPtr(); }

      /*
       * Serialization/Unserialization straps.
       */
      bool serialize ( Connection * c ) { return c->Write ( objectId ); }
      bool unserialize ( Connection * c ) { return c->Read ( &objectId ); }

      /*
       * Setting Object
       */
      inline bool setObject ( const T_linkedObj * o )
      { AssertFatal ( o != NULL, "Been given a NULL object ! Use unset() to delete link...\n" );
	if ( ! addLink ( o->getObjectId(), o ) )
	  return false;
	if ( hasReverse )
	  return getReverse()->addLink ( getBaseObject()->getObjectId(), getBaseObject() );
	return true;
      }
      inline ObjectUniqueLink& operator= (const T_linkedObj * o )   { setObject ( o ); return *this; }
      inline ObjectUniqueLink& operator= (const T_linkedObj & o )   { setObject ( &o ); return *this; }

      /*
       * Getting Object
       */
      inline T_linkedObj * getObject() 
	{ Log ( "objectId 0x%x object %p\n", objectId, object ); 
	  if ( objectId == 0 ) { Warn ( "No object linked..\n" ); return NULL; }
	  if ( object != NULL ) return dynamic_cast<T_linkedObj*> (object);
	  object = getBaseObject()->getForge()->getObject ( objectId );
	  if ( object == NULL ) { Warn ( "Could not get object 0x%x\n", objectId ); return NULL; }
	  Log ( "Found object at '%p'\n", object );
	  if ( hasReverse ) getReverse()->addLink ( getBaseObject()->getObjectId(), getBaseObject(), true );
	  return dynamic_cast<T_linkedObj*> ( object ); }
      inline operator T_linkedObj * ()    { return getObject(); }
      inline T_pointer operator->()       { return getObject(); }

      /*
       * cleanLink : here this is really simple
       * due to the fact that it's a unique link.
       */
      bool cleanLink ( const ObjectId id )
      { Log ( "cleanLink for id 0x%x\n", id );
	if ( objectId == id ) { object = NULL; return true; } 
	else { Warn ( "Not linked to '0x%x' : linked to '0x%x'\n", id, objectId ); return false; } }
      bool cleanLinks ( )
      { Log ( "cleanLinks objectId=0x%x, object=%p\n", objectId, object );
	if ( object != NULL ) { getReverse()->cleanLink ( getBaseObject()->getObjectId() ); }
	object = NULL; return true; }

      /*
       * removeLink
       */
      bool removeLink ( const ObjectId id )
      { if ( objectId == id ) { object = NULL; objectId = 0x00; return true; } 
	else { Warn ( "Not linked to '0x%x' : linked to '0x%x'\n", id, objectId ); return false; } }
      bool removeLinks ( )
      { if ( object != NULL ) { getReverse()->removeLink ( getBaseObject()->getObjectId() ); }
	object = NULL; objectId = 0x00; return true; }
    }; // ObjectUniqueLink

    template<class T_linkedObj>
      class ObjectMultiLink : public ObjectLink
    {
      typedef T_linkedObj * T_pointer;
      typedef strongmap<ObjectId, Object*> ObjectMap;
      typedef typename ObjectMap::iterator ObjectMapIterator;
      protected:
      /*
       * Static values
       */
      static ObjectParamId linkId;
      static unsigned int offset;
      static bool hasReverse;
      static unsigned int * reverseOffsetPtr; // Pointer to the runtime-computed offset of the reverse link
      
      /*
       * Dynamic values
       */
      ObjectMap objects;

      inline T_obj * getBaseObject () const { return (T_obj *) ((unsigned int) this - offset); }	
    public:
      inline void setOffset ( T_obj * o )	   { offset = (unsigned int) this - (unsigned int) o; }
      inline void setReverse ( ObjectLink * ol )   { hasReverse = true; reverseOffsetPtr = ol->getOffsetPtr(); }
      unsigned int * getOffsetPtr() { return &offset; }

      /*
       * Iterator
       */
      class iterator
      {
      protected:
      public:
	ObjectMapIterator iter;
	inline iterator& operator++()   { iter++; return *this; }
	inline iterator operator++(int) { iterator __tmp = *this; iter++; return __tmp; }
	inline T_pointer operator->()   
	  { if ( iter.second != NULL ) return dynamic_cast<T_pointer>(iter.second); 
	    AssertBug ( iter.first != 0x00, "Been given a zero objectId !\n" );
	    Object * obj = getBaseObject()->getForge()->getObject ( iter.first );
	    iter.second = obj;
	    return dynamic_cast<T_pointer>(obj); }
      };
    protected:
      iterator iterBegin, iterEnd;
    public:
      iterator& begin()	{ iterBegin.iter = objects.begin() ; return iterBegin; }
      iterator& end() { iterEnd.iter = objects.end() ; return iterEnd; }
      
      bool addLink ( const ObjectId id, const Object * obj, bool force = false )
      { return objects.put ( id, (Object*) obj ); }

      bool cleanLink ( const ObjectId id ){ return true; }
      bool cleanLinks ( ){return true; }
      bool removeLink ( const ObjectId id ) { return true; }
      bool removeLinks ( ) { return true; }

      bool serialize( Connection * ) { return true; }
      bool unserialize( Connection * ) { return true; }

      
    }; // ObjectMultiLink
  }; // ObjectFakeLink


}; // NameSpace Doolp

#define doolpuniquelink Doolp::ObjectFakeLink<__LINE__,DoolpObject_CurrentObject>::ObjectUniqueLink
#define doolpmultilink Doolp::ObjectFakeLink<__LINE__,DoolpObject_CurrentObject>::ObjectMultiLink

#define DoolpObjectLink_setReverse( __baseLink, __reverseLink )


#define DoolpObjectLink__StaticValues(__line,T_obj,__index,__linktype,...) \
  Doolp::ObjectParamId Doolp::ObjectFakeLink<__line,T_obj>::__linktype<__VA_ARGS__>::linkId = __index; \
  unsigned int Doolp::ObjectFakeLink<__line,T_obj>::__linktype<__VA_ARGS__>::offset = 0; \
  bool Doolp::ObjectFakeLink<__line,T_obj>::__linktype<__VA_ARGS__>::hasReverse = false; \
  unsigned int * Doolp::ObjectFakeLink<__line,T_obj>::__linktype<__VA_ARGS__>::reverseOffsetPtr = 0
  


#endif // __DOOLP_DOOLPOBJECT_LINK_H
