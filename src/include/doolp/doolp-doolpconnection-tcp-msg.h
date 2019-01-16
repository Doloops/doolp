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
};

typedef struct DoolpMsg_NewCall_Header
{
  DoolpObjectId objId;
  DoolpObjectSlotId    slotId;
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

/*
#define DoolpMsg_BlockFlag_GModeBase ( 0 << 7 )
#define DoolpMsg_GModeBase_Max ( ( 1 << 7 ) - 1 )
#define DoolpMsg_BlockFlag_GModeExt  ( 1 << 7 )
#define DoolpMsg_BlockFlag_GModeMask ( 1 << 7 )
*/
// Mode : 3 bits ( 1 to 7 )

// #define DoolpMsg_BlockType_Mask   (7 << 4) //  0b01110000
#define DoolpMsg_BlockType_None            0
#define DoolpMsg_BlockType_NewCall_Header  5
#define DoolpMsg_BlockType_Call_Params     6
#define DoolpMsg_BlockType_List            7
#define DoolpMsg_BlockType_DoolpStream     10
#define DoolpMsg_BlockType_DoolpStreamEnd  12
#define DoolpMsg_BlockType_DoolpException  20
#define DoolpMsg_BlockType_DoolpObjectId   30
#define DoolpMsg_BlockType_DoolpObject     35
#define DoolpMsg_BlockType_DoolpFullContextId 40
#define DoolpMsg_BlockType_Raw             100
#define DoolpMsg_BlockType_Int             101
#define DoolpMsg_BlockType_String          102
#define DoolpMsg_BlockType_Float           103


// #define DoolpMsg_BlockMode_Mask          15 //   0b00001111
// #define DoolpMsg_BlockMode_Raw                   1
// #define DoolpMsg_BlockMode_List                  2
#define DoolpMsg_BlockMode_None                  0
#define DoolpMsg_BlockMode_SubSection            3
#define DoolpMsg_BlockMode_SubSectionEnd         4
#define DoolpMsg_BlockMode_EndBlocks             5


typedef struct DoolpMsg_BlockHeader
{
  //  DoolpMsg_BlockFlags blockFlags;
  unsigned char blockType;
  unsigned char blockMode;
  unsigned short int blockSize; // Total block size, without header.
  unsigned int blockIndex;
};

// Message Block Type and Mode functions (straps)
#define DoolpMsg_getBlockType(_head_) \
  ((_head_)->blockType)
#define DoolpMsg_setBlockType(_head_, _type_) \
  (_head_)->blockType = _type_
#define DoolpMsg_getBlockMode(_head_) \
  ((_head_)->blockMode)
#define DoolpMsg_setBlockMode(_head_, _mode_) \
  (_head_)->blockMode = _mode_

// Message Block advanced Functions.
#define DoolpMsg_clearBlock(_head_) \
  memset ( _head_, 0, sizeof ( DoolpMsg_BlockHeader ) )
#define DoolpMsg_isBlockEnd(_head_)					\
  ((_head_)->blockMode == DoolpMsg_BlockMode_EndBlocks)

#define DoolpMsg_setBlockRawSize(_head_, _sz_) \
  (_head_)->blockType = DoolpMsg_BlockType_Raw; \
  (_head_)->blockSize = _sz_

#define DoolpMsg_getBlockRawSize(_head_) \
  ((_head_)->blockSize)
// Should better check if the block is Raw ???


typedef struct DoolpMsg_DoolpObjectHeader
{
  DoolpObjectNameId nameId;
  DoolpObjectId objectId;
  DoolpAgentId ownerAgentId;
};

#endif //  __DOOLP__DOOLPMSG_H

