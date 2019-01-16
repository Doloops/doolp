#include <stdlib.h> // for NULL

char * glog_keyword_list[] = { "program", "offset", "glog", "t", "tm", "thread", 
			       "module", "file", "func", "line", "prior", "text", "id", "value", NULL };
char * doolp_keyword_list[] = 
  { 
    // HandShaking commands
    "queryAgentId",
    "tellAgentId",
    "provideAgentId",
    // Message basics
    "Call",
    "Reply",
    "InCall",
    "CallParams",
    "Params",
    "agentId",
    "contextId",
    "stepping",
    "type",
    "value",
    "index",
    "fromAgentId",
    "toAgentId",
    "objectId",
    "objectNameId",
    "ownerAgentId",
    "exceptionId",
    "slotId",
    "slotName",
    "message",
    // Types lits
    "int",
    "string",
    "float",
    "FullContextId",
    "Object",
    "ObjectId",
    "list",
    "Exception",
    "Stream",
    // Stream
    "StreamEnd",
    // Ping mechanism
    "ping",
    "ping_reply",
    NULL
  };
