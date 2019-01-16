#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glogers.h>
#include <DMLXDocument.h>

setGlog ("dmlxTransform");


int main ( int argc, char ** argv )
{
  addGlog ( new Gloger_File ( stderr ) );
  DMLX::ParserFile parserXsl ( argv[1] );
  DMLX::Document docXsl;
  docXsl.buildFromParser ( parserXsl );

  docXsl.log();

  DMLX::ParserFile parserSrc ( argv[2] );
  parserSrc.setParseKeepAllText ( true );
  DMLX::Document docSrc;

  docSrc.buildFromParser ( parserSrc );
  Log ( "Transforming...\n" );
  docXsl.log();
  docSrc.transform ( STDOUT_FILENO, &docXsl );
  Log ( "Transformed.\n" );
  return 0;
}
