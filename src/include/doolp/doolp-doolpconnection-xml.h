#ifndef __DOOLP_DOOLPCONNECTIONXML_H
#define __DOOLP_DOOLPCONNECTIONXML_H

#include <sys/timeb.h>

#include <map>
#include <list>
#include <string>

using namespace std;

#include <doolp/doolp-doolpconnection.h>
#include <DMLXParser.h>
#include <DMLXWriter.h>

#define __DOOLPXML_Log(...) __DOOLP_Log ( __VA_ARGS__)
#define __DOOLPXML_LogEvents(...) __DOOLP_Log(__VA_ARGS__)
#define __DOOLPXML_FWD_Log(...) __DOOLP_Log ( __VA_ARGS__)

#define __DOOLPCONNECTIONXML_WRITEBUFF_SZ 1024

namespace Doolp
{

  class ConnectionXML : public Connection
  {
    friend class ConnectionTCPServer;
  protected:
    Forge * forge;
    Socket mySocket;
    int inputDump, outputDump;
    static const unsigned int writeBufferMax = __DOOLPCONNECTIONXML_WRITEBUFF_SZ;
    unsigned int writeBufferIdx;
    char writeBuffer[__DOOLPCONNECTIONXML_WRITEBUFF_SZ];
  
    // Current contextId
    FullContextId * contextId; 
    DMLX::Parser * parser;
    DMLX::Writer * writer;
    // Next Block Index Handling, set by setNextBlockIndex () function
    unsigned int nextBlockIndex;

    // Stream-specific methods
    bool doolpStreamIsInside; // Inside of a Stream section
    bool doolpStreamHasSentHeader; // when setStream() must send header, doolpStreamHasSentHeader is set to true

    // Ping mechanism
    struct timeb pingTime;
  protected:
    void __initXML ( Forge * _forge, int _inputDump, int _outputDump );
    ConnectionXML ( Forge * _forge, Socket _mySocket, int _inputDump, int _outputDump );

  public:
    ConnectionXML ( Forge * _forge );
    ConnectionXML ( Forge * _forge, int _inputDump, int _outputDump );
    ConnectionXML ( Forge * _forge, char * inputDumpFile, char * outputDumpFile );
  
    bool tryConnect ( char * host, int port );
    bool endConnection ();
    bool flushWrite ();
  
  protected:
    bool handShake ();

  public:
    // Handling Mechanism
    unsigned int handleMsg ( Call * specificCall, StreamVirtual * specificStream );
    bool runHandleThread ( Call * specificCall, StreamVirtual * specificStream );
    
    bool ping ();
    // Forward Capabilities
  protected:
    bool forwardMessage ( Connection * connection );
    bool Write ( DMLX::Event* event );

  public: // Virtual Read/Write Funcs from Connection
#define __DOOLP_VIRTUAL__ 
#define __DOOLP_VIRTUAL_IMPL__ 
#include <doolp/doolp-doolpconnection-virtualfuncs.h>
  public: // Shared Keywords
    class Keywords
    {
      typedef DMLX::Keyword Keyword;
      DMLX::HashTree hashTree;
    public:
      Keywords();
      inline DMLX::HashTree * getHashTree () { return &hashTree; }
#define keyword(__k) Keyword __k;
      #include <doolp/doolp-doolpconnection-xml-keywords.h>
#undef keyword
    };
    static Keywords keywords;
  };

}; // NameSpace Doolp
#endif // __DOOLP_DOOLPCONNECTIONXML_H
