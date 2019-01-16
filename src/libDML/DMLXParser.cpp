#include <DMLXParser.h>
#include <string.h>
#include <stdlib.h>

#define __DMLX_PARSER_READBUFFER_SZ 1024
#define __DMLX_PARSER_NAMESPACE_SZ  __DMLX_PARSER_READBUFFER_SZ * 1024 * 2 // 8192 * 32

DMLX::Parser::Parser() :
  storeSpace(__DMLX_PARSER_NAMESPACE_SZ)
{ 
  Log ( "New Parser\n" );
  canDeleteHashTree = true;
  parseBin = false;
  bufferIdx = 0;
  bufferSz = 0;
  firstEvent = NULL;
  lastEvent = NULL;
  eventsNumber = 0;
  endEventsNumber = 0;
  parseKeepAllText = false;
  parseCanFillBufferOnce = false;
  maxEndEventsInStoreSpace = 0;
  bufferMax = __DMLX_PARSER_READBUFFER_SZ;
  buffer = (char*) malloc ( bufferMax + 1 );
  destructorShallFreeBuffer = true;
  hashTree = new HashTree();
  AssertFatal ( buffer != NULL, "Could not malloc read buffer (size=%d) !\n", bufferMax );
}

DMLX::Parser::~Parser()
{
#if 0
  if ( destructorShallFreeBuffer )
    free ( buffer );
  if ( canDeleteHashTree )
    delete ( hashTree );
#endif
}

bool DMLX::Parser::doFillBuffer ()
{
  if ( bufferSz > 0 )
    {
      Warn ( "Could not fillBuffer() : already filled !\n" );
      return false;
    }
  AssertBug ( bufferIdx == 0, "bufferSz is zero, but not bufferIdx !\n" );
  if ( ! canFill() )
    {
      Warn ( "Could not fillBuffer() : canFill() is false\n" );
      return false;
    }
  bufferSz = fillBuffer ( buffer, bufferMax );
  if ( bufferSz == 0 )
    {
      Warn ( "Could not fillBuffer() : returned zero\n" );
      return false;
    }
  return true;
}

DMLX::Parser::Parsing::Parsing ()
{ 
  state = parseStateMarkupOutside; event = NULL; lastAttr = NULL;
  totalParsed = 0;
  totalLinesParsed = 0;
  name = NULL; value = NULL; text = NULL; symbolSz = 0;
  eventStackLevel = 0;
  nameBuffer[0] = '\0'; nameBufferSz = 0;
  unsigned int symbolMax = 32, symb = 0;
  symbolLong = (char**) malloc ( sizeof(char*) * symbolMax );
  symbolShort = (char**) malloc ( sizeof(char*) * symbolMax );
#define __addSymbol(_c,_d)				\
  symbolLong[symb] = _c; symbolShort[symb] = _d;	\
  symb++; 
  __addSymbol( "lt","<" );
  __addSymbol( "gt",">" );
  __addSymbol( "amp","&" );
  __addSymbol( "quot","\"" );
  __addSymbol( NULL, NULL );
#undef __addSymbol
  reqLength = 0;
  valueFlag = 0;
}

DMLX::Parser::Parsing::~Parsing ()
{
  free ( symbolLong );
  free ( symbolShort );
}

/*
 * Options Setting
 */

void DMLX::Parser::setParseKeepAllText ( bool value )
{
  parseKeepAllText = value;
}

void DMLX::Parser::setMaxEndEventsInStoreSpace ( unsigned int number )
{
  maxEndEventsInStoreSpace = number;
}
void DMLX::Parser::setParseCanFillBufferOnce ( bool value )
{
  parseCanFillBufferOnce = value;
}
void DMLX::Parser::setParseBin ( bool value )
{
  parseBin = value;
}

/*
 * Event Handling
 */ 

unsigned int DMLX::Parser::getEventsNumber ()
{
  if ( eventsNumber > 0 )
    return eventsNumber;
  if ( canFill () )
    parse ();
  return eventsNumber;
}

bool DMLX::Parser::isFinished ()
{
  return ( getEventsNumber() == 0 );
}

DMLX::Event * DMLX::Parser::getEvent ()
{
#warning TODO : Clean and make this function work.
  //  Log ( "getEvent : eventsNumber=%d, canFill()=%d\n", eventsNumber, canFill() );
  if ( eventsNumber == 0 )
    {
      try
	{
	  parse ();
	}
      catch ( ParserException * e )
	{
	  if ( eventsNumber == 0 )
	    throw e;
	}
      catch ( ParserFullSpaceException * e )
	{
	  if ( eventsNumber == 0 )
	    throw e;
	}
    }
  AssertBug ( eventsNumber > 0, "Pipe still empty !\n" );
  AssertBug ( firstEvent != NULL, "eventsNumber > 0, but first event is NULL !\n" );
  if ( ! firstEvent->isFull() )
    {
      try
	{
	  parse ();
	}
      catch ( ParserException * e )
	{
	  if ( ! firstEvent->isFull() )
	    throw e;
	}
      catch ( ParserFullSpaceException * e )
	{
	  if ( ! firstEvent->isFull() )
	    {
	      logEvents ();
	      Fatal ( "Can not retrieve a full event !\n" ); 
	    }
	}
    }
  return firstEvent; 
}

bool DMLX::Parser::popEvent ()
{
  AssertBug ( eventsNumber > 0, "Empty Event Stack !\n" );
  AssertBug ( firstEvent != NULL, "First Event is NULL\n" );

  if ( firstEvent->isEnd() )
    {
      endEventsNumber--;
    }
  eventsNumber--;
  Event * nextEvent = firstEvent->next;
  storeSpace.free ( firstEvent );
  firstEvent = nextEvent;
  if ( firstEvent == NULL )
    lastEvent = NULL;
  return true;
}
bool DMLX::Parser::popEventEnd ()
{
  AssertBug ( firstEvent != NULL, "NULL firstEvent !\n" );
  AssertBug ( firstEvent->isEnd(), 
	      "this is not yet event end : currently on '%s'\n", 
	      firstEvent->name->getKeyword() );
  return popEvent ();
}

bool DMLX::Parser::popEventAndEnd ()
{ popEvent(); return popEventEnd (); }

char * DMLX::Parser::getEventName ()
{ return ( getEvent()->name->getKeyword() ); }

bool DMLX::Parser::isEventText ()
{ return getEvent()->isText(); }

bool DMLX::Parser::isEventEnd ( )
{ return getEvent()->isEnd(); }

bool DMLX::Parser::isEventName ( Keyword& keyword )
{ 
  AssertBug ( getEvent()->isMarkup()
	      || getEvent()->isEnd(), "Event is not Markup nor Markup End !\n" );
  return ( getEvent()->name->isEqual(&keyword) ); 
}

void DMLX::Parser::checkEventName ( Keyword& keyword )
{
  AssertBug ( isEventName ( keyword ), "Parser : checkEventName : Expected : '%s'. Had '%s'\n", 
	      keyword.getKeyword(), getEvent()->name->getKeyword() );
}

bool DMLX::Parser::isEventName ( const char * name )
{ Keyword keyword(name); return isEventName ( keyword ); }

void DMLX::Parser::checkEventName ( const char * name )
{ Keyword keyword(name); checkEventName ( keyword ); }


/*
 * Attributes
 * Note : addAttr() has been pushed to Parser-parse-ng.cpp
 */


bool DMLX::Parser::hasAttr ( Keyword& keyword )
{
  AssertBug ( getEvent()->isMarkup(), "Event '%s' is end, cannot add attributes.\n", 
	      getEvent()->name->getKeyword() );
  for ( Attr * attr = getEvent()->first;
	attr != NULL ; attr = attr->next )
    if ( attr->name->isEqual ( &keyword) ) 
      {
	AssertBug ( attr->value != NULL, "Argument value of '%s' is null !\n", keyword.getKeyword() );
	return true;
      }
  return false;
}

char * DMLX::Parser::getText ()
{
  AssertBug ( getEvent()->isText(), "Event is not text !\n" );
  return getEvent()->text;
}
char * DMLX::Parser::getAttr ( Keyword& keyword )
{
  if ( getEvent()->isEnd() == true )
    {
      Warn ( "Event '%s' is end, cannot get attribute '%s'\n", 
	     getEvent()->name->getKeyword(), keyword.getKeyword() );
      logEvents ();
      Bug ( "This is invalid.\n" );
    }
  for ( Attr * attr = getEvent()->first;
	attr != NULL ; attr = attr->next )
    if ( attr->name->isEqual ( &keyword ) ) 
      {
	AssertBug ( attr->value != NULL, "Argument value of '%s' is null !\n", keyword.getKeyword() );
	return attr->value;
      }
  logEvents();
  Bug ( "Could not get attribute named '%s' in event '%s'\n",
	keyword.getKeyword(), getEvent()->name->getKeyword() );
  return NULL;
}

int DMLX::Parser::getAttrInt ( Keyword& keyword )
{
  char * strValue = getAttr ( keyword );
  int value;
  if ( strValue [0] == '0' && strValue [1] == 'x' )
    sscanf ( strValue, "%p", (void **)&value );
  else
    value = atoi ( strValue );
  return value;
}

bool DMLX::Parser::getAttrBool ( Keyword& keyword )
{
  return ( strcmp ( getAttr (keyword), "true" ) == 0 );
}

float DMLX::Parser::getAttrFloat ( Keyword& keyword )
{
  return atof ( getAttr ( keyword ) );
}

bool DMLX::Parser::hasAttr ( const char * name )
{ Keyword keyword(name); return hasAttr ( keyword ); }

char* DMLX::Parser::getAttr ( const char * name )
{ Keyword keyword(name); return getAttr ( keyword ); }

int DMLX::Parser::getAttrInt ( const char * name )
{ Keyword keyword(name); return getAttrInt ( keyword ); }

bool DMLX::Parser::getAttrBool ( const char * name )
{ Keyword keyword(name); return getAttrBool ( keyword ); }

float DMLX::Parser::getAttrFloat ( const char * name )
{ Keyword keyword(name); return getAttrFloat ( keyword ); }

