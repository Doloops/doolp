#include <doolp/doolp-doolpobjectindexerbyruleservice.h>
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpobjectcache.h>
#include <doolp/doolp-doolpforge.h>

#include <doolp/doolp-doolpexceptions.h>


Doolp::ObjectIndexerByRuleService::ObjectIndexerByRuleService ( Doolp::Forge * _forge )
{
  myForge = _forge;
  ObjectIndexerRule rule;
  rule.step = 2;
  serviceName = "DoolpObjectIndexer";
  rule.start = 1; freeRules.push_back ( rule );
  rule.start = 2; freeRules.push_back ( rule );
  createRulesFromRules ( 2048 );
  ObjectIndexerByRule * br = getObjectIndexerByRule ( getForge()->getAgentId() );
  getForge()->setObjectIndexer ( br );
}


Doolp::Object * Doolp::ObjectIndexerByRuleService::getServiceAsObject ()
{
  __DOOLP_Log ( "ObjectIndexer for agentId '%d'\n", getForge()->getAgentIdForThisJob() );
  if ( getForge()->getAgentIdForThisJob() == 0 )
    {
      return NULL;
    }
  AssertBug ( getForge()->getAgentIdForThisJob() != getForge()->getAgentId (), " Job is called local !?!\n" );
  Object * obj = getObjectIndexerByRule ( getForge()->getAgentIdForThisJob() );
  getForge()->addObject ( obj ); //, getForge()->getFullContextId () );
  obj->setTTL ( 0 );
  obj->setOwner ( getForge()->getAgentIdForThisJob() );
  return obj;
}

void Doolp::ObjectIndexerByRuleService::createRulesFromRules ( unsigned int number )
{
  unsigned int created = 0;
  AssertBug ( freeRules.size () > 0, " No Rules !\n" );
  while ( created != number )
    {
      ObjectIndexerRule rule1, rule2;
      rule1 = freeRules.front ();
      freeRules.pop_front ();
      rule2.start = rule1.start + rule1.step;
      rule1.step *= 2;
      rule2.step = rule1.step;
      freeRules.push_back ( rule1 );
      freeRules.push_back ( rule2 );
      created ++;
    }
  __DOOLP_Log ( "Created : %d rules of %d.\n", created, number );
}

Doolp::ObjectIndexerRule * Doolp::ObjectIndexerByRuleService::getFreeRule ()
{
  unsigned int minRules = 32;
  if ( freeRules.size () < minRules )
    {
      createNewRules ();
    }
  if ( freeRules.size () != 0 )
    {
       ObjectIndexerRule * rule = &(freeRules.front() );
       freeRules.pop_front ();
       return rule;
    }
  return NULL;
}

Doolp::ObjectIndexerByRule * Doolp::ObjectIndexerByRuleService::getObjectIndexerByRule ( Doolp::AgentId id )
{
  Doolp::ObjectIndexerRule * rule = getFreeRule ();
  if ( rule == NULL )
    Bug ( "Could not create a new rule.\n" );
  Doolp::ObjectIndexerByRule * oi = new Doolp::ObjectIndexerByRule ( rule->start, rule->step );
  objectIndexersByRule.push_back ( oi );
  return oi;
}

bool Doolp::ObjectIndexerByRuleService::splitObjectIndexer ( Doolp::ObjectIndexerByRule * oi )
{
  ObjectId oldStep = oi->step; 
  try
    {
      ObjectId newStart = oi->setNewRule ( oi->step * 2 );
      oi->start = newStart;
      
      if ( oi->step != oldStep * 2 )
	{
	  Bug ( "Wrong modification of rule for object '%d', "
		"new step is %d (should be %d)\n",
		oi->getObjectId(), (ObjectId) oi->step, oldStep * 2 );
	  oi->step = oldStep * 2;
	}
      else
	{
	  __DOOLP_Log ( "Good modification of rule for object '%d'\n",
			oi->getObjectId() );
	}
      ObjectIndexerRule rule;
      rule.start = newStart + oldStep; rule.step = oldStep + 2; 
      freeRules.push_back ( rule );
      __DOOLP_Log ( "New Rule : (%d, %d)\n", rule.start, rule.step );
    }
  catch ( CallNoAgentFound * e )
    {
      Warn ( "Exception raised : '%s'\n", e->getMessage () );
      delete ( e );
      return false;
    }
  return true;
}

void Doolp::ObjectIndexerByRuleService::createNewRules ()
{
  unsigned int created = 0, mustCreate = 1;
  refreshStarts ();
  while ( created < mustCreate )
    {
      __DOOLP_Log ( "Created %d/%d\n", created, mustCreate );
      list<ObjectIndexerByRule*>::iterator it = objectIndexersByRule.begin();
      list<ObjectIndexerByRule*>::iterator oit = it;
      ObjectId minId = (ObjectId)(*it)->start + (ObjectId)(*it)->step;
      ObjectIndexerByRule * oi = *it;
      it++;
      for ( ; it != objectIndexersByRule.end() ; it++ )
	{
	  __DOOLP_Log ( "At doolpObject 0x%x\n", (*it)->getObjectId() );
	  if ( (ObjectId)(*it)->start + (ObjectId)(*it)->step < minId )
	    { oi = *it; oit = it; minId = (*it)->start + (*it)->step; }
	} 
      __DOOLP_Log ( "Found the indexer %d with min start %d, step %d, total %d.\n", 
		    oi->getObjectId(), (ObjectId) oi->start, (ObjectId) oi->step,
		    (ObjectId) oi->start + (ObjectId) oi->start ); 
      if ( splitObjectIndexer ( oi ) ) 
	{
	  __DOOLP_Log ( "Success !\n" );
	  created++;
	}
      else
	{
	  __DOOLP_Log ( "Failed !\n" );
	  objectIndexersByRule.erase ( oit );
	  Warn ( "erased.\n" );
	  delete ( oi );
	}
    }
  __DOOLP_Log ( "Created %d rules (active objectIndexersByRule %d, freeRules %d)\n", 
		created, objectIndexersByRule.size (), freeRules.size () );
}

void Doolp::ObjectIndexerByRuleService::refreshStarts ()
{
  list<ObjectIndexerByRule*>::iterator it;
  for ( it = objectIndexersByRule.begin(); it != objectIndexersByRule.end() ; it++ )
    {
      __DOOLP_Log ( "Trying to getStart : objId=%d\n", (*it)->getObjectId() );
      try
	{
	  (*it)->start = (*it)->getStart ( true );
	}
      catch ( CallNoAgentFound * e )
	{
	  __DOOLP_Log ( "ObjectId=%d dead\n", (*it)->getObjectId() );
	  delete ( *it );
	  objectIndexersByRule.erase ( it );
	  return;
	}
    }
}


void Doolp::ObjectIndexerByRuleService::log ()
{
  //  refreshStarts ();
  list<ObjectIndexerByRule*>::iterator it;
  for ( it = objectIndexersByRule.begin(); it != objectIndexersByRule.end() ; it++ )
    {
      __DOOLP_Log ( "start %d, step %d, owner %d, objectId %d\n",
		    (ObjectId) (*it)->start.value, 
		    (ObjectId) (*it)->step.value, 
		    (*it)->getOwnerAgentId (), (*it)->getObjectId() );
    }
}
