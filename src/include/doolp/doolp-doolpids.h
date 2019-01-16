#ifndef __DOOLP__DOOLPIDS_H
#define __DOOLP__DOOLPIDS_H

#include <time.h>

typedef int Socket;

namespace Doolp
{
  typedef unsigned int ObjectId;
  typedef unsigned int ObjectNameId;
  typedef unsigned int ObjectSlotId;
  typedef unsigned int ObjectParamId;
  typedef unsigned int ExceptionId;
  typedef ObjectId AgentId;
  typedef unsigned int ContextId;
  typedef unsigned int ContextStepping;
  typedef unsigned int StreamIndex; 
  
  typedef time_t ObjectTTL;
  typedef time_t CallTimeOut;

  
#define fullContextIdCmp(_f1_,_f2_)				\
  (   ( (_f1_)->agentId   == (_f2_)->agentId   )		\
      ? ( (_f1_)->contextId == (_f2_)->contextId )		\
      ? ( (_f1_)->stepping  == (_f2_)->stepping  ) : 0 : 0  )
  
  typedef struct FullContextId 
    // class FullContextId
    // This is the only unique key identifier for contexts
  {
    public:
    AgentId agentId;
    ContextId contextId;
    ContextStepping stepping;
    /*
      FullContextId operator< ( FullContextId &dfc )
      {
      if ( agentId < dfc.agentId ) return *this;
      if ( agentId > dfc.agentId ) return dfc;
      if ( contextId < dfc.contextId ) return *this;
      if ( contextId > dfc.contextId ) return dfc;
      return ( stepping < dfc.stepping ? *this : dfc );
      }
    */
    bool operator== ( FullContextId &dfc )
    {
      return fullContextIdCmp ( this, &dfc );
    }
  };
  
  // extern FullContextId nullFullContextId;
//  = { 0, 0, 0 };
  
#define logDoolpFullContextId(_f_)			\
  (_f_)->agentId, (_f_)->contextId, (_f_)->stepping
  
#define cpyDoolpFullContextId(_to_,_from_)				\
  memcpy ( (void*) _to_, (void*) _from_, sizeof ( FullContextId ) )
  
}; // NameSpace Doolp

#endif  // __DOOLP__DOOLPIDS_H
  
