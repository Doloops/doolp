#include <DML.h>



bool DMLSection::writeXML ( FILE * fp, int tab )
{
#define _MKTAB \
  for ( idx = 0 ; idx < tab ; idx ++ ) fprintf ( fp, "\t" );

  int idx;
  if ( isTextSection() )
    {
      fprintf ( fp, "%s\n", params["(text)"] );
      return true;
    }
  _MKTAB;
  fprintf ( fp, "<%s", name );
  
  DMLSectionParams::iterator param;
  for ( param = params.begin () ; param != params.end () ; param ++ )
    {
      Log ( "In '%s' : writing param '%s' = '%s'\n", getName(), param->first, param->second );
      fprintf ( fp, " %s", param->first ); // "%s\"", param->first, param->second );
      fprintf ( fp, "=\"" );

      if ( param->second == NULL ) 
	{
	  fprintf ( fp, "\"" );
	  continue;
	}
      char * c = param->second;
      while ( *c != '\0' )
	{
	  
	  switch ( *c )
	    {
	    case '<': fprintf ( fp, "&lt;" ); break;
	    case '>': fprintf ( fp, "&gt;" ); break;
	    case '&': fprintf ( fp, "&amp;" ); break;
	    default: fprintf ( fp, "%c", *c );
	    }
	  c++;
	}
      fprintf ( fp, "\"" );
    }
  if ( sections.size () == 0 )
    {
      fprintf ( fp, "/>\n" );
      return true;
    }
  else 
    {
      fprintf ( fp, ">\n" );
    }

  list<DMLSection*>::iterator section;
  for ( section = sections.begin () ; section != sections.end () ; section ++ )
    (*section)->writeXML ( fp, tab + 1 );



  _MKTAB;
  fprintf ( fp, "</%s>\n", name );
#undef _MKTAB
  return true;
}

bool DMLSection::writeXML ( FILE * fp )
{ 
  fprintf ( fp, "<?xml version=\"1.0\"?>\n" );
  return writeXML ( fp, 0 ); 
}
