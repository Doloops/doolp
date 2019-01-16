#ifndef __DOOLP_DOOLPAGENT_H
#define __DOOLP_DOOLPAGENT_H

#include <doolp/doolp-doolpids.h>

/*
 * Local data collection of known agents
 */
namespace Doolp
{
  typedef struct AgentCapabilities
  {
    bool agentIdProvider;
    bool objectIdProvider;
    bool messageForwarder;
  };
  
  typedef struct Agent
  {
    AgentId agentId;
    AgentCapabilities capabilities;
  };
}; // NameSpace Doolp;

#endif // __DOOLP_DOOLPAGENT_H
