#ifndef __DOOLP_DOOLPFORGESERVICES_H
#define __DOOLP_DOOLPFORGESERVICES_H

#include <string>
#include <map>
using namespace std;

#include <doolp/doolp-doolpclasses.h>

namespace Doolp
{

  class ForgeService
  {
  protected:
    string serviceName;
  public:
    ForgeService () 
      {
	__DOOLP_Log ( "New Service.\n" );
	serviceName = "unknown";
      }

    inline string& getServiceName ()  { return serviceName; }
    virtual bool start () = 0;
    virtual bool stop () = 0;
    virtual Object * getServiceAsObject () = 0;

    virtual ~ForgeService () 
      {
	__DOOLP_Log ( "End of (virtual) Service\n" );
      }
  };

  class ForgeServices
  {
    map<string, ForgeService*> services;
    map<string, AgentId> distantServices;
    Forge * myForge;
  public:
    ForgeServices ( Forge * _forge ) { myForge = _forge; }
    bool add ( ForgeService * service );
    bool hasService ( string &name );
    ForgeService * getService ( string &name );
    list<string> * listServices ();

    bool addDistant ( AgentId agentId, string & name );
    AgentId getDistant ( string &name );
    bool hasDistant ( string &name );
  };

}; // NameSpace Doolp

#endif // __DOOLP_DOOLPFORGESERVICES_H
