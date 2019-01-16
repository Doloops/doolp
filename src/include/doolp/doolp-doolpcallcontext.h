#ifndef __DOOLP_DOOLPCALLCONTEXT_H
#define __DOOLP_DOOLPCALLCONTEXT_H

#include <doolp/doolp-doolpobject-slotvirtual.h>

#include <map>
using namespace std;

namespace Doolp
{
  typedef enum CallContextType
    {
      CallContextType_Job,
      CallContextType_Call
    };
#define CallContextMaxStreams 4
  class CallContext
  {
  public:
    CallContextType type;
    ObjectId objId;
    ObjectNameId objNameId;
    Object * obj;
    ObjectSlotId slotId;
    SlotVirtual * slot;
    AgentId fromAgentId, toAgentId;
    FullContextId fullContextId;
    Connection * preferedConn;
    ContextStepping serializeFromStepping;
    StreamVirtual *doolpStreams[CallContextMaxStreams];
  protected:
    CallContext() 
      {
	objId = 0; objNameId = 0; obj = NULL; 
	slotId = 0; slot = NULL;
	fromAgentId = toAgentId = 0;
	preferedConn = NULL;
	serializeFromStepping = 0;
	for ( int i = 0 ; i < CallContextMaxStreams ; i++ )
	  doolpStreams[i] = NULL;
	
      }
    ~CallContext()
      {}
  public:
    StreamIndex addStream ( StreamVirtual * );
    StreamVirtual * getStream ( StreamIndex idx )
      {
	AssertBug ( doolpStreams[idx] != NULL, "No stream of index '%d' in this call !\n", idx );
	return doolpStreams[idx];
      }
  };
  
}; // NameSpace Doolp
#endif // __DOOLP_DOOLPCALLCONTEXT_H
