#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <strongmap.h>
#include <DMLXWriter.h>
#include <DMLXParser.h>
#include <glogers.h>

typedef unsigned int DMLXKeyHash;

setGlog ( "decodeDMLXBin" );

#undef __NO_OUTPUT__
// #define __NO_OUTPUT__

void usage ()
{
  char program[] = "decodeDMLXBin";
  fprintf ( stderr, "%s usage :\n", program );
  fprintf ( stderr, "\t%s --help : show this message\n", program );
  fprintf ( stderr, "\t%s 'file.dmlx' ['keyword list file']\n", program );
  fprintf ( stderr, "\t%s --list 'list name' 'file.dmlx']\n", program );
  fprintf ( stderr, "\t\t where 'list name' is one of :\n" );
  fprintf ( stderr, "\t\t\t- glog : libglog dmlx logging.\n" );
  fprintf ( stderr, "\t\t\t- doolp : libDoolp Doolp::ConnectionXML binary protocol.\n" );
}

extern char * glog_keyword_list[];
extern char * doolp_keyword_list[];
int main ( int argc, char ** argv )
{
  addGlog ( new Gloger_File ( stderr ) );
  showGlogPrior ( __GLOG_PRIOR_LOG, true );
  char * source = NULL;
  char * keyfilename = NULL;
  char * keylist = NULL;
  if ( argc >= 2 )
    {
      if ( strcmp ( argv[1], "--help" ) == 0 ) 
	{ usage(); return 0; }
      else if ( strcmp ( argv[1], "--list" ) == 0 )
	{
	  if ( argc != 4 )
	    { usage (); return -1; }
	  keylist = argv[2];
	  source = argv[3];
	}
      else
	{
	  source = argv[1];
	  if ( argc == 3 )
	    keyfilename = argv[2];
	}
    }
  else
    {
      usage (); return -1;
    }
  DMLX::ParserFile parser ( source );
  if ( keyfilename != NULL )
    {
      FILE * keyfile = fopen ( keyfilename, "r" );
      if ( keyfile == NULL )
	{
	  printf ( "Could not open key file '%s'\n", keyfilename );
	  return -1;
	}
      while ( ! feof ( keyfile) )
	{
	  char * key = (char *) malloc ( 64 );
	  key[0] = '\0';
	  if ( fscanf ( keyfile, "%s", key ) == 0 )
	    continue;
	  if ( key[0] == '\0' )
	    continue;
	  DMLX::KeyHash hash = DMLX::Keyword::rsHash ( key );
	  Log ( "Key '%s', hash '0x%x'\n", key, hash );
	  parser.getHashTree()->add ( new DMLX::Keyword ( key ) );
	}
    }
  else if ( keylist != NULL )
    {
      char ** list = NULL;
      if ( strcmp ( keylist, "glog" ) == 0 )
	{
	  list = glog_keyword_list;
	}
      else if ( strcmp ( keylist, "doolp" ) == 0 )
	{
	  list = doolp_keyword_list;
	}
      else
	{
	  Fatal ( "Invalid keyword list '%s'\n", keylist );
	}
      for ( unsigned int i = 0 ; list[i] != NULL ; i++ )
	{
	  Log ( "Keyword '%s'\n", list[i] );
	  parser.getHashTree()->add ( new DMLX::Keyword ( list[i] ) );
	}
    }
  parser.setParseBin ( true );
  parser.doFillBuffer ();
  fprintf ( stdout, "<?xml version=\"1.0\"?>\n" );
  fflush ( stdout );
  DMLX::Writer writer ( STDOUT_FILENO, false );
  writer.setIndentation ( true );
  unsigned int eventsNumber = 0;
  Info ( "Start Parsing...\n" );
  try
    {
      while (true)
	{
	  DMLX::Event * event = parser.getEvent();
	  //	  Log ( "Event %d\n", eventsNumber );
	  eventsNumber++;
#ifdef __NO_OUTPUT__
	  parser.popEvent(); continue;
#endif
	  AssertBug ( event != NULL, "Been given an empty event !\n" );
	  if ( event->isMarkup() )
	    {
	      writer.writeMarkup ( *(event->name) );
	      for ( DMLX::Attr * attr = event->first ; 
		    attr != NULL ; attr = attr->next )
		{
		  writer.writeAttr ( *(attr->name), attr->value, true );
		}
	    }
	  else if ( event->isEnd() )
	    {
	      writer.writeMarkupEnd ();
	    }
	  else if ( event->isText() )
	    {
	      writer.writeText ( event->text, true );
	    }
	  else
	    {
	      Bug ( "Unknown event type\n" );
	    }
	  parser.popEvent();
	}
    }
  catch ( DMLX::ParserException * e )
    {
      Log ( "Exception '%s'\n", e->getMessage() );
    }
  Info ( "Finished Parsing : %d events parsed.\n", eventsNumber );
  if ( parser.getEventsNumber () > 0 )
    {
      Warn ( "Event stack is not empty !\n" );
      parser.logEvents ();
    }
  writer.flush ();
  fsync ( STDOUT_FILENO );
  return 0;
}
