#ifndef __DOOLP_DOOLPCONNECTIONTCP_H
#define __DOOLP_DOOLPCONNECTIONTCP_H

/*
  TCP Implementation of the DoolpConnection.

*/
#include <arpa/inet.h>
#include <signal.h>
#include <semaphore.h>

#include <doolp/doolp-doolpconnection.h>
#include <doolp/doolp-doolpconnection-tcp-msg.h>

typedef struct DoolpConnectionTCP_Identifier
{
  char name[32];
  u8 version;
  u8 subversion;
  u8 patchlevel; 
} __DoolpConnectionTCP_Identifier;

#define __tryReadBlockHeader \
  if ( ! blockHeaderIsRead ) \
    { readBlockHeader ( &bh ); \
      blockHeaderIsRead = true; } 


class DoolpConnectionTCP : public DoolpConnection
{
 public:
  DoolpConnectionTCP_Identifier identifier;
 protected:
#ifdef __DOOLPCONNECTIONTCP_WRITEBUFF
  char writeBuff[__DOOLPCONNECTIONTCP_WRITEBUFF_SZ];
  int writeBuffSz;
#endif // __DOOLPCONNECTIONTCP_WRITEBUFF
  unsigned int retryNb;
  unsigned int retryWait;
  Socket mySocket;

  unsigned int nextBlockIndex; // To Be Set by setNextBlockIndex () function
  bool blockHeaderIsRead;
  DoolpMsg_Header msgh; // There may be one for read, one for write
  DoolpMsg_BlockHeader bh;
  void __initTCP(DoolpForge * forge );
 protected:
  DoolpConnectionTCP::DoolpConnectionTCP ();

 public:
  DoolpConnectionTCP::DoolpConnectionTCP (DoolpForge *);
  DoolpConnectionTCP::DoolpConnectionTCP (DoolpForge *, Socket);

  DoolpConnectionTCP::~DoolpConnectionTCP ();

  bool tryConnect ( char * host, int port );
  bool endConnection ();

  /*
    Handshaking 
  */
  bool handShake ();
  bool checkIdentifier (  DoolpConnectionTCP_Identifier * distIdentifer );

  bool runHandleThread ( DoolpCall * specificCall, DoolpStreamVirtual * specificStream);

  /*
    Handle Incoming Data Section
   */
 protected:
  unsigned int handleMsg ( DoolpCall * specificCall, DoolpStreamVirtual * specificStream);
  bool handleMsg ( );

  bool handleMsgCall ( );
  bool handleMsgNewCall ( );
  bool handleMsgReply ( );
  bool handleMsgForward ( );
  bool handleMsgBroadcast ( );

  bool forwardMessage ( DoolpConnection * to ); 

  bool isMessageForCall ( DoolpMsg_Header * head, DoolpCall * call );

  int waitRead ( int timeout_sec, int timeout_usec );
  int waitRead ( );

  /*
   * Read and write low-level DoolpConnection protocols
   */
 protected:
  /*
   * TCP Lowlevel writing
   */
  ssize_t rawWrite(const void *buf, size_t nbyte);
  ssize_t rawRead(void *buf, size_t nbyte);

  bool writeBlockHeader(DoolpMsg_BlockHeader * h);
  bool readBlockHeader(DoolpMsg_BlockHeader * h);

  /*
   * Doolp Prototype for Raw writing, into Doolp Protocol
   */
  ssize_t Write (unsigned int type, const void * buf, size_t nbyte );

 public:
  ssize_t Write(const void *buf, size_t nbyte)
    { return Write ( DoolpMsg_BlockType_Raw, buf, nbyte ); }

  ssize_t Read(void *buf, size_t nbyte);
  bool flushWrite ();
 protected:
  bool readMsgType ( unsigned int type );
  bool sendZeroSizeBlock ( unsigned int mode,
			   unsigned int type );

  bool startMessage ( DoolpCallContext * callContext,
		      DoolpMsg_CallFlags extraFlags );
  bool startMessage ( DoolpMsg_Header * header );

  bool startSubSection ( unsigned int type );

  bool doolpStream_hasSentHeader;
  bool readStreamContents ( DoolpCallContext * callContext, DoolpStreamIndex idx );

 public:
#define __DOOLP_VIRTUAL__ virtual
#define __DOOLP_VIRTUAL_IMPL__ 
#include <doolp/doolp-doolpconnection-virtualfuncs.h>

};

void * DoolpConnectionTCP_runAcceptThread ( void * arg );
void * DoolpConnectionTCP_runHandleThread ( void * arg );


void  __DOOLPCONNECTIONTCP_LogMsg ( FILE * fp, void * buffer, unsigned int bufferIdx );

#endif 
