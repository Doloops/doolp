#include <DML.h>

#include "DMLTest.h"

int DMLTest ( int argc, char ** argv )
{
  Log ( "Starting DMLTest\n" );
  
  FILE * fp = fopen ( "test1.xml", "r+" );
  if ( fp == NULL )
    Bug ( "Could not open XML source file\n" );

  DMLSection * plouf = readDMLSectionXML ( fp );
  fclose ( fp );


}
