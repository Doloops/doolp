#include <doolp/doolp-doolpclasses.h>

#include <doolp/doolp-doolpconnection-tcp.h>
#include <doolp/doolp-doolpconnection-tcp-msg.h>

#include <doolp/doolp-doolpobject.h>
#include <doolp/doolp-doolpforge.h>

/*
  DoolpConnectionTCP : read/write mechanisms inside of Doolp Protocol..
*/

/*
 * Low-level protocol funcs
 */


bool DoolpConnectionTCP::sendZeroSizeBlock ( unsigned int mode,
					     unsigned int type )
{
  DoolpMsg_BlockHeader bkh;
  DoolpMsg_clearBlock ( &bkh );
  DoolpMsg_setBlockMode ( &bkh, mode );
  DoolpMsg_setBlockType ( &bkh, type );
  writeBlockHeader ( &bkh );
  return true;
}

bool DoolpConnectionTCP::writeBlockHeader(DoolpMsg_BlockHeader * h)
{
  h->blockIndex = nextBlockIndex;
  nextBlockIndex = 0;
  __DOOLPCONNECTIONTCP_Log ( "Writing blockheader of size %u\n",
		h->blockSize );
  return rawWrite ( h, sizeof ( DoolpMsg_BlockHeader ) );
}

bool DoolpConnectionTCP::readBlockHeader(DoolpMsg_BlockHeader * h)
{
  if ( blockHeaderIsRead ) 
    Bug ( "Should not be here : blockHeader already read.\n" );
  rawRead ( h, sizeof ( DoolpMsg_BlockHeader ) );
  __DOOLPCONNECTIONTCP_Log ( "Read Header type = %p, mode %p, blockIndex = %p, blockSize = %p\n",
	        h->blockType, h->blockMode, h->blockIndex, h->blockSize );
  // blockHeaderIsRead = true;
  return true;
}



/*
 * Data Read/Write
 */ 
ssize_t DoolpConnectionTCP::Write(unsigned int type, const void *buf, size_t nbyte)
{
  DoolpMsg_BlockHeader h;
  DoolpMsg_clearBlock ( &h );
  DoolpMsg_setBlockRawSize ( &h, nbyte );
  DoolpMsg_setBlockType ( &h, type );
  if ( ! writeBlockHeader ( &h ) )
    return false;
  return rawWrite ( buf, nbyte );
}
ssize_t DoolpConnectionTCP::Read(void *buf, size_t nbyte)
{
  // DoolpMsg_BlockHeader h;
  if ( ! blockHeaderIsRead )
    readBlockHeader ( &bh );
  blockHeaderIsRead = false;
  if ( DoolpMsg_getBlockRawSize ( &bh ) != nbyte )
    {
      Bug ( "While Reading raw : wanted %d, recieved %d\n",
	    DoolpMsg_getBlockRawSize ( &bh ), nbyte );
    }
  return rawRead ( buf, nbyte );
}

bool DoolpConnectionTCP::Write ( int * i )
{  return Write ( *i ); }

bool DoolpConnectionTCP::Write ( int i )
{ 
  __DOOLPCONNECTIONTCP_Log ( "Will write integer : %d\n", i );
  DoolpMsg_BlockHeader h;
  DoolpMsg_clearBlock ( &h );
  DoolpMsg_setBlockRawSize ( &h, sizeof (int) );
  h.blockType = DoolpMsg_BlockType_Int;
  if ( ! writeBlockHeader ( &h ) )
    return false;
  return rawWrite ( &i, sizeof (int) );
}

bool DoolpConnectionTCP::Read ( int * i )
{ 
  __DOOLPCONNECTIONTCP_Log ( "Trying to read int\n" );
  //  DoolpMsg_BlockHeader h;
  __DOOLPCONNECTIONTCP_Log ( "blockHeaderIsRead = '%s'\n",
			     blockHeaderIsRead ? "true" : "false" );
  if ( ! blockHeaderIsRead )
    readBlockHeader ( &bh );

  //  if ( ! readBlockHeader ( &h ) )
  // return false;
  if ( DoolpMsg_getBlockRawSize ( &bh ) != sizeof (int) )
    {
      Bug ( "While reading int : header says block size is %d\n",
	    DoolpMsg_getBlockRawSize ( &bh ) );
    }
  blockHeaderIsRead = false;

  return rawRead ( i, sizeof (int) );
}

bool DoolpConnectionTCP::Read ( bool * b ) { return Read ( (int*) b ); }
bool DoolpConnectionTCP::Read ( DoolpObjectId * id ) { return Read ( (int *) id ); }
bool DoolpConnectionTCP::Write ( DoolpObjectId id ) { return Write ( (int) id ); }

// String

bool DoolpConnectionTCP::Write ( char * s )
{ 
  if ( s == NULL ) return false;
  int size = strlen ( s ) + 1; 
  DoolpMsg_BlockHeader h;
  DoolpMsg_clearBlock ( &h );
  DoolpMsg_setBlockRawSize ( &h, size );
  h.blockType = DoolpMsg_BlockType_String;
  if ( ! writeBlockHeader ( &h ) )
    return false;
  return rawWrite ( s, size );
}
bool DoolpConnectionTCP::Read ( char ** s)
{ 
  int size;
  //  DoolpMsg_BlockHeader h;
  /*
  if ( ! blockHeaderIsRead )
    readBlockHeader ( &bh );
  blockHeaderIsRead = false;
  */
  __tryReadBlockHeader;
  //  readBlockHeader ( &h );
  if ( bh.blockSize == 0 )
    {
      Bug ( "Raw size is zero.\n" );
    }
  size = bh.blockSize;
  *s = (char*) malloc ( size );
  if ( rawRead ( *s, size ) != size ) 
    return false; 
  __DOOLPCONNECTIONTCP_Log ( "Read string %s (sz %d), at %p\n", *s, size, *s );
  blockHeaderIsRead = false;
  return true; 
}
bool DoolpConnectionTCP::Write ( string * s )
{
  Bug ( "NOT IMPLEMENTED !\n" );
  return false;
}
bool DoolpConnectionTCP::Read ( string ** s )
{
  Bug ( "NOT IMPLEMENTED !\n" );
  return false;
}

bool DoolpConnectionTCP::Write ( string & s )
{
  Bug ( "NOT IMPLEMENTED !\n" );
  return false;
}
bool DoolpConnectionTCP::Read ( string * s )
{
  Bug ( "NOT IMPLEMENTED !\n" );
  return false;
}
bool DoolpConnectionTCP::WriteRaw ( void * buffer, int size )
{
  Bug ( "NOT IMPLEMENTED !\n" );
  return false;
}
bool DoolpConnectionTCP::ReadRaw ( void ** buffer, int * size )
{
  Bug ( "NOT IMPLEMENTED !\n" );
  return false;
}


bool DoolpConnectionTCP::Write ( float f )
{ 
  __DOOLPCONNECTIONTCP_Log ( "Will write float : %f\n", f );
  DoolpMsg_BlockHeader h;
  DoolpMsg_clearBlock ( &h );
  DoolpMsg_setBlockRawSize ( &h, sizeof (float) );
  h.blockType = DoolpMsg_BlockType_Float;
  if ( ! writeBlockHeader ( &h ) )
    return false;
  return rawWrite ( &f, sizeof (float) );
}

bool DoolpConnectionTCP::Read ( float * f )
{ 
  __DOOLP_Log ( "Trying to read float\n" );
  __tryReadBlockHeader;
  if ( DoolpMsg_getBlockRawSize ( &bh ) != sizeof (float) )
    {
      Bug ( "While reading float : header says block size is %d\n",
	    DoolpMsg_getBlockRawSize ( &bh ) );
    }
  blockHeaderIsRead = false;

  return rawRead ( f, sizeof (float) );
}

bool DoolpConnectionTCP::Read ( DoolpFullContextId ** id )
{
  DoolpFullContextId * _id = new DoolpFullContextId ();
  *id = _id;
  return Read ( _id, sizeof ( DoolpFullContextId ) );
}
bool DoolpConnectionTCP::Write ( DoolpFullContextId * id )
{
  return Write ( DoolpMsg_BlockType_DoolpFullContextId, 
		 id, sizeof ( DoolpFullContextId ) );
}

bool DoolpConnectionTCP::WriteObjectHead ( DoolpObject * obj )
{
  DoolpMsg_BlockHeader bkh;
  DoolpMsg_clearBlock ( &bkh );
  DoolpMsg_setBlockType ( &bkh, DoolpMsg_BlockType_DoolpObject );
  DoolpMsg_setBlockMode ( &bkh, DoolpMsg_BlockMode_SubSection );
  if ( obj == NULL )
    { 
      Warn ( "Attempt to write an empty obj\n" ); 
      nextBlockIndex = 0;
      bkh.blockSize = sizeof ( DoolpMsg_DoolpObjectHeader );
      writeBlockHeader ( &bkh );
      DoolpMsg_DoolpObjectHeader doh;
      doh.nameId = 0;
      doh.objectId = 0;
      doh.ownerAgentId = 0;
      rawWrite ( &doh, sizeof ( DoolpMsg_DoolpObjectHeader ) );
      return true;
    }


  __DOOLP_Log ( "Writing obj : 0x%x\n", obj->getObjectId() );
  if ( ( obj->getObjectId() == 0 ) || ( obj->getForge() == NULL ) )
    Bug ( "Object not recorded to a DoolpForge, and/or not indexed !!\n" );
  nextBlockIndex = 0; // obj->objectId;
  bkh.blockSize = sizeof ( DoolpMsg_DoolpObjectHeader );

  // bkh.blockSize = sizeof ( DoolpNameId );
  writeBlockHeader ( &bkh );
  DoolpMsg_DoolpObjectHeader doh;
  doh.nameId = obj->getNameId ();
  doh.objectId = obj->getObjectId ();
  doh.ownerAgentId = obj->getOwnerAgentId ();
  rawWrite ( &doh, sizeof ( DoolpMsg_DoolpObjectHeader ) );
  return true;
}

bool DoolpConnectionTCP::Write ( DoolpObject * obj, DoolpContextStepping fromStepping )
{
  if ( obj == NULL )
    {
      Warn ( "Writing an empty object !\n" );
      WriteObjectHead ( obj );
    }
  else
    obj->serializeFromStepping ( this, fromStepping );
  return true;
}

bool DoolpConnectionTCP::Read ( DoolpObject ** obj, bool canResolve, bool mustResolve )
{
  __tryReadBlockHeader;
  if ( DoolpMsg_getBlockType ( &bh ) != 
       DoolpMsg_BlockType_DoolpObject )
    Bug ( "This is not a DoolpObject.\n" );
  if ( bh.blockSize != sizeof ( DoolpMsg_DoolpObjectHeader ) )
    Bug ( "BlockSize must be DoolpMsg_DoolpObjectHeader !\n" );
  blockHeaderIsRead = false;
  DoolpMsg_DoolpObjectHeader doh;
  rawRead ( &doh, sizeof ( DoolpMsg_DoolpObjectHeader ) );
  __DOOLP_Log ( "Recieved objId=0x%x, nameId=0x%x\n", doh.objectId, doh.nameId );
  if ( doh.objectId == 0 && doh.nameId == 0 )
    {
      Warn ( "Recieved a NULL object.\n" );
      *obj = NULL;
      return true;
    }
  DoolpObject * _obj = NULL;
  if ( canResolve )
    {
      __DOOLP_Log ( "Trying to Resolve : %d\n", doh.objectId );

      _obj = myForge->doolpfunclocal(getDistObject) ( &(msgh.fullContextId),
						      doh.objectId );
      if ( mustResolve )
	Bug ( "Could not resolve object objId=%d, nameId=%x, ownerAgentId=%d\n",
	      doh.objectId, doh.nameId, doh.ownerAgentId );

    }
  if ( _obj == NULL )
    {
      _obj = myForge->createEmptyObject ( doh.nameId, 
					  &(msgh.fullContextId), 
					  doh.objectId );
    }
  if ( _obj == NULL )
    Bug ( "Could not find a suitable object.\n" );
  _obj->setOwner ( doh.ownerAgentId );
  _obj->unserialize ( this );
  readSubSectionEnd ( );
  *obj = _obj;
  return true;
}


bool DoolpConnectionTCP::Write ( DoolpException * obj )
{
  Bug ( "__ not implemented\n" );
  return true;
}

bool DoolpConnectionTCP::Read ( DoolpException ** obj )
{
  Bug ( "__ not implemented\n" );
  return true;
}
