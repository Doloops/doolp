#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <DMLXDocument.h>
#include <glog.h>
#include <glogers.h>
#include <parseCtags.h>
#include <generateDML.h>
#include <generateCPP.h>
#include <crc32.h>

#include <list>

#define __DUMP_PARSEDCTAGS

#define __DOOLPCC_HAS_BUILTIN_GENERATECPP_XSL

#ifdef __DOOLPCC_HAS_BUILTIN_GENERATECPP_XSL
DMLX::Document * getDMLXDocument_generateCPP();
#endif // __DOOLPCC_HAS_BUILTIN_GENERATECPP_XSL

/*
  Three steps in Preproc mechanims :
  1. Header parsing (using --headers [list of headers]) -> DML generation
  2. Code parsing (using --headers [list of cpp]) -> DML alteration
  3. XSL Transform
 */

setGlog ( "doolpCC" );


bool Mode_stdRPC;

void printWelcome ( )
{
  fprintf ( stderr,  "doolpCC [Version 0.7.5-b (doolp-1)], with DMLX support.\n" );
}

void printUsage ( )
{
  /*
    Syntax
  */
  fprintf ( stderr,  "Usage :\n" );
  fprintf ( stderr,  "\tdoolpCC [--stdRPC] [--toDML file.dml] [--onlyDML]\n"
	   "\t\t--headers 1.h 2.h ... --cpp 1.cpp 2.cpp ...\n" );
  fprintf ( stderr,  "\n" );
}

int main (int argc, char ** argv)
{
  addGlog ( new Gloger_File ( stderr ) );
  list<char *> headers;
  list<char *> cpps;
  char * toXML = "doolpCC.__doolp.xml";
  char * fromXSL = NULL;
  char * DoolpObjectsInfoName = "getDoolpObjectsInfo" ;
  bool DoolpObjectsInfoExtern = true;
  bool Mode_toXML   = true;
  // bool Mode_fromDML = false;
  // bool Mode_onlyDML = false;
  bool Mode_dumpParsedCtags = false;
  crc32_init ();
  
  printWelcome ( );
  Info ( "argc=%d\n", argc );
  if ( argc == 1 )
    { 
      Warn ( "Not enough parameters\n" ); 
      printUsage ( );
      return -1;
    }
#define isOption(__what__) ( strcmp ( argv[arg] + 2, __what__) == 0 )
  for ( int arg = 1 ; arg < argc ; arg ++ )
    {
      if ( strlen ( argv[arg] ) < 4 || 
	   ! ( ( argv[arg][0] == '-' ) && ( argv[arg][1] == '-' ) ) )
	{
	  Fatal ( "Wrong parameter : %s\n", argv[arg] );
	}
      if ( isOption ( "dumpParsedCtags") )
	{ Mode_dumpParsedCtags = true; continue ; }
      if ( isOption ( "stdRPC" ) )
	{ Mode_stdRPC = true; continue; }
      if ( isOption ( "transform" ) )
	{
	  arg++;
	  fromXSL = argv[arg++];
	  // Bug ( "NOT IMPLEMENTED...\n" );
	}
      if ( isOption ( "headers" ) )
	{
	  arg++;
	  while ( arg < argc && argv[arg][0] != '-' ) headers.push_back ( argv[arg++] );
	  arg--;
	  continue; 
	}
      if ( isOption ( "cpp" ) )
	{
	  arg++;
	  while ( arg < argc && argv[arg][0] != '-' )
	    cpps.push_back ( argv[arg++] );
	  arg--;
	  continue;
	}
      if ( isOption ( "DoolpObjectsInfoName" ) )
	{
	  arg++;
	  DoolpObjectsInfoName = argv[arg];
	  continue;
	}
      if ( isOption ( "DoolpObjectsInfoExtern" ) )
	{
	  DoolpObjectsInfoExtern = true;
	  continue;
	}
      if ( isOption ( "DoolpObjectsInfoNotExtern" ) )
	{
	  DoolpObjectsInfoExtern = false;
	  continue;
	}

      Fatal ( "Wrong parameter : %s\n", argv[arg] );
    }
#undef isOption

  list<char*>::iterator f;
  Log ( "%d Headers :\n", headers.size () );
  for ( f = headers.begin () ; f != headers.end () ; f++ )
    Log ( "\t%s\n", *f);
  Log ( "%d CPP Files :\n", cpps.size () );
  for ( f = cpps.begin () ; f != cpps.end () ; f++ )
    Log ( "\t%s\n", *f );
  Log ( "\n" );


  // Parsing Headers
  list<ctagsLine> * parsedCtags_headers = parseCtags ( &headers ); // &(argv[argc_cur]), nbHeaders );

  if ( Mode_dumpParsedCtags )
    {
      FILE * dumpH = fopen ( "doolpCC.headers.ctags.dump", "w" );
      dumpCtags ( dumpH, parsedCtags_headers );
      fclose ( dumpH );
    }

      
  list<char *> baseClasses;
  baseClasses.push_back ( "Doolp::Object" );
  baseClasses.push_back ( "Doolp::Exception" );
  
  vector<classHierarchy*> * hierarchy = 
    generateDML_buildClassHierarchy ( parsedCtags_headers , &baseClasses );

  DMLX::Document dml;
  DMLX::Node * metaSection = generateDML ( parsedCtags_headers, hierarchy );
  metaSection->log ();
  dml.addNode ( metaSection );

  DMLX::Keyword keyw_generateDoolpObjectsInfo("generateDoolpObjectsInfo");
  DMLX::Keyword keyw_DoolpObjectsInfoName("DoolpObjectsInfoName");
  DMLX::Keyword keyw_DoolpObjectsInfoExtern("DoolpObjectsInfoExtern");
  
  metaSection->addAttribute ( keyw_generateDoolpObjectsInfo, (char*)"true" );
  metaSection->addAttribute ( keyw_DoolpObjectsInfoName, DoolpObjectsInfoName );
  metaSection->addAttribute ( keyw_DoolpObjectsInfoExtern, DoolpObjectsInfoExtern ? "true" : "false" );
  
  // Parsing Codes
#if 0
  if ( cpps.size () > 0 )
    {
      Warn ( "Parsing CPP files is DEPRECATED !!!\n" );
      list<ctagsLine> * parsedCtags_code = parseCtags ( &cpps ); // &(argv[argc_cur]), argc - argc_cur );
      if ( parsedCtags_code != NULL )
	checkCode ( metaSection, parsedCtags_code );
    }
  else
    {
      Warn ( "No CPP Files Provided.\n" );
    }
  generateDML_Ids ( metaSection );
#endif
  if ( Mode_toXML )
    {
      creat ( toXML, S_IRUSR | S_IWUSR );
      int fp_toXML = open ( toXML, O_WRONLY );
      DMLX::Writer writer(fp_toXML, false);
      dml.write ( writer );
    }
  DMLX::Document * transform = NULL;
  if ( fromXSL )
    {
      DMLX::ParserFile parserxml( fromXSL );
      transform = new DMLX::Document;
      transform->buildFromParser ( parserxml );
      //      Bug ( "Not implemented.\n" );
    }
#ifdef __DOOLPCC_HAS_BUILTIN_GENERATECPP_XSL
  else
    {
      transform = getDMLXDocument_generateCPP();
    }
#endif
  AssertFatal ( transform != NULL, "Could not get the Transform !\n" );
  dml.transform ( STDOUT_FILENO, transform );
  return 0;
}
