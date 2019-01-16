#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpobjectbuffer.h>
#include <doolp/doolp-doolppubsub-filter.h>

Doolp::PubSubFilter * Doolp::PubSubFilter::buildFilter ( DMLX::Parser& parser )
{
  Log ( "At event '%s'\n", parser.getEventName () );
  if ( parser.isEventName ( "nameId" ) )
    {
      ObjectNameId nameId = parser.getAttrInt ( "equals" );
      PubSubFilter * ft = new PubSubFilterEqual<ObjectNameId> ( new PubSubFilterAccessorObjectNameId, nameId );
      parser.popEventAndEnd ();
      return ft;
    }
  else if ( parser.isEventName ( "objectId" ) )
    {
      ObjectNameId nameId = parser.getAttrInt ( "equals" );
      PubSubFilter * ft = new PubSubFilterEqual<ObjectNameId> ( new PubSubFilterAccessorObjectId, nameId );
      parser.popEventAndEnd ();
      return ft;
    }
  else if ( parser.isEventName ( "and" ) )
    {
      parser.popEvent();
      PubSubFilterAnd * f = new PubSubFilterAnd();
      while ( ! parser.isEventEnd() )
	{
	  f->add ( buildFilter ( parser ) ); 
	}
      parser.popEventEnd();
      return f;
    }
  else if ( parser.isEventName ( "or" ) )
    {
      parser.popEvent();
      PubSubFilterOr * f = new PubSubFilterOr();
      while ( ! parser.isEventEnd() )
	{
	  f->add ( buildFilter ( parser ) ); 
	}
      parser.popEventEnd();
      return f;
    }
  else if ( parser.isEventName ( "parameter" ) )
    {
      PubSubFilter * f;
      if ( ( ! parser.hasAttr ( "type" ) ) 
	   || ( strcmp ( parser.getAttr ( "type" ), "string" ) == 0 ) )
	{
	  char * value = parser.getAttr ( "equals" );
	  AssertBug ( value != NULL, "Could not get the 'equals' attribute\n" );
	  char *v; v = (char*)malloc(strlen(value)+1); strcpy(v, value);
	  f = new PubSubFilterEqual<char *>( new PubSubFilterAccessorParameter<char*> ( parser.getAttrInt ( "nameId" ) ), (char*)v );
	}
      else if ( strcmp ( parser.getAttr ( "type" ), "int" ) == 0 )
	{
	  int i = parser.getAttrInt ( "equals" );
	  f = new PubSubFilterEqual<int>( new PubSubFilterAccessorParameter<int> ( parser.getAttrInt ( "nameId" ) ), i );
	}
      else if ( strcmp ( parser.getAttr ( "type" ), "float" ) == 0 )
	{
	  float fv = parser.getAttrFloat ( "equals" );
	  f = new PubSubFilterEqual<float>( new PubSubFilterAccessorParameter<float> ( parser.getAttrInt ( "nameId" ) ), fv );
	}
      else
	Fatal ( "Unknown parameter type '%s'\n", parser.getAttr ( "type" ) );
      parser.popEventAndEnd();
      return f;
    }
  Fatal ( "Invalid token : '%s'\n", parser.getEventName() );
  parser.popEventAndEnd ();
  return NULL;
}

Doolp::PubSubFilter * Doolp::PubSubFilter::buildFilter ( string & subscription )
{
  Log ( "Building filter for '%s'\n", subscription.c_str() );
  PubSubFilter * filter = NULL;
  
  const char * sb = subscription.c_str ();
  int sb_sz = subscription.length ();
  DMLX::ParserMMap parser ( sb, sb_sz, sb_sz );
  parser.giveNewBytesToRead ( sb_sz );
  try
    {
      //      parser.parse();
      Log ( "First event '%s'\n", parser.getEventName () );
      if ( ! parser.isEventName ( "filter" ) )
	{
	  Error ( "Invalid filter : event '%s'\n", parser.getEventName() );
	}
      parser.popEvent();
      filter = buildFilter ( parser );
      if ( ! ( parser.isEventName ( "filter" ) && parser.isEventEnd() ) )
	{
	  Warn ( "Invalid filter : event '%s'\n", parser.getEventName() );
	  return NULL;
	}
      parser.popEventEnd ();
    }
  catch ( DMLX::ParserException * e )
    {
      Warn ( "Exception given : '%s'\n", e->getMessage() );
    }
  if ( filter != NULL )
    Log ( "Built filter : '%s'\n", filter->toString()->c_str() );
  return filter;
}

int Doolp::PubSubFilterAccessorParameter<int>::getValue ( Doolp::Object * obj )
{
  ObjectBuffer  *b = dynamic_cast<ObjectBuffer*> ( obj );
  AssertBug ( b != NULL, "Could not get Object Buffer ?\n" );
  ObjectBufferParam * p = b->getParam ( paramId );
  AssertBug ( p != NULL, "Could not get Object Param '0x%x'\n", paramId );
  AssertBug ( p->type == ObjectBufferParam::type_int, "Param is not type int !\n" );
  return ((int*)p->buffer)[0];
}

unsigned int Doolp::PubSubFilterAccessorParameter<unsigned int>::getValue ( Doolp::Object * obj )
{
  ObjectBuffer  *b = dynamic_cast<ObjectBuffer*> ( obj );
  AssertBug ( b != NULL, "Could not get Object Buffer ?\n" );
  ObjectBufferParam * p = b->getParam ( paramId );
  AssertBug ( p != NULL, "Could not get Object Param '0x%x'\n", paramId );
  AssertBug ( p->type == ObjectBufferParam::type_int, "Param is not type int !\n" );
  return ((unsigned int*)p->buffer)[0];
}

float Doolp::PubSubFilterAccessorParameter<float>::getValue ( Doolp::Object * obj )
{
  ObjectBuffer  *b = dynamic_cast<ObjectBuffer*> ( obj );
  AssertBug ( b != NULL, "Could not get Object Buffer ?\n" );
  ObjectBufferParam * p = b->getParam ( paramId );
  AssertBug ( p != NULL, "Could not get Object Param '0x%x'\n", paramId );
  AssertBug ( p->type == ObjectBufferParam::type_string, "Param is not type int !\n" );
  return ((float*)p->buffer)[0];
}

 
char * Doolp::PubSubFilterAccessorParameter<char *>::getValue ( Doolp::Object * obj )
{
  Log ( "Requesting param 0x%x in obj 0x%x\n",
	paramId, obj->getObjectId() );
  ObjectBuffer  *b = dynamic_cast<ObjectBuffer*> ( obj );
  AssertBug ( b != NULL, "Could not get Object Buffer ?\n" );
  ObjectBufferParam * p = b->getParam ( paramId );
  AssertBug ( p != NULL, "Could not get Object Param '0x%x'\n", paramId );
  AssertBug ( p->type == ObjectBufferParam::type_string, "Param is not type int !\n" );
  Log ( "Got param value '%s'\n", (char*)p->buffer );
  return ((char*)p->buffer);
}

bool Doolp::PubSubFilterEqual<int>::filter ( Doolp::Object * obj )
{ return ( accessor->getValue ( obj ) == value ); }

bool Doolp::PubSubFilterEqual<unsigned int>::filter ( Doolp::Object * obj )
{ return ( accessor->getValue ( obj ) == value ); }

bool Doolp::PubSubFilterEqual<float>::filter ( Doolp::Object * obj )
{ return ( accessor->getValue ( obj ) == value ); }

bool Doolp::PubSubFilterEqual<char *>::filter ( Doolp::Object * obj )
{ 
  Log ( "Equal : getValue='%s', value='%s'\n", accessor->getValue(obj), value );
  return ( strcmp ( accessor->getValue ( obj ), value ) == 0 ); 
}
