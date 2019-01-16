#ifndef __DOOLP__DOOLPOBJECT_SLOT_H__INSIDE
#error Include error : shall be included from doolp-doolpobject-slot.h
#endif

#ifndef __DoolpSlot__AllArgs
#error Not defined : __DoolpSlot__AllArgs
#endif

#undef __DoolpSlot__Type
#undef __DoolpSlot__Value
#undef __DoolpSlot__Sep
#ifdef __DoolpSlot__MaxArgs
#define __DoolpSlot__Type(__x)    , class __x = void
#else
#define __DoolpSlot__Type(__x)    , class __x
#endif
#define __DoolpSlot__Value(__x) 
#define __DoolpSlot__Sep
template <class T_return __DoolpSlot__AllArgs >
class Slot
#ifndef __DoolpSlot__MaxArgs
#undef __DoolpSlot__Type
#define __DoolpSlot__Type(__x) , __x
  <T_return __DoolpSlot__AllArgs ,__DoolpSlot__FillTemplateArgs>
#endif
  : public SlotVirtual
{
 public:
#undef __DoolpSlot__Type
#undef __DoolpSlot__Value
#undef __DoolpSlot__Sep
#define __DoolpSlot__Type(__x) __x
#define __DoolpSlot__Value(__x)
#define __DoolpSlot__Sep ,
  typedef T_return (T_obj::*function_type) (__DoolpSlot__AllArgs);
  typedef AgentId (T_obj::*function_chooseAgentId) (__DoolpSlot__AllArgs);
 protected:
  /*
   * Mutualise all functions for all objects of extra same class (inheritance excluded)
   */
  class Functions
  {
  public:
    Functions () { hasFunction = false; hasChooseAgentId = false; }
    function_type function_pointer;
    bool hasFunction;
    function_chooseAgentId chooseAgentId_pointer;
    bool hasChooseAgentId;
  };
  Functions * functions; // Only thing not static, the way functions are impl local.
  static ObjectSlotId slotId;
  static SlotVirtual * staticSlot; // Static version of this.
  static char * slotName;
  static unsigned int offset;
 public:
  Slot() { functions = NULL; }
  /*
   * Accessors.
   */
  ObjectSlotId getSlotId() const { return slotId; }
  SlotVirtual * getStaticSlot() const { return staticSlot; }
  char * getSlotName () const { return slotName; }
  inline void * getFunctions () const { return (void*) functions; } // Should think about typecheck this... 
  inline bool setFunctions(void * __functions) { functions = (Functions*)__functions; return true; } // (no use of void*)...
 protected:
  inline T_obj * getObject ( ) 
    { return ((T_obj*) ((unsigned int)this - offset ) ); } 
 public:								
  inline bool setOffset ( T_obj * _A_obj )
  {									
    offset = (unsigned int)this - (unsigned int) _A_obj;		
    return true;							
  }									
  template<class T_obj_source>
    bool assign ( T_return (T_obj_source::*_func_ptr) (__DoolpSlot__AllArgs) )
    {					
      if ( functions == NULL )
	functions = new Functions ();
      Log ( "Adding slotId '0x%x'\n", slotId );			
      functions->function_pointer = (function_type)_func_ptr;						
      functions->hasFunction = true;
      return true;							
    }
  template<class T_obj_source>
    bool checkOffset ( T_obj_source * _A_obj )
    {
      unsigned int newOffset = (unsigned int)this - (unsigned int) _A_obj;
      AssertBug ( newOffset == offset, "Incorrect offsets ! (previous = %d, new = %d\n",
		  offset, newOffset );
      Log ( "Offsets checked.\n" );
      return true;
    }
  bool assignChooseAgentId ( const function_chooseAgentId _chooseAgentId_ptr)				
  { 
    AssertBug ( functions != NULL, "No functions defined !\n" );
    functions->chooseAgentId_pointer = _chooseAgentId_ptr;
    functions->hasChooseAgentId = true;
    return true;
  }
public:
#undef __DoolpSlot__Type
#undef __DoolpSlot__Value
#undef __DoolpSlot__Sep
#define __DoolpSlot__Type(__x)     SlotCallParam<__x>
#define __DoolpSlot__Value(__x) __x;
#define __DoolpSlot__Sep

  class CallParams : public CallParamsVirtual	
  {						
  public:					
    CallParams() {}				
    __DoolpSlot__AllArgs
  };


  class ReplyParams : public ReplyParamsVirtual
  {				 
  public:					
    SlotCallParam<T_return> retval;	
    ReplyParams() {}			       
    bool Write ( Job * job, Connection * conn )
    { return retval.Write ( job, conn ); }
  };

#undef __DoolpSlot__Type
#undef __DoolpSlot__Value
#undef __DoolpSlot__Sep
#define __DoolpSlot__Type(__x)  
#define __DoolpSlot__Value(__x) params->__x.Read ( job, conn );
#define __DoolpSlot__Sep
  CallParamsVirtual * prepare ( Connection * conn, Job * job )
  {
    CallParams * params = new CallParams ();
    __DoolpSlot__AllArgs
    Log ( "End of Param reading. (conn=%p)\n", conn );
    if ( functions == NULL )
      Warn ( "No functions defined !\n" );
    if ( functions != NULL && ! functions->hasFunction )
      Warn ( "Carefull, function not assigned, prepared but not shure I will be able to run ! (inheritance..)\n" );
    return params;
  }


#undef __DoolpSlot__Type
#undef __DoolpSlot__Value
#undef __DoolpSlot__Sep
#define __DoolpSlot__Type(__x)  
#define __DoolpSlot__Value(__x) params->__x.getValue()
#define __DoolpSlot__Sep ,
  SlotVirtual::ReplyParamsVirtual * run ( Doolp::Job * job )		       
  {
    AssertBug ( functions != NULL, "No functions defined in this slot !\n" );
    AssertBug ( functions->hasFunction, "Function not implemented !\n" );
    CallParams * params = (CallParams*) (job->callParams);		       
    Log ( "Running slotId '0x%x' (params at %p)\n", slotId, params );
    ReplyParams * replyParams = new ReplyParams ();
    replyParams->retval.setValue ( (((T_obj*)getObject())->*(functions->function_pointer)) ( __DoolpSlot__AllArgs ) );
    replyParams->retval.keepalive = false;
    //    delete ( params );			
    return replyParams;				   
  }

  ReplyParamsVirtual * handleReply ( Connection * conn )	       
  {							
    ReplyParams * params = new ReplyParams ();		
    params->retval.keepalive = true;			
    params->retval.Read ( NULL, conn );
    return params;					
  }


#undef __DoolpSlot__Type
#undef __DoolpSlot__Value
#undef __DoolpSlot__Sep
#define __DoolpSlot__Type(__x)  __x
#define __DoolpSlot__Value(__x) __x
#define __DoolpSlot__Sep ,
  inline Call * callAsync ( __DoolpSlot__AllArgs )
    {
#undef __DoolpSlot__Type
#define __DoolpSlot__Type(__x) ,
#undef __DoolpSlot__Sep
#define __DoolpSlot__Sep
      SlotAttributes defaultAttributes;
      return callAsync ( defaultAttributes __DoolpSlot__AllArgs );
    }
#undef __DoolpSlot__Type
#define __DoolpSlot__Type(__x) ,__x
#undef __DoolpSlot__Sep
#define __DoolpSlot__Sep
  Call * callAsync ( SlotAttributes &attributes  __DoolpSlot__AllArgs )
  { 
    Log ( "Calling remote slotId='0x%x'\n", slotId );
    AgentId destAgentId = attributes.destAgentId;
    if ( destAgentId == 0 && functions != NULL && functions->hasChooseAgentId )
      {
	fprintf ( stderr, "Slot has a chooseAgentId !\n" );
#undef __DoolpSlot__Type
#define __DoolpSlot__Type(__x)
#undef __DoolpSlot__Sep
#define __DoolpSlot__Sep ,
	destAgentId = (getObject()->*(functions->chooseAgentId_pointer)) ( __DoolpSlot__AllArgs );
	Log ( "chooseAgentId : Got agentId '%d'\n", destAgentId );
      }
    if ( destAgentId == 0 && getObject()->getForceCallsTOOwner() )
      {
	destAgentId = getObject()->getOwnerAgentId ();
      }
    if ( destAgentId == 0 )
      {
	destAgentId = getAgentId ( getObject() );
      }
    AssertFatal ( destAgentId != 0, "Could not get distant AgentId for this rpc\n" );
    if ( destAgentId == getObject()->getForge()->getAgentId() )
      {
	Warn ( "Local Call !\n" );
	return NULL;
      }
    Call * call = getObject()->getForge()->newCall ( this, getObject(), destAgentId );
    AssertBug ( call != NULL, "Could not create call !\n" );
    AssertBug ( call->preferedConn != NULL, "Could not get a valid connection\n" );
    Connection * conn = call->preferedConn;
#undef __DoolpSlot__Type
#undef __DoolpSlot__Value
#undef __DoolpSlot__Sep
#define __DoolpSlot__Type(__x) { SlotCallParam<__x> param(false); 
#define __DoolpSlot__Value(__x) param.setValue (__x); param.Write ( call, conn ); }
#define __DoolpSlot__Sep
    conn->startParamSubSection ();
    __DoolpSlot__AllArgs    
    conn->endSubSection ();
    conn->endMessage ();
    getObject()->getForge()->addCall ( call );
    return call;
  }

 public:
  T_return getResult (Call * call)
  {
    ReplyParams * reply = (ReplyParams*) getReply ( call ); 
    AssertBug ( reply != NULL, "Could not get reply !\n" );
    T_return retval = reply->retval.getValue();
    //    delete ( reply );
    return retval;
  }

#undef __DoolpSlot__Type
#undef __DoolpSlot__Value
#undef __DoolpSlot__Sep
#define __DoolpSlot__Type(__x)  __x
#define __DoolpSlot__Value(__x) __x
#define __DoolpSlot__Sep ,
  T_return operator() ( __DoolpSlot__AllArgs )
  { 
#undef __DoolpSlot__Type
#define __DoolpSlot__Type(__x) ,
#undef __DoolpSlot__Sep
#define __DoolpSlot__Sep
    SlotAttributes defaultAttributes;
    return operator() ( defaultAttributes __DoolpSlot__AllArgs );
  }
#undef __DoolpSlot__Type
#define __DoolpSlot__Type(__x) , __x
#undef __DoolpSlot__Sep
#define __DoolpSlot__Sep
  T_return operator() ( SlotAttributes &attributes  __DoolpSlot__AllArgs )
  {
#undef __DoolpSlot__Type
#define __DoolpSlot__Type(__x) ,
    Call * call = callAsync ( attributes __DoolpSlot__AllArgs );
    if ( call == NULL )
      {
	Warn ( "We assert we should perform a local call...\n" );
	AssertBug ( functions != NULL, "No functions defined in this slot !\n" );
	AssertFatal ( functions->hasFunction, "Local call, but function not assigned !\n" );
	function_type func_ptr = functions->function_pointer;
#undef __DoolpSlot__Type
#define __DoolpSlot__Type(__x)
#undef __DoolpSlot__Sep
#define __DoolpSlot__Sep ,
	return ( (((T_obj*)getObject())->*func_ptr) ( __DoolpSlot__AllArgs ) );
      }
    return getResult ( call );
  }
};
