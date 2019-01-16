#include <doolp/doolp-doolppubsub-generic.h>
#include <doolp/doolp-doolppubsub-filter.h>

namespace Doolp
{
  /*
   * Simple Broker
   */ 
  class PubSubBroker : public PubSubGeneric
  {
#define DoolpObject_CurrentObject PubSubBroker
    DoolpObject_stdFeatures ();
    DoolpObject_Option ( forceObjectAs, PubSubGeneric );
    pthread_mutex_t _lock;

    void lock () { pthread_mutex_lock ( &_lock ); }
    void unlock () { pthread_mutex_unlock ( &_lock ); }

    class Subscription
    {
    public:
      string subscription;
      string options;
      PubSubFilter * filter;
      Subscription ( string & _subscription, string& _options )
	{ subscription = _subscription; 
	  options = _options; 
	  filter = NULL;
	}
      bool isValid ( Object * obj );
    };
    strongmap<Stream<Object*>*,Subscription*> subscriptions;

    bool addSubscription ( Stream<Object*>* stream, string & subscription, string& options );
    bool removeSubscription ( Stream<Object*>* stream);

    bool distribute ( Object * obj );

  public:
    PubSubBroker();

    bool doolpfunclocal(subscribe) (Stream<Object*>*,string&,string&);

    void setOptions () 
    {
      contextDependant = false;
      ttl = 0;
      forceCallsTOOwner = true;
    }
  };
};
