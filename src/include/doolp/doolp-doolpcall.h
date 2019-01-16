#ifndef __DOOLP_DOOLPCALL_H
#define __DOOLP_DOOLPCALL_H


#ifdef __DOOLPCALL_USE_SEM
#include <semaphore.h>
#endif

#include <doolp/doolp-doolpcallcontext.h>

namespace Doolp
{
  class Call : public CallContext
  {
  public:
    bool replied;
    bool async;
#ifdef __DOOLPCALL_USE_SEM
    sem_t semaphore;
#endif
    Exception * exception;
    SlotVirtual::ReplyParamsVirtual * replyParams;
    
    bool waitSpecific;
    Call ( )
      {
	this->type = CallContextType_Call;
#ifdef __DOOLPCALL_USE_SEM
	sem_init ( &(semaphore), 0, 0 );
#endif 
	replied = false;
	async = false;
	slot = NULL;
	obj = NULL;
	exception = NULL;
	replyParams = NULL;
      }
    ~Call ()
      { Log ( "Destructor for call at %p\n", this ); }
  };
}; // NameSpace Doolp
#endif // __DOOLP_DOOLPCALL_H
