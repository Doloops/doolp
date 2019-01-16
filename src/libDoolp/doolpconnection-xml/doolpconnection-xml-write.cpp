#include <doolp/doolp-doolpconnection-xml.h>

#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpcall.h>
#include <doolp/doolp-doolpjob.h>
#include <doolp/doolp-doolpcallcontext.h>

#include <doolp/doolp-doolpexception.h>
#include <doolp/doolp-doolpobjectbuffer.h>

/*
 * DoolpConnection Highlevel functions
 */

#define __write_nextBlockIndex() \
  if ( nextBlockIndex != 0 ) { writer->writeAttrHex ( keywords.index, nextBlockIndex ); }

bool Doolp::ConnectionXML::Write ( int i )
{
  writer->writeMarkup ( keywords._int );
  __write_nextBlockIndex();
  writer->writeAttrInt ( keywords.value, i );
  writer->writeMarkupEnd ();
  nextBlockIndex = 0;
  return true;
}

bool Doolp::ConnectionXML::Write ( int * i ) 
{ return Write ( *i ); }

bool Doolp::ConnectionXML::Write ( Doolp::ObjectId i ) 
{ return Write ( (int)i ); }

bool Doolp::ConnectionXML::Write ( char * s )
{
  writer->writeMarkup ( keywords._string );
  __write_nextBlockIndex();
  writer->writeAttr ( keywords.value, s, true );
  writer->writeMarkupEnd ();
  nextBlockIndex = 0;
  return true;
}
bool Doolp::ConnectionXML::Write ( string * s )
{ return Write ( (char*) s->c_str () ); }

bool Doolp::ConnectionXML::Write ( string &s )
{ 
  char s_c[128];
  strncpy ( s_c, s.c_str(), 128 );
  Write ( s_c ); 
  return true;
}

bool Doolp::ConnectionXML::Write ( float f )
{
  writer->writeMarkup ( keywords._float );
  __write_nextBlockIndex();
  writer->writeAttrFloat ( keywords.value, f );
  writer->writeMarkupEnd ();
  nextBlockIndex = 0; return true;
}

bool Doolp::ConnectionXML::Write ( Doolp::FullContextId * fci )
{
  writer->writeMarkup ( keywords._FullContextId );
  __write_nextBlockIndex();
  writer->writeAttrInt ( keywords.agentId, fci->agentId );
  writer->writeAttrInt ( keywords.contextId, fci->contextId );
  writer->writeAttrInt ( keywords.stepping, fci->stepping );
  writer->writeMarkupEnd ();
  nextBlockIndex = 0; return true;
}

bool Doolp::ConnectionXML::Write ( Doolp::Object * obj, Doolp::ContextStepping fromStepping )
{
  if ( obj == NULL )
    {
      Warn ( "Writing a null object !\n" );
      WriteObjectHead ( obj );
    }
  else
    obj->serializeFromStepping ( this, fromStepping );
  return true;
}
 
bool Doolp::ConnectionXML::WriteObjectHead ( Doolp::Object * obj )
{
  if ( obj == NULL )
    {
      writer->writeMarkup ( keywords._Object );
      writer->writeAttrHex ( keywords.objectNameId, 0 );
      writer->writeAttrHex ( keywords.objectId, 0 );
      writer->writeAttrInt ( keywords.ownerAgentId, 0 );
      writer->writeMarkupEnd ();
      return true;
    }
  writer->writeMarkup ( keywords._Object );
  writer->writeAttrHex ( keywords.objectNameId, obj->getNameId() );
  writer->writeAttrHex ( keywords.objectId, obj->getObjectId() );
  writer->writeAttrInt ( keywords.ownerAgentId, 0 );
  return true;
}


bool Doolp::ConnectionXML::Write ( Doolp::Exception * e )
{
  writer->writeMarkup ( keywords._Exception );
  writer->writeAttrHex ( keywords.exceptionId, e->getExceptionId () );
  writer->writeAttr ( keywords.message, e->getMessage (), true );
  writer->writeMarkupEnd ();
  return true;
}


bool Doolp::ConnectionXML::Write ( Doolp::ObjectBufferParam * param )
{
  switch ( param->type )
    {
    case ObjectBufferParam::type_int:
      AssertFatal ( param->size == sizeof(int), "Invalid size for int...\n" );
      return Write ( *((int*)param->buffer) );
    case ObjectBufferParam::type_float:
      AssertFatal ( param->size == sizeof(float), "Invalid size for int...\n" );
      return Write ( *((float*)param->buffer) );
    case ObjectBufferParam::type_string:
      AssertFatal ( param->size == (int)strlen ( (char*)param->buffer ) + 1, "Wrong param->size !\n" );
      return Write ( (char*)param->buffer );
    }
  Bug ( "Unhandled param->type %d\n", param->type );
  return false;
}

bool Doolp::ConnectionXML::WriteRaw ( void * buffer, int size )
// This is to be largely reimplemented !
{
  // Bug ( "NOT IMPLEMENTED !! TO REIMPLEMENT !\n" );
  Bug ( "DEPRECATED.\n" );
#if 0
  Warn ( "TO REIMPLEMENT !!!\n" );
  AssertFatal ( size > 9, "Raw Section too short\n" );
  char buff[10];
  memcpy ( buff, buffer, 10 );
  buff[9] = '\0';
  AssertFatal ( strcmp ( buff, "<XMLRAW/>" ) == 0, "Wrong format for XMLRAW Section.\n" );
  ((char*)buffer)+=9;
  writer->writeText ( (char*) buffer, false );
#endif
  return true;
}


/*
 * Message Forging
 */


bool Doolp::ConnectionXML::startNewCall ( Doolp::Call * call )
{
  lockWrite ();
  currentCallContext = call;
  writer->writeMarkup ( keywords.Call );
  //  writer->writeAttr ( keywords.type, "NewCall", false );
  writer->writeAttrInt ( keywords.fromAgentId, call->fromAgentId );
  writer->writeAttrInt ( keywords.toAgentId, call->toAgentId );
  Write ( &(call->fullContextId) );
  AssertBug ( call->slot != NULL, "No slot defined !\n" );
  writer->writeMarkup ( keywords.CallParams );
  writer->writeAttrHex ( keywords.objectId, call->objId );
  writer->writeAttrHex ( keywords.objectNameId, call->obj->getNameId() );
  writer->writeAttrHex ( keywords.slotId, call->slot->getSlotId() );
  writer->writeAttr ( keywords.slotName, call->slot->getSlotName(), true );  
  writer->writeMarkupEnd ();
  return true;
}
 
bool Doolp::ConnectionXML::startReply ( Doolp::Job * job )
{
  lockWrite ();
  currentCallContext = job;
  writer->writeMarkup ( keywords.Reply );
  //  writer->writeAttr ( keywords.type, "Reply", false );
  writer->writeAttrInt ( keywords.fromAgentId, job->fromAgentId );
  writer->writeAttrInt ( keywords.toAgentId, job->toAgentId );
  Write ( &(job->fullContextId) );
  return true;
}

bool Doolp::ConnectionXML::startParamSubSection ( )
{
  writer->writeMarkup ( keywords.Params );
  return true;
}

bool Doolp::ConnectionXML::endSubSection ( )
{
  writer->writeMarkupEnd ();
  return true;
}

bool Doolp::ConnectionXML::endMessage ( ) 
{ 
  writer->writeMarkupEnd();
#if 0
  if ( markupLevel > 0 )
    {
      __DOOLP_Log ( "Remaining unclosed markups inside of Message !\n" );
      for ( unsigned int i = 0 ; i < markupLevel ; i++)
	__DOOLP_Log ( "Remaining markup : '%s'\n", markups[i]->getKeyword() );
    }
  AssertBug ( markupLevel == 0, "endMessage without finishing inside sections\n" );
  currentCallContext = NULL;
#endif
  unlockWrite ();
  return true;
}

bool Doolp::ConnectionXML::startList ( unsigned int listIndex )
{
  writer->writeMarkup ( keywords._list );
  writer->writeAttrInt ( keywords.index, listIndex );
  return true;
}

bool Doolp::ConnectionXML::setNextBlockIndex ( unsigned int _blockIndex )
{ 
  nextBlockIndex = _blockIndex; return true; 
}

bool Doolp::ConnectionXML::setStream ( Doolp::CallContext * callContext, Doolp::StreamIndex idx )
{
  AssertBug ( doolpStreamIsInside == false, "Already in a Doolp::Stream section !\n" );
  doolpStreamIsInside = true;
  if ( !isLockWrite() )
    {
      lockWrite ();
      doolpStreamHasSentHeader = true;
      currentCallContext = callContext;
      writer->writeMarkup ( keywords.InCall );
      writer->writeAttrInt ( keywords.fromAgentId, callContext->fromAgentId );
      writer->writeAttrInt ( keywords.toAgentId, callContext->toAgentId );
      Write ( &(callContext->fullContextId) );
    }
  writer->writeMarkup ( keywords._Stream );
  writer->writeAttrHex ( keywords.index, idx );
  return true;
}

bool Doolp::ConnectionXML::leaveStream ( ) 
{ 
  AssertBug ( doolpStreamIsInside == true, "Not in a DoolpStream section !\n" );
  endSubSection (); 
  if ( doolpStreamHasSentHeader )
    {
      endMessage ();
      doolpStreamHasSentHeader = false;
    }
  doolpStreamIsInside = false;
  return true;
}
bool Doolp::ConnectionXML::endStream ( ) 
{
  AssertBug ( doolpStreamIsInside == true, "Not in a DoolpStream section !\n" );
  writer->writeMarkup ( keywords.StreamEnd );
  writer->writeMarkupEnd ();
  return true;
}
