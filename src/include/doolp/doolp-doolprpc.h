#ifndef __DOOLP_DOOLPRPC_H
#define __DOOLP_DOOLPRPC_H

#warning DEPRECATED

#if 0

#include <doolp/doolp-doolpids.h>
#include <doolp/doolp-doolpclasses.h>

typedef struct DoolpRPC
{
  DoolpRPCId rpcId;
  char * rpcName;
  bool isStdRPC;

  int (*prepare) ( DoolpJob * job, DoolpConnection * conn, void ** buffer );
  bool (*run) ( DoolpJob * job, DoolpConnection * conn, 
		DoolpObject * object,
		void * buffer, 
		int size );
  bool (*handleReply) ( DoolpCall * call, 
			DoolpConnection * conn );
  // Should add : precond, postcond, meta-data
};



#define DoolpRPC_set(_root_,_name_) \
  DoolpRPC _root_ = {		    \
    _root_##_rpcId, _name_,	    \
    _root_##_prepare,		    \
    _root_##_run,		    \
    _root_##_handleReply };

#endif

#endif // __DOOLP_DOOLPRPC_H
