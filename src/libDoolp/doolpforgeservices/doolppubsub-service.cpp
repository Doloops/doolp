#include <doolp/doolp-doolppubsub-service.h>

Doolp::PubSubService::PubSubService ( Doolp::Forge * _forge, PubSubGeneric * _pubSubGeneric ) 
{ 
  myForge = _forge; 
  serviceName = "PubSub";
  pubSubGeneric = _pubSubGeneric;
  myForge->addObject ( pubSubGeneric ); 
}

