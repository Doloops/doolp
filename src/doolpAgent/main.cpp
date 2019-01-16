#include <DMLXParser.h>
#include <glogers.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define __DOOLP_INCLUDES_FOR_DOOLPFORGESERVICES__
#define __DOOLP_INCLUDES_FOR_DOOLPCC__
#include <doolp.h>


setGlog ( "doolpAgent" );

void doolpAgentInterruption(int sig)
{
  Fatal ( "DoolpAgent Interrupted.\n" );
}

bool runAgent ( char * agentConf )
{
  DMLX::ParserFile parser = DMLX::ParserFile ( agentConf );
  try
    {
      if ( ! parser.canFill() )
	Fatal ( "Can not read file : '%s'\n", agentConf );
      parser.parse();
      parser.logEvents();
    }
  catch ( DMLX::ParserException * e )
    {
      Warn ( "Exception : '%s' (fatal=%d)\n", e->getMessage(), e->isFatal() );
      //      if ( e->isFatal() )
      return false;
    }
  
  parser.checkEventName ( "DoolpAgent" );
  Doolp::AgentId myAgentId = 0;
  Log ( "----------------------------------------------\n" );
  if ( parser.getAttrBool ( "glog" ) )
    {
      Log ( "Adding XMLSock glog.\n" );
      Log ( "Glog with name '%s'\n", parser.getAttr ( "name" ) );
      Warn ( "Gloger_XMLSock now deprecated...\n" );
      //      addGlog ( new Gloger_XMLSock ( parser.getAttr ( "name" ), "127.0.0.1", 13807 ) );
    }
  myAgentId = parser.getAttrInt ( "myAgentId" );
  Doolp::Forge * myForge = new Doolp::Forge ( myAgentId );
  myForge->options.useWaitSpecificCall = true;
  myForge->options.forceJobsInSameThread = true;

  Log ( "------------ new Doolp::Forge agentId='%d' ------------\n", myForge->getAgentId () );
  bool log = parser.getAttrBool ( "logStats" );
  parser.popEvent ();
      
  while ( ! parser.isEventEnd () ) 
    {
      Log ( "--------------------------------------------\n" );
      Log ( "At section : '%s'\n", parser.getEventName() );
      if ( parser.isEventName ( "ObjectIndexerService" ) )
	{
	  if ( strcmp ( parser.getAttr ( "type" ), "ByRule" ) == 0 )
	    {
	      Doolp::ObjectIndexerByRuleService * ois = new Doolp::ObjectIndexerByRuleService ( myForge );
	      myForge->getServices()->add ( ois );
	    }
	  else
	    Fatal ( "Unkown Doolp::ObjectIndexerService type : '%s'\n", parser.getAttr ( "type" ) );
	  parser.popEventAndEnd ();
	  continue;
	}
      if ( parser.isEventName ( "Persistance" ) )
	{
	  Doolp::Persistance * pr = NULL;
	  if ( strcmp ( parser.getAttr ( "type" ), "FTree" ) == 0 )
	    {
	      pr = new Doolp::PersistanceFTree ( parser.getAttr ( "root" ) );
	      myForge->getServices()->add ( (Doolp::PersistanceFTree*)pr );
	      myForge->options.allowDoolpObjectBuffer = true;
	    }
	  else
	    Fatal ( "Unkown Doolp::Persistance type : '%s'\n", parser.getAttr ( "type" ) );
	  AssertFatal ( pr != NULL, "Could not create a Doolp::Persistance of type '%s'\n", 
			parser.getAttr ( "type" ) );
	  myForge->setPersistance ( pr );
	  parser.popEventAndEnd ();
	  Log ( "Successfully added persistance service\n" );
	  continue;
	}
      if ( parser.isEventName ( "Naming" ) )
	{
	  Doolp::Naming * nc = NULL;
	  if ( strcmp ( parser.getAttr ( "type" ), "Cache" ) == 0 )
	    {
	      nc = new Doolp::NamingCache ();
	      myForge->getServices()->add ( (Doolp::NamingCache*)nc );
	    }
	  else if ( strcmp ( parser.getAttr ( "type" ), "File" ) == 0 )
	    {
	      nc = new Doolp::NamingFile ( parser.getAttr ( "file" ) );
	      myForge->getServices()->add ( (Doolp::NamingFile*)nc );
	    }
	  else
	    Fatal ( "Unkown Doolp::Naming type : '%s'\n", parser.getAttr ( "type" ) );
	  AssertFatal( nc != NULL, "Could not create a Doolp::Naming of type '%s'\n", 
		       parser.getAttr ( "type" ) );
	  myForge->setNaming ( nc );
	  parser.popEventAndEnd ();
	  continue;
	}
      if ( parser.isEventName ( "TCPServer" ) )
	{
	  __DOOLP_Log ( "Starting a TCP Server type '%s' named '%s' at %s:%s\n",
			parser.getAttr ( "type" ),
			parser.getAttr ( "name" ),
			parser.getAttr ( "bindAddress" ),
			parser.getAttr ( "port" ) );
	  Doolp::ConnectionTCPServer * server = 
	    new Doolp::ConnectionTCPServer ( myForge,
					   parser.getAttr ( "type" ),
					   parser.getAttr ( "bindAddress" ),
					   parser.getAttrInt ( "port" ) );
	  server->start();
	  __DOOLP_Log ( "Server openned\n" );
	  parser.popEventAndEnd ();
	  continue;		  
	}
      if ( parser.isEventName ( "PubSub" ) )
	{
	  Doolp::PubSubService * ps = new Doolp::PubSubService ( myForge, new Doolp::PubSubBroker() );
	  myForge->getServices()->add ( ps );
	  ps->start ();
	  parser.popEventAndEnd ();
	  continue;
	}
      if ( parser.isEventName ( "ObjectsLib" ) )
	{
	  __DOOLP_Log ( "Adding lib : %s\n",
			parser.getAttr ( "path" ) );
	  char * libName =  parser.getAttr ( "path" );
	  myForge->getObjectStaticInfo()->addLib ( libName );
	  parser.popEventAndEnd ();
	  continue;		  
	}
      if ( parser.isEventName ( "Connection" ) )
	{
	  if ( parser.hasAttr ( "logfile" ) )
	    {
	      Bug ( "To reimplement : logfile\n" );
	    }
	  myForge->tryConnect ( parser.getAttr ( "path" ) );
	  parser.popEventAndEnd ();
	  continue;		  
	}
      Warn ( "Invalid section '%s'\n", parser.getEventName () );
      parser.popEventAndEnd();
    } // Reading Doolp::Agent contents
  AssertFatal ( parser.isEventEnd() && parser.isEventName ( "DoolpAgent" ), "Invalid XML Syntax.\n" );
  parser.popEventEnd ();
  Log ( "Parse of '%s' finished.\n", agentConf );
  unsigned int iter = 0;
  while ( true )
    {
      iter++;
      //      Log ( "." );
      if ( log )
	myForge->logStats ();
      sleep ( 1 );
      if ( iter == 200 )
	exit (-1);
    }
}

bool parseAgentConf ( char * confFile )
{
  DMLX::ParserFile parser = DMLX::ParserFile ( confFile );
  try
    {
      if ( ! parser.canFill() )
	Fatal ( "Can not read file !\n" );
      parser.parse();
      __DOOLP_Log ( "Parsing of file '%s' finished.\n", confFile );
      __DOOLP_Log ( "Conf name '%s'\n", parser.getEventName() );
    }
  catch ( DMLX::ParserException * e )
    {
      Warn ( "Exception : '%s' (fatal=%d)\n", e->getMessage(), e->isFatal() );
      if ( e->isFatal() )
	return false;
    }

  while ( ! parser.isEventEnd () ) // Reading Doolp::AgentConfig
    {
      parser.checkEventName ( "Doolp::AgentConfig" );
      __DOOLP_Log ( "Running configuration '%s'\n", parser.getAttr ( "name" ) );
      parser.popEvent();
      while ( ! parser.isEventEnd() )
	{
	  parser.checkEventName ( "Doolp::Agent" );
	  AssertFatal ( parser.hasAttr ( "config" ), "Item Doolp::Agent has no attribute 'config'\n" );
	  char * conf = (char*) malloc ( 128 );
	  strcpy ( conf, parser.getAttr ( "config" ) );
	  Log ( "Running agent with conf = '%s'\n", conf );

	  pid_t child = fork ();
	  
	  if ( child > 0 ) // Pere
	    {
	      Log ( "Run child with pid=%d\n", child );
	    }
	  else if ( child == 0 ) // Fils
	    {
	      runAgent ( conf );
	    }
	  else
	    {
	      Bug ( "Could not fork\n" );
	    }
	  parser.popEventAndEnd ();
	}
    } // Reading Doolp::AgentConfig
  AssertFatal ( parser.isEventEnd() && parser.isEventName ( "Doolp::AgentConfig" ), "Invalid XML Syntax.\n" );
  parser.popEventEnd ();
  return true;
}

int main ( int argc, char ** argv )
{
  
  addGlog ( new Gloger_File ( stderr ) );
  //  addGlog ( new Gloger_BinFile ( "log.bin" ) );
  //  addGlog ( new Gloger_DMLXBinFile ( "log.dmlx" ) );
  //  showGlogPrior ( __GLOG_PRIOR_LOG, false );

  signal ( SIGINT, doolpAgentInterruption );
  //  addGlog ( new GlogInfo_XMLFile ( "log.xml" ) );
  __DOOLP_Log ( "DoolpAgent started.\n" );
  
  if ( argc == 3 && ( strcmp ( argv[1], "--conf" ) == 0 ) )
    {
      Fatal ( "Wrong number of arguments\n" );
      __DOOLP_Log ( "XML configuration given : %s\n",
		    argv[2] );
      char * confFile = argv[2];
      parseAgentConf ( confFile );
    }
  else if ( argc == 2 )
    {
      runAgent ( argv[1] );
    }
  else
    {
      Bug ( "Wrong syntax.\n" );
    }
  //  parser.popEventEnd ();
  __DOOLP_Log ( "End of configuration initing.\n" );
  while ( true )
    sleep ( 100 );
  return 0;
}
