#ifndef __DOOLP_DOOLPOBJECT_DOOLPABLE_H
#define __DOOLP_DOOLPOBJECT_DOOLPABLE_H

/*
 * Doolpable parameters
 */
namespace Doolp
{
  template<unsigned int id_field,class T_obj>					
    class FakeDoolpable							
  {									
  public:								
    template<typename T_param>						
      class Doolpable							
      {									
      public:								
	mutable T_param value;						
	static ObjectParamId paramId;				
	static unsigned int offset;					
	inline void setModified()					
	  { getDoolpObject()->setModified ( paramId );			
	    __DOOLP_Log ( "[objId=%d paramId=0x%x : WRITE]",		
			  getDoolpObject()->getObjectId(), paramId );	
	  }								
	Doolpable::Doolpable ()						
	  {								
									
	  }								
	inline Doolpable & operator=(const T_param & op)			
	  {								
	    value = op;	setModified (); return *this;			
	  }								
	inline Doolpable & operator += ( const T_param & op )			
	  {								
	    value += op; setModified (); return *this;			
	  }								
	inline operator void* ()					
	  {								
	  return (void*) value;						
	}								
	inline operator T_param * ()						
	{								
	  return &value;						
	}								
	inline operator T_param ()						
	  {								
	    /* __DOOLP_Log ( "[objId=%d paramId=0x%x : READ]",		
	       getDoolpObject()->getObjectId(), paramId );  */
	    return value;						
	  }								
	void log ()							
	{								
	  printf ( "value=%d (at %p)\n", value, &value );		
	  printf ( "paramId=%d\n", paramId );				
	}								
	inline void setOffset ( T_obj * o )			
	  { offset = (unsigned int) this - (unsigned int) o; }		
	inline T_obj * getDoolpObject ()				
	  { return (T_obj *) ((unsigned int) this - offset); }	
      };								
  };
  

#ifndef doolpparam
#define doolpparam Doolp::FakeDoolpable<__LINE__,DoolpObject_CurrentObject>::Doolpable
// #define __doolpable_options__(...)
#endif // doolpable


#define DoolpObjectParam__StaticValues(__line,T_obj,__index,...) \
  Doolp::ObjectParamId Doolp::FakeDoolpable<__line,T_obj>::Doolpable<__VA_ARGS__>::paramId = __index; \
  unsigned int Doolp::FakeDoolpable<__line,T_obj>::Doolpable<__VA_ARGS__>::offset = 0
 
}; // NameSpace Doolp
#endif // __DOOLP_DOOLPOBJECT_DOOLPABLE_H
