#include <DMLXParser.h>
#include <string.h>
#include <stdlib.h>

/*
 * Cyclic buffer for names : StoreSpace
 */ 

DMLX::Parser::StoreSpace::StoreSpace ( unsigned int _max )
{
  max = _max;
  space = (char*) malloc ( max );
  AssertFatal ( space != NULL, "Could not malloc StoreSpace buffer (size=%d) !\n", max );
  first = 0;
  front = 0;
}

DMLX::Parser::StoreSpace::~StoreSpace ()
{
  
}

unsigned int DMLX::Parser::StoreSpace::avail ( )
{
  if ( first == front )
    return max;
  if ( first < front )
    return ( max - ( front - first ) );
  return ( first - front );
}

unsigned int DMLX::Parser::StoreSpace::topointer ( void * ptr ) // Converts a global pointer to an offset in space.
{
  AssertBug ( ((unsigned int)ptr >= (unsigned int)space)
	      && ((unsigned int)ptr < (unsigned int)space + max ), "Pointer not in the nameSpace !\n" );
  return ((unsigned int)ptr - (unsigned int)space);
}

bool DMLX::Parser::StoreSpace::free ( DMLX::Event * event )
{
  if ( event->next == NULL )
    first = front;
  else
    first = topointer ( event->next );
  return true;
}

#if 0
#define AssertFull(__cond,...) { if ( ! (__cond) ) { Warn ( __VA_ARGS__ ); throw new DMLX::ParserFullSpaceException(); } }

void * DMLX::Parser::StoreSpace::alloc ( unsigned int reqLength )
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


void * DMLX::Parser::StoreSpace::alloc ()
{
  AssertBug ( front < max,
	      "Went out of bounds !!! front=%d, max=%d\n", front, max );
  curLength = 0;
  return &space[front];
}

#if 0
bool DMLX::Parser::StoreSpace::extend ( char **s, char c )
{
  //  unsigned int length = front - ((unsigned int )(*s) - (unsigned int) space );
  /*
  AssertBug ( ( front + 1 ) <= max, "Out of bounds in extend."
	      "(front=%d, first=%d, max=%d, curLength=%d, reqL=%d)"
 	      "(max=%d, s=%d/%p, space=[%p-%p]\n",
	      front, first, max, 
	      curLength, curLength + 1,
	      max, ((unsigned int)*s - (unsigned int)space),
	      *s, space, (void*)((unsigned int)space+max) );
	      */

  AssertFull ( ( front + 1 ) != first,
	       "extend : Full space (front=%d, first=%d, max=%d, curLength=%d, reqL=%d)"
	       "(max=%d, s=%d/%p, space=[%p-%p]\n",
	       front, first, max, 
	       curLength, curLength + 1,
	       max, ((unsigned int)*s - (unsigned int)space),
	       *s, space, (void*)((unsigned int)space+max) );
  if ( ( front + 1 ) == max )
    *s = (char*) realloc_begin ( *s, curLength, curLength + 1 );
  else
    front++;
  curLength++;
  space[front - 1] = c;
  return true;
}
#endif

void * DMLX::Parser::StoreSpace::realloc_begin ( void * old, 
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


bool DMLX::Parser::StoreSpace::free ( DMLXEvent * event )
{
  AssertBug ( event->spaceEnd < max, "free : out of bounds till=%d\n", till );
  first = event->spaceEnd; 
  return true;
}

unsigned int DMLX::Parser::StoreSpace::avail ( )
{
  if ( first == front )
    return max;
  if ( first < front )
    return ( max - ( front - first ) );
  return ( first - front );
}

unsigned int DMLX::Parser::StoreSpace::topointer ( void * ptr ) // Converts a global pointer to an offset in space.
{
  AssertBug ( ((unsigned int)ptr >= (unsigned int)space)
	      && ((unsigned int)ptr < (unsigned int)space + max ), "Pointer not in the nameSpace !\n" );
  return ((unsigned int)ptr - (unsigned int)space);
}
#endif 
