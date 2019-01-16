#include <DMLXParser.h>
#include <DMLXBin.h>

#include "DMLXParser-StoreSpace-inline.h"


#undef Log
#define Log(...)


/*parseStateMarkupTextSymbol
 * DMLXBin Reader
 * Warn : We cannot assert that the buffer contain enougth !!
 */

#define curChar() (buffer[bufferIdx])
#define __DMLX_PARSER_PARSEBIN_CHECK_BUFFERIDX_
#ifdef __DMLX_PARSER_PARSEBIN_CHECK_BUFFERIDX_
#define nextChar()							\
  { AssertBug ( bufferIdx < bufferSz,					\
		"curChar is out of bounds idx=%d, sz=%d!\n",		\
		bufferIdx, bufferSz );					\
    bufferIdx++; parsing.totalParsed ++; }
#else
#define nextChar() { bufferIdx++; parsing.totalParsed++; }
#endif
#define isRemaining(__sz) (bufferIdx + __sz <= bufferSz)
#define skipBytes(__sz) { bufferIdx+=__sz; parsing.totalParsed+=__sz; AssertBug ( bufferIdx < bufferSz, "OutOfBounds"); }
#define gotoState(__state) { parsing.state = __state; break; }
#define gotoSameState() { continue; }
#define InvalidFlag() { Bug ( "State %s, Buffer(%d/%d) : Invalid Flag '0x%x' at offset %d (0x%x)\n", \
			      DMLXParserStatesStr[parsing.state], bufferIdx, bufferSz,	\
			      flag, parsing.totalParsed, parsing.totalParsed ); }
#define InvalidState() { Bug ( "Invalid State '%d'\n", parsing.state ); }

#define endStrName()							\
  { parsing.name = hashTree->getUnique ( hash ); }

#define newEvt()							\
  { Log ( "New Markup\n" );						\
    endStrName();							\
    if ( parsing.event != NULL )  parsing.event->setFull (true);	\
    parsing.event = newEvent (false);					\
    parsing.event->name = parsing.name;					\
    parsing.eventStack[parsing.eventStackLevel] = parsing.name;		\
    parsing.eventStackLevel++;						\
    AssertFatal ( parsing.eventStackLevel < parsing.eventStackMax,	\
		  "Too many levels in event stack : enlarge eventStackMax\n" ); \
    parsing.name = NULL;						\
    parsing.lastAttr = NULL; }


#define newEvtText(__s)							\
  { AssertBug ( parsing.eventStackLevel != 0,				\
		"Can not add text outside of a markup : shall have taken care of it before...\n" );  \
    Log ( "Last event '%p'\n", parsing.event );				\
    if ( parsing.event != NULL )  parsing.event->setFull (true);	\
    parsing.event = newEvent ( false );					\
    parsing.event->setText();						\
    parsing.lastAttr = NULL;						\
    parsing.event->name = NULL;						\
    parsing.event->text = __s;						\
    parsing.event->setFull (true); parsing.event = NULL; }


#define newEvtEnd()							\
  { Log ( "New Markup End\n" );						\
    Log ( "Last event '%p'\n", parsing.event );				\
    if ( parsing.event != NULL )  parsing.event->setFull (true);	\
    parsing.eventStackLevel--;						\
    parsing.event = newEvent ( true );					\
    parsing.event->setFull (true);					\
    parsing.event->name = parsing.eventStack[parsing.eventStackLevel];	\
    parsing.event = NULL; parsing.name = NULL;				\
    parsing.lastAttr = NULL; }

extern char * DMLXParserStatesStr[];


void DMLX::Parser::parseBufferBin () throw (DMLX::ParserException*, DMLX::ParserFullSpaceException*)
{
  unsigned int nbFills = 0; // Number of times we run fillBuffer() here.
  Log ( "Beginning of parse : idx=%d, sz=%d\n", bufferIdx, bufferSz );
  DMLXBinFlag flag = 0;
  DMLXBinHash hash = 0;
  DMLXBinTextLength ln = 0;
  char * tempBuff;
  while ( true )
    {
      Log ( "Step : state=%s, idx=%d, sz=%d, char='0x%x', avail='%d'\n",
      	    DMLXParserStatesStr[parsing.state], bufferIdx, bufferSz,
      	    curChar() & 0xff, storeSpace.avail() );

      if ( maxEndEventsInStoreSpace > 0 && ( endEventsNumber == maxEndEventsInStoreSpace ) )
	{
	  return;
	}
      if ( storeSpace.avail() < 100 )
	{
	  Log ( "Prevention : storeSpace is gonna be full !\n" );
	  throw new ParserFullSpaceException ();
	}
      if ( bufferSz == 0 || (bufferIdx == bufferSz) )
	{
	  if ( parseCanFillBufferOnce && ( nbFills > 0 ) )
	    throw new ParserException ( "Could not fill : option parseCanFillBufferOnce is set.\n" );
	  if ( ! canFill() )
	    {
	      Warn ( "Can not fill : canFill() returned false (%d events in queue).\n", eventsNumber );
	      throw new ParserException ( "Could not fill : canFill() returned false\n" );
	    }
	  Log ( "Buffer parsed, reloading (%d reloads).\n", nbFills );
	  bufferIdx = 0;
	  bufferSz = fillBuffer ( buffer, bufferMax );
	  nbFills++;
	  if ( bufferSz == 0 )
	    throw new ParserException ( "Could not fill : canFill() returned true, but fillBuffer returned 0\n" );
	}
      switch ( parsing.state )
	{
	case parseStateMarkupOutside:
	  flag = curChar(); nextChar(); 
	  if ( flag == DMLXBinFlag_Markup )
	    { 
	      parsing.valueFlag = DMLXBinFlag_Markup;
	      parsing.nameBufferSz = 0; gotoState ( parseStateMarkupName ); 
	    }
	  if ( flag == DMLXBinFlag_MarkupEnd )
	    { 
	      newEvtEnd (); gotoSameState ();
	    }
	  if ( flag == DMLXBinFlag_Text )
	    {
	      parsing.symbolSz = 0; parsing.valueFlag = 0;
	      gotoState ( parseStateMarkupText );
	    }
	  if ( flag == DMLXBinFlag_Keyword )
	    {
	      parsing.symbolSz = 0; parsing.valueFlag = DMLXBinFlag_Keyword;
	      parsing.keywordInMarkup = 0;
	      gotoState ( parseStateMarkupName );
	    }
	  InvalidFlag();

	case parseStateMarkupText:
	  if ( isRemaining ( DMLXBinTextLengthSz ) && parsing.symbolSz == 0 )
	    {
	      memcpy ( &ln, &(buffer[bufferIdx]), DMLXBinTextLengthSz );
	      skipBytes ( DMLXBinTextLengthSz );
	    }
	  else
	    {
	      parsing.symbol[parsing.symbolSz] = curChar ();
	      parsing.symbolSz++; nextChar ();
	      if ( parsing.symbolSz < DMLXBinTextLengthSz )
		gotoSameState ();
	      AssertBug ( parsing.symbolSz == DMLXBinTextLengthSz, "Read too much for text length..\n" );
	      memcpy ( &ln, parsing.symbol, DMLXBinTextLengthSz );
	    }
	  Log ( "Reading : '%d' bytes for text\n", ln );
	  parsing.reqLength = ln;
	  parsing.symbolSz = 0;
	  parsing.text = (char*) storeSpace.alloc ();
	  gotoState ( parseStateMarkupTextSymbol );

	case parseStateMarkupTextSymbol:
	  if ( isRemaining ( parsing.reqLength ) && parsing.symbolSz == 0 )
	    {
	      parsing.text = (char*)storeSpace.alloc ( parsing.reqLength+1 );
	      memcpy ( parsing.text, &(buffer[bufferIdx]), parsing.reqLength );
	      parsing.text[parsing.reqLength] = '\0';
	      skipBytes ( parsing.reqLength );
	    }
	  else
	    {
	      storeSpace.extend( &(parsing.text), curChar() );
	      parsing.symbolSz++; nextChar ();
	      if ( parsing.symbolSz < parsing.reqLength )
		gotoSameState ();
	      AssertBug ( parsing.symbolSz ==  parsing.reqLength, "Read too much for text length..\n" );
	      storeSpace.extend ( &parsing.text, '\0' );
	      parsing.symbolSz = 0;
	    }
	  if ( parsing.valueFlag == 0 )
	    {
	      // Insert a new text item
	      Log ( "New text section '%s'\n", parsing.text );
	      newEvtText ( parsing.text );
	      gotoState ( parseStateMarkupOutside );
	    }
	  if ( parsing.valueFlag == DMLXBinFlag_Keyword )
	    {
	      Log ( "New Keyword '%s' (hash '0x%x')\n",
		    parsing.text, parsing.hash );
	      Keyword * keyword = hashTree->getUnique ( parsing.text );
	      AssertFatal ( keyword->getHash() == parsing.hash, "Given Keyword has an invalid hash !\n" );
	      if ( parsing.keywordInMarkup )
		{ gotoState ( parseStateMarkupInside ); }
	      gotoState ( parseStateMarkupOutside );
	    }
	  // Insert as attribute
	  Log ( "New attribute text '%s'\n", parsing.text );
	  parsing.lastAttr = addAttr ( parsing.event, parsing.lastAttr, parsing.name, parsing.text );
	  gotoState ( parseStateMarkupInside );

	case parseStateMarkupName:
	  if ( isRemaining ( DMLXBinHashSz ) && parsing.nameBufferSz == 0 )
	    {
	      memcpy ( &hash, &(buffer[bufferIdx]), DMLXBinHashSz );
	      skipBytes ( DMLXBinHashSz );
	    }
	  else
	    {
	      parsing.nameBuffer[parsing.nameBufferSz] = curChar();
	      parsing.nameBufferSz++; nextChar ();
	      if ( parsing.nameBufferSz < DMLXBinHashSz )
		{
		  gotoSameState();
		}
	      AssertBug ( parsing.nameBufferSz == DMLXBinHashSz, "Read too much for hash !\n" );
	      memcpy ( &hash, parsing.nameBuffer, DMLXBinHashSz );
	    }
	  Log ( "Got hash '0x%x' (valueFlag=%d)\n", hash, parsing.valueFlag );
	  if ( parsing.valueFlag == DMLXBinFlag_Keyword )
	    { parsing.hash = hash; gotoState ( parseStateMarkupText ); }
	  AssertBug ( valueFlag == DMLXBinFlag_Markup, "Invalid valueFlag.\n" );
	  newEvt ();
	  gotoState ( parseStateMarkupInside );

	case parseStateMarkupInside:
	  flag = curChar(); nextChar();
	  if ( flag == DMLXBinFlag_Markup )
	    {
	      parsing.nameBufferSz = 0; 
	      parsing.valueFlag = DMLXBinFlag_Markup;
	      gotoState ( parseStateMarkupName ); 
	    }
	  if ( flag == DMLXBinFlag_MarkupEnd )
	    {
	      newEvtEnd ();
	      gotoState (parseStateMarkupOutside);
	    }
	  if ( flag == DMLXBinFlag_Text )
	    {
	      Log ( "Found Text Section\n" );
	      parsing.symbolSz = 0; parsing.valueFlag = 0;
	      gotoState (parseStateMarkupText);
	    }
	  if ( flag == DMLXBinFlag_Attribute )
	    {
	      Log ( "Found Attribute\n" );
	      parsing.nameBufferSz = 0;
	      gotoState (parseStateMarkupAttrName);
	    }	  
	  if ( flag == DMLXBinFlag_Keyword )
	    {
	      Log ( "Found Keyword\n" );
	      parsing.symbolSz = 0; parsing.valueFlag = DMLXBinFlag_Keyword;
	      parsing.keywordInMarkup = true;
	      gotoState ( parseStateMarkupName );
	    }
	  InvalidFlag ();

	case parseStateMarkupAttrName:
	  if ( isRemaining ( DMLXBinHashSz ) && parsing.nameBufferSz == 0 )
	    {
	      memcpy ( &hash, &(buffer[bufferIdx]), DMLXBinHashSz );
	      skipBytes ( DMLXBinHashSz );
	    }
	  else
	    {
	      parsing.nameBuffer[parsing.nameBufferSz] = curChar();
	      parsing.nameBufferSz++; nextChar ();
	      if ( parsing.nameBufferSz < DMLXBinHashSz )
		{
		  gotoSameState();
		}
	      AssertBug ( parsing.nameBufferSz == DMLXBinHashSz, 
			  "Read too much for hash ! (%d of %d)\n",
			  parsing.nameBufferSz, DMLXBinHashSz );
	      memcpy ( &hash, parsing.nameBuffer, DMLXBinHashSz );
	    }
	  Log ( "Got attr hash '0x%x'\n", hash );
	  parsing.name = hashTree->getUnique ( hash ); 
	  gotoState ( parseStateMarkupAttrValue );

	case parseStateMarkupAttrValue:
	  flag = curChar (); nextChar ();
	  parsing.valueFlag = flag;
	  if ( flag == DMLXBinFlag_Text )
	    {
	      parsing.symbolSz = 0;
	      gotoState ( parseStateMarkupText );
	    }
	  if ( flag == DMLXBinFlag_Integer )
	    parsing.reqLength = DMLXBinIntegerSz;
	  else if ( flag == DMLXBinFlag_Hex )
	    parsing.reqLength = DMLXBinHexSz;
	  else if ( flag == DMLXBinFlag_Float )
	    parsing.reqLength = DMLXBinFloatSz;
	  else
	    { InvalidFlag(); }
	  parsing.symbolSz = 0;
	  gotoState ( parseStateMarkupAttrValueSymbol );

	case parseStateMarkupAttrValueSymbol:
	  if ( isRemaining ( parsing.reqLength ) && parsing.symbolSz == 0 )
	    {
	      tempBuff = &(buffer[bufferIdx]);
	      skipBytes ( parsing.reqLength );
	    }
	  else
	    {
	      parsing.symbol[parsing.symbolSz] = curChar();
	      parsing.symbolSz++;
	      nextChar ();
	      if ( parsing.symbolSz < parsing.reqLength )
		{ gotoSameState(); }
	      AssertBug ( parsing.symbolSz == parsing.reqLength, "Read too much for symbol.\n" );
	      flag = parsing.valueFlag;
	      tempBuff = parsing.symbol;
	    }
	  int i;
	  float f;
	  char * value;
	  value = (char*) storeSpace.alloc ( 20 );
	  if ( flag == DMLXBinFlag_Integer )
	    { memcpy ( &i, tempBuff, DMLXBinIntegerSz ); sprintf ( value, "%d", i ); }
	  else if ( flag == DMLXBinFlag_Hex )
	    { memcpy ( &i, tempBuff, DMLXBinHexSz ); sprintf ( value, "0x%x", i ); }
	  else if ( flag == DMLXBinFlag_Float )
	    { memcpy ( &f, tempBuff, DMLXBinFloatSz ); sprintf ( value, "%f", f ); }
	  else
	    { InvalidFlag (); }
	  Log ( "Got attr '%s'='%s'\n", parsing.name->getKeyword(), value );
	  parsing.lastAttr = addAttr ( parsing.event, parsing.lastAttr, parsing.name, value );
	  gotoState (parseStateMarkupInside);
	case parseStateMarkupEnd:             InvalidState();
	case parseStateMarkupEndName:         InvalidState();
	case parseStateXMLSignature:          InvalidState();	  
	case parseStateMarkupAttrBeforeEqual: InvalidState();
	case parseStateMarkupAttrEqual:       InvalidState();
	case parseStateMarkupEnter:       InvalidState();
	}
    }


}
