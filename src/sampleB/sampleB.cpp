#include <doolp.h>

#include <doolp/doolp-doolppubsub.h>
#include <doolp/doolp-doolppubsub-filter.h>
#include <doolp/doolp-doolpobjectbuffer.h>
#include <car.h>
#include <glogers.h>

#define DoolpPort 13815
#define DoolpPortXML 13816

setGlog ( "sampleB" );

// #define asyncSwitchOn
// #define nbCalls 100

extern "C"
{
  bool registerDoolpObjects(Doolp::Forge * forge);
}

Doolp::Forge * initForge ( )
{
  Doolp::Forge * myForge = new Doolp::Forge ( 0 ); // 3
  Log ( "myForge at %p\n", myForge );
  registerDoolpObjects ( myForge );
  myForge->tryConnect ( "doolpxml://127.0.0.1:13816" );
  if ( ! myForge->bindContext ( "mainContext" ) )
    {
      /*
      string mainContextName = "mainContext";
      Doolp::FullContextId * mainContext = myForge->getPersistance()->getNewContext();
      myForge->getNaming()->setNamedContext ( mainContextName, mainContext );
      if ( ! myForge->bindContext ( "mainContext" ) )
	{
	  Bug ( "Could not create mainContext !\n" );
	}
      */
      Bug ( "Could not create mainContext !\n" );
    }
  //  DoolpConnectionXML * xmlConn = new DoolpConnectionXML ( myForge, "connectionLog.in.bin", "connectionLog.out.bin" );
  //  if ( ! xmlConn->tryConnect ( "127.0.0.1", DoolpPortXML ) )
  //    {
  //      Fatal ( "Could not connect.\n" );
  //    }
  return myForge;
}

Car * createCar ( Doolp::Forge * myForge ) 
{
  Car * myCar = new Car ();
  myCar->name = new string ( "Bubble" );
  myCar->serial = "00-AA" ;
  myCar->switchedOn = false;
  myCar->oilGauge = 0.0f;
  myCar->fuelGauge = 0.1f;
  myForge->addObject ( myCar );
  return myCar;
}

class NotifyCars : public Doolp::StreamNotifiable<Car*>
{
public:
  NotifyCars() {}
  bool notifyStream ( Car *& c )
  {
    Log ( "New car '0x%x'\n", c->getObjectId() );
    return false;
  } 
};

void testPubSubFilter ()
{
  string s = 
    "<filter>"
    "<or>"
    "<and>"
    "<objectId equals=\"0x56545\"/><nameId equals=\"0x34343\"/>"
    "</and>"
    "<and>"
    "<objectId equals=\"0x7458\"/><nameId equals=\"0xab7858\"/>"
    "</and>"
    "<objectId equals=\"0x5854\"/>"
    "<parameter nameId=\"0x2304\" equals=\"Hello\" type=\"string\"/>"
    "</or>"
    "</filter>";
  Doolp::PubSubFilter * filter = Doolp::PubSubFilter::buildFilter ( s );

  Doolp::ObjectBuffer o ( 0x56545, 0x34343 );
  Log ( "Filter : '%d'\n", filter->filter ( &o ) );

  Doolp::ObjectBuffer o2 ( 0x7458, 0xab7858 );
  Log ( "Filter : '%d'\n", filter->filter ( &o2 ) );

  Doolp::ObjectBuffer o3 ( 0x25, 0x28 );
  char * h = "Hello";
  Doolp::ObjectBufferParam p = { Doolp::ObjectBufferParam::type_string, (void*)h, strlen(h) + 1 };
  o3.params[0x2304] = &p;
  Log ( "Filter : '%d'\n", filter->filter ( &o3 ) );

}

int testPubSub ( Doolp::Forge * myForge )
{
  Doolp::PubSubByType<Car> pubSub (myForge);
  pubSub.setNotify ( new NotifyCars() );
  string noOptions = "";
  pubSub.subscribe ( noOptions );

  for ( int i = 0 ; i < 10 ; i++ )
    {
      Car * myCar = createCar ( myForge );
      pubSub.publish ( myCar );
      // sleep ( 1 );
    }
  return 0;
}

int main ( int argc, char ** argv )
{
  addGlog ( new Gloger_File ( stdout ) );
  //  showGlogPrior ( __GLOG_PRIOR_LOG, false );
  Doolp::Forge * myForge = initForge ();
  Log ( "Forge at %p\n", myForge );
  for ( unsigned i = 0 ; i < 100 ; i++ )
    {
      Car * myCar = createCar ( myForge );
      Radio * r = new Radio ();
      myForge->addObject ( r );
      myCar->myRadio = r;
      Log ( "1. r2->getObjectId() = 0x%x\n", myCar->myRadio->getObjectId() );
      //      myCar->myRadio.cleanLinks();
      //      Log ( "2. r2->getObjectId() = 0x%x\n", myCar->myRadio->getObjectId() );
      //  Log ( "ObjectId : '0x%x'\n", myCar->myRadio.objectId );
      myForge->commit ();
      //  Log ( "ObjectId : '0x%x'\n", myCar->myRadio.objectId );
      delete ( r );
      //  Log ( "ObjectId : '0x%x'\n", myCar->myRadio.objectId );
      Log ( "3a. object at %p\n", myCar->myRadio.getObject() );
      Log ( "3b. objectId 0x%x\n", myCar->myRadio->getObjectId() );
      //  Log ( "3. r2->getObjectId() = 0x%x\n", myCar->myRadio->getObjectId() );
    }
  //  myCar->myRadio.unset();
  //  delete
  return 0;
}


#if 0 // GARBAGE
bool Car__switchOn__getResult ( DoolpCall * call );

int test100Calls ( int argc, char ** argv )
{
  DoolpForge * myForge = initForge ();
  Car * myCar = new Car ();
  myCar->name = new string ( "Bubble" ) ;
  myCar->serial = "00AA" ;
  myCar->switchedOn = false;
  myCar->oilGauge = 0.0f;
  myCar->fuelGauge = 0.1f;
  myForge->addObject ( myCar );

#ifdef asyncSwitchOn
  DoolpCall * trySwitch[nbCalls];
#endif 
  for ( unsigned int call = 0 ; call < nbCalls ; call ++ )
    {
#ifdef asyncSwitchOn
      myForge->setAsyncCall ( &trySwitch[call] );
      myCar->switchOn ( call );
#else
      bool res = myCar->switchOn ( call );
      Log ( "switchOn #%d gave result = %d\n", call, res );
      //      res = myCar->switchOff (  );
      //      Log ( "switchOff #%d gave result = %d\n", call, res );
#endif
    }
#ifdef asyncSwitchOn
  for ( unsigned int call = 0 ; call < nbCalls ; call ++ )
    {
      
      Log ( "waiting for call n %d, at %p\n", call, trySwitch[call] );
      myForge->waitCall ( trySwitch[call] );
      bool res = Car__switchOn__getResult ( trySwitch[call] );
      Log ( "switchOn %p gave result = %d\n", call, res );
    }
#endif 
  usleep ( 10 );
  // conn->endConnection ( );
  myForge->close ();
  Info ( "sampleB OK\n" );
  return 0;
}


Car * testRecieveObject ( DoolpForge * myForge )
{
  string mainContextName ( "mainContext" );
  DoolpFullContextId * mainContext;
  Car * myCar;
  string carName ( "MySuperCar" );  

  if ( false )
    {
      mainContext = myForge->getPersistance()->getNewContext();
      myForge->getNaming()->setNamedContext ( mainContextName, mainContext );
      __DOOLP_Log ( "Got persistance context [%d,%d,%d]\n",
		    logDoolpFullContextId ( mainContext ) );
    }

  mainContext = myForge->getNaming()->getNamedContext ( mainContextName );
  myForge->setContextFather ( mainContext ); // encapsulate in bindToContext ( string &contextName )
  DoolpObjectId myCarId = myForge->getNaming()->getNamedObject ( mainContextName, carName );
 
  //  myCar = (Car*) myForge->getPersistance()->retrieveObject ( mainContext, myCarId );
  //  myCar = (Car*) myForge->getObject ( mainContext, myCarId );
  DoolpCall * call = myForge->getObject.callAsync ( mainContext, myCarId );
  __DOOLP_Log ( "Waiting call reply for getObject() !!\n" );
  myCar =  (Car*) myForge->getObject.getResult ( call );
  __DOOLP_Log ( "myCar has name '%s'\n", ((string*)myCar->name)->c_str() );

  return (Car*) myCar;
}

bool testCommitObject ( DoolpForge * myForge, DoolpObject * obj, string& contextName, string& objName )
{
  //  myForge->getNaming()->setNamedObject ( contextName, objName, obj->getObjectId() );
  myForge->bindContext ( contextName );
  myForge->addObject ( obj );
  myForge->commit ( );
#if 0
  DoolpFullContextId * ctxt = myForge->getNaming()->getNamedContext ( contextName );
  myForge->getNaming()->setNamedObject ( contextName, objName, obj->getObjectId() );
  DoolpStream<DoolpObjectId> deletedObjectsStream;
  DoolpStream<DoolpObject*> objectsStream;
  deletedObjectsStream.setFinished ();
  DoolpCall * commitCall =  myForge->getPersistance()->commit.callAsync ( ctxt, &deletedObjectsStream, &objectsStream );
  objectsStream.push ( (DoolpObject*)obj );
  objectsStream.setFinished ();
  bool res = myForge->getPersistance()->commit.getResult ( commitCall );
  Info ( "Commit %s\n", res ? "Success" : "Failed" );
#endif
  return true;
}

bool testCommit ( DoolpForge * myForge, Car * myCar )
{
  string mainContext = "mainContext";
  string carName = "MySuperCar";
  myForge->bindContext ( mainContext );

  myForge->getNaming()->setNamedObject ( mainContext, carName, myCar->getObjectId() );
  myForge->commit ( );
  myCar->oilGauge = 5.25f;
  myCar->serial = "77-FF";
  myForge->commit ();
  //  testCommitObject ( myForge, myCar, mainContext, carName );
  testRecieveObject ( myForge );

}

int main ( int argc, char ** argv )
{
  addGlog ( new GlogInfo_File ( stdout ) );
  //  showGlogPrior ( __GLOG_PRIOR_LOG, false );

  //  addGlog ( new GlogInfo_XMLSock ( "sampleB", "127.0.0.1", 13807 ) );
  //  dmalloc_log_unfreed ();
  DoolpForge * myForge = initForge ();
  Log ( "Forge at %p\n", myForge );
  Car * myCar = createCar ( myForge );

  bool res = myCar->switchOn ( 23 );
  Info ( "switchOn gave result %s\n", res ? "true" : "false" );
  //  testCommit ( myForge, myCar );
  return 0;
  //  Car * myCar;
  //  testRecieveObject ( myForge );
  //  Car * myCar = testRecieveObject ( myForge );
  /*
  return 0;

  Car * myCar = new Car ();
  myCar->name = new string ( "Bubble" );
  myCar->serial = "00AA" ;
  myCar->switchedOn = false;
  myCar->oilGauge = 0.0f;
  myCar->fuelGauge = 0.1f;
  myForge->getObjectCache()->add ( myCar );
  */
  /*
    DoolpStream<DoolpObject*> objectsStream;
    objectsStream.push ( myCar );
    objectsStream.setFinished ();
    myForge->getPersistance()->commit ( mainContext, &objectsStream );
  */

  //  delete ( myCar );
  Log ( "car : objectId=%d\n", myCar->getObjectId() );

  //  while ( true ) {}

  //  myCar->switchOn (5);

  // return true;
  DoolpStream<int> * radio_freq = new DoolpStream<int>;
  DoolpStream<char *> * radio_name = new DoolpStream<char *>;
  DoolpCall * getRadioCall;

  try
    {
      myForge->setAsyncCall ( &getRadioCall );  
      myCar->getRadioList ( radio_freq, radio_name );
      int i = 0, j = 0;

      for ( i = 0 ; i < 10 ; i++ )
	radio_freq->push ( 4215 ); 
      radio_freq->push ( 7456 );
      // radio_freq->flush ();
      
      radio_freq->setFinished ( );
      i = 0;
      int nbtests = 5;
      while ( ! radio_name->isFinished () )
	{
	  char * name = NULL;
	  radio_name->pop ( name );
	  j++;
	  Info ( "%d/%d : ################# %p %s ##\n", i, j, name, name );
	  /*
	    if ( i < nbtests )
	    {
	    radio_freq->push ( 7456 );
	    i++;
	    if ( i == nbtests )
	    {
	    radio_freq->setFinished ( );
	    i++;
	    }
	    }
	  */
	  
	}
      myForge->waitCall ( getRadioCall );
    }
  catch ( DoolpException * e )
    {
      __DOOLP_Log ( "Recieved exception 0x%x : '%s'\n",
		    e->getExceptionId (), e->getMessage () );
    }

  Info ( "sampleB finished, killing myForge.\n" );
  //  sleep ( 1 );
  //  usleep ( 20 );
  myForge->close ();
  delete ( myForge );
  Info ( "sampleB OK\n" );
  // Info ( "dmalloc_log_unfreed\n" );
  //  dmalloc_log_unfreed ();
  // Info ( "dmalloc_shutdown\n" );
  // dmalloc_shutdown ();
  return 0;
}


/*
  string d ( "dummy" );
  myForge->getServices()->add ( d, new DoolpForgeService () );
  //  d = new string ( "dummy ");
  string * e = new string ( "dummy" );
  string &g = *(new string ( "dummy" ) );
  __DOOLP_Log ( "has dummy : %d\n", myForge->getServices()->hasService ( *(new string ( "dummy" ) ) ) );

  return 0;


*/
#endif // 0 : GARBAGE
