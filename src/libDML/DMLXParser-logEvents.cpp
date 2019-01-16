#include <DMLXParser.h>
#include <string.h>
#include <stdlib.h>

#define checkValidPointer(p) \
  AssertBug ( (unsigned int)p >= (unsigned int)storeSpace.getSpace() \
	      && ((unsigned int)p < ( (unsigned int) storeSpace.getSpace() + storeSpace.getMax() ) ), \
	      "Invalid Pointer : %p, shall be between %p and %p\n",	\
	      p, storeSpace.getSpace(), (void*) ((unsigned int) storeSpace.getSpace() + storeSpace.getMax() ) );
#define showPtr(__p) \
  (__p == NULL ? 0 : ((unsigned int)(__p) - (unsigned int)storeSpace.getSpace()))
bool DMLX::Parser::logEvents ( )
{
  Log ( "******** %d Events (%d total parsed, %d bytes avail in storeSpace) *********\n", 
	eventsNumber, parsing.totalParsed, storeSpace.avail() );
  for ( Event * event = firstEvent ;
	event != NULL ; event = event->next )
    {
      checkValidPointer ( event );
      if ( event->isMarkup() )
	{
	  AssertBug ( event->name != NULL, "Event has no name !\n" );
	  Log ( "(event=%d/%p,next=%p,flags=%d,isFull=%d) "
		"Markup '%s', (first=%d)\n",
		showPtr(event), event, event->next,
		event->getFlags(), event->isFull(),
		event->name->getKeyword(),
		showPtr(event->first) );
	  if ( event->first != NULL )
	    checkValidPointer ( event->first );
	  if ( event->next != NULL )
	    checkValidPointer ( event->next );
	  for ( Attr * attr = event->first;
		attr != NULL ; attr = attr->next )
	    {
	      Log ( "\tAttr (%d : %d) '%s'='%s'\n",
		    showPtr(attr), showPtr(attr->value), 
		    attr->name->getKeyword(), attr->value );
	      checkValidPointer ( attr );
	      checkValidPointer ( attr->value );
	    }
	}
      else if ( event->isText() )
	{
	  checkValidPointer ( event->text );
	  Log ( "(event=%d/%p,next=%p,flags=%d,isFull=%d) Text at %p.\n", 
		showPtr(event), event, event->next, event->getFlags(),
		event->isFull(), event->text );
	  char * limit = (char*)event;
	  if ( limit < event->text )
	    limit = (char*) ((unsigned int) storeSpace.getSpace() + storeSpace.getMax() );
	  Log ( "Computed '%d' bytes\n", limit - event->text );
	  int computedSize = limit - event->text - 1;
	  int size = strlen ( event->text );
	  AssertBug ( size == computedSize, "Sizes differ : strlen=%d, calculed=%d\n",
		      size, computedSize );
	  // Count % signs
	  int nbS = 0;
	  char * s = event->text;
	  if ( s[0] != '\0' )
	    {
	      if ( s[0] == '%' )
		nbS++;
	      while ( ( s = strchr ( s+1, '%' ) ) != NULL )
		nbS++;
	    }
	  Log ( "Counted '%d' percent signs.\n", nbS );
	  s = event->text;
	  char __buffer[strlen(s)+nbS+1];
	  int i,j;
	  for ( i = 0, j = 0 ; i <= size ; i++, j++ )
	    {
	      __buffer[j] = s[i];
	      if ( s[i] == '%' )
		__buffer[++j] = '%';
	    }
	  AssertBug ( i + nbS == j, "Wrong count of '%%' : i=%d, j=%d, nbS=%d\n",
		      i, j, nbS );
	  Log ( "%s\n", __buffer );
	}
      else if ( event->isEnd() )
	{
	  AssertBug ( event->name != NULL, "Event has no name !\n" );
	  Log ( "(event=%d/%p,next=%p,flags=%d,isFull=%d) End '%s'.\n", 
		showPtr(event), event, event->next, event->getFlags(),
		event->isFull(), event->name->getKeyword() );
	  continue;
	}
      else
	{
	  Bug ( "Invalid flag : event is nor Markup, Text or End !!\n" );
	}
    }
  Log ( "********* end of Events *********\n" );
  return true;
};

