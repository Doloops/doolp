#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpcontext.h>
#include <doolp/doolp-doolppubsub-broker.h>

Doolp::PubSubBroker::PubSubBroker()
{
  pthread_mutex_init ( &_lock, NULL );
}

bool Doolp::PubSubBroker::addSubscription ( Stream<Object*>* stream, string & subscription, string& options )
{
  lock ();
  Subscription * sub = new Subscription ( subscription, options );
  sub->filter = PubSubFilter::buildFilter ( subscription );
  if ( sub->filter == NULL )
    {
      delete (sub);
      return false;
    }
  subscriptions.put ( stream, sub );
  unlock ();
  return true;							
}

bool Doolp::PubSubBroker::removeSubscription ( Stream<Object*>* stream )
{
  if ( ! subscriptions.has ( stream ) )
    return false;
  delete ( subscriptions.get ( stream ) );
  subscriptions.remove ( stream );
  return true;
}

bool Doolp::PubSubBroker::Subscription::isValid ( Object * obj )
{
  return filter->filter ( obj );
}

bool Doolp::PubSubBroker::distribute ( Object * obj )
{
  lock ();
  for ( strongmap<Stream<Object*>*,Subscription*>::iterator subscription = subscriptions.begin ();
	subscription != subscriptions.end () ; subscription++ )
    {
      if ( subscription->second->isValid ( obj ) )
	{
	  subscription->first->push ( (Object*)obj );
	  subscription->first->flush ();
	}
    }
  unlock ();
  return true;
}

bool Doolp::PubSubBroker::doolpfunclocal(subscribe) (Stream<Object*>* stream,string& subscription,string& options)
{
  string pubSubName = "PubSub";
  if ( ! addSubscription ( stream, subscription, options ) )
    {
      throw new PubSubCouldNotSubscribe();
    }
  while ( true )
    {
      Log ( "In Subscription\n" );
      if ( ! getForge()->getContext()->backgroundWork ( stream ) )
	break;
      if ( stream->isFinished () )
	break;
      if ( ! stream->canPop () )
	{
	  Warn ( "backgroundWork gave us hand, but we can not pop...\n" );
	  continue;
	}
      Object * obj;
      stream->pop ( obj );
      Log ( "Got Object '0x%x'\n", obj->getObjectId() );
      distribute ( obj );
    }
  removeSubscription ( stream );
  return true;
}
