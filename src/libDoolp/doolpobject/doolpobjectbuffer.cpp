#include <doolp/doolp-doolpconnection.h>
#include <doolp/doolp-doolpobjectbuffer.h>

bool Doolp::ObjectBuffer::serialize ( Doolp::Connection * conn )
{
  conn->WriteObjectHead ( this );
  map<ObjectParamId, ObjectBufferParam*>::iterator param;
  for ( param = params.begin () ; param != params.end () ; param++ )
    {
      conn->setNextBlockIndex ( param->first );
      conn->Write ( params[param->first] );
    }
  conn->endSubSection ();
  return true;
}
bool Doolp::ObjectBuffer::serialize ( Doolp::Connection * conn, Doolp::ObjectParamId paramId )
{
  if ( params[paramId] != NULL )
    {
      conn->setNextBlockIndex ( paramId );
      conn->Write ( params[paramId] );
      //      conn->WriteRaw ( params[paramId]->buffer, params[paramId]->size );
    }
  else
    {
      Warn ( "Could not get paramId='0x%x'\n", paramId );
    }
  return true;
}
bool Doolp::ObjectBuffer::unserialize ( Doolp::Connection * conn )
{
  unsigned int blockIndex;
  while ( ( blockIndex = conn->getNextBlockIndex () ) )
    {
      __DOOLP_Log ( "Unserialize paramId '0x%x'\n", blockIndex );
      ObjectBufferParam * buff; // = new ObjectBufferParam ();
      //      conn->ReadRaw ( &(buff->buffer), &(buff->size ) );
      conn->Read ( &buff );
      __DOOLP_Log ( "Retrieved '%d' bytes\n", buff->size );
      params[blockIndex] = buff;
    }
  return true;
}

Doolp::ObjectSlotMap * Doolp::ObjectBuffer::__slotmap = NULL;
