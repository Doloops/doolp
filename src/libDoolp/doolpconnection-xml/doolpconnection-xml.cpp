#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpconnection-xml.h>
#include <doolp/doolp-doolpexceptions.h>
#include <doolp/doolp-doolpobjectstaticinfo.h> // For tellImplementedSlots
#include <arpa/inet.h>
#include <fcntl.h>

void Doolp::ConnectionXML::__initXML ( Doolp::Forge * _forge, int _inputDump, int _outputDump )
{
  __init ( _forge );
  if ( myForge == NULL )
    Bug ( "Could not set myForge !\n" );
  inputDump = _inputDump;
  outputDump = _outputDump;
  nextBlockIndex = 0;
  mySocket = 0;
  parser = NULL;
  doolpStreamIsInside = false;
  doolpStreamHasSentHeader = false;
}

Doolp::ConnectionXML::ConnectionXML ( Doolp::Forge * _forge )
{
  __DOOLP_Log ( "Creating new DoolpConnectionXML with forge=%p, no dumps.\n",
		_forge );
  __initXML ( _forge, -1, -1 );
  __DOOLP_Log ( "New Connection XML at %p\n", this );
}
Doolp::ConnectionXML::ConnectionXML ( Doolp::Forge * _forge, int _inputDump, int _outputDump )
{
  __DOOLP_Log ( "Creating new DoolpConnectionXML with forge=%p, inputDump=%d, outputDump=%d\n",
		_forge, _inputDump, _outputDump );
  __initXML ( _forge, _inputDump, _outputDump );
  __DOOLP_Log ( "New Connection XML at %p\n", this );
}
Doolp::ConnectionXML::ConnectionXML ( Doolp::Forge * _forge, Socket _mySocket, int _inputDump, int _outputDump )
{
  __DOOLP_Log ( "Creating new DoolpConnectionXML with forge=%p, inputDump=%d, outputDump=%d\n",
		_forge, _inputDump, _outputDump );
  __initXML ( _forge, _inputDump, _outputDump );
  mySocket = _mySocket;
  __DOOLP_Log ( "New Connection XML at %p with socket %d\n", this, mySocket );
}
Doolp::ConnectionXML::ConnectionXML ( Doolp::Forge * _forge, char * inputDumpFile, char * outputDumpFile )
{
  __DOOLP_Log ( "Creating new Doolp::ConnectionXML with forge=%p, inputDump='%s', outputDump='%s'\n",
		_forge, inputDumpFile, outputDumpFile );
  __initXML ( _forge, -1, -1 );
  inputDump = creat ( inputDumpFile, S_IRUSR | S_IWUSR );
  outputDump = creat ( outputDumpFile, S_IRUSR | S_IWUSR );
  close ( inputDump ); close ( outputDump );
  inputDump = open ( inputDumpFile, O_WRONLY | O_TRUNC );
  if ( inputDump == -1 )
    { Error ( "Could not open input dump file '%s' : Error %d:%s\n",
	      inputDumpFile, errno, strerror ( errno ) ); }
  outputDump = open ( outputDumpFile, O_WRONLY | O_TRUNC );
  if ( outputDump == -1 )
    { Error ( "Could not open output dump file '%s' : Error %d:%s\n",
	      outputDumpFile, errno, strerror ( errno ) ); }

  __DOOLP_Log ( "New Connection XML at %p (in=%d,out=%d)\n", this, inputDump, outputDump );
 
}

Doolp::ConnectionXML::Keywords Doolp::ConnectionXML::keywords;

Doolp::ConnectionXML::Keywords::Keywords ()
{
#define keyword(__k) { char * c = (char*)#__k; if ( c[0] == '_' ) c++; __k.set(c); hashTree.add ( &__k ); }
  #include <doolp/doolp-doolpconnection-xml-keywords.h>
#undef keyword
}

bool Doolp::ConnectionXML::tryConnect ( char * host, int port )
{
  
  int res;
  struct sockaddr_in AdrServ;
  
  __DOOLP_Log ( "Connecting to : %s:%d\n", host, port );
  
  int __sockAddrSz__ = sizeof ( struct sockaddr );
  
  if (  (mySocket = socket(PF_INET, SOCK_STREAM, 0)) <= -1)
    { Fatal ( "Unable to create client socket" ); }
  memset(&AdrServ,0,sizeof AdrServ);
  AdrServ.sin_port = htons(port);
  AdrServ.sin_family = PF_INET;
  inet_aton(host,&(AdrServ.sin_addr));
  
  if ( (res = connect(mySocket, (struct sockaddr *) &AdrServ, __sockAddrSz__ )) <= -1 )
    { Fatal ("Unable to connect() : result %d, errno %d:%s\n", 
	     res, errno, strerror ( errno ) ); }
  
  Info ( "New connection (%s:%d)\n", host, port );
  
  if ( ! handShake () )
    {
      close ( mySocket );
      return false;
    }
  myForge->addConnection ( this );
  if ( myForge->getDefaultDistAgentId () == 0 )
    myForge->setDefaultDistAgentId ( distAgentId );
  status = ConnectionStatus_ToBeAuthenticated;
  handle ();
  myForge->getObjectStaticInfo()->tellImplementedSlots ( distAgentId );
  ping ();
  return true;
}

bool Doolp::ConnectionXML::endConnection ()
{
  cancelHandleThread();
  status = ConnectionStatus_Disconnected;
  return true;
}

bool Doolp::ConnectionXML::flushWrite ()
{
  AssertBug ( isLockWrite(), "Connection is not locked for writing !\n" );
  try
    {
      writer->flush ();
    }
  catch ( DMLX::WriterException * e )
    {
      Warn ( "Got DMLX::WriterException '%s'\n", e->getMessage() );
      delete (e);
      throw new ConnectionCouldNotWrite();
    }
  return true;
}

bool Doolp::ConnectionXML::ping ()
{
  lockWrite ();
  ftime ( & pingTime );
  writer->writeMarkup ( keywords.ping );
  writer->writeMarkupEnd ();
  unlockWrite ();
  return true;
}

bool Doolp::ConnectionXML::handShake ()
{
  __DOOLP_Log ( "HandShaking DoolpConnectionXML\n" );

  status = ConnectionStatus_Handshaking;

  lockRead ();
  lockWrite ();

  AssertBug ( parser == NULL, "Already have a parser inited !\n" );
  AssertBug ( mySocket != 0, "No target socket defined !\n" );

  bool __parseBin = true;
  try
    {
      parser = new DMLX::ParserSocket ( mySocket );
      ((DMLX::ParserSocket *)parser)->dumpToSocket ( inputDump );
      //      parser->setParseCanFillBufferOnce ( true );
      parser->setMaxEndEventsInStoreSpace ( 1 );
      parser->setHashTree ( keywords.getHashTree() );
      parser->setParseBin ( __parseBin );
    } 
  catch (DMLX::ParserException * e)
    {
      Warn ( "Caught DMLX::ParserExcetion '%s'\n", e->getMessage() );
      delete (e);
      return false;
    }
  Log ( "DMLX::ParserSocket Created.\n" );
  try
    {
      writer = new DMLX::Writer ( mySocket, __parseBin );
      writer->setDumpFidles ( outputDump );
      writer->setWriteBinKeywords ( false );
      writer->setUseSend ( true );
      //      int fd = open ( "doolpConnection.log", O_CREAT + O_WRONLY );
      //      writer->setDumpFidles ( fd );
    }
  catch ( DMLX::WriterException * e )
    {
      Warn ( "Caught DMLX::WriterExcetion '%s'\n", e->getMessage() );
      delete (e);
      return false;
    }
  AgentId myAgentId = myForge->getAgentId ();
  if ( myAgentId > 0 )
    {
      writer->writeMarkup ( keywords.tellAgentId );
      writer->writeAttrInt ( keywords.agentId, myAgentId );
      writer->writeMarkupEnd ();
    }
  else
    {
      writer->writeMarkup ( keywords.queryAgentId );
      writer->writeMarkupEnd ();
    }
  flushWrite ();
  try
    {
      if ( parser->isEventName ( keywords.tellAgentId ) )
	{
	  distAgentId = parser->getAttrInt ( keywords.agentId );
	  if ( distAgentId == 0 )
	    Bug ( "Malformed agentId : '%s'\n", parser->getAttr ( keywords.agentId ) );
	  parser->popEventAndEnd ();
	  __DOOLP_Log ( "Distant agent says its agentId is '%d'\n", distAgentId );
	}
      else if ( parser->isEventName ( keywords.queryAgentId ) )
	{
	  distAgentId = myForge->getFreeAgentId ();
	  writer->writeMarkup ( keywords.provideAgentId );
	  writer->writeAttrInt ( keywords.agentId, distAgentId );
	  writer->writeMarkupEnd ();

	  flushWrite ();
	  __DOOLP_Log ( "Provided agentId='%d'\n", distAgentId );
	  parser->popEventAndEnd ();
	}
      else
	{
	  Bug ( "Unknown event name='%s'\n", parser->getEvent()->name->getKeyword() );
	}
      if ( myAgentId == 0 )
	{
	  parser->parse ();
	  if ( parser->isEventName ( keywords.provideAgentId ) )
	    {
	      myAgentId = parser->getAttrInt ( keywords.agentId );
	      if ( myAgentId == 0 )
		Bug ( "Malformed agentId : '%s'\n", parser->getAttr ( keywords.agentId ) );
	      myForge->setAgentId ( myAgentId );
	      __DOOLP_Log ( "Have been given agentId='%d'\n",
			    myAgentId );
	      parser->popEventAndEnd ();
	    }
	  else
	    {
	      Bug ( "Unknown event name='%s'. Expected 'provideAgentId'\n",
		    parser->getEvent()->name->getKeyword() );
	    }
	}
    }
  catch ( DMLX::ParserException * e )
    {
      Warn ( "DMLX::Parser returned exception '%s'\n",
	     e->getMessage() );
      unlockWrite ();
      unlockRead ();
      return false;
    }
  catch ( DMLX::ParserFullSpaceException * e )
    {
      Bug ( "Too little NameSpace ! Enlarge !!\n" );
    }
  __DOOLP_Log ( "Handshaked !\n" );
  status = ConnectionStatus_Handshaked;
  unlockWrite ();
  unlockRead ();
  return true;
}




