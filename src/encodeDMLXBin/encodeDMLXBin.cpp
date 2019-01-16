#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <DMLXParser.h>
#include <DMLXWriter.h>
#include <glogers.h>

#undef Log
#define Log(...)

setGlog ( "encodeDMLXBin" );

unsigned int idx;
bool isHexTable[256];

#define STDOUT STDOUT_FILENO
// #define __NO_OUTPUT__

inline bool isInteger ( const char * value )
{
  for ( idx = 0 ; value[idx] != '\0' ; idx ++ )
    {
      if ( ! ( value[idx] >= '0' && value[idx] <= '9' ) )
	return false;
    }
  return true;
}

inline bool isHex ( const char * value )
{
  if ( value[0] != '0' )
    return false;
  if ( value[1] != 'x' )
    return false;
  for ( idx = 2 ; value[idx] != '\0' ; idx ++ )
    {
      if ( ! isHexTable[(unsigned int)value[idx]] )
	return false;
      if ( idx > 10 )
	return false;
    }
  return true;
}


inline bool isFloat ( const char * value )
{
  for ( idx = 0 ; value[idx] != '\0' ; idx ++ )
    {
      if ( ! ( ( value[idx] >= '0' && value[idx] <= '9' )
	       || ( value[idx] == '.' ) ) )
	return false;
    }
  return true;
}

int main ( int argc, char ** argv )
{
  addGlog ( new GlogInfo_File ( stderr ) );
  //  showGlogPrior ( __GLOG_PRIOR_LOG, false );
  memset ( isHexTable, 0, sizeof ( isHexTable ) );
  for ( int c = '0' ; c <= '9' ; c++ )
    isHexTable[c] = true;
  for ( int c = 'a' ; c <= 'f' ; c++ )
    isHexTable[c] = true;
    
  if ( ( argc != 2 )
       && ( argc != 3 ) )
    {
      Error ( "Error : shall take one arguments\n" );
      Error ( "Usage : encodeDMLXBin \"XML File\" [\"Dump Keywords File\"]\n" );
      return -1;
    }
  DMLX::ParserFile parser( (char*) argv[1] );
  parser.setMaxEndEventsInStoreSpace ( 1024 );
  DMLX::Writer writer ( STDOUT, true );
  writer.setWriteBinKeywords ( true );
  int valHex;
  DMLX::Event * event = NULL;
  DMLX::Attr * attr;

  if ( ! parser.doFillBuffer() )
    {
      Error ( "Could not fill buffer...\n" );
    }
  Info ( "Start parsing..\n" );
  while ( true )
    {
      event = NULL;
      try
	{
	  event = parser.getEvent ();
	}
      catch ( DMLX::ParserException * e )
	{
	  Error ( "Got exception '%s'\n", e->getMessage () );
	  goto parseEnd;
	}
#ifdef __NO_OUTPUT__
      parser.popEvent(); continue;
#endif
      AssertBug ( event != NULL, "Been given an empty event !\n" );
      Log ( "New event\n" );
      if ( event->isMarkup() )
	{
	  Log ( "New Markup '%s'\n", event->name->getKeyword() );
	  writer.writeMarkup ( *(event->name) );
	  for ( attr = event->first ; attr != NULL ; attr = attr->next )
	    {
	      Log ( "New attr '%s'='%s'\n", 
			attr->name->getKeyword (),
			attr->value );
	      if ( isInteger ( attr->value ) )
		{
		  writer.writeAttrInt ( *(attr->name), atoi ( attr->value ) );
		  continue;
		}
	      if ( isHex ( attr->value ) )
		{
		  sscanf ( attr->value, "0x%x", &valHex );
		  writer.writeAttrHex ( *(attr->name), valHex );
		  continue;
		}
	      if ( isFloat ( attr->value ) )
		{
		  writer.writeAttrFloat ( *(attr->name), atof ( attr->value) );
		  continue;
		}
	      writer.writeAttr ( *(attr->name), attr->value, false );
	    }
	  parser.popEvent();
	  continue;
	}
      if ( event->isText() )
	{
	  Log ( "New Event Text '%s'\n", event->text );
	  writer.writeText ( event->text, false );
	  parser.popEvent();
	  continue;
	}
      if ( event->isEnd() )
	{
	  Log ( "New Event End '%s'\n", event->name->getKeyword() );
	  writer.writeMarkupEnd ();
	  parser.popEventEnd();
	  continue;
	}
    }
 parseEnd:
  Info ( "Parse Finished.\n" );
  if ( argc == 3 )
    {
      parser.dumpKeywords ( argv[2] );
    }
  writer.flush ();

  parser.logEvents ();
  parser.logKeywords ();
  return 0;
}
