#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpforgeservices.h>
#include <doolp/doolp-doolpexceptions.h>

Doolp::Object * Doolp::Forge::doolpfunclocal(getService) ( string & serviceName )
{ 
  __DOOLP_Log ( "Finding service '%s'\n", serviceName.c_str() );
  Object * obj = NULL;
  if ( getServices()->hasService ( serviceName ) )
    {
      ForgeService * service = getServices()->getService ( serviceName );
      if ( service == NULL )
	{
	  Warn ( "Could not find service '%s'\n", serviceName.c_str() );
	  throw new NoForgeServiceFound ();
	}
      obj = service->getServiceAsObject ();
      if ( obj == NULL )
	{
	  Warn ( "Could not find service '%s'\n", serviceName.c_str() );
	  throw new NoForgeServiceFound ();
	}
      if ( obj->getObjectId() == 0 )
	{
	  addObject ( obj );
	}
      AssertBug ( obj->getForge() != NULL, "Invalid Forge !\n" );
    }
  return obj; 
}
Doolp::Object * Doolp::Forge::getService_char ( char * sc )
{ string s ( sc ); return getService ( s ); }

Doolp::AgentId Doolp::Forge::getService_chooseAgentId ( string & serviceName )
{ 
  return getAgentIdForService ( serviceName ); 
}

Doolp::AgentId Doolp::Forge::doolpfunclocal(getAgentIdForService) ( string& serviceName )
{
  __DOOLP_Log ( "Asked for service : '%s'\n", serviceName.c_str () );
  if ( getServices()->hasService ( serviceName ) )
    return getAgentId();
  if ( getServices()->hasDistant ( serviceName ) )
    return getServices()->getDistant ( serviceName );
  return 0;
}

Doolp::AgentId Doolp::Forge::getAgentIdForService_char ( char * sc )
{ string s ( sc ); return getAgentIdForService ( s ); }


bool Doolp::ForgeServices::add ( Doolp::ForgeService * service )
{
  AssertFatal ( service->getServiceName() != "unknown", "Service name not given !\n" );
  services[service->getServiceName()] = service;
  Object * obj = service->getServiceAsObject();
  if ( obj != NULL )
    {
      if ( obj->getObjectId () == 0 )
	{
	  myForge->addObject ( obj );
	  Log ( "Obj : given objectId '0x%x'\n", obj->getObjectId() );
	}
    }
  __DOOLP_Log ( "Added new service : '%s'\n", service->getServiceName().c_str() );
  return true;
}

bool Doolp::ForgeServices::hasService ( string &name )
{
  return ( services[name] != NULL );
}

Doolp::ForgeService * Doolp::ForgeServices::getService ( string &name )
{
  AssertFatal ( services[name] != NULL, "No service named '%s'\n", name.c_str() );
  return services[name];
}


bool Doolp::ForgeServices::addDistant ( Doolp::AgentId agentId, string & name )
{
  distantServices[name] = agentId;
  return true;
}

Doolp::AgentId Doolp::ForgeServices::getDistant ( string &name )
{
  if ( distantServices[name] == 0 )
    Warn ( "Could not find any distant service for '%s'\n",
	   name.c_str() );
  return distantServices[name];
}

bool Doolp::ForgeServices::hasDistant ( string &name )
{ return (distantServices[name] != 0); }

/*
 * Probing Mechanism
 */

bool Doolp::Forge::probeServices ( Doolp::AgentId agentId )
{
  list<string> * services = tellServices ( agentId );
  while ( ! services->empty () )
    {
      string & s = services->front();
      services->pop_front ();
      getServices()->addDistant ( agentId, s );
      __DOOLP_Log ( "Adding distant '%d' : '%s'\n",
		    agentId, s.c_str() );
    }
  return true;
}


list<string> * Doolp::Forge::tellServices_local ( Doolp::AgentId agentId )
{
  return getServices()->listServices ();
}

list<string> * Doolp::ForgeServices::listServices ()
{
  list<string> * s = new list<string>;
  map<string, ForgeService*>::iterator serv;
  for ( serv = services.begin () ; serv != services.end () ; serv++ )
    {
      s->push_back ( serv->first );
    }
  return s;
}
