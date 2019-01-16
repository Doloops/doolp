// #ifdef __DMLTRANSFORM_CPP__

#include "DML.h"
#include "glog.h"

int _readLine ( FILE * fp, char * ch )
{
  int i = 0;
  char c;
  while ( ! feof ( fp ) )
    {
      //      int result = 
      fscanf ( fp, "%c", &c );
      if ( c == 13 ) 
	continue;
      if ( c == 10 )
	{
	  ch[i] = '\0';
	  return i;
	}
      ch[i] = c;
      i++;
    }
  return -1;
}

#define _isKeyWord( __key__ ) \
  ( strncmp ( buff, __key__, strlen ( __key__ ) ) == 0 )

#define isKeyWord(__line__, __key__) \
  ( strncmp ( __line__, __key__, strlen ( __key__ ) ) == 0 )
#define _cpBuff(__offset__,__to__) \
      __to__ = (char *) malloc ( strlen (buff) + 1 - (__offset__) ); \
      strcpy ( __to__, buff + (__offset__) );


DMLTransform * readDMLTransform ( FILE * fp )
{
  char buff[256]; int i;
  bool isInSection = false;
  DMLTransform * transform = new DMLTransform ();
  DMLTransformSection * section = NULL;
  while ( ( i = _readLine ( fp, buff ) ) >= 0 )
    {
      if ( i == 0 )
	{
	  if ( isInSection )
	    section->lines.push_back ( "" );
	  continue; 
	}
      
      
      if ( buff[0] == '#' ) continue; // Comment.
      // printf ( "Read Line %s\n", buff );
      if ( _isKeyWord ( "@@section" ) )
	{
	  if ( isInSection )
	    Bug ( "Already in section !\n" );
	  isInSection = true;
	  char * sectionName; // = buff + strlen ( "@@section" ) + 1;
	  _cpBuff ( strlen ( "@@section") + 1, sectionName );
	  // printf ( "New Section : '%s'\n", sectionName );
	  section = new DMLTransformSection ( sectionName );
	  transform->sections.insert ( make_pair ( sectionName, section ) );
	  continue;
	}
      if ( _isKeyWord ( "@@endSection" ) )
	{
	  if ( ! isInSection )
	    Bug ( "Not in section !! \n" );
	  isInSection = false;
	  // printf ( "end of section\n" );
	  continue;
	};
      
      if ( ! isInSection )
	Bug ( "Content out of section : '%s'\n", buff );
      char * line; _cpBuff ( 0, line );
      section->lines.push_back ( line );
    }
  
  return transform;
}

void __dumpDMLTransform ( DMLTransform * trans )
{
  for ( map<char *,DMLTransformSection*>::iterator sec = trans->sections.begin ();
	sec != trans->sections.end (); sec ++ )
    {
      printf ( "Section : '%s'\n", sec->first );
      for ( list<char*>::iterator line = sec->second->lines.begin ();
	    line != sec->second->lines.end (); line ++ )
	printf ( "\tLine : '%s'\n", *line );
    }
}

inline bool _insertWord ( char * from, char * what, int whatSz, int offset )
{
  int i;
  // printf ( "from '%s', what '%s', whatSz %d, offset %d\n", from, what, whatSz, offset );
  if ( offset < 0 )
    {
      for ( i = whatSz - offset ; from [i] != '\0'  ; i++ )
	{
	  // printf ( "(o %d) i %d : from[i] = %c\n", offset, i, from[i] );
	  from [ i + offset ] = from [i ];
	}
      from [ i + offset ] = '\0';
    }
  else if ( offset > 0 )
    {
      // printf ( "counting ..\n" );
      /*
      i = 0;
      while ( from [i] != '\0' ) 
	{ 
	  //	  printf ( "i %d, c=%c\n", i, from [i] );
	  i++;
	}
      */
      unsigned int sz = strlen ( from );
      // printf ( "sz = %d\n", sz );
      // The remaining part (if any ) is from (whatSz-offset-1) to sz.
      for ( i = sz - 1 ; i != whatSz - offset - 1 ; i-- )
	{
	  /*
	  printf ( "(o %d) i %d, to %d : from[i] = %c, from[i - offset]=%c\n",
		   offset, i, i + offset, from[i], from[i - offset] );
	  */
	  from [ i + offset ] = from [ i ];
	  
	}
      from [ sz + offset ] = '\0';
      
    }

  // printf ( "post split : from '%s'\n", from );
  memcpy ( from, what, whatSz );
  //  printf ( "post copy : from '%s'\n", from );
  return true;
}

inline bool _replace ( char * from, char * what, char * byWhat )
{
  char * arg;
  while ( ( arg = strstr ( from, what ) ) != NULL )
    _insertWord ( arg, byWhat, strlen ( byWhat ), strlen ( byWhat ) - strlen ( what ) );
  return true;
    
}

inline bool _split3 ( char * from, char * mid, char * left, char * right )
{
  char * _mid;
  if ( ( _mid = strstr ( from, mid ) ) != NULL )
    {
      memcpy ( left, from, _mid - from );
      left[_mid-from] = '\0';
      strcpy ( right, _mid + strlen ( mid ) );
      printf ( "Splitted using '%s', left='%s', right='%s'\n",
		mid, left, right );
      return true;
    }
  return false;
}

inline bool _eval ( char * expr )
{
  // char  * op, * left, * right;
  //  char op[2];
  char left[512], right[512];
  if ( _split3 ( expr, "&&", left, right ) )
    {
      return ( _eval (left) && _eval (right) );
    }
  if ( _split3 ( expr, "||", left, right ) )
    {
      return ( _eval (left) || _eval (right ) );
    }
  if ( _split3 ( expr, "==", left, right ) )
    {
      return ( strcmp ( left, right ) == 0 );
    }
  if ( _split3 ( expr, "!=", left, right ) )
    {
      return ( strcmp ( left, right ) != 0 );
    }
  Bug ( "Could not parse expression : '%s'\n", expr );
  return false;
}

inline bool DMLSection::transformLine ( char * to, char * from )
{
  // TODO : must process : @father.name, @father.type, @(father.something)
  printf ( "transformLine : at section %s, processing %s\n",
	   name, from );
  char * arg;
  if ( to != from )
    strcpy ( to, from );
  _replace ( to, "@name", name );
  //  _replace ( to, "@type", type );
  printf ( "My father is %p\n", father );
  if ( father != NULL )
    {
      _replace ( to, "@father.name", father->name );
      //      _replace ( to, "@father.type", father->type );
    }
  while ( ( arg = strstr ( to, "@(" ) ) != NULL )
    {
      printf ( "Found param : %s\n", arg);
      char * argEnd;
      if ( ( argEnd = strstr ( arg, ")" ) ) == NULL )
	Bug ( "Could not find end of param\n" );
      char subRepl[128];
      memcpy ( subRepl, arg + 2, argEnd - arg - 2 );
      subRepl [ argEnd - arg - 2 ] = '\0';
      char * value;
      if ( strncmp ( subRepl, "father.", 7 ) == 0 )
	{
	  value = father->getParam ( subRepl + 7 );
	}
      else
	{
	  Log ( "getParam for subRepl='%s'\n", subRepl );
	  value = getParam ( subRepl );
	}
      // printf ( "Value = %s\n", value );
      if ( value == NULL )
	Bug ( "Could not find value for subRepl '%s'\n", subRepl );
      // _replace ( to, value, subRepl );
      _insertWord ( arg, value, strlen ( value ), strlen ( value ) - strlen ( subRepl ) - 3 );
      // printf ( "to = '%s'\n", to );
      // exit (-1);
    }

  printf ( "transformLine : finished with to=%s\n", to );

  return true;
}

bool DMLSection::transform ( FILE * fp_out, DMLTransform * trans, char * line, char * precond )
{
  char buff[512]; buff[0] = '0';
  char * arg;
  printf ( "transform : at section %s, doing line '%s'\n",
	   name, line );
  if ( precond != NULL ) // && precond[0] != '\0' )
    {
      if ( precond[0] != '\0' )
	{
	  printf ( "precondition string = '%s'\n", precond );
	  char transPrecond[512];
	  transformLine ( transPrecond, precond );
	  printf ( "precondition string after transformation : '%s'\n", transPrecond );
	  if ( !_eval ( transPrecond ) )
	    {
	      printf ( "precond is false.\n" );
	      return true;
	    }
	}
    }
  if ( strcmp ( line, "" ) == 0 )
    {
      fprintf ( fp_out, "\n" );
      return true;
    }
  //  strcpy ( buff, line );
  if ( isKeyWord ( line, "@forAllSections" )
       ||  isKeyWord ( line, "@forSections" ) )
    {
      printf ( "doing forSections\n" );
      arg = strstr ( line, " " ) + 1;
      printf ( "Doing sub section '%s'\n", arg );
      char subLine[256];
      char subPrecond[256]; subPrecond[0] = '\0';

      //      strcpy ( subLine, arg );


      // char * subLine = strstr ( line, " " ) + 1;
      // char subPrecond[512]; //  = arg ; // strstr ( line, "(" ) + 1;
      strcpy ( subLine, arg );
      if ( isKeyWord ( line, "@forSections" )  )
	{
	  // Must REALLY MAKE THIS CLEANER...
	  char * startPrecond = strstr ( line, "(" );
	  strncpy ( subPrecond, startPrecond + 1, arg - startPrecond - 3  ) ; // , strstr ( arg, " ") - strstr ( arg, "(")  - 2);
	  subPrecond[arg - startPrecond - 3] = '\0';
	  printf ( "Precond '%s'\n", subPrecond);
	  // subPrecond[subLine-arg-2] = '\0';
	}
      char * optionnal = NULL; // This is the @. construction.
      if ( ( optionnal = strstr ( subLine, "@." ) ) != NULL )
	{
	  // char * nextSpace = strstr ( optionnal, " " );
	  //	  memset ( optionnal, ' ', nextSpace - optionnal + 1 );
	  _replace ( subLine, "@.", "" );

	}
      // DMLTransformSection * transSubSec = trans->getSection ( arg );
      list<DMLSection *>::iterator subSecPlus = sections.begin (); subSecPlus++;
      for ( list<DMLSection *>::iterator subSec = sections.begin ();
	    subSec != sections.end () ; subSec ++ )
	{
	  if ( optionnal && subSecPlus == sections.end () )
	    {
	      // subSec is the last element of the list...
	      printf ( "#################### LAST ELEMENT !!!!!!!!!!!\n" );
	      printf ( "optionnal = %s\n", optionnal );
	      strcpy ( subLine, arg );
	      // _replace ( subLine, "@.", "" );
	      char * nextSpace = strstr ( optionnal, " " );
	      memset ( optionnal, ' ', nextSpace - optionnal + 1 );
	      
	    }
	  (*subSec)->transform ( fp_out, trans, subLine, subPrecond ); // transSubSec );
	  subSecPlus++;
	}
      // continue;
      return true;
    }
  if ( isKeyWord ( line, "@do:" ) ) // ( arg = strstr ( line, "@do:" ) ) != NULL )
    {
      arg = line + 4;
      printf ( "doing the sub query : '%s'\n", arg );
      // arg += 4;
      DMLTransformSection * transSubSec = trans->getSection ( arg );
      if ( transSubSec == NULL )
	Bug ( "Could not find subquery : '%s'\n", arg );
      transform ( fp_out, trans, transSubSec, precond );
      return true;
    }
  if ( isKeyWord ( line, "@if" ) 
       || isKeyWord ( line, "@else" )
       || isKeyWord ( line, "@ndif" ) )
    Bug ( "This could not be handled here !!!\n" );
  if ( line[0] == '@' )
    {
      Bug ( "Invalid processing command : '%s'\n", line );
    }
  printf ( "processing normally with line '%s'\n", line );
  transformLine ( buff, line );
  
  fprintf ( fp_out, "%s\n", buff );

  return true;
}


bool DMLSection::transform ( FILE * fp_out, DMLTransform * trans, DMLTransformSection * sec, char * precond )
{
  printf ( "transform : at section %s, doing %s\n",
	   name, sec->name );
  unsigned int line = 0;
  unsigned int onIfs = 0;
  char *buff;
  for ( list<char*>::iterator _line = sec->lines.begin ();
	_line != sec->lines.end () ; _line ++ )
    {
      line ++;
      // char * arg;
      buff = *_line;
      printf ( "at line %d : '%s'\n", line, buff );
      if ( isKeyWord ( buff, "@endif" ) )
	{
	  if ( onIfs == 0 )
	    Bug ( "Unbalanced @if/@endif : onIfs=%d\n", onIfs );
	  onIfs --;
	  continue;
	
	}
      if ( isKeyWord ( buff, "@else" ) )
	{
	  // Must browse to the next @endif
	  //Bug ( "Not Implemented\n" );
	  int curP = 0;

	  for ( ; _line != sec->lines.end () ; _line ++, buff = *_line )
	    {
	      // printf ( "curP %d, line '%s'\n", curP, buff );
	      if ( isKeyWord ( buff, "@if" ) )
		{
		  curP ++;
		}
	      if ( isKeyWord ( buff, "@endif" ) )
		{
		  curP --;
		  if ( curP < 0 )
		    {
		      onIfs --;
		      break;
		    }
		}
	    }
	  if ( _line == sec->lines.end () )
	    Bug ( "Could not find @else or @endif\n" );
	  continue;
	} 
	
      if ( isKeyWord ( buff, "@if" ) )
	{
	  char cond[512];
	  char * condBegin = strstr ( buff, "(" );
	  if ( condBegin == NULL )
	    Bug ( "no ( at @if\n" );
	  strcpy ( cond, condBegin + 1 );
	  cond[strlen(cond) - 1] = '\0';
	  printf ( "Condtion : '%s'\n", cond );
	  transformLine ( cond, cond );
	  printf ( "Transformed Condition : '%s'\n", cond );
	  onIfs++;
	  if ( _eval ( cond ) )
	    {
	      printf ( "@if success.\n" );
	      continue;
	    }
	  else 
	    {
	      printf ( "@if fail\n" );
	      int curP = 0;
	      // Must browse till the next @else or @endif whichever comes first.
	      _line++; buff = *_line;
	      for ( ; _line != sec->lines.end () ; _line ++, buff = *_line )
		{
		  printf ( "onIfs %d, curP %d, line '%s'\n", onIfs, curP, buff );
		  if ( isKeyWord ( buff, "@if" ) )
		    {
		      curP ++;
		    }
		  if ( isKeyWord ( buff, "@else" ) )
		    { 
		      
		      if ( curP == 0 )
			{
			  // onIfs --; 
			  break;
			}
		    }
		  if ( isKeyWord ( buff, "@endif" ) )
		    {
		      curP --;
		      if ( curP < 0 )
			{
			  onIfs --;
			  break;
			}
		    }
		  /*
		  if ( isKeyWord ( buff, "@if" ) ) 
		    {
		      onIfs ++;
		    }
		  if ( isKeyWord ( buff, "@else" ) )
		    {
		      
		      break;
		    }
		  if ( isKeyWord ( buff, "@endif" ) )
		    {
		      onIfs --;
		      break;
		    }
		  */
		}
	      if ( _line == sec->lines.end () )
		Bug ( "Could not find @else or @endif\n" );
	      continue;
	    }
	  exit (0);
	}
      transform ( fp_out, trans, buff, NULL );
    }
  if ( onIfs )
    Bug ( "Function finished without finishing its endif\n" );
}



bool DMLSection::transform ( FILE * fp_out, DMLTransform * trans )
{
  __dumpDMLTransform ( trans );
  DMLTransformSection * _main = trans->getSection ( "main" );// (trans->sections.find ("main"))->second;
  if ( _main == NULL )
    Bug ( "No 'main' section found.\n" );
  return transform ( fp_out, trans, _main, NULL );
}

// #endif 
