#include <doolp.h>
#include <doolp/doolp-doolppubsub.h>

#include <glogers.h>

#include "car.h"

setGlog ( "sampleA" );

extern "C"
{
  bool registerDoolpObjects(Doolp::Forge * forge);
}

class NotifyCars : public Doolp::StreamNotifiable<Car*>
{
public:
  NotifyCars() {}
  bool notifyStream ( Car *& c )
  {
    Log ( "New car '0x%x'\n", c->getObjectId() );
    //    delete ( c );
    c->getForge()->removeObject ( c );
    return false;
  } 
};


int main (int argc, char ** argv)
{
  addGlog ( new GlogInfo_File ( stdout ) );
  //  showGlogPrior ( __GLOG_PRIOR_LOG, false );

  Doolp::Forge * myForge = new Doolp::Forge ( 0 );
  /*
  myForge->options.useWaitSpecificCall = true ;
  myForge->options.forceJobsInSameThread = true;
  */
  registerDoolpObjects ( myForge );
  myForge->tryConnect ( "doolpxml://127.0.0.1:13816" );

  Log ( "---------------\n" );

  Doolp::PubSubByType<Car> pubSub (myForge);
  pubSub.setNotify ( new NotifyCars() );
  string noOptions = "";
  pubSub.subscribe ( noOptions );
  //  Car * car;
  for (;;)
    {
      /*
      if ( ( car = pubSub.notify () ) != NULL )
	{
	  Log ( "Got News : new car '0x%x'\n", car->getObjectId () );
	  delete ( car );
	}
      */
      Log ( "Still Alive.\n" );
      myForge->logStats ();
      sleep ( 1 );
    }
  return 0;
}
