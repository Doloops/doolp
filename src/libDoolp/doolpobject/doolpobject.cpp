#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpconnection.h>
#include <doolp/doolp-doolpcontext.h>
#include <doolp/doolp-doolpobjectcache.h>

Doolp::Object::~Object ()
{ 
  Info ( "Destructing DoolpObject at %p (objId=0x%x,forge=%p)\n", this, getObjectId(), getForge() );
  if ( objectId != 0 && forge != NULL )
    {
      forge->getObjectCache()->remove ( this, true );
    }
}

void Doolp::Object::setModified ( Doolp::ObjectParamId paramId )
{
  if ( getForge () == NULL )
    {
      Warn ( "Object not linked to a forge\n" );
      return;
    }
  if ( getForge()->getContext() == NULL )
    Bug ( "Could not get Context !\n" );

  __DOOLP_Log ( "modified paramId 0x%x at stepping %d\n",
		paramId, getForge()->getStepping () );

  changeSet[getForge()->getStepping()].remove ( paramId );
  changeSet[getForge()->getStepping()].push_back ( paramId );
  getForge()->getContext()->setModifiedObject ( this );
}

void Doolp::Object::logChangeSet ()
{
  // When do we have to clean the changeSet ?
  ObjectChangeSet::iterator change;
  for ( change = changeSet.begin ();
	change != changeSet.end ();
	change ++ )
    {
      __DOOLP_Log ( "\tStepping '%d'\n", change->first );
      list<ObjectParamId>::iterator param;
      for ( param = change->second.begin ();
	    param != change->second.end ();
	    param ++ )
	{
	  __DOOLP_Log ( "\t\tparamId '0x%x'\n", *param );
	}
    }

}

bool Doolp::Object::serialize ( Doolp::Connection * conn, list<Doolp::ObjectParamId> * paramList )
{
  if ( paramList == NULL )
    return serialize ( conn );
  conn->WriteObjectHead ( this );
  for ( list<Doolp::ObjectParamId>::iterator param = paramList->begin ();
	param != paramList->end () ; param++ )
    serialize ( conn, *param );
  conn->endSubSection ();
  return true;
}


bool Doolp::Object::serializeFromStepping ( Doolp::Connection * conn, 
					    Doolp::ContextStepping stepping )
{
  if ( stepping == 0 )
    return serialize ( conn );
  __DOOLP_Log ( "Serialize objId 0x%x (%p) from stepping %d\n",
		objectId, this, stepping );
  if ( getForge () == NULL )
    Bug ( "Object not linked to a forge.\n" );
  conn->WriteObjectHead ( this );
  ObjectChangeSet::iterator change;
  for ( change = changeSet.begin ();
	change != changeSet.end ();
	change ++ )
    {
      __DOOLP_Log ( "Stepping '%d'\n", change->first );
      if ( change->first < stepping )
	continue;
      list<ObjectParamId>::iterator param;
      for ( param = change->second.begin ();
	    param != change->second.end ();
	    param ++ )
	{
	  __DOOLP_Log ( "Serialize paramId '0x%x'\n", *param );
	  serialize ( conn, *param );
	}
    }
  conn->endSubSection ();
  __DOOLP_Log ( "finished.\n" );
  return true;
}


