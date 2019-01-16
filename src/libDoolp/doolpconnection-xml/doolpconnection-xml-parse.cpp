#warning DEPRECATED
#if 0

#include <doolp/doolp-doolpconnection-xml.h>


typedef enum parseState
  {
    parseOutside,
    parseBody,
    parseText
  };

#define seekTo(c) while ( currentChar() != c )				\
    { __DOOLPXML_Log ( "seekTo'%c' : %d\n", c, readBufferIdx ); nextChar() ; }
#define seekToOrEnd(c) while ( currentChar() != c && ! isFinished () ) { nextChar(); }

#define currentChar() readBuffer[readBufferIdx]

#define nextChar()							\
  {									\
    readBufferIdx++;							\
    AssertBug ( readBufferIdx <= readBufferSz, "Getting out of bounds !\n" ); \
    if ( readBufferIdx == readBufferSz )				\
      {									\
	__DOOLPXML_Log ( "in nextChar : trying to fillBuffer ?\n" );	\
	fillBuffer ();							\
      }									\
  }
#define fillBuffer()							\
  readBufferIdx = 0;							\
  readBufferSz = 0;							\
  if ( level != 0 )							\
    {									\
      __DOOLPXML_Log ( "Try to reload !\n" );				\
      readBufferSz = (unsigned int) rawRead ( readBuffer, readBufferMax ); \
      if ( readBufferSz == 0 )						\
	finished = true;						\
      else								\
	{								\
	  readBuffer[readBufferSz] = '\0';				\
	  __DOOLPXML_Log ( "%d bytes to parse\n", readBufferSz );	\
	  __DOOLPXML_Log ( "\n%s\n", readBuffer );			\
	}								\
    }									\
  else									\
    {									\
      finished = true;							\
    }									\
  
#define isFinished() ( finished )

#define getWord(s,stopAt)						\
  {									\
    while ( currentChar() == ' ' )					\
      { nextChar(); continue; }						\
    s = readSpaceGet ();						\
    if ( currentChar() == '"' )						\
      {									\
	nextChar();							\
	while ( currentChar() != '"' )					\
	  { readSpaceExtend ( &s, currentChar () ); nextChar();  }	\
	nextChar();							\
      }									\
    for ( int i = 0 ; currentChar() != stopAt[i] ; i++ )		\
      {									\
	if ( stopAt[i+1] == '\0' )					\
	  { readSpaceExtend ( &s, currentChar () ); nextChar();		\
	    i = -1; }							\
      }									\
    readSpaceExtend ( &s, '\0' );					\
  }
bool DoolpConnectionXML::parseInput ( )
{
  __DOOLPXML_Log ( "eventsNumber=%d, readBufferIdx=%d, readBufferSz=%d\n",
		eventsNumber, readBufferIdx, readBufferSz );
  bool finished = false;
  unsigned int level = 1;
  parseState state = parseOutside;
  DoolpConnectionXMLEvent * event = NULL;
  bool justCreated = false;
  char * endName;
  unsigned int _fullEvents = 0;

  if ( eventsNumber > 0 )
    {
      __DOOLPXML_Log ( "There are already events in the pipe.\n" );
      return true;
    }
  if ( readBufferSz == 0 )
    {
      fillBuffer ();
    }
  if ( readBufferSz == 0 )
    return false; 

  level = 0;

  while ( ! isFinished () )
    {
      __DOOLPXML_Log ( "** readBuffer(Idx=%d/Sz=%d), state=%d\n", 
		       readBufferIdx, readBufferSz, state );
      switch ( state )
	{
	case parseOutside:
	  seekToOrEnd ( '<' );
	  if ( isFinished () )
	    {
	      break;
	    }
	  nextChar();
	  state = parseBody;
	  continue;
	case parseBody:
	  justCreated = false;
	  if ( currentChar() != '/' )
	    {
	      event = newEvent ( false );
	      level++;
	      getWord ( event->name, " />" );
	      __DOOLPXML_Log ( "new event name='%s' (at %p)\n",
			    event->name, event->name );
	      justCreated = true;
	      while ( currentChar() != '>'
		      && currentChar() != '/' )
		{
		  while ( currentChar() == ' ' )		
		    { nextChar(); continue; }
		  char * _name; char * _value;
		  getWord ( _name, " =" );
		  seekTo ( '=' );
		  nextChar();
		  getWord ( _value, " />" );
		  addAttr ( event, _name, _value );
		  while ( currentChar() == ' ' )		
		    { nextChar(); continue; }
		}
	      event->readSpaceEnd = readSpaceCurrent ();
	    }
	  if ( currentChar() != '/' )
	    {
	      AssertBug ( justCreated, "Internal bug, shall have a '/' here.\n" );
	      AssertBug ( currentChar() == '>', 
		       "Parse Error : shall have a '>' here, but has a '%c' (at idx=%d)\n",
		      currentChar(), readBufferIdx);
	      nextChar();
	      state = parseText;
	      continue;
	    }
	  if ( justCreated )
	    {
	      nextChar();
	      AssertBug  ( currentChar() == '>',
			"Malformed XML Input !! wrong param end.\n" );
	      nextChar();
	      unsigned int length = strlen ( event->name ) + 1;
	      endName = (char*) readSpaceAlloc ( length );
	      memcpy ( endName, event->name, length );
	    }
	  else
	    {

	      nextChar();
	      getWord ( endName, " >" );
	      nextChar();
	    }
	  event = newEvent ( true );
	  event->name = endName;
	  event->readSpaceEnd = readSpaceCurrent ();
	  level--;
	  if ( level == 0 )
	    {
	      _fullEvents++;
	    }
	  __DOOLPXML_Log ( "At end of section : level=%d, fullEvents=%d\n",
			level, _fullEvents );
	  if ( level == 0 && _fullEvents == 1 && readBufferSz == readBufferMax )
	    {
	      __DOOLPXML_Log ( "Premature exit due to sufficient data parsed.\n" );
	      goto parseFinished;
	    }
	  state = parseOutside;
	  continue;
	case parseText:
	  while ( currentChar() == ' '
		  || currentChar() == '\n' 
		  )		
	    { 
	      if ( readBufferIdx == readBufferSz )
		break;
	      nextChar(); 
	      continue; 
	    }
	  if ( isFinished () ) 
	    break;
	  if ( currentChar() != '<' )
	    {
	      getWord ( event->text, "<" );
	    }
	  nextChar();
	  state = parseBody;
	  continue;
	}
    }
  if ( readBufferIdx != readBufferSz )
    {
      for ( unsigned int i = readBufferIdx ; i != readBufferSz ; i++ )
	__DOOLPXML_Log ( "Remaining char at %i : %c\n", i, readBuffer[readBufferIdx] );
      Bug ( "Did not finish to parse ! \n" );
    }
 parseFinished: 
  __DOOLPXML_Log ( "at the parse end : idx=%d, sz=%d\n",
		readBufferIdx, readBufferSz );
  logEvents ();

  return true;

}

#define checkValidPointer(p) \
  AssertBug ( (unsigned int)p >= (unsigned int)readSpace \
	      && ((unsigned int)p < ( (unsigned int) readSpace + readSpaceMax ) ), \
	      "Invalid Pointer : %p, shall be between %p and %p\n",	\
	      p, readSpace, (void*) ((unsigned int) readSpace + readSpaceMax ) );

bool DoolpConnectionXML::logEvents ( )
{
  __DOOLPXML_LogEvents ( "******** %d Events *********\n", eventsNumber );
  for ( DoolpConnectionXMLEvent * event = firstEvent ;
	event != NULL ; event = event->next )
    {
      checkValidPointer ( event );
      checkValidPointer ( event->name );
      if ( event->text != NULL )
	checkValidPointer ( event->text );

      switch ( event->isEnd )
	{
	case true: 
	  __DOOLPXML_LogEvents ( "(event=%p,spEnd=%d) End '%s'.\n", event, event->readSpaceEnd, event->name );
	  continue;
	case false: 
	  __DOOLPXML_LogEvents ( "(event=%p,spEnd=%d) Name='%s'(%p), Text='%s'(%p) (first=%p, last=%p)\n",
			event, event->readSpaceEnd, event->name, event->name, event->text, event->text,
			event->first, event->last );
	  if ( event->first != NULL )
	    checkValidPointer ( event->first );
	  if ( event->last != NULL )
	    checkValidPointer ( event->last );
	  AssertBug ( event->readSpaceEnd <= readSpaceMax,
		   "Invalid readSpaceEnd=%d (max=%d)\n", event->readSpaceEnd, readSpaceMax );
	  for ( DoolpConnectionXMLAttr * attr = event->first;
		attr != NULL ; attr = attr->next )
	    {
	      __DOOLPXML_LogEvents ( "\tAttr (%p, %p) '%s'='%s'\n",
			    attr->name, attr->value, attr->name, attr->value );
	      checkValidPointer ( attr );
	      checkValidPointer ( attr->name );
	      checkValidPointer ( attr->value );
	    }
	  break;
	}
    }
  __DOOLPXML_LogEvents ( "********* end of Events *********\n" );
  return true;
};
#endif // 0 : FILE TO BE REMOVED
