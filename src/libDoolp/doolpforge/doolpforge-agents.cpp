#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpobjectdynamicinfo.h>


Doolp::Agent * Doolp::Forge::getAgent ( Doolp::AgentId agentId )
{
  list<Agent*>::iterator agent;
  for ( agent = agents.begin () ;
	agent != agents.end () ;
	agent ++ )
    {
      if ( (*agent)->agentId == agentId )
	return (*agent);
    }
  Warn ( "Could not get agent %d\n", agentId );
  return NULL;
}

bool Doolp::Forge::addAgent ( Doolp::Agent * agent )
{
  if ( getAgent ( agent->agentId ) )
    {
      Warn ( "Agent %d already added.\n",
	     agent->agentId );
      return true;
    }
  agents.push_back ( agent );
  return true;
}

bool Doolp::Forge::removeAgent ( Doolp::AgentId agentId )
{
  return getObjectDynamicInfo()->removeAgent ( agentId );
}
