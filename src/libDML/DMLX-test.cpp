#include <glogers.h>
#include <DMLXParser.h>
#include <DMLXDocument.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifdef __DMLX__TEST__

setGlog ( "DMLX-test" );

int main ( int argc, char ** argv )
{
  addGlog ( new GlogInfo_File ( stderr ) );
  DMLX::ParserFile parserxml( argv[1] );
  //  return 0;
  DMLX::Document docxml;
  docxml.buildFromParser ( parserxml );
  docxml.log(); // return 0;
  //  {DMLX::Writer dumpw ( STDOUT_FILENO, false );
  //  docxml.write ( dumpw );}
  //  return 0;
  return 0;
  DMLX::ParserFile parserxsl ( argv[2] );
  DMLX::Document docxsl;

  docxsl.buildFromParser ( parserxsl );

  return 0;
  //  docxml.log();
  //  docxsl.log();
  //  {DMLX::Writer dumpw ( STDOUT_FILENO, false );
  //    docxsl.write ( dumpw );}
  Log ( "docxml=%p, docxsl=%p\n", &docxml, &docxsl );
  std::list<DMLX::Keyword *> * l = parserxsl.getHashTree()->getKeywordList ();
  for ( std::list<DMLX::Keyword *>::iterator k = l->begin () ;
	k != l->end () ; k++ )
    {
      Log ( "Keyword 0x%x : '%s'\n", (*k)->getHash(), (*k)->getKeyword() );
    }
  //  creat ( "test.out", S_IRUSR | S_IWUSR );
  //  int fd = open ( "test.out", O_WRONLY );
  docxml.transform ( STDOUT_FILENO, &docxsl );
  //  fsync ( fd );
  //  close ( fd );
}
#endif
