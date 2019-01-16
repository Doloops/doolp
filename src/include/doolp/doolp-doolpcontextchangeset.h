#ifndef __DOOLP_DOOLPCONTEXTCHANGESET_H
#define __DOOLP_DOOLPCONTEXTCHANGESET_H

#include <list>
#include <map>

using namespace std;

namespace Doolp
{
typedef struct ContextChange
{
  Object * obj;
  ContextStepping creation;
  ContextStepping deletion;
  ContextStepping firstModified;
  ContextStepping lastModified;
};


typedef map<ObjectId, ContextChange*> ContextChangeSet;

}; // NameSpace Doolp
#endif // __DOOLP_DOOLPCONTEXTCHANGESET_H
