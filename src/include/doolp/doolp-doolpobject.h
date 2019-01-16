#ifndef __DOOLP_DOOLPOBJECT_H
#define __DOOLP_DOOLPOBJECT_H

#include <list>

using namespace std;


#include <doolp/doolp-doolpids.h>
#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpobject-slotvirtual.h>

#include <strongmap.h>

namespace Doolp
{
  typedef strongmap<ObjectSlotId, unsigned int> ObjectSlotMap;
  typedef map<ContextStepping,list<ObjectParamId> > ObjectChangeSet;
  class Object
  {
    friend class Forge;
  private:
    Forge * forge;
    AgentId ownerAgentId;
  protected:
    ObjectId objectId;
    inline bool setObjectId(ObjectId _objId) { objectId = _objId; return true; }
    /*
     * Default Constructor
     */
    Object() 
      {
	Log ( "Default constructor for DoolpObject\n" );
	forge = 0; ownerAgentId = 0; objectId = 0;
      }
  public:
    /*
     * Internal values.
     */
    inline ObjectId getObjectId() const { return objectId; }
    inline Forge * getForge() { return forge; }
    inline bool setForge ( Forge * _forge ) { forge = _forge; return true; }
    inline bool setOwner ( AgentId owner ) { ownerAgentId = owner; return true; }
    inline AgentId getOwnerAgentId () { return ownerAgentId; }
    inline ObjectId getObjectId () { return objectId; }
    ObjectChangeSet changeSet;
    void logChangeSet ();
  public:
    /*
     * Virtual Destructor
     */
    virtual ~Object ();
    
    /* 
     * Serialize and UnSerialize functions 
     */
    virtual bool Object::serialize ( Connection * connection ) = 0;
    virtual bool Object::serialize ( Connection * conn, ObjectParamId paramId ) = 0;
    virtual bool Object::unserialize ( Connection * connection ) = 0;
    bool Object::serialize ( Connection * conn, list<ObjectParamId> * paramList );
    bool serializeFromStepping ( Connection * conn, ContextStepping stepping );
    
    virtual ObjectNameId Object::getNameId ( ) = 0;
    
    /*
     * Slots and Relatives
     */
  public:
    virtual SlotVirtual * __getSlot ( ObjectSlotId slotId ) = 0;
    virtual bool __initStatic (Forge *) = 0;
    virtual bool __initSlots () = 0; // Do this for all objects
    virtual bool __assignSlots () = 0; // Do this once to prepare the functions
    virtual bool __registerSlots (Forge *) = 0; // Do this once to inform Forge
    void setModified ( ObjectParamId paramId ); // TODO : Should set friends ?

    /*
     * DoolpObject Options
     */
  protected:
    bool contextDependant;
  public:  
    inline bool isContextDependant () const { return contextDependant; }
    inline void setContextDependant ( bool b ) { contextDependant = b; }
    
  protected:
    ObjectTTL ttl;
  public:
    inline ObjectTTL getTTL () const { return ttl; }
    inline void setTTL ( ObjectTTL t ) { ttl = t; }
    
  protected:
    bool forceCallsTOOwner;
    
  public:
    bool getForceCallsTOOwner () const { return forceCallsTOOwner; }
    virtual void setOptions () { contextDependant = true; ttl = 0; forceCallsTOOwner = false; }
    virtual bool isObjectBuffer () { return false; }
  };
  
#define DoolpObject_stdFeatures()					\
  public: DoolpObject_CurrentObject (Doolp::ObjectId id);		\
  virtual Doolp::ObjectNameId getNameId ();				\
  static Doolp::ObjectNameId getNameIdStatic ();			\
  virtual bool serialize ( Doolp::Connection * connection );		\
  virtual bool serialize ( Doolp::Connection * conn,			\
			   Doolp::ObjectParamId paramId );		\
  virtual bool unserialize ( Doolp::Connection * connection );		\
  virtual Doolp::SlotVirtual * __getSlot ( Doolp::ObjectSlotId slotId ); \
  virtual bool __initStatic ( Doolp::Forge *);				\
  virtual bool __initSlots ();						\
  virtual bool __assignSlots ();					\
  virtual bool __registerSlots ( Doolp::Forge *);
  
  
#define DoolpObject_Option(__name,__value)
#define DoolpObject_Options(...)
  

}; // NameSpace Doolp
#endif // __DOOLP_DOOLPOBJECT_H
