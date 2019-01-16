#include <doolp.h>

/*
DoolpObjectInfo obj =
  {
    "Toto", 1, NULL,
    { &{ 1, "Alfred", NULL, NULL, NULL } }
    , 1
   
  };
*/
int main ( int argc, char ** argv )
{

  DoolpForge * myForge = new DoolpForge (1);
  DoolpConnectionTCP * conn = new DoolpConnectionTCP ( myForge );
  conn->openServer ( "127.0.0.1", 13815 );
  // sleep (1);
  for (;;)
    {
      myForge->logStats ();
      sleep (1) ;
    }
  delete ( conn );

}
