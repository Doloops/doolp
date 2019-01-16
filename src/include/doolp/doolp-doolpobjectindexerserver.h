#ifndef __DOOLP_DOOLPOBJECTINDEXERSERVER_H
#define __DOOLP_DOOLPOBJECTINDEXERSERVER_H

#include <doolp/doolp-doolpids.h>
#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpobjectindexer.h>
#include <doolp/doolp-doolpforgeservices.h>

// #include <doolp/doolp-doolpforge.h>
#include <map>
using namespace std;

typedef struct 
{
  DoolpObjectId start, step;
} DoolpObjectIndexerRule;

class DoolpObjectIndexerServer : public DoolpForgeService
{
 protected:
  list<DoolpObjectIndexerRule> freeRules;
  list<DoolpObjectIndexer *> objectIndexers;

  DoolpObjectIndexerRule * getFreeRule ();
  bool splitObjectIndexer ( DoolpObjectIndexer * indexer );
  void createNewRules ();
  void createRulesFromRules ( unsigned int number );
 public:
  DoolpObjectIndexerServer::DoolpObjectIndexerServer();

  bool start () { return true; }
  bool stop () { return true; }
  
  DoolpObject * getServiceAsDoolpObject () { return NULL; }

  DoolpObjectIndexer * getDoolpObjectIndexer ( DoolpAgentId id );
  void refreshStarts ( );
  void log ();
};

#endif // __DOOLP_DOOLPOBJECTINDEXERSERVER_H
