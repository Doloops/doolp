#include "DML.h"
#include "glog.h"

/*
  XML Raw high-speed parser.
  States :
  outside : is outside of everything
  header : between <..>
//  paramName
//  paramValue
  footer : between </>


*/

typedef enum xRS_enum
  {
    xRS_Outside,
    xRS_Header,
    xRS_Body
  };

// bool readXMLHeader ( FILE * fp, list<char*> * words, bool * begin )
// {


// }


bool fillParams ( DMLSection * section, list<char *> * words )
{
  char * name, * value;
  while ( words->size () > 0 )
    {
      name = words->front ();
      value = NULL;
      words->pop_front ();
      if ( words->size () > 0 )
	if ( strcmp ( words->front(), "=" ) == 0 )
	  {
	    words->pop_front ();
	    if ( words->size () == 0 )
	      { Fatal ( "Error, bad syntax with '='\n" ); }
	    else
	      {
		value = words->front ();
		words->pop_front ();
	      }
	  }
      Log ( "name '%s' = value '%s'\n", name, value );
      section->addParam ( name, value );

    }
  return true;
}

DMLSection * readDMLSectionXML ( FILE * fp, xRS_enum state ) // Starting at xRS_Oustide...
{
  DMLSection * metaSection = new DMLSection ( "__ROOT__" );
  list <char*> words;
  char term;
  //  xRS_enum state = xRS_Outside;
  char * body;
  //  char * currentSection = NULL;
  list<DMLSection *> sections; // Current Sections Stack.
  sections.push_front ( metaSection );
  DMLSection * curSection;
  Log ( "Starting readDMLSectionXML\n" );
  while ( ! feof ( fp ) )
    {
      Log ( "Current State : %d, Section list Size %d\n", state, sections.size () );
      switch ( state )
	{
	case xRS_Outside:
	  if ( sections.size() > 1 )
	    state = xRS_Body;
	  else
	    {
	      term = seekTo ( fp, "<" );
	      state = xRS_Header;
	    }
	  break;
	case xRS_Header:
	  term = readWords ( fp, &words, " \n\t", "=", "/>", "" ); // End of tag
	  dumpWords ( &words );
	  Log ( "term : %c\n", term );
	  if ( words.size () > 0 )
	    if ( words.front()[0] == '?' )
	      {
		Info ( "a <?..?> flag\n" );
		state = xRS_Outside;
		break;
	      }
	  if ( term == '/' )
	    {
	      if ( words.size () == 0 )
		{
		  seekTo ( fp, ">", &body );
		  Log ( "Section end : '%s'\n", body );

		  curSection = sections.front ();
		  Log ( "Shall terminate : '%s'\n", curSection->getName () );
		  
		  if ( strcmp ( body, curSection->getName () ) != 0 )
		    Fatal ( "Bad syntax.\n" );
		  sections.pop_front ();
		  state = xRS_Outside;
		  break;
		}
	      else
		{
		  Log ( "Direct <../>\n" );
		  term = seekTo ( fp, ">" );
		  state = xRS_Outside;
		  curSection = sections.front ()->addSection ( words.front () );
		  words.pop_front ();
		  fillParams ( curSection, &words );
		  break;
		}
	    }

	  // currentSection = words.front ();
	  curSection = sections.front ()->addSection ( words.front () );
	  words.pop_front ();
	  fillParams ( curSection, &words );

	  sections.push_front ( curSection );
	  Log ( "Section %s has a body.\n", words.front() );
	  
	  // Read inside of the body of the truc.
	  state = xRS_Body;
	  break;
	case xRS_Body:
	  body = NULL;
	  term = seekTo ( fp, "<", &body );
	  Log ( "body at %p\n", body );
	  if ( body )
	    {
	      Log ( "body is %s\n", body );
	      sections.front()->addTextSection ( body, "ENCODING ?" );
	      
	    }
	  state = xRS_Header;

	  break;
	default:
	  Fatal ( "No state to handle this !\n" );
	}

    }






  /*
  term = readWords ( fp, &words, "" , "<", "" );
  if ( term != '<' )
    Fatal ( "Could not find beginning of doc\n" );
  
  
  term = readWords ( fp, &words, " ", "/>", "" );
  printf ( "First word : '%s'\n", words.front() );
  //  if ( words.front ()[0] != '?' )
  if ( strcmp ( words.front(), "?xml" ) == 0 )
    Info ( "Got the xml tag.\n" );
  
  dumpWords ( &words );

  term = readWords ( fp, &words, "", "<", "" ); // Beginnning
  
  term = readWords ( fp, &words, " ", "/>", "" ); // End of tag

  */
  
    //    if ( term == '/' )
    //      term = readWords ( fp, &words, "", ">", "" );



  return metaSection;
}

DMLSection * readDMLSectionXML ( FILE * fp )
{ return readDMLSectionXML ( fp, xRS_Outside ); }
