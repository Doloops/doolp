#ifndef __DOOLP_DOOLPEXCEPTION_H
#define __DOOLP_DOOLPEXCEPTION_H

#include <doolp/doolp-doolpids.h>

namespace Doolp
{
  class Exception
  {
  public:
    virtual ExceptionId getExceptionId () = 0;
    virtual char * getMessage () = 0;
    virtual bool __initStatic (Forge * __forge) = 0;
    virtual void throwMyself () = 0;
  };
  
};

#define __Doolp_DoolpException(__name,__text)				\
  __name : public Exception						\
    {									\
    public:								\
      bool __initStatic (Forge * __forge);				\
      __name () {}							\
      ExceptionId getExceptionId ();					\
      char * getMessage () { return #__name/**/" : "/**/__text; };	\
      void throwMyself() { throw this; }				\
    };

#endif // __DOOLP_DOOLPEXCEPTION_H
