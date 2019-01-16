#ifndef __DOOLP_DOOLPOBJECT_SLOT_CALLPARAMS_H
#define __DOOLP_DOOLPOBJECT_SLOT_CALLPARAMS_H

#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpcallcontext.h>
namespace Doolp
{
  template <typename T_arg>
    class SlotCallParam
    {
    public:
      T_arg arg;
      bool keepalive;
      SlotCallParam () { keepalive = false; }
      SlotCallParam (bool _keepalive) { keepalive = _keepalive; }
      ~SlotCallParam () { Log ( "Destructor for default SlotCallParam, keepalive=%d\n", keepalive ); }
      T_arg & getValue () { return arg; }
      bool setValue ( T_arg _arg ) { arg = _arg; keepalive = true; return true; }
      bool Read ( CallContext * callContext, Connection * conn )
      { return conn->Read ( & arg ); }
      bool Write ( CallContext * callContext, Connection * conn )
      { return conn->Write ( arg ); }
    };
  
  /*
   * Reference
   */
  template<typename T_type>						
    class SlotCallParam<T_type&>
    {									
    public:								
      T_type * arg;								
      bool keepalive;	
      SlotCallParam () { arg = NULL; keepalive = false; }
      SlotCallParam (bool _keepalive) { arg = NULL; keepalive = _keepalive; } 
      ~SlotCallParam()							
	{ Log ( "Destructor for SlotCallParam<ptr*>\n" );		
	  if ( ! keepalive ) free ( arg ); }				
      T_type& getValue()							
	{ return (*arg); }						
      bool Read(CallContext * job,					
		Connection * conn)					
      {									
	conn->Read ( &arg );
	return true;
      }
      bool setValue ( T_type & v )
      { arg = &v; keepalive = true; return true; }
      bool Write(CallContext* call, Connection * conn )
      { conn->Write ( arg ); return true; }
    };
  
  
  /*
   * Pointer
   */
  template<typename T_type>						
    class SlotCallParam<T_type*>					
    {									
    public:								
      T_type * arg;								
      bool keepalive;	
      SlotCallParam () { arg = NULL; keepalive = false; }
      SlotCallParam (bool _keepalive) { arg = NULL; keepalive = _keepalive; } 
      ~SlotCallParam()							
	{ Log ( "Destructor for SlotCallParap<ptr*>\n" );		
	  if ( ! keepalive ) free ( arg ); }				
      T_type* getValue()							
	{ return (T_type*)(arg); }						
      bool Read(CallContext * job,					
		Connection * conn)					
      {									
	conn->Read ( &arg );
	return true;
      }
      bool setValue ( T_type * v )
      { arg = v; keepalive = true; return true; }
      bool Write(CallContext* call, Connection * conn )
      { conn->Write ( (T_type*)arg ); return true; }
    };
  
  /*
   * Stream
   */
  template<typename T_type>						
    class SlotCallParam<Stream<T_type>*>
    {
    public:
      Stream<T_type>* arg;
      bool keepalive;
      SlotCallParam () { arg = NULL; keepalive = false; }
      SlotCallParam(bool _keepalive) { arg = NULL; keepalive = _keepalive; }
      ~SlotCallParam()	{}									
      Stream<T_type>* getValue() { return arg; }					
      bool Read(CallContext * job, Connection * conn) 
      { 
	AssertBug ( job != NULL, "No job given.\n" );			
	arg = new Stream<T_type>();				
	unsigned int streamIdx = job->addStream ( (StreamVirtual*) arg );	
	Log ( "Stream idx=%d\n", streamIdx );				
	if ( conn->readStreamSubSection() &&				
	     ( conn->getNextBlockIndex () == streamIdx ) )			
	  {									
	    Log ( "Read stream content for stream %d\n", streamIdx );	
	    conn->readStream ( job, (Stream<T_type>*) arg, streamIdx ); 
	  }									
	return true;							
      }									
      bool setValue ( Stream<T_type>* v ) 
      { arg = v; keepalive = true; return true; }			
      bool Write(CallContext* call, Connection * conn )	
      { call->addStream ( (StreamVirtual*) arg ); return true; }
    };

}; // NameSpace
#endif // __DOOLP_DOOLPOBJECT_SLOT_CALLPARAMS_H
