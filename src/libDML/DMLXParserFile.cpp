#include <DMLXParser.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

DMLX::ParserFile::ParserFile( char * file )
{
  fp = fopen ( file, "r" );
  if ( fp == NULL )
    {
      Warn ( "Could not open file '%s'.\n", file );
      throw new DMLX::ParserException ( "Could not open file.\n" ); 
    }
    //    Fatal ( "DMLX::ParserFile : Can not read input file '%s'\n", file );
}

DMLX::ParserFile::~ParserFile()
{
  if ( fp != NULL )
    fclose ( fp );
}

unsigned int DMLX::ParserFile::fillBuffer ( void * buff, int max_size )
{
  //  Log ( "Trying to refill\n" );
  if ( fp == NULL )
    throw new DMLX::ParserException ( "Input file not openned.\n" ); 

    //    return 0;
  int rd = fread ( buff, 1, max_size, fp );
  //  Log ( "Refilled %d of %d\n", rd, max_size );
  if ( rd == 0 )
    fp = NULL;
  return rd;
}

bool DMLX::ParserFile::canFill ()
{
  if ( fp == NULL )
    {
      //      Warn ( "fp is NULL !\n" );
      return false;
    }
  if ( feof (fp ) )
    {
      //      Warn ( "feof reached : %d bytes parsed yet\n", parsing.totalParsed );
      fclose ( fp );
      fp = NULL;
      return false;
    }
  return true;
}
