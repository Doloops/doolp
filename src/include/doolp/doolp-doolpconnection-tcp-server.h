#ifndef __DOOLP_DOOLPCONNECTIONTCP_SERVER_H
#define __DOOLP_DOOLPCONNECTIONTCP_SERVER_H

#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolpforgeservices.h>

namespace Doolp
{
  class ConnectionTCPServer: public ForgeService
  {
    friend void * DoolpConnectionTCPServer_runAcceptThread ( void * arg );
  protected:
    Forge * forge;
    char type[16], address[128];
    int port;
    Socket listenSocket;
    pthread_t acceptThread;
    void * runAcceptThread ( );
  public:
    ConnectionTCPServer ( Forge * _forge, char * _type, char * _address, int _port );
    bool start();
    bool stop();
    Object * getServiceAsObject() { Warn ( "NOT APPLICABLE !\n" ); return NULL; }
  };
};
#endif // __DOOLP_DOOLPCONNECTIONTCP_SERVER_H
