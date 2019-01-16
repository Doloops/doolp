#ifndef __DOOLP_DOOLPPUBSUB
#define __DOOLP_DOOLPPUBSUB

#include <doolp/doolp-doolpstream-notify.h>
#include <doolp/doolp-doolpcall.h>
#include <doolp/doolp-doolppubsub-generic.h>

namespace Doolp
{
  /*
   * Publish and Subscribe : by Type (shall be a DoolpObject)
   */
  template<class T_Object>
  class PubSubByType
    {
    protected:
      Forge * myForge;
      StreamWithNotify<Object*> stream;
      bool subscribed;
      ObjectNameId getObjectNameId()
      { return T_Object::getNameIdStatic(); }

    public:
      PubSubByType ( Forge * _forge ) { myForge = _forge; subscribed = false; }

      void setNotify ( StreamNotifiable<T_Object*> * n )
      {
	stream.setNotifiable ( (StreamNotifiable<Object*>*) n );
      }
      
      bool subscribe ( string & options )
      {
	PubSubGeneric * pubSub = (PubSubGeneric*) myForge->getService_char ( "PubSub" );
	string subscription;
	char subscription_str[48];
	sprintf ( subscription_str, "<filter><nameId equals=\"0x%x\"/></filter>", getObjectNameId() );
	subscription = subscription_str;
	Call * subscribeCall = pubSub->subscribe.callAsync ( &stream, subscription, options );
	subscribeCall->serializeFromStepping = 0; // Full Serialize of objects.
	subscribed = true;
	return true;
      }
      bool unsubscribe ()
      {
	if ( subscribed == false )
	  return false;
	stream.setFinished ();
	return true;
      }

      bool publish ( T_Object * obj )
      {
	if ( subscribed == false )
	  { Warn ( "Not Subscribed !\n" ); return false; }
	stream.push ( obj );
	stream.flush ();
	return true;
      }
      T_Object * notify () 
	{
	  if ( ! stream.canPop () )
	    return NULL;
	  Object * obj;
	  stream.pop ( obj );
	  return dynamic_cast<T_Object*> ( obj );
	}
    };

}; // NameSpace Doolp

#endif // __DOOLP_DOOLPPUBSUB
