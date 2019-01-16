#include <parseCtags.h>
#include <list>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#undef __PARSECTAGS_DEBUG

#ifdef __PARSECTAGS_DEBUG
#define Debug(...) Log(__VA_ARGS__)
#else
#define Debug(...)
#endif

list<attrValue> * parseAttrValues ( FILE * input )
{
  list<attrValue> * values = new list<attrValue>;
  char c;
  char buffer[512];
  int idx = 0;
  char *name = NULL, *value = NULL;
  attrValue aValue;
  bool onName = true;
  while ( ! feof ( input ) )
    {
      fscanf( input, "%c", &c );
      switch ( c )
	{
	case ':': if (!onName) { buffer[idx++] = c ; break; }
	  name = (char*) malloc ( strlen ( buffer ) + 1 );		
	  strcpy ( name, buffer );
	  idx = 0;
	  onName = false;
	  break;
	case '\n': 
	case '\t': 
	  if ( onName) continue; // Fatal ( "Unexpected tabulation" ); 
	  value = (char*) malloc ( strlen (buffer) + 1);
	  strcpy ( value, buffer );
	  idx = 0;
	  onName = true;
	  aValue.name = name;
#if 0
	  if ( strcmp ( name, "class" ) == 0 )
	    {
	      char * doubleDots = strstr( value, "::" );
	      if ( doubleDots != NULL )
		{
		  doubleDots[0] = '\0';
				
		}
	    }
#endif
	  aValue.value = value;
			 
	  values->push_back ( aValue );
	  // printf ( "Value : %s = %s\n", name, value );
	  if ( c == '\n' ) return values;
	  break;
	default:
	  buffer[idx++] = c;
	  buffer[idx] = '\0';
	}
    }
  Fatal ( "Unexpected end !\n" );
}

bool ctagsEnded = false;

list<ctagsLine> * parseCtags ( FILE * input )
{
  list<ctagsLine> * ctagsLines = new list<ctagsLine>;
  char c;
  char buffer[256];
  char * item, * code;
  char * sourceFile;
  int idx = 0;
  while ( ! feof ( input ) )
    {
      buffer[0] = '\0';
      if ( fscanf ( input, "%s\t", buffer ) == 0 ) break;
      if ( buffer[0] == '\0' ) break;
      item = (char*) malloc ( strlen (buffer) + 1 );
      strcpy ( item, buffer );

      if ( fscanf ( input, "%s\t", buffer ) == 0 ) break;
      sourceFile = (char*) malloc ( strlen (buffer) + 1 );
      strcpy ( sourceFile, buffer );


      if ( item[0] == '!' ) /* skip comment */
	{ c = '\0'; while ( c != '\n' ) fscanf ( input, "%c", &c ); continue; }

      Debug ( "item %s, sourceFile %s\n", item, sourceFile );

      fscanf ( input, "%c", &c );
      char * pattern; int pattern_sz;
      if ( c == '/' )
	{
	  // Here we must find the pattern : "$/;\""
	  pattern = "$/;\"";
	  pattern_sz = 4;
	}
      else
	{
	  pattern = "\t";
	  pattern_sz = 1;
	}
      int pattern_idx = 0;
		
      for ( idx = 0 ; (idx < 256) ; idx ++ )
	{
	  fscanf ( input, "%c", &c );
	  // if ( c == '\t' )
	  // break;
	  buffer[idx] = c;
	  buffer[idx + 1] = '\0' ;
			
	  // printf ( "idx %d, char %d, buffer %s\n", idx, c, buffer );
	  if ( c == 13 || c == 10 ) break;
	  if ( c == pattern[pattern_idx] )
	    pattern_idx++;
	  else
	    pattern_idx = 0;					
	  if ( pattern_idx == pattern_sz ) break;
	}
      buffer[++idx] = '\0';

      code = (char*) malloc ( strlen (buffer) + 1 );
      strcpy ( code, buffer );

      ctagsLine line;
      line.item = item;
      line.sourceFile = sourceFile;
      line.code = code;
      line.aValue = parseAttrValues ( input );
      ctagsLines->push_back(line);		
    }
  Debug ( "** ctags : %d elements parsed.\n", ctagsLines->size () );
  return ctagsLines;
}

void sigChild (int t)
{
  // printf ( "sigChild : ctags ended.%d\n", t );
  ctagsEnded = true;
}

// list<ctagsLine> * parseCtags ( char ** files, int filesnb )
list<ctagsLine> * parseCtags ( list<char *> * files )
{
  int ctags_pid;
  int filesnb = files->size ();
  char ** ctags_params;
  if ( filesnb == 0 ) { Warn ( "No Input Provided.\n" ); return NULL; }
  ctags_params = (char **) malloc ( sizeof ( char *) * (filesnb + 10 ) );
/*
	ctags is run like this : 
	ctags --fields=afikKlmnsSz --extra=q --c++-kinds=cdefgmnpstuvx --language-force=c++ --sort=no ../libCar/car.h 

*/
  ctags_params[0] = "ctags";
  ctags_params[1] = "--fields=afikKlmnsSz";
  ctags_params[2] = "--extra=q";
  ctags_params[3] = "--c++-kinds=cdefgmnpstuvx";
  ctags_params[4] = "--language-force=c++";
  ctags_params[5] = "--sort=no";
  /*
  ctags_params[6] = "-f";
  ctags_params[7] = "-";
  */
  int extraparams = 0;
  int fileparamsStart = 6;
  /*
  for ( extraparams = 0 ; extraparams != filesnb ; extraparams++ )
    {
      ctags_params[extraparams + fileparamsStart] = files[extraparams];
    }
  */

  list<char*>::iterator file;
  for ( file = files->begin ( ) ; file != files->end () ; file ++ )
    { ctags_params[(extraparams++) + fileparamsStart] = *file; }

  extraparams = filesnb + fileparamsStart;
  ctags_params[extraparams] = NULL;

  signal ( SIGCHLD, sigChild );
  ctags_pid = fork ();
  if ( ctags_pid == 0 )
    {
      if ( execvp ( "ctags", ctags_params ) < 0 )
	{ 
	  fprintf ( stderr, "Could not run ctags\n" ); 
	  fprintf ( stderr, "Errno %d:%s\n", errno, strerror(errno) );
	  exit (-1); 
	}
      exit ( 0 );
    }
  while ( !ctagsEnded )
    usleep ( 200 );

  ctagsEnded = false;
  FILE * ctags_in;
  ctags_in = fopen ( "tags", "r" );
  free ( ctags_params );
  return parseCtags ( ctags_in );
}

bool dumpCtagsLine ( FILE * out, ctagsLine * line )
{
  list<attrValue>::iterator attr;
  fprintf ( out, "item '%s', sourceFile '%s', code '%s'\n",
	    line->item, line->sourceFile, line->code );
  for ( attr = line->aValue->begin ( ) ; attr != line->aValue->end ( ) ; attr ++ )
    {
      fprintf ( out, "\t'%s' = '%s'\n", attr->name, attr->value );
    }
  return true;
}
bool dumpCtags ( FILE * out, list<ctagsLine> * lines )
{
  list<ctagsLine>::iterator line;
  for ( line = lines->begin () ; line != lines->end () ; line ++ )
    {
      dumpCtagsLine ( out, &(*line) );
    }
  return true;

}
