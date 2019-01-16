#include "DML.h"
#include "glog.h"

#ifdef __DML_EXTENSIVE_DEBUG
bool dumpWords ( list<char *> * words )
{
  list<char*>::iterator word;
  Log ( "Vector has %d words\n", words->size() );
  for ( word = words->begin() ; word != words->end () ; word ++)
    Log ( "Word %s\n", (*word) );
  return true;
}
#endif


/*
DMLParam * DMLSection::getParam ( char * paramName )
{
	list<DMLParam *>::iterator param;
	Log ( "getParam %s\n", paramName );
	for ( param = params.begin() ; param != params.end() ; param++ )
	  {
	    Log ( "Param %p:%s\n", (void*)*param, (*param)->name );
	    if ( strcmp ( (*param)->name, paramName ) == 0 ) 
	      {
		Log ( "Found Parameter %s\n", paramName );
		return *param;
	      }
	  }
#ifdef __DML_EXTENSIVE_DEBUG
	Error ( "In section %s : could not find param %s\n", name, paramName );
#endif // __DML_EXTENSIVE_DEBUG
	return NULL;
}
*/

DMLSection * DMLSection::getSection ( char * sectionName )
{
  list<DMLSection *>::iterator section;
  for ( section = sections.begin() ; section != sections.end() ; section++ )
    if ( strcmp ( (*section)->name, sectionName ) == 0 ) return *section;
  Error ( "In section %s : could not find section %s\n", name, sectionName );
  return NULL;
}

DMLSection * DMLSection::getSectionWithParam ( char * param, char * value )
{
  list<DMLSection *>::iterator section;
  char * paramValue;
  for ( section = sections.begin() ; section != sections.end() ; section++ )
    {
      //      Log ( "At section : '%s'\n", (*section)->getName() );
      paramValue = (*section)->getParam(param);
      if ( paramValue == NULL ) continue;
      if ( strcmp ( paramValue /* (*section)->getParam(param) */, value ) == 0 ) return *section;
    }
  Error ( "In section %s : could not find section with '%s'='%s'\n", getName(), param, value );
  return NULL;
}

char * DMLSection::getParam ( char * paramName )
{
  /*
  Log ( "In '%s' : Searching paramName '%s'\n", getName(), paramName );
  Log ( "Section has '%d' params\n", params.size () );
  */
  /*
  char * value = params[paramName];
  Log ( "value = %p\n", value );
  Log ( "value = '%s'\n", value );
  */
  DMLSectionParams::iterator param;
  for ( param = params.begin () ; param != params.end () ; param ++ )
    {
      // Log ( "param '%s' = '%s'\n", param->first, param->second );
      if ( strcmp ( param->first, paramName ) == 0 )
	return param->second;
    }
  Error ( "In '%s' : Could not get paramName '%s'\n", getName(), paramName );
  return NULL;
}

bool DMLSection::setParam ( char * paramName, char * paramValue )
{
  Log ( "Searching paramName '%s'\n", paramName );
  // char * value = params[paramName];
  
  DMLSectionParams::iterator param;
  for ( param = params.begin () ; param != params.end () ; param ++ )
    {
      Log ( "param '%s'\n", param->first );
      if ( strcmp ( param->first, paramName ) == 0 )
	{ param->second = paramValue; return true; }
    }
  
  Log ( "Could not get paramName '%s', creating new one.\n", paramName );
  return addParam ( paramName, paramValue );
}
