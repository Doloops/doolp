
#include <DML.h>


int main ( int argc, char ** argv )
{
  //  DMLSection * plouf = new DMLSection ( "toto" );
  Info ( "Starting. \n" );
  FILE * fp = fopen ( "UT/test1.xml", "r" );
  DMLSection * plouf = readDMLSectionXML ( fp );
  Log ( "Plouf at : %p\n", plouf );

  plouf->writeXML ( stdout );
  fclose ( fp );
}
