#ifndef __DOOLP_DOOLPOBJECTINDEXERSERVER_H
#define __DOOLP_DOOLPOBJECTINDEXERSERVER_H

#include <doolp/doolp-doolpids.h>
#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpobjectindexerbyrule.h>
#include <doolp/doolp-doolpforgeservices.h>

// #include <doolp/doolp-doolpforge.h>
#include <map>
using namespace std;

namespace Doolp
{
  typedef struct 
  {
    ObjectId start, step;
  } ObjectIndexerRule;

  class ObjectIndexerByRuleService : public ForgeService
  {
  protected:
    Forge * myForge;
    inline Forge * getForge() { return myForge; }

    list<ObjectIndexerRule> freeRules;
    list<ObjectIndexerByRule *> objectIndexersByRule;

    ObjectIndexerRule * getFreeRule ();
    bool splitObjectIndexer ( ObjectIndexerByRule * indexer );
    void createNewRules ();
    void createRulesFromRules ( unsigned int number );
  public:
    ObjectIndexerByRuleService ( Forge * _forge );

    bool start () { return true; }
    bool stop () { return true; }
  
    Object * getServiceAsObject ();

    ObjectIndexerByRule * getObjectIndexerByRule ( AgentId id );
    void refreshStarts ( );
    void log ();
  };
};
#endif // __DOOLP_DOOLPOBJECTINDEXERSERVER_H
