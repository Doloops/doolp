#ifndef __DOOLP_DOOLPNAMINGCACHE_H
#define __DOOLP_DOOLPNAMINGCACHE_H

#include <doolp/doolp-doolpnaming.h>
#include <doolp/doolp-doolpforgeservices.h>
#include <strongmap.h>

namespace Doolp
{
  class NamingCache : public Naming, public ForgeService
  {
#define DoolpObject_CurrentObject NamingCache
    DoolpObject_stdFeatures ( );
    DoolpObject_Option ( forceObjectAs, Naming );
    // DoolpObject_IsLocalImplementationOf ( DoolpNaming );
    //  DoolpObject_stdFeatures ( DoolpNamingCache );
  protected:
    strongmap<string,FullContextId*> contexts;
    strongdoublemap<string,string,ObjectId> objects;

  public:
    NamingCache () 
      {
	serviceName = "DoolpNaming";
      }

    virtual bool start () { return true; }
    virtual bool stop () { return true; }
    virtual Object * getServiceAsObject () { return (Object*) this; }

    FullContextId * doolpfunclocal(getNamedContext) ( string& name );
    bool doolpfunclocal(setNamedContext) ( string& name, FullContextId * id );
    ObjectId doolpfunclocal(getNamedObject) ( string& contextName, string& name );
    bool doolpfunclocal(setNamedObject) ( string& contextName, string& name, ObjectId objId );

    void setOptions () 
    {
      contextDependant = false;
      ttl = 0;
    }

    virtual bool flush() { return true; }

    //  DoolpNameId getDoolpNameId ();
#undef DoolpObject_CurrentObject
  };
};
#endif // __DOOLP_DOOLPNAMINGCACHE_H
