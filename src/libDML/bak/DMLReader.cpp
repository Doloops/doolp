#include "DML.h"
#include "glog.h"



char seekTo ( FILE * fp, const char * to, char ** text, bool fillText )
{
  char c;
  int idx = 0, result;
  int rs = 0;
#define bufferMax 2048
  char buffer[bufferMax];
  bool relevant = false;
  while ( ! feof (fp) )
    {
      result = fscanf ( fp, "%c", &c );
      //      Log ( "Read c = %c\n", c );
      for ( idx = 0 ; to[idx] != '\0' ; idx ++ )
	if ( to[idx] == c )
	  {
	    goto fnd;
	  }
      if ( fillText )
	{
	  buffer[rs++] = c;
	  if ( rs == bufferMax )
	    Fatal ( "Buffer overflow\n" );
	  if ( c != '\n' && c != ' ' )
	    relevant = true;
	}
      continue;
    fnd:
      Info ( "Found seekTo terminator : %c\n", c );
      if ( fillText && relevant )
	{
	  *text = (char *) malloc ( rs + 1 );
	  memcpy ( *text, buffer, rs );
	  (*text)[rs] = '\0';
	}
      else if ( text !=NULL )
	{
	  (*text) = NULL;
	}
      return c;
    }
  return 0;
#undef bufferMax
}

char seekTo ( FILE * fp, const char * to )
{  return seekTo ( fp, to, NULL, false ); }
char seekTo ( FILE * fp, const char * to, char ** text )
{  return seekTo ( fp, to, text, true ); }


char readWords ( FILE * fp, list<char *> * words, 
		 const char * separators, 
		 const char * explicitSeparators,
		 const char * terminators, 
		 const char * avoids )
{
	char c;
	int index = 0, result, i;
	char buffer[2048];
	char * word;
	words->clear ();
#define addWord\
	if ( index > 0) { \
		word = (char*) malloc ( index + 1 ); \
		memcpy ( word, buffer, index + 1 ); \
		word[index] = '\0'; \
		words->push_back ( word ); \
		index = 0; }
startToRead:
	while ( ! feof (fp) )
	{
		result = fscanf ( fp, "%c", &c );
		// Log ( "Reading char %c %d [res %d]\n", c, c, result );
		if ( c == '"' )
		  {
		    // Scan till next "
		    while ( ! feof (fp) )
		      {
			result = fscanf ( fp, "%c", &c );
			if ( c == '"' )
			  { addWord; goto startToRead; }
			buffer[index++] = c;
		      }
		    Fatal ( "End of file before end of string\n" );
		  }
		for ( i = 0 ; separators[i] != '\0'; i++ )
		  if ( c == separators[i] )
		    { addWord; goto startToRead; }
		for ( i = 0 ; explicitSeparators[i] != '\0'; i++ )
		  if ( c == explicitSeparators[i] )
		    { 
		      char * u = (char *) malloc ( 2 );
		      u[0] = c; u[1] = '\0';
		      addWord; 
		      words->push_back ( u );
		      goto startToRead; 
		    }
		for ( i = 0 ; terminators[i] != '\0'; i++ )
		  if ( c == terminators[i] )
		    { addWord; return c; }
		for ( i = 0 ; avoids[i] != '\0'; i++ )
		  if ( c == avoids[i] )
		    { goto startToRead; }
		buffer[index++] = c;
	}
	return '\0';
}



#ifdef __DML_DMLREADER_CPP__

bool readInsideDMLSection ( FILE * fp, DMLSection * section )
{
	list<char *> words;
	char term;
	DMLSection * subSection;
	//	DMLParam * param;
	Log ( "Parsing Section %s..\n", section->name );
readInsideDMLSection_startToRead:
	while ( ( term = readWords (fp, &words, " \n", "{=}", "\r\t" ) ) != '\0' )
	  {
	    if ( term == '}' )
	      {
		// Log ( "End of Section %s\n", section->name );			
		if ( words.size () > 0 )
		  {
		    Error ( "Remaining words at end of Section.\n" );
		    dumpWords ( &words );
		    Fatal ( "Syntax error.\n" );
		  }
		return true;
	      }

	    dumpWords ( &words );

	    switch ( term)
	      {
	      case '{':
		subSection = new DMLSection ( words.back() );
		words.pop_back();
		subSection->type = words.front ();
		// words.pop_front();
		subSection->options = words;
		if ( readInsideDMLSection ( fp, subSection ) )
		  section->addSection ( subSection );
		else
		  Error ( "Section %s : Could not read subsection %s\n",
			  section->name, subSection->name );
		//	return NULL;
		goto readInsideDMLSection_startToRead;
	      case ';':
		param = new DMLParam ( words.back () );
		words.pop_back();
		param->type = words.front ();
		// param->options = words;
#ifdef __DML_EXTENSIVE_DEBUG
		Log ( "New Param %s (type %s)\n", param->name, param->type /*, param->value[0] */);
#endif 
		section->addParam ( param );
		break;
	      case '=':
		param = new DMLParam ( words.back () );
		words.pop_back();
#ifdef __DML_EXTENSIVE_DEBUG
		Log ( "After param creation, words are :\n" );
#endif
		dumpWords ( &words );
		if ( words.size () > 0 )
		  {
		    param->type = words.front ();
		    words.erase ( words.begin(), words.begin () );
		  }
		else 
		  { 
		    param->type = NULL; 
		  }
		// param->options = words;
		// object->getParamValue ( "objectId" )
		readWords (fp, &words, "\n", ";", "\r\t" );
		//		param->setValue ( *(words.begin() )) ;
		char * val = *(words.begin () );
		while ( val[0] == ' ' )
		  val++;
		param->setValue ( val ); // *(words.begin() )) ;

		// shall strap a little, no ?
		// param->setValue ( words );
#ifdef __DML_EXTENSIVE_DEBUG
		Log ( "New Param %s (type %s)\n", param->name, param->type );
#endif 
		section->addParam ( param );
		break;
	      }
	  }
	return false;
}

DMLSection * readDMLSection ( FILE * fp )
{
	list<char *> words;
	char term;
	// Log ( "Reading Words...\n" );
	term = readWords (fp, &words, " \n", "{", "\r" );
	// Log ( "Terminated by : %d\n", term );
	DMLSection * section = new DMLSection ( words.back() );
	words.pop_back();
	section->type = words.front ();
	// words.pop_front();
	section->options = words;
	if ( readInsideDMLSection ( fp, section ) )
		return section;
	else
		return NULL;
}

#endif 
