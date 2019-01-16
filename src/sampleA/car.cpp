#include <carImplemented.h>


bool CarImplemented::doolpfunclocal(switchOn) (unsigned int)
{
  return true;
}
bool CarImplemented::doolpfunclocal(switchOff) (unsigned int)
{
  return true;
}
bool CarImplemented::doolpfunclocal(getRadioList) (Doolp::Stream<int>*,Doolp::Stream<char*>*)
{
  return true;
}


#if 0

bool Car::switchOn ( unsigned int key )
{
  Log ( "switchOn with key %d\n", key );
  if ( switchedOn )
    {
      Log ( "Already switched On ! key=%d\n", key );
      return false;
    }
  switchedOn = true;
  //  setAlteredParam ( this, switchedOn );
  return true;
}
bool Car::switchOff ( )
{
  if ( ! switchedOn )
    return false;
  switchedOn = false;
  //  setAlteredParam ( this, switchedOn );
  //  setAlteredParam ( this, switchedOn );

  return true;
}

doolpable bool Car::getRadioList (DoolpStream<int> * freq,
				  DoolpStream<char *> * name)
{
  __DOOLP_Log ( "Starting !!!\n" );
  while ( ! freq->isFinished () )
    {
      //      int _freq = freq->pop ();
      int _freq; freq->pop ( _freq );
      __DOOLP_Log ( "Freq : %d\n", _freq );
      switch ( _freq )
	{
	case 4215:
	  name->push ( "Radio Future" );
	  break;
	case 7456:
	  name->push ( "Radio Past" );
	  break;
	default:
	  name->push ( "No idea" );
	}
      __DOOLP_Log ( "Waiting for new freq...\n" );
    }
  __DOOLP_Log ( "finished\n" );
  // name->setFinished ();
  return true;
}
#endif
