#include <doolp/doolp-doolpconnection-xml.h>
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpobjectcache.h>
#include <doolp/doolp-doolpexceptions.h>
#include <doolp/doolp-doolpstream.h>


#include <doolp/doolp-doolpobjectbuffer.h>

#define __checkEventName(__key) \
  AssertBug ( parser->isEventName ( __key ), "Not on the right event : Excepted '%s', Had '%s'.\n", \
	      (__key).getKeyword(), parser->getEventName () );


bool Doolp::ConnectionXML::Read ( int * i )  
{
  __checkEventName ( keywords._int );
  AssertBug ( ! ( parser->getEvent()->isEnd() ), "I am at the end event of <int/> !!\n" );
  //  AssertBug ( parser->getEvent()->text != NULL, "No text in this event ! (reading int)\n" );
  //  *i = atoi ( parser->getEvent()->text );
  *i = parser->getAttrInt( keywords.value );
  parser->popEventAndEnd ();
  return true;
}
bool Doolp::ConnectionXML::Read ( bool * b )  
{
  return Read ( (int*) b );
}

bool Doolp::ConnectionXML::Read ( Doolp::ObjectId * id )  
{
  return Read ( (int*) id );
}
bool Doolp::ConnectionXML::Read ( char ** s)  
{
  __checkEventName ( keywords._string );
  char * s2 = parser->getAttr ( keywords.value );
  *s = (char*) malloc ( strlen ( s2 ) + 1 );
  memcpy ( *s, s2, strlen ( s2 ) + 1 );
  parser->popEventAndEnd ();
  return true;
}
bool Doolp::ConnectionXML::Read ( string ** s )
{
  __checkEventName ( keywords._string );
  *s = new string ( parser->getAttr( keywords.value ) );
  parser->popEventAndEnd ();
  return true;
}
bool Doolp::ConnectionXML::Read ( string * s ) // THIS IS SUPPOSED THE STRING IS CREATED !!
{
  __checkEventName ( keywords._string );
  s->assign ( parser->getAttr( keywords.value ) );
  parser->popEventAndEnd ();
  return true;
}
bool Doolp::ConnectionXML::Read ( float * f ) 
{
  __checkEventName ( keywords._float );
  *f = parser->getAttrFloat ( keywords.value );
  parser->popEventAndEnd ();
  return true;
}
bool Doolp::ConnectionXML::Read ( Doolp::FullContextId ** id )  
{
  if ( ! parser->isEventName ( keywords._FullContextId ) )
    Bug ( "Expected DoolpFullContextId ! Had : '%s'\n",
	  parser->getEvent()->name->getKeyword() );
  *id = new FullContextId ();
  (*id)->agentId = parser->getAttrInt ( keywords.agentId );
  (*id)->contextId = parser->getAttrInt ( keywords.contextId );
  (*id)->stepping = parser->getAttrInt ( keywords.stepping );
  parser->popEventAndEnd ();
  return true;
}
bool Doolp::ConnectionXML::Read ( Doolp::Object ** obj, bool canResolve, bool mustResolve )
{
  __DOOLP_Log ( "Reading Object\n" );
  __checkEventName ( keywords._Object );
  AssertBug ( ! parser->getEvent()->isEnd(), " ! " );
  
  if ( contextId == NULL )
    Bug ( "No contextId defined : bug ?\n" );
  ObjectId objId = parser->getAttrInt ( keywords.objectId );
  ObjectNameId nameId =  parser->getAttrInt ( keywords.objectNameId );
  AgentId ownerAgentId =  parser->getAttrInt ( keywords.ownerAgentId );
  __DOOLP_Log ( "objId=0x%x, nameId=0x%x, ownerAgentId=%d\n",
		objId, nameId, ownerAgentId );
  parser->popEvent ();
  *obj = NULL; // TO BE CHANGED.. But How ?
  if ( objId == 0 && nameId == 0 )
    {
      Warn ( "Recieved an empty Object\n" );
      *obj = NULL;
      parser->popEventEnd ();
      return true;
    }
  if ( canResolve )
    {
      __DOOLP_Log ( "trying to resolve object %p locally\n", (void*) objId );
      *obj = myForge->getObjectCache()->get ( objId, contextId );
      if ( *obj == NULL && mustResolve )
	Bug ( "Could not resolve object objId=%d, nameId=%x, ownerAgentId=%d\n",
	      objId, nameId, ownerAgentId );
    }
  if ( *obj == NULL )
    {
      __DOOLP_Log ( "Creating empty object of nameId=%p, objId=%p\n",
		    (void*) nameId, (void*) objId );

      *obj = myForge->createEmptyObject ( nameId,
					  contextId,
					  objId );
    }
  if ( *obj == NULL )
    Bug ( "Could not create empty object of nameId=%p, objId=%p\n",
	  (void*) nameId, (void*) objId );
  (*obj)->setOwner ( ownerAgentId );
  (*obj)->unserialize ( this );
  parser->popEventEnd ();
  return true;
}

bool Doolp::ConnectionXML::Read ( Doolp::Exception ** e )  
{
  __checkEventName ( keywords._Exception );
  ExceptionId id = parser->getAttrInt ( keywords.exceptionId );
  *e = myForge->getException ( id );
  parser->popEventAndEnd ();
  return true;
}

bool Doolp::ConnectionXML::Read ( Doolp::ObjectBufferParam ** _param )
{
  Doolp::ObjectBufferParam * param = new Doolp::ObjectBufferParam();
  *_param = param;
  if ( parser->isEventName ( keywords._int ) )
    {
      param->type = ObjectBufferParam::type_int;
      param->size = sizeof (int);
      param->buffer = malloc ( param->size );
      int i;
      Read ( &i );
      memcpy ( param->buffer, &i, sizeof (int) );
      return true;
    }
  if ( parser->isEventName ( keywords._float ) )
    {
      param->type = ObjectBufferParam::type_float;
      param->size = sizeof (float);
      param->buffer = malloc ( param->size );
      float f;
      Read ( &f );
      memcpy ( param->buffer, &f, sizeof (float) );
      return true;
    }
  if ( parser->isEventName ( keywords._string ) )
    {
      param->type = ObjectBufferParam::type_string;
      char * s;
      Read ( &s );
      param->size = strlen (s) + 1;
      param->buffer = s;
      return true;
    }
  Error ( "Could not read Object Parameter : event '%s'\n", parser->getEventName() );
  return false;
}

bool Doolp::ConnectionXML::ReadRaw ( void ** buffer, int * size )
{
  Bug ( "DEPRECATED.\n" );
#define __push(...) idx+=sprintf ( &(buff[idx]), __VA_ARGS__ ); 
  unsigned int idx = 0;
  int level = 0;
  char buff[2048];
  __push ( "<XMLRAW/>" );
  
  while ( true )
    {
      if ( parser->isEventEnd() )
	{
	  __push ( "</%s>", parser->getEvent()->name->getKeyword() );
	  parser->popEventEnd ();
	  level--;
	  if ( level == 0 ) break;
	  continue;
	}
      if ( parser->isEventText() )
	{
	  __push ( "%s", parser->getText() );
	  parser->popEvent();
	  continue;
	}
      __push ( "<%s", parser->getEvent()->name->getKeyword() );
      for ( DMLX::Attr * attr = parser->getEvent()->first;
	    attr != NULL ; attr = attr->next )
	{
	  __push ( " %s=\"%s\"", attr->name->getKeyword(), attr->value );
	}
      __push ( ">" );
      //      if ( parser->getEvent()->text != NULL )
      //	__push ( "\"%s\"", parser->getEvent()->text );
      parser->popEvent ();
      level++;
    }
  buff[idx] = '\0';
  idx++;
  *buffer = malloc ( idx );
  AssertFatal ( *buffer != NULL, "Could not allocate buffer space !\n" );
  memcpy ( *buffer, buff, idx );
  *size = idx;
#undef __push
  return true;
}


// Sections Reading


bool Doolp::ConnectionXML::readParamSubSection ()  
{  
  if ( ! parser->isEventName ( keywords.Params ) )
    return false;
  parser->popEvent (); // Params
  return true;
}

bool Doolp::ConnectionXML::readSubSectionEnd ( )  
{
  if ( ! parser->isEventEnd () )
    {
      return false;
    }
  parser->popEventEnd ();
  return true;
}
bool Doolp::ConnectionXML::readMessageEnd ( )  
{
  if ( ! parser->isEventEnd () )
    return false;
  AssertBug ( parser->isEventName ( keywords.Call )
	      || parser->isEventName ( keywords.Reply )
	      || parser->isEventName ( keywords.InCall ),
	      "Waited for a message end here ! Had '%s'\n", parser->getEvent()->name->getKeyword() );
  parser->popEventEnd ();
  return true;
}

unsigned int Doolp::ConnectionXML::readList ( )
{  
  AssertFatal ( ! parser->isEventEnd () );
  __checkEventName ( keywords._list );
  unsigned int listIndex = getNextBlockIndex ();
  parser->popEvent ();
  return listIndex;
}

unsigned int Doolp::ConnectionXML::getNextBlockIndex ( )  
{
  if ( parser->isEventEnd () )
    return 0;
  return parser->getAttrInt ( keywords.index );
}

bool Doolp::ConnectionXML::readObjectSubSection ()
{  
  return parser->isEventName ( keywords._Object );
}

bool Doolp::ConnectionXML::readExceptionSubSection ()
{  
  return parser->isEventName ( keywords._Exception );
}


bool Doolp::ConnectionXML::readStreamSubSection ()  
{
  return parser->isEventName ( keywords._Stream );
} 

bool Doolp::ConnectionXML::readStreamEnd ()  
{
  if ( ! parser->isEventName ( keywords.StreamEnd ) )
    return false;
  parser->popEventAndEnd ();
  return true;
}

bool Doolp::ConnectionXML::readStream ( Doolp::CallContext * callContext, Doolp::StreamVirtual * stream, 
					   Doolp::StreamIndex idx )
{
  if ( ! readStreamSubSection() )
    return false;
  if ( getNextBlockIndex() != idx )
    {
      Warn ( "Is a Stream, but not the good index (ask for %d, but blockIndex %d)\n",
	     idx, getNextBlockIndex() );
      return false;
    }
  parser->popEvent();
  while ( ! readSubSectionEnd() )
    {
      stream->readFrom ( this, callContext );
    }
  return true;
}
