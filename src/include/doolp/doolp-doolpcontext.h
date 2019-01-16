#ifndef __DOOLP_DOOLPCONTEXT_H
#define __DOOLP_DOOLPCONTEXT_H


#include <doolp/doolp-doolpcontextchangeset.h>

namespace Doolp
{
  class Context
  {
    friend class Forge;
  protected:
    FullContextId fullContextId;
    Forge * forge;
#ifdef __DOOLPFORGE_ALLOW_PUSHED_CONTEXTS
    Context * pushedContext;
#endif // __DOOLPFORGE_ALLOW_PUSHED_CONTEXTS
    FullContextId father;
    
    Job * currentJob;
    ContextChangeSet changeSet;
    Connection * handledConnection;
    
    FullContextId * commitContext;
    ContextStepping lastCommitStepping;
  public:
    Context ( Forge * forge, 
	      AgentId agentId,
	      ContextId contextId );
    ~Context ();
    
    inline Forge * getForge () { return forge; }
    inline FullContextId * getFullContextId () { return &fullContextId; }
    inline Job * getCurrentJob () { return currentJob; }
    inline void setCurrentJob ( Job * job ) { currentJob = job; }
    inline bool isOnJob ( ) { return currentJob != NULL; }
    AgentId getJobCaller ();
    
    inline void setHandledConnection ( Connection * conn )
      { handledConnection = conn; }
    bool backgroundWork ( Call * specificCall );
    bool backgroundWork ( StreamVirtual * specificStream );
    
    // Object ChangeSet Modifier
    void setNewObject ( Object * obj );
    void setDeleteObject ( ObjectId objId );
    void setModifiedObject ( Object * obj ); 
    
    
  public:
    // Various
    void log ();
  };
  
}; // NameSpace Doolp
#endif // __DOOLP_DOOLPCONTEXT_H
