#ifndef __DOOLP_DOOLPJOB_H
#define __DOOLP_DOOLPJOB_H


#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpcallcontext.h>
#include <doolp/doolp-doolpobject-slotvirtual.h>

/*
  This defines a locally running DoolpSlot
  */
namespace Doolp
{
  typedef enum JobStatus
    {
      JobStatus_Unknown   = 0,
      JobStatus_Preparing = 1,
      JobStatus_Prepared  = 2,
      JobStatus_Enqueued  = 3,
      JobStatus_getObject = 4,
      JobStatus_Running   = 5,
      JobStatus_Finished  = 6
    };

  class Job : public CallContext
  {
  public:
    JobStatus status;
    Forge * forge;
    SlotVirtual::CallParamsVirtual * callParams;
    Job ( Forge * _forge, 
	  ObjectId _objId,
	  ObjectNameId _objNameId,
	  ObjectSlotId _slotId )
      {
	type = CallContextType_Job;
	objId = _objId ;
	objNameId = _objNameId;
	slotId = _slotId;
	forge = _forge;
	status = JobStatus_Preparing;
      }
  };

}; // NameSpace Doolp
#endif // __DOOLP_DOOLPJOB_H
