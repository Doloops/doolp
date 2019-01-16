#ifndef __DOOLP__DOOLPMSG_H
#define __DOOLP__DOOLPMSG_H

#include <doolp/doolp-doolpids.h>

typedef unsigned int DoolpMsg_CallFlags;

#define DoolpMsg_CallFlag_ModeCall      1 // 0b0001
#define DoolpMsg_CallFlag_ModeReply     2 // 0b0010

#define DoolpMsg_CallFlag_NewCall       4 // 0b0100
// #define DoolpMsg_CallFlag_Terminator    8 // 0b1000
#define DoolpMsg_CallFlag_EndConnection 0 // 0b0000

typedef struct DoolpMsg_Header
{
  DoolpMsg_CallFlags callFlags;
  DoolpAgentId fromAgentId;
  DoolpAgentId toAgentId;
  DoolpFullContextId fullContextId;
  //  DoolpObjectId objectId;
  //  DoolpRPCId rpcId;
};

typedef struct DoolpMsg_NewCall_Header
{
  DoolpObjectId objId;
  DoolpRPCId    rpcId;
};
#define dumpDoolpMsg_Header(_h_)						\
  Log ( "Flags %d, from %d to %d, "					\
	"Context [%d,%d,%d]\n",						\
	(_h_)->callFlags, (_h_)->fromAgentId, (_h_)->toAgentId,		\
	(_h_)->fullContextId.agentId, (_h_)->fullContextId.contextId,	\
	(_h_)->fullContextId.stepping );
#ifdef DOOLP_Log
#define logDoolpMsg_Header(_h_) dumpDoolpMsg_Header(_h_)
#else
#define logDoolpMsg_Header(_h_)
#endif
// Message header flags functions :

#define DoolpMsg_getFlags(__head__) (__head__->callFlags)

#define DoolpMsg_isCall(_head_) \
  ( DoolpMsg_getFlags(_head_) & DoolpMsg_CallFlag_ModeCall )
#define DoolpMsg_isReply(_head_) \
  ( DoolpMsg_getFlags(_head_) & DoolpMsg_CallFlag_ModeReply )

#define DoolpMsg_isNewCall(_head_) \
  ( DoolpMsg_getFlags(_head_) & DoolpMsg_CallFlag_NewCall )
#define DoolpMsg_isTerminator(_head_) \
  ( DoolpMsg_getFlags(_head_) & DoolpMsg_CallFlag_Terminator )

#define DoolpMsg_isEndConnection(_head_) \
  ( DoolpMsg_getFlags(_head_) == DoolpMsg_CallFlag_EndConnection )




typedef unsigned int DoolpMsg_BlockFlags;

#define DoolpMsg_BlockFlag_GModeBase ( 0 << 7 )
#define DoolpMsg_GModeBase_Max ( ( 1 << 7 ) - 1 )
#define DoolpMsg_BlockFlag_GModeExt  ( 1 << 7 )
#define DoolpMsg_BlockFlag_GModeMask ( 1 << 7 )

// Mode : 3 bits ( 1 to 7 )

#define DoolpMsg_BlockType_Mask   (7 << 4) //  0b01110000
// #define DoolpMsg_BlockType_Raw             1
#define DoolpMsg_BlockType_DoolpStream     2
#define DoolpMsg_BlockType_DoolpException  3
#define DoolpMsg_BlockType_DoolpObjectId   4
#define DoolpMsg_BlockType_DoolpObject     5

#define DoolpMsg_BlockMode_Mask          15 //   0b00001111
#define DoolpMsg_BlockMode_Raw                   1
#define DoolpMsg_BlockMode_List                  2
#define DoolpMsg_BlockMode_DoolpStream           3
#define DoolpMsg_BlockMode_EndBlocks             4


typedef struct DoolpMsg_BlockHeader
{
  DoolpMsg_BlockFlags blockFlags;
  unsigned int blockSize; // Total block size, without header.
};


// Message block header flags functions :

#define DoolpMsg_getBlockFlags(__head__) ((__head__)->blockFlags)

#define DoolpMsg_blockIsGModeBase(__head_)			      \
  ( ( DoolpMsg_getBlockFlags(__head_) & DoolpMsg_BlockFlag_GModeMask ) \
    == DoolpMsg_BlockFlag_GModeBase )
#define DoolpMsg_blockIsGModeExt(__head_)			      \
  ( ( DoolpMsg_getBlockFlags(__head_) & DoolpMsg_BlockFlag_GModeMask ) \
    == DoolpMsg_BlockFlag_GModeExt )

// End of block list
#define DoolpMsg_isBlockEnd(_head_)					\
  ( DoolpMsg_blockIsGModeExt ( _head_ )					\
    && ( DoolpMsg_getBlockFlags ( _head_) & DoolpMsg_BlockMode_EndBlocks ) )

// Message block type functions :
#define DoolpMsg_getBlockType(_head_) \
  ( DoolpMsg_blockIsGModeExt(_head_) ? \
  ( DoolpMsg_getBlockFlags(_head_) & DoolpMsg_BlockType_Mask ) >> 4 : 0 )
#define DoolpMsg_setBlockType(_head_, _type_) \
  DoolpMsg_getBlockFlags(_head_) |= DoolpMsg_BlockFlag_GModeExt; \
  DoolpMsg_getBlockFlags(_head_) |= ( _type_ ) << 4;

#define DoolpMsg_getBlockMode(_head_)					\
  ( DoolpMsg_blockIsGModeExt(_head_) ?					\
  ( DoolpMsg_getBlockFlags(_head_) & DoolpMsg_BlockMode_Mask ) : 0 )
#define DoolpMsg_setBlockMode(_head_, _mode_)				\
  DoolpMsg_getBlockFlags(_head_) |= DoolpMsg_BlockFlag_GModeExt; /*UseLess ?*/ \
  DoolpMsg_getBlockFlags(_head_) |= ( _mode_ )


#define DoolpMsg_setBlockRawSize(_head_,_size_)		     \
  if (_size_ <= DoolpMsg_GModeBase_Max )		     \
    DoolpMsg_getBlockFlags(_head_) = _size_;		     \
  else							     \
    { DoolpMsg_setBlockMode(_head_, DoolpMsg_BlockMode_Raw); \
      (_head_)->blockSize = _size_; }
#define DoolpMsg_getBlockRawSize(_head_) \
  ( DoolpMsg_blockIsGModeBase ( _head_ ) ? \
    DoolpMsg_getBlockFlags(_head_) : \
    DoolpMsg_getBlockMode(_head_) == DoolpMsg_BlockMode_Raw ? \
    (_head_)->blockSize : 0 )

#define DoolpMsg_getBlockSize(_head_)	   \
  ( DoolpMsg_blockIsGModeBase ( _head_ ) ? \
    DoolpMsg_getBlockFlags(_head_) :	   \
    (_head_)->blockSize )

#endif //  __DOOLP__DOOLPMSG_H

