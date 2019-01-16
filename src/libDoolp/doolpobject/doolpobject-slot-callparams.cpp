#include <doolp/doolp-doolpconnection.h>
#include <doolp/doolp-doolpobject-slot-callparams.h>
#include <doolp/doolp-doolpstream.h>

#define DoolpSlotCallParam__Int(T_type)					\
  DoolpSlotCallParam<T_type>::~DoolpSlotCallParam()			\
  {}									\
  T_type DoolpSlotCallParam<T_type>::getValue()				\
  { return (T_type)arg; }						\
  bool DoolpSlotCallParam<T_type>::Read(DoolpCallContext * job,		\
					DoolpConnection * conn)		\
  { conn->Read ( (int*)&arg ); return true; }				\
  bool DoolpSlotCallParam<T_type>::setValue ( T_type v )		\
  { arg = (void**) v; return true; }					\
  bool DoolpSlotCallParam<T_type>::Write(DoolpCallContext* call,	\
					 DoolpConnection * conn )	\
  { conn->Write ( (int)arg ); return true; }				\
  DoolpSlotCallParam<T_type&>::~DoolpSlotCallParam()			\
  {}									\
  T_type& DoolpSlotCallParam<T_type&>::getValue()			\
  { return (T_type&)arg; }						\
  bool DoolpSlotCallParam<T_type&>::Read(DoolpCallContext * job,	\
					 DoolpConnection * conn)	\
  { conn->Read ( (int*)&arg ); return true; }				\
  bool DoolpSlotCallParam<T_type&>::setValue ( T_type&v )		\
  { arg = (void**) (T_type)v; return true; }				\
  bool DoolpSlotCallParam<T_type&>::Write(DoolpCallContext* call,	\
					 DoolpConnection * conn )	\
  { conn->Write ( (int)arg ); return true; }				


// DoolpSlotCallParam__Int(int)
// DoolpSlotCallParam__Int(unsigned int)
// DoolpSlotCallParam__Int(bool)


// DoolpSlotCallParam__Int(short int)
  
#define DoolpSlotCallParam__Pointer(T_type)				\
  DoolpSlotCallParam<T_type*>::~DoolpSlotCallParam()			\
  { if ( ! keepalive ) free ( arg ); }					\
  T_type* DoolpSlotCallParam<T_type*>::getValue()			\
  { return (T_type*)(arg); }						\
  bool DoolpSlotCallParam<T_type*>::Read(DoolpCallContext * job,	\
					 DoolpConnection * conn)	\
  {									\
    conn->Read ( (T_type**) &arg );					\
    return true;							\
  }									\
  bool DoolpSlotCallParam<T_type*>::setValue ( T_type * v )		\
  { arg = (void**) v; keepalive = true; return true; }			\
  bool DoolpSlotCallParam<T_type*>::Write(DoolpCallContext* call,	\
					  DoolpConnection * conn )	\
  { conn->Write ( (T_type*) arg ); return true; }
// DoolpSlotCallParam__Pointer(char)
// DoolpSlotCallParam__Pointer(DoolpObject)
// DoolpSlotCallParam__Pointer(DoolpFullContextId)
/*
DoolpSlotCallParam<string&>::~DoolpSlotCallParam()
{ if ( ! keepalive ) free ( arg ); }				
string& DoolpSlotCallParam<string&>::getValue()			
{ return (string&)(*arg); }					
bool DoolpSlotCallParam<string&>::Read(DoolpCallContext * job, DoolpConnection * conn)	
{								
  Log ( "Asked to read a string (conn=%p)\n", conn );
  arg = (void**) new string();
  conn->Read ( (string*) arg );
  Log ( "Read : '%s'\n", ((string*)arg)->c_str() );
  return true;						
}
bool DoolpSlotCallParam<string&>::setValue ( string& v )	       
{ arg = (void**) &v; keepalive = true; return true; }			
bool DoolpSlotCallParam<string&>::Write(DoolpCallContext* call,		
					DoolpConnection * conn )	
{
  conn->Write ( (string&) *arg );
  return true;
}
*/


#define DoolpSlotCallParam__DoolpStream(T_type)				\
  DoolpSlotCallParam<DoolpStream<T_type>*>::~DoolpSlotCallParam()	\
  {}									\
  DoolpStream<T_type>* DoolpSlotCallParam<DoolpStream<T_type>*>::getValue() \
  { return (DoolpStream<T_type>*)arg; }					\
  bool DoolpSlotCallParam<DoolpStream<T_type>*>::Read(DoolpCallContext * job,	\
						      DoolpConnection * conn) \
  { AssertBug ( job != NULL, "No job given.\n" );			\
    arg = (void**) new DoolpStream<T_type>();				\
    unsigned int streamIdx = job->addDoolpStream ( (DoolpStreamVirtual*) arg );	\
    Log ( "DoolpStream idx=%d\n", streamIdx );				\
    if ( conn->readDoolpStreamSubSection() &&				\
	 ( conn->getNextBlockIndex () == streamIdx ) )			\
      {									\
	/* Shall read stream content here... */				\
	Log ( "Read stream content for stream %d\n", streamIdx );	\
	conn->readDoolpStream ( job, (DoolpStream<T_type>*) arg, streamIdx ); \
      }									\
    return true;							\
  }									\
  bool DoolpSlotCallParam<DoolpStream<T_type>*>::setValue ( DoolpStream<T_type>* v ) \
  { arg = (void**) v; keepalive = true; return true; }			\
  bool DoolpSlotCallParam<DoolpStream<T_type>*>::Write(DoolpCallContext* call, \
						       DoolpConnection * conn )	\
  { call->addDoolpStream ( (DoolpStreamVirtual*) arg );			\
    return true; }
/*
DoolpSlotCallParam__DoolpStream(int)
DoolpSlotCallParam__DoolpStream(char*)
DoolpSlotCallParam__DoolpStream(string)
DoolpSlotCallParam__DoolpStream(DoolpObject*)
DoolpSlotCallParam__DoolpStream(DoolpObjectId)
*/
