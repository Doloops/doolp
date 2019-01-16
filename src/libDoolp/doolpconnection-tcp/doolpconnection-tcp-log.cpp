#include <doolp/doolp-doolpconnection-tcp.h>
#include <doolp/doolp-doolpconnection-tcp-msg.h>


char * DoolpMsg_BlockType_TypeName[256];
char * DoolpMsg_BlockMode_ModeName[256];

void _define_DoolpMsg_BlockType_TypeName()
{
#define _addType(_n) \
  DoolpMsg_BlockType_TypeName[DoolpMsg_BlockType_##_n] = #_n

  for ( int p = 0 ; p < 256 ; p++ )
    DoolpMsg_BlockType_TypeName[p] = NULL;
  _addType (None);
  _addType (NewCall_Header);
  _addType (Call_Params);
  _addType (DoolpStream);
  _addType (DoolpStreamEnd);
  _addType (DoolpException);
  _addType (DoolpObjectId);
  _addType (DoolpObject);
  _addType (DoolpFullContextId);
  _addType (Raw);
  _addType (Int);
  _addType (String);
  _addType (Float);
  _addType (List);
#undef _addType
}

void _define_DoolpMsg_BlockMode_ModeName()
{
#define _addMode(_n) \
  DoolpMsg_BlockMode_ModeName[DoolpMsg_BlockMode_##_n] = #_n

  for ( int p = 0 ; p < 256 ; p++ )
    DoolpMsg_BlockMode_ModeName[p] = NULL;
  _addMode (None);
  _addMode (SubSection);
  _addMode (SubSectionEnd);
  _addMode (EndBlocks);
#undef _addMode
}

void  __DOOLPCONNECTIONTCP_LogMsg ( FILE * fp, void * buffer, unsigned int bufferIdx )
{
  unsigned int index = 0;
  _define_DoolpMsg_BlockType_TypeName();
  _define_DoolpMsg_BlockMode_ModeName();
#define _get(_what, _type) \
  _type * _what = (_type *) ((int)buffer+index); \
  index += sizeof ( _type ) ; 
#define _skip(_sz) index+= _sz
#define lineHead \
      fprintf ( fp, "|0x%.4x| ", index ); \
      for ( int f = 0 ; f < deep ; f ++ ) fprintf ( fp, "* " );

  _get ( mh, DoolpMsg_Header );
  fprintf ( fp, "Message Flag %x, from %d to %d, fullContextId [%d,%d,%d]\n",
	    mh->callFlags, mh->fromAgentId, mh->toAgentId,
	    logDoolpFullContextId ( &(mh->fullContextId) ) );
  int deep = 1;

  while ( true )
    {
      lineHead;
      _get ( bh, DoolpMsg_BlockHeader );
      if ( DoolpMsg_isBlockEnd ( bh ) )
	{
	  fprintf ( fp, "| End of blocks. (total size 0x%.4x)\n", index );
	  fprintf ( fp, "\n" );
	  return;
	}
      if ( bh->blockMode == DoolpMsg_BlockMode_SubSection )
	deep++;
      else if ( bh->blockMode == DoolpMsg_BlockMode_SubSectionEnd )
	deep--;
      fprintf ( fp, "Block Type=%2x (%s) Mode=%2x (%s) Size=%d Index=%x\n",
		bh->blockType, DoolpMsg_BlockType_TypeName[bh->blockType],
		bh->blockMode, DoolpMsg_BlockMode_ModeName[bh->blockMode],
		bh->blockSize, 
		bh->blockIndex );
      if ( bh->blockMode == DoolpMsg_BlockMode_SubSectionEnd )
	{
	  lineHead;
	  fprintf ( fp, "End of subsection\n" );
	}
      switch ( bh->blockType )
	{
	case DoolpMsg_BlockType_NewCall_Header:
	  fprintf ( fp, "\tNew Call : objId=%d; slotId=%p\n",
		    ((DoolpMsg_NewCall_Header*) ((int) buffer + index ))->objId,
		    ((DoolpMsg_NewCall_Header*) ((int) buffer + index ))->slotId ); 
	  break;
	case DoolpMsg_BlockType_Int:
	  fprintf ( fp, "\tInteger %d\n",
		    *((int*) ((int) buffer + index )) ); 
	  break;
	case DoolpMsg_BlockType_Float:
	  fprintf ( fp, "\tFloat %f\n",
		    *((float*) ((int) buffer + index )) ); 
	  break;
	case DoolpMsg_BlockType_String:
	  fprintf ( fp, "\tString %s\n",
		    ((char*) ((int) buffer + index )) ); 
	  break;
	case DoolpMsg_BlockType_DoolpFullContextId:
	  fprintf ( fp, "\tFullContextId [%d,%d,%d]\n",
		    logDoolpFullContextId ( ((DoolpFullContextId *) ((int) buffer+index)) ) );
	  break;
	case DoolpMsg_BlockType_DoolpObject:
	  fprintf ( fp, "\tDoolpObject nameId : 0x%.8x, objectId : 0x%.8x, owner : %d\n",
		    *((int*) ((int) buffer + index) ),
		    *((int*) ((int) buffer + index + sizeof (DoolpObjectNameId) ) ),
		    *((int*) ((int) buffer + index + sizeof (DoolpObjectNameId) + sizeof (DoolpObjectId) ) )
		    );
	case DoolpMsg_BlockType_None:
	  break;
	default:
	  if ( bh->blockSize > 0 )
	    fprintf ( fp, "\tUnkown.\n" );
	}
      _skip ( bh->blockSize );
    }
#undef _get
#undef _skip
}


/*

#ifdef __DOOLPCONNECTIONTCP_DUMPBUFFERS_FW
#define _FWdumpBuffer(_fd,_buff,_sz)				\
  if ( _buff == NULL )							\
    Bug ( "Dont want to dump a null buffer\n" );			\
  for ( int _pf = 0 ; _pf < _sz ; _pf+=16 )				\
    {									\
      fprintf ( _fd, "%4x:\t", _pf ); \
      for ( int _f = 0 ; _f < 4 ; _f ++ ) \
        fprintf ( _fd, "0x%8x ", ((int*)buffer)[_f + (_pf / 4)]);  \
      for ( int _f = _pf ; _f < _pf + 16 ; _f ++ ) \
        { if ( _f % 4 == 0 ) fprintf ( _fd, " | " ) ; \
          fprintf ( _fd, "%2x", *((char*) _buff+_f) & 0xFF ); }	\
      fprintf ( _fd, "\n" ); \
    }									\
//  if ( _sz % 16 != 1 ) fprintf ( _fd, "\n\n" );
#else 
#define _FWdumpBuffer(_fd,_buff,_sz)
#endif
*/
