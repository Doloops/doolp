/*
 * Cyclic buffer for names : StoreSpace
 * Carefull : only include by DMLXParser-parse-ng.cpp
 */ 

#undef __DMLXPARSER_NAMESPACE_VERBOUS_ASSERTFULL

#ifdef __DMLXPARSER_NAMESPACE_VERBOUS_ASSERTFULL
#define AssertFull(__cond,...) { if ( ! (__cond) ) { Warn ( __VA_ARGS__ ); throw new ParserFullSpaceException(); } }
#else
#define AssertFull(__cond,...) { if ( ! (__cond) ) throw new ParserFullSpaceException(); }
#endif 

#undef AssertBug
#define AssertBug(...)

inline void * DMLX::Parser::StoreSpace::alloc ( unsigned int reqLength )
{
  AssertBug ( first < max,
	      "Incoherent space : first=%d, max=%d, (front=%d)\n",
	      first, max, front );
  AssertFull ( !(front + reqLength == first), "alloc : Full Space front=%d, first=%d, reqLength=%d\n",
	       front, first, reqLength );
  if ( front >= first )
    {
      if ( front + reqLength < max )
	{
	  char * g = &space[front];
	  front += reqLength;
	  return g;
	}
      else
	{
	  return realloc_begin ( NULL, 0, reqLength );
	}
    }
  if ( front + reqLength + 1 < first )
    {
      char * g = &(space[front]);
      front += reqLength;
      return g;
    }
  AssertFull ( false, "alloc : Full space (front=%d, first=%d, max=%d, reqL=%d)\n",
	       front, first, max, reqLength );
  return NULL;
}


inline void * DMLX::Parser::StoreSpace::alloc ()
{
  AssertBug ( front < max,
	      "Went out of bounds !!! front=%d, max=%d\n", front, max );
  curLength = 0;
  return &space[front];
}

inline void DMLX::Parser::StoreSpace::extend ( char **s, char c )
{ 
  AssertFull ( (front+1) != first, "Full space in extend, curLength='%d'\n",
	       curLength );
  if ( (front+1) == max )
    *s = (char*) realloc_begin ( (void*)(*s), curLength,
					     curLength + 1 );
  else
    front++;
  curLength++;
  space[front - 1] = c;
}


inline void * DMLX::Parser::StoreSpace::realloc_begin ( void * old, 
							unsigned int prevLength, 
							unsigned int reqLength )
{
  // At the beginning of the space
  AssertFull ( reqLength < first,
	       "Full space ! prevlength=%d, reqLenght=%d, old=%p, start=%p, first=%d, front=%d\n",
	       prevLength, reqLength, old, space, first, front);
  front = reqLength;
  if ( prevLength > 0 )
    memcpy ( space, old, prevLength );
  return &(space[0]);
}


/*
 * Event (Markup) creation and conclusion
 * In newEvt() and newEvtEndWithName(), notice that endStrName 
 * is performed AFTER the newEvent(), in case the newEvent fails 
 * because of a full StoreSpace.
 * newEvtEnd() asserts that parsing.name is set with uniqueKey()
 */
inline DMLX::Event * DMLX::Parser::newEvent ( bool isEnd )
{
  Event* event = (Event*) storeSpace.alloc ( sizeof (Event) );
  event->__init();
  //  memset ( event, 0, sizeof (Event) );
  if ( isEnd )
    event->setEnd ();
  eventsNumber++;
  if ( isEnd )
    endEventsNumber++;
  AssertBug ( ( firstEvent == NULL && lastEvent == NULL )
	      || ( firstEvent != NULL && lastEvent != NULL ),
	      "firstEvent and lastEvent shall be NULL at the same time !\n" );
  if ( lastEvent != NULL )
    lastEvent->next = event;
  else
    { firstEvent = event; lastEvent = event; }
  lastEvent = event;
  return event;
}

/*
 * Attribute creation.
 */
inline DMLX::Attr * DMLX::Parser::addAttr ( Event * event, 
					    Attr * lastAttr,
					    Keyword * name, 
					    char * value )
{
  Attr * attr = (Attr *)
    storeSpace.alloc ( sizeof ( Attr ) );
  attr->next = NULL;
  attr->name = name;
  attr->value = value;
  if ( event->first == NULL )
    event->first = attr;
  if ( lastAttr != NULL )
    lastAttr->next = attr;
  return attr;
}
