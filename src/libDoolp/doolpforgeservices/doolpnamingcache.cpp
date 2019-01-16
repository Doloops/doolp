#include <doolp/doolp-doolpnamingcache.h>

// TODO : Check Doubles !!

Doolp::FullContextId nullFCI = { 0, 0, 0 };

Doolp::FullContextId * Doolp::NamingCache::doolpfunclocal(getNamedContext) ( string& name )
{
  FullContextId * fcId = contexts.get(name);
  if ( fcId != NULL )
    return fcId;
  throw new NoNamedContextFound();
}
bool Doolp::NamingCache::doolpfunclocal(setNamedContext) ( string& name, Doolp::FullContextId * id )
{
  FullContextId * fcId = new FullContextId();
  *fcId = *id;
  contexts.put (name, fcId);
  flush ();
  return true;
}

Doolp::ObjectId Doolp::NamingCache::doolpfunclocal(getNamedObject) ( string& contextName, string& name )
{
  return objects.get(contextName,name);
}

bool Doolp::NamingCache::doolpfunclocal(setNamedObject) ( string& contextName, string& name, Doolp::ObjectId objId )
{
  if ( objects.has(contextName, name) )
    {
      Warn ( "Already have a named object context='%s', object='%s'\n",
	     contextName.c_str(), name.c_str() );
      objects.remove ( contextName, name );
    }
  objects.put(contextName, name, objId);
  flush ();
  return true;
}
