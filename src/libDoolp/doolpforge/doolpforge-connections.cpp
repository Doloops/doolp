#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpconnection.h>
// #include <doolp/doolp-doolpconnection-tcp.h>
#include <doolp/doolp-doolpconnection-xml.h>

/*
 * Connection handling
 */


bool Doolp::Forge::addConnection ( Doolp::Connection * connection )
{
  if ( getConnectionForAgentId ( connection->distAgentId, true ) != NULL )
    { Bug ( "Duplicate Connection for agentId %d\n",
	    connection->distAgentId ); }
 
  __DOOLP_Log ( "Adding connection %p\n", connection );
  lockConnections ();
  connections.push_back ( connection );
  unlockConnections ();
  return true;
}

bool Doolp::Forge::removeConnection ( Doolp::Connection * connection )
{
  lockConnections ();
  connections.remove ( connection );
  unlockConnections ();
  return true;
}

bool Doolp::Forge::tryConnect ( const char * connectionString )
// Format : [PROTO]://host:port/options
{
  char * s = (char*) malloc ( strlen ( connectionString ) + 1 );
  strcpy ( s, connectionString );
  char * proto = s, * host, * port;
  host = strstr ( s, "://" );
  if ( host == NULL )
    {
      Error ( "Invalid syntax : '%s'\n", connectionString );
      free ( s );
      return false;
    }
  host[0] = '\0';
  host += 3;
  port = strchr ( host, ':' );
  port[0] = '\0';
  port++;
  unsigned int port_i = atoi ( port );
  Log ( "Proto='%s', Host='%s', Port='%d'\n", proto, host, port_i );
  if ( strcmp ( proto, "doolpxml" ) == 0 )
    {
      ConnectionXML * conn = new ConnectionXML ( this, "connectionLog.in.bin", "connectionLog.out.bin" );
      return conn->tryConnect ( host, port_i );
    }
  Error ( "Unknown proto '%s'\n", proto );
  return false;
}

Doolp::Connection * Doolp::Forge::getConnectionForAgentId (Doolp::AgentId agentId,
							   bool strict)
{

  if ( agentId == 0 ) 
    {
      Bug ( "Could not be given a zero agentId\n" );
      return NULL;
    }
  lockConnections ();
  for ( list<Connection*>::iterator conn = connections.begin () ;
	conn != connections.end (); conn++ )
    {
      if ( (*conn)->disableGetConnectionForAgentId )
	continue;
      if ( (*conn)->distAgentId == agentId )
	{
	  __DOOLP_Log ( "Found Connection %p for agentId %d\n", (*conn), agentId );
	  unlockConnections ();
	  return (*conn);
	}
    }
  unlockConnections ();
  __DOOLP_Log ( "Could not find Connection to agentId %d\n", agentId );
  if ( strict ) return NULL;
  __DOOLP_Log ( "Chosing default connection\n" );
  if ( agentId == getDefaultDistAgentId () )
    {
      Bug ( "the default DistAgentId is myself !!\n" );
    }
  if ( getDefaultDistAgentId () == 0 )
    return NULL;
  return getDefaultConnection ();
}

