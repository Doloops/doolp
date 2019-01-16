#ifndef __DOOLP_DOOLPOBJECT_LINKVIRTUAL_H
#define __DOOLP_DOOLPOBJECT_LINKVIRTUAL_H

/*
 * Generic Doolp::Object to Doolp::Object link (pointer)
 * 
 * Notas : difference between cleanLink and removeLink
 * cleanLink[s] deletes the reference of the Object, but keeps the ObjectId
 * removeLink[s] removes both Object and ObjectId.
 */

namespace Doolp
{
  class ObjectLink
  {
    friend class ObjectUniqueLink; // USELESS ?
  protected:
  public:
    virtual bool addLink ( const ObjectId id, const Object * obj, bool force = false ) = 0;
    virtual bool cleanLink ( const ObjectId id ) = 0; // Clean link of objectId id. Do not call reverse.
    virtual bool cleanLinks ( ) = 0; // Clean ALL links... Call reverse for all links
    virtual bool removeLink ( const ObjectId id ) = 0;
    virtual bool removeLinks ( ) = 0; // Remove ALL links
    virtual unsigned int * getOffsetPtr() = 0;

    virtual ~ObjectLink() { Log ( "Virtual destructor for ObjectLink\n" ); }
    virtual bool serialize( Connection * ) = 0;
    virtual bool unserialize( Connection * ) = 0;
  };

};


#endif // __DOOLP_DOOLPOBJECT_LINKVIRTUAL_H
