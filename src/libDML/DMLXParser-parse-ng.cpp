#include <DMLXParser.h>
#include <string.h>
#include <stdlib.h>

// #define __DMLXPARSER_PARSE_NG_LOG_STEPS
#define __DMLXPARSER_NAMESPACE_VERBOUS_ASSERTFULL

#if 1
#undef Log
#define Log(...)
#endif
#include "DMLXParser-StoreSpace-inline.h"

char * DMLXParserStatesStr[] = 
  { "Oustide", "Text", "TextSymbol", "Enter", "Name", "Inside", "AttrName",
    "AttrBeforeEqual", "AttrEqual", "AttrValue", "AttrValueSymbol", 
    "End", "EndName", "XMLSignature" };

/*
 * InvalidInput() logs
 * InvalidInputLogState is complex because we cannot touch the read buffer
 * In the other hand, we must take care that we do not explode the maximum Glog message size.
 */
#define InvalidInputLogState_MaxBufferDumpSize (__GLOG_GLOGITEM_MAXTEXTSIZE - 10)
#define InvalidInputLogState()						\
  { Warn ( "At line %d : char %d:'%c', state=%s, "			\
	   "(char %d of buffer size %d)\n",				\
	   parsing.totalLinesParsed, curChar(), curChar(),		\
	   DMLXParserStatesStr[parsing.state],				\
	   bufferIdx, bufferSz );					\
    Warn ( "front=%d, first=%d, avail='%d'\n",				\
	   storeSpace.getFront(), storeSpace.getFirst(),			\
	   storeSpace.avail() );						\
    char _dumpBuff[InvalidInputLogState_MaxBufferDumpSize];		\
    unsigned int _dumpBuffStart, _dumpBuffSize;				\
    _dumpBuffStart = bufferIdx - (InvalidInputLogState_MaxBufferDumpSize / 2); \
    _dumpBuffSize = bufferSz - _dumpBuffStart;				\
    if ( _dumpBuffSize > InvalidInputLogState_MaxBufferDumpSize )	\
      _dumpBuffSize = InvalidInputLogState_MaxBufferDumpSize;		\
    memcpy ( _dumpBuff, &(buffer[_dumpBuffStart]), _dumpBuffSize );	\
    _dumpBuff[_dumpBuffSize-1] = '\0';					\
    Warn ( "Buffer : '%s'\n", _dumpBuff ); }

#define InvalidInput(...)					\
  { Warn ( "Invalid Input : "/**/__VA_ARGS__ );			\
    InvalidInputLogState() ;					\
    char * exceptionMessage = (char*) malloc ( 64 );		\
    snprintf ( exceptionMessage, 64, __VA_ARGS__ );		\
    throw new ParserException ( exceptionMessage, true );	\
  }

/*
 * Transition between states
 * This shall be the last statement of a case:
 */
#define gotoState(__state) parsing.state = __state; goto case_begin;
#define gotoSameState() goto case_begin;
/*
 * Access to current Character
 */
#define curChar() (buffer[bufferIdx])
#define nextChar()							\
  { AssertBug ( bufferIdx < bufferSz,					\
		"curChar is out of bounds idx=%d, sz=%d!\n",		\
		bufferIdx, bufferSz );					\
    if ( isCur ( '\n' ) ) parsing.totalLinesParsed++;			\
    bufferIdx++; parsing.totalParsed ++; }
#define isText() ( ( curChar() >= 'A' && curChar() <= 'Z' )		\
		   || ( curChar() >= 'a' && curChar() <= 'z' )		\
		   || ( curChar() >= '0' && curChar() <= '9' )		\
		   || ( curChar() == ':' ) || ( curChar() == '-' )	\
		   || ( curChar() == '.' ) || ( curChar() == '_' ) )

#define isCur(__c) ( curChar() == __c )

/*
 * StoreSpace Cyclic buffer : 
 * Grow strings char-by-char for values and text between markup
 * Optimized version of the StoreSpace::extend() function
 */ 

#define __extend__(__s,__c) /* Macro Binding to StoreSpace::extend(char**,char)*/ \
  storeSpace.extend(__s,__c)

#define extendStr(__s) __extend__ ( &(__s), curChar() ); nextChar ();
#define endStr(__s) __extend__ ( &(__s), '\0' );

#define newStrName() {parsing.nameBufferSz = 0; ((int*)parsing.nameBuffer)[0] = 0;}
#define extendStrName() {parsing.nameBuffer[parsing.nameBufferSz++] = curChar(); nextChar();}
#define endStrName()							\
  { parsing.nameBuffer[parsing.nameBufferSz] = '\0';			\
    parsing.nameBufferSz = 0;						\
    parsing.name = hashTree->getUnique(parsing.nameBuffer); }

#define newEvt() {							\
    parsing.event = newEvent ( false );					\
    endStrName();							\
    parsing.event->name = parsing.name;					\
    parsing.eventStack[parsing.eventStackLevel] = parsing.name;		\
    parsing.eventStackLevel++;						\
    AssertFatal ( parsing.eventStackLevel < parsing.eventStackMax,	\
		  "Too many levels in event stack : enlarge eventStackMax\n" ); \
    parsing.name = NULL;						\
    parsing.lastAttr = NULL; }
#define newEvtText(__s) {						\
    AssertBug ( parsing.eventStackLevel != 0,				\
		"Can not add text outside of a markup : shall have taken care of it before...\n" ); \
    parsing.event = newEvent ( false );					\
    parsing.event->setText();						\
    parsing.lastAttr = NULL;						\
    parsing.event->name = NULL;						\
    parsing.event->text = __s; }
#define newEvtEnd() { /* Here we assert parsing.name is provided */	\
    AssertFatal ( parsing.eventStackLevel != 0,				\
		  "Can not close event : eventStack is empty\n" );	\
    parsing.eventStackLevel--;						\
    AssertFatal ( parsing.eventStack[parsing.eventStackLevel]->isEqual ( parsing.name ), \
		  "Invalid Markup end '%s' : shall close '%s'\n",	\
		  parsing.name->getKeyword(), parsing.eventStack[parsing.eventStackLevel]->getKeyword() ); \
    parsing.event = newEvent ( true );					\
    parsing.event->name = parsing.name;					\
    parsing.name = NULL;						\
    parsing.lastAttr = NULL; }
#define newEvtEndWithName() {					\
    endStrName();						\
    newEvtEnd(); }
#define finishEvt() { parsing.event->setFull (true); }

/*
 * Symbol resolution : will write result to __s
 * Be carefull, __s can change because of the __extend__ mechanism.
 * Notes : writeSymbol() consumes the ';' character that triggered its call.
 */
#define writeSymbol(__s)						\
  { parsing.symbol[parsing.symbolSz] = '\0';				\
    int symb;								\
    for ( symb = 0 ; parsing.symbolLong[symb] != NULL ; symb++ )	\
      {									\
	if ( strcmp ( parsing.symbolLong[symb], parsing.symbol ) == 0 )	\
	  {								\
	    /* Can't do it OneShot, because of __extend__...		\
	       So we have to perform copy char-by-char */		\
	    for ( char * c = parsing.symbolShort[symb] ;		\
		  *c != '\0' ; c++ )					\
	      __extend__ ( &(__s), *c );				\
	    break;							\
	  }								\
      }									\
    if ( parsing.symbolLong[symb] == NULL )				\
      { InvalidInput ( "Unknown symbol : '%s'\n", parsing.symbol ); }	\
    parsing.symbolSz = 0;						\
    nextChar(); }

/*
 * The Parsing Function.
 * It is designed to be re-entrant, id est to resume exactly
 * where it has exited before with a throw of DMLXParserException or DMLXParserFullSpaceException
 */
void DMLX::Parser::parseBufferXML () throw (DMLX::ParserException*, DMLX::ParserFullSpaceException*)
{
  static void * caseParseLabels[] = { &&case_parseStateMarkupOutside,
				      &&case_parseStateMarkupText,
				      &&case_parseStateMarkupTextSymbol,
				      &&case_parseStateMarkupEnter,
				      &&case_parseStateMarkupName,
				      &&case_parseStateMarkupInside,
				      &&case_parseStateMarkupAttrName,
				      &&case_parseStateMarkupAttrBeforeEqual,
				      &&case_parseStateMarkupAttrEqual,
				      &&case_parseStateMarkupAttrValue,
				      &&case_parseStateMarkupAttrValueSymbol,
				      &&case_parseStateMarkupEnd,
				      &&case_parseStateMarkupEndName,
				      &&case_parseStateXMLSignature };
  unsigned int nbFills = 0; // Number of times we run fillBuffer() here.
  Log ( "Beginning of parse : idx=%d, sz=%d\n", bufferIdx, bufferSz );
 case_begin:
  if ( maxEndEventsInStoreSpace > 0 ) 
    {
      AssertBug ( endEventsNumber <= maxEndEventsInStoreSpace,
		  "Parsed too much end events than expected : "
		  "%d parsed, limited to %d\n",
		  endEventsNumber, maxEndEventsInStoreSpace );
      if ( endEventsNumber == maxEndEventsInStoreSpace )
	{
	  Log ( "Read %d end events (max=%d)\n",
		endEventsNumber, maxEndEventsInStoreSpace );
	  goto case_end;
	}
    }
  if ( bufferSz == 0 || (bufferIdx == bufferSz) )
    {
      if ( parseCanFillBufferOnce && ( nbFills > 0 ) )
	throw new ParserException ( "Could not fill : option parseCanFillBufferOnce is set.\n" );
      if ( ! canFill() )
	{
	  if ( nbFills == 0 )
	    throw new ParserException ( "Could not fill : canFill() returned false\n" );
	  else
	    {
	      Log ( "parse end ? canFill() == false.\n" );
	      goto case_end;
	    }
	}
      Log ( "Buffer parsed, reloading.\n" );
      bufferIdx = 0;
      bufferSz = fillBuffer ( buffer, bufferMax );
      nbFills++;
      if ( bufferSz == 0 )
	throw new ParserException ( "Could not fill : canFill() returned true, but fillBuffer returned 0\n" );
    }
#ifdef __DMLXPARSER_PARSE_NG_LOG_STEPS
  Log ( "Step : state=%s, idx=%d, sz=%d, char=%d:'%c', avail='%d'\n",
	DMLXParserStatesStr[parsing.state], bufferIdx, bufferSz,
	curChar(), curChar (), storeSpace.avail() );
#endif
  goto *caseParseLabels[parsing.state];
 case_parseStateMarkupOutside:
  if ( isCur ( '<' ) )
    { nextChar(); gotoState ( parseStateMarkupEnter ); }
  // #ifdef __DMLXPARSER_RESTRICT_TEXT
  if ( ! parseKeepAllText )
    {
      if ( isCur ( ' ' ) || isCur ( '\n' ) || isCur ( '\t' ) || isCur ( 13 ) || isCur ( 10 ) )
	{ nextChar(); gotoSameState(); }
    }
  // #endif // __DMLXPARSER_RESTRICT_TEXT
  //   if ( isCur ( '>' ) )
  //    { InvalidInput ( "Outside text can not contain '>'\n" ); }
  if ( parsing.eventStackLevel == 0 )
#ifdef __DMLXPARSER_RESTRICT_TEXT
    InvalidInput ( "Providing text but not in an event !\n" );
#else
    { nextChar(); gotoSameState(); }
#endif
  parsing.text = (char*) storeSpace.alloc ();
  gotoState ( parseStateMarkupText );
  
 case_parseStateMarkupText:
  if ( isCur ( '&' ) )
    { nextChar(); gotoState ( parseStateMarkupTextSymbol ); }
  if ( isCur ( '<' ) )
    { 
      endStr ( parsing.text );
      newEvtText ( parsing.text );
      parsing.text = NULL;
      finishEvt ();
      nextChar(); gotoState ( parseStateMarkupEnter ); 
    }
  extendStr ( parsing.text );
  gotoSameState();
  
 case_parseStateMarkupTextSymbol:
  if ( isCur ( ';' ) )
    { writeSymbol ( parsing.text );
      gotoState ( parseStateMarkupText ); }
  if ( parsing.symbolSz == 64 )
    { InvalidInput ( "Symbol too long !\n" ); }
  parsing.symbol[parsing.symbolSz] = curChar();
  nextChar(); parsing.symbolSz++;
  gotoSameState();
  
 case_parseStateMarkupEnter:
  if ( isText() )
    { if ( parsing.text != NULL )
	InvalidInput ( "Entering a markup with text before !\n" );
      newStrName ( );
      gotoState ( parseStateMarkupName ); }
  if ( isCur ( '/' ) )
    { nextChar ();
      newStrName ( );
      gotoState ( parseStateMarkupEndName ); }
  if ( isCur ( '?' ) || isCur ( '!') )
    { gotoState ( parseStateXMLSignature ); }
  InvalidInput ( "Invalid character at beginning of markup : '%c'\n", curChar() );
  
 case_parseStateMarkupName:
  if ( isCur (' ') )
    { newEvt(); nextChar ();
      gotoState ( parseStateMarkupInside ); }
  if ( isCur ('>') )
    { newEvt(); finishEvt();
      nextChar (); gotoState ( parseStateMarkupOutside ); }
  if ( isCur ('/') )
    { newEvt();
      gotoState ( parseStateMarkupInside ); }
  if ( isText () )
    { extendStrName(); gotoSameState(); }
  InvalidInput ( "Markup names can only contain alphanumeric characters "
		 "(and char '%c' is not)\n",
		 curChar() );
  
 case_parseStateMarkupInside:
  if ( isText() )
    { newStrName ( );
      gotoState ( parseStateMarkupAttrName ); }
  if ( isCur ( '/' ) )
    { nextChar ();
      gotoState ( parseStateMarkupEnd ); }
  if ( isCur ( '>' ) )
    { nextChar (); finishEvt();
      gotoState ( parseStateMarkupOutside ); }
  if ( isCur ( ' ' ) || isCur ( '\n') || isCur ( 13 ) )
    { nextChar (); gotoSameState(); }
  InvalidInput ( "Invalid character inside of the markup : '%c'\n", curChar() );
  
 case_parseStateMarkupAttrName:
  if ( isText () )
    { extendStrName (); gotoSameState(); }
  if ( isCur ( '=' ) )
    { endStrName ( ); nextChar ();
      gotoState ( parseStateMarkupAttrEqual ); }
  if ( isCur ( ' ' ) )
    { endStrName ( ); nextChar ();
      gotoState ( parseStateMarkupAttrBeforeEqual ); }
  InvalidInput ( "Markup attribute names can only contain alphanumeric characters "
		 "(and char '%c' is not)\n",
		 curChar() );
  
 case_parseStateMarkupAttrBeforeEqual:
  if ( isCur ( '=' ) )
    { nextChar ();
      gotoState ( parseStateMarkupAttrEqual ); }
  if ( isCur ( ' ' ) || isCur ( '\n' ) )
    { nextChar (); gotoSameState(); }
  InvalidInput ( "Invalid character before attribute name and equal sign : '%c'\n", curChar() );
  
 case_parseStateMarkupAttrEqual:
  if ( isCur ( '"' ) || isCur ( '\'' ) )
    { nextChar ();
      parsing.value = (char*) storeSpace.alloc ();
      gotoState ( parseStateMarkupAttrValue ); }
  if ( isCur ( ' ' ) )
    { nextChar (); gotoSameState(); }
  InvalidInput ( "Invalid character after equal sign : '%c'\n", curChar() );
  
 case_parseStateMarkupAttrValue:
  if ( isCur ( '"' ) || isCur ( '\'' ) )
    { endStr( parsing.value );
      parsing.lastAttr = addAttr ( parsing.event, parsing.lastAttr, parsing.name, parsing.value );
      nextChar ();
      gotoState ( parseStateMarkupInside ); }
  if ( isCur ( '&' ) )
    { nextChar ();
      gotoState ( parseStateMarkupAttrValueSymbol ); }
  extendStr(parsing.value);
  gotoSameState();
  
 case_parseStateMarkupAttrValueSymbol:
  if ( isCur ( ';' ) )
    { writeSymbol ( parsing.value );
      gotoState ( parseStateMarkupAttrValue ); }
  if ( parsing.symbolSz == 64 )
    { InvalidInput ( "Symbol too long !\n" ); }
  parsing.symbol[parsing.symbolSz] = curChar();
  nextChar(); parsing.symbolSz++;
  gotoSameState();
  
 case_parseStateMarkupEnd:
  if ( isCur ('>' ) )
    { finishEvt();
      parsing.name = parsing.event->name;
      newEvtEnd();
      finishEvt();
      nextChar ();
      parsing.event = NULL;
      parsing.text = NULL;
      gotoState ( parseStateMarkupOutside ); }
  InvalidInput ( "Invalid character for Markup end : '%c'\n",
		 curChar () );
  
 case_parseStateMarkupEndName:
  if ( isCur ('>') )
    { newEvtEndWithName();
      finishEvt();
      nextChar ();
      parsing.event = NULL;
      parsing.text = NULL;
      gotoState ( parseStateMarkupOutside ); }
  if ( isText () )
    { extendStrName ();
      gotoSameState();
    }
  InvalidInput ( "Invalid character at markup end : '%c'\n", curChar() );
      
 case_parseStateXMLSignature:
  if ( isCur ('>' ) )
    {
      nextChar();
      gotoState ( parseStateMarkupOutside );
    }
  nextChar();
  gotoSameState();
  
 case_end:
  Log ( "Parse Finished. Total parsed '%d' bytes, StoreSpace total '%d' bytes, "
	"events '%d', endEvents '%d'\n",
	parsing.totalParsed, storeSpace.getMax() - storeSpace.avail(),
	eventsNumber, endEventsNumber );
  //  logEvents();
}

void DMLX::Parser::parse () throw (DMLX::ParserException*, DMLX::ParserFullSpaceException*)
// TO BE DEPRECATED : that is just a stub
{
  try
    {
      if ( parseBin )
	parseBufferBin ();
      else
	parseBufferXML ();
    }
  catch ( ParserException * e )
    {
      Log ( "DMLXParserException : '%s' isFatal : '%d'\n",
	     e->getMessage (), e->isFatal() );
      Log ( "Total parsed '%d', StoreSpace total '%d'\n",
	    parsing.totalParsed, storeSpace.getMax() - storeSpace.avail() );
      //      logEvents();
      throw e;
    }
  catch ( ParserFullSpaceException * e )
    {
      Log ( "Full Space Buffer.\n" );
      throw e;
    }
}

// #define __DMLXPARSER_PARSE_NG_TEST__
#ifdef __DMLXPARSER_PARSE_NG_TEST__
#include <glogers.h>
setGlog ( "DMLXParser-parse-ng" );

class DMLXParserText : public DMLXParser
{
  char ** text;
  unsigned int tx;
public:
  DMLXParserText ( char ** _text )
  { text = _text; tx = 0; }
  bool canFill () { if ( text[tx] == NULL ) return false; return true; }
  unsigned int fillBuffer ( void * buff, int max_size )
  {
    if ( text[tx] == NULL ) return 0;
    int ln = strlen ( text[tx] );
    if ( max_size < ln )
      { Fatal ( "DMLXParserText : Too small window" ); }
    memcpy ( buff, text[tx], ln );
    Log ( "DMLXParserText : provide '%s'\n", text[tx] );
    tx++;
    return ln;
  }
};

int main ( int argc, char ** argv )
{
  addGlog ( new GlogInfo_File ( stdout ) );
  Log ( "Event has size '%d', Attribute has size '%d'\n",
	sizeof ( DMLXParser::DMLXEvent ), sizeof ( DMLXParser::DMLXAttr ) );

  /*
  char * text[] =
    { "<te", "st valu","e=\"toto\"",">\n",
      "<markup attr=\"&lt;value&gt;\" truc=\"&quot;machin&","quot",";\">\"&quot;TOTO&quot;\"</markup>",
      "</test>", NULL };
  */
  char * text[] =
    { "<test>plouf<machin/>toto<machine param=\"truc\">hello</machine>tritruc", " pouet&amp;toto</test>", NULL };
  DMLXParser * parser = new DMLXParserText ( text );
  
  //  DMLXParser * parser = new DMLXParserFile ( "test.xml" );
  while ( true )
    {
      try
	{
	  parser->parse ();
	  /*
	  if ( ! parser->parse () )
	    {
	      Log ( "Enlarge Buffer !\n" );
	    }
	  */
	  Log ( "---------------\n" );
	}
      catch ( DMLXParserFullSpaceException * e )
	{
	  Log ( "Full Exception !\n" );
	  parser->popEvent();
	}
      catch ( DMLXParserException * e)
	{
	  //	  parser->logEvents();
	  Warn ( "Exception : '%s'\n", e->getMessage() );
	  parser->logKeywords();
	  parser->logEvents();
	  delete ( parser );
	  exit (-1);
	  throw e;
	  break;
	}
    }
  return 0;
}
#endif // __DMLXPARSER_PARSE_NG_TEST__
