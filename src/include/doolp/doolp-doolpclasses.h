#ifndef __DOOLP_DOOLPCLASSES_H
#define __DOOLP_DOOLPCLASSES_H

#include <doolp-config.h>

#include <doolp/doolp-doolpids.h>
#include <glog.h>

#include <list>
#include <map>
#include <string>
using namespace std;


#ifndef doolpable
#define doolpable
// #define __doolpable_options__(...)
#endif // doolpable

namespace Doolp
{

  class Forge;
  class ForgeScheduler;
  class Exception;
  class ForgeServices;
  class Naming;
  class Persistance;
  
  typedef struct ObjectInfo;
  typedef struct ObjectsInfo;
  typedef struct ExceptionInfo;
  
  class ObjectStaticInfo;
  class ObjectDynamicInfo;
  class ObjectCache;
  
  class Connection;
  class ConnectionTCP;
  
  class Context;
  typedef struct FullContextId;
  
  class CallContext;
  class Job;
  class Call;
  
  class StreamVirtual;
  template<typename T> class Stream;
  class Object;
  class ObjectLink;
  class ObjectBuffer;
  class ObjectBufferParam;
  class ObjectIndexer;
  
  typedef struct Forge_runJobData;
  
  typedef struct Agent;
  
  // typedef struct DoolpMsg_Header;
  // typedef struct DoolpMsg_NewCall_Header;
  // typedef struct DoolpMsg_BlockHeader;
  
  // List Aliases
  
  
  // Standard Aliases for numbers
  // typedef unsigned char u8;
  // typedef unsigned short u16;
  // typedef unsigned int u32;
  
  
}; // NameSpace Doolp
#endif 
