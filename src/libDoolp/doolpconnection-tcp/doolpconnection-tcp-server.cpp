#include <doolp/doolp-doolpconnection-tcp-server.h>
#include <doolp/doolp-doolpforge.h>
#include <doolp/doolp-doolpconnection-tcp.h>
#include <doolp/doolp-doolpconnection-xml.h>

DoolpConnectionTCPServer::DoolpConnectionTCPServer ( DoolpForge * _forge, char * _type, 
						     char * _address, int _port )
{
  forge = _forge;
  strncpy ( type, _type, 16 ); // TODO : check size
  strncpy ( address, _address, 128 );
  port = _port;
  listenSocket = 0;
  __DOOLP_Log ( "New Connection Server : %s:%d\n", address, port );
}

void * DoolpConnectionTCPServer_runAcceptThread ( void * arg )
{ return ((DoolpConnectionTCPServer*) arg)->runAcceptThread ( ); }

bool DoolpConnectionTCPServer::start()
{
  if ( forge == NULL )
    Bug ( "Can not openServer with myForge == NULL !\n" );
  if ( forge->getAgentId () == 0 )
    {
      Fatal ( "Can not open a TCP Server with an agentId=0\n" );
    }
  int res;
  struct sockaddr_in AdrServ;
  __DOOLP_Log ( "Starting Connection Server : %s:%d\n", address, port );
  
  int __sockAddrSz__ = sizeof ( struct sockaddr );
  
  if (  (listenSocket = socket(PF_INET, SOCK_STREAM, 0)) <= -1)
    //	if (  (listenSocket = socket(PF_INET, SOCK_SEQPACKET, 0)) <= -1)
    { Fatal ( "Unable to create listening socket" ); }
  memset(&AdrServ,0,sizeof AdrServ);
  AdrServ.sin_port = htons(port);
  AdrServ.sin_family = PF_INET;
  inet_aton(address,&(AdrServ.sin_addr));
  int reuse = 1;
  res = setsockopt ( listenSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof ( reuse ) );
  if ( (res = bind(listenSocket, (struct sockaddr *) &AdrServ, __sockAddrSz__ )) <= -1 )
    { Fatal ("Unable to bind() : result %d, error %d:%s\n", res, errno, strerror (errno) ); }
  if ( (listen(listenSocket, 5)) <= -1 )
    { Fatal ( "Unable to listen()\n" ); }
  Info ( "Server started at (%s:%d)\n", address, port );
  
  pthread_create ( &acceptThread, NULL, 
		   DoolpConnectionTCPServer_runAcceptThread,
		   (void*) this );
  return true;
}

void * DoolpConnectionTCPServer::runAcceptThread ( )
{
  __DOOLP_Log ( "Starting acceptThread for DoolpConnectionTCPServer %p, Forge %p, sock %d\n",
		this, forge, listenSocket );
  Socket clientSocket;
  
  while (true)
    {
      struct sockaddr_in clientAddr;
      socklen_t clientAddrSz = sizeof (struct sockaddr_in);
      Info ( "DoolpConnectionTCPServer %p : Waiting Clients\n", this );
      clientSocket = accept ( listenSocket, 
			      (struct sockaddr *)&clientAddr, 
			      &clientAddrSz );
      __DOOLP_Log ( "New Connection from %s:%d\n", 
		    inet_ntoa ( clientAddr.sin_addr ),
		    clientAddr.sin_port );
      if ( strcmp ( type, "TCP::BIN" ) == 0 )
	{
	  DoolpConnectionTCP * newConnection = new DoolpConnectionTCP ( forge, clientSocket );
	  newConnection->name = inet_ntoa ( clientAddr.sin_addr );
	  __DOOLP_Log ( "Created connection at %p\n", newConnection );
	  Info ( "New %s Client from %s\n", type, newConnection->name );
	  newConnection->status = DoolpConnectionStatus_ToBeHandShaked;
	  newConnection->handle ();
	}
      else if ( strcmp ( type, "TCP::XML" ) == 0 )
	{
	  DoolpConnectionXML * newConnection = new DoolpConnectionXML ( forge, clientSocket, -1, -1 );
	  newConnection->name = inet_ntoa ( clientAddr.sin_addr );
	  __DOOLP_Log ( "Created connection at %p\n", newConnection );
	  Info ( "New %s Client from %s\n", type, newConnection->name );
	  newConnection->status = DoolpConnectionStatus_ToBeHandShaked;
	  newConnection->handle ();
	}
      else
	Bug ( "Invalid DoolpConnectionTCPServer type specifier : '%s'\n",
	      type );
    }
  return NULL;
}


bool DoolpConnectionTCPServer::stop()
{
  return true;
}








#if 0
#warning DoolpConnectionTCP::openServer() will be deprecated : to delete
bool DoolpConnectionTCP::openServer ( char * listenAddress, int port )
{
  if ( myForge == NULL )
    Bug ( "Can not openServer with myForge == NULL !\n" );
  if ( myForge->getAgentId () == 0 )
    {
      Fatal ( "Can not open a TCP Server with an agentId=0\n" );
    }
  Socket listenSocket;
  int res;
  struct sockaddr_in AdrServ;
  isServer = true;
  __DOOLP_Log ( "New Connection Server : %s:%d\n", listenAddress, port );
  
  int __sockAddrSz__ = sizeof ( struct sockaddr );
  
  if (  (listenSocket = socket(PF_INET, SOCK_STREAM, 0)) <= -1)
    //	if (  (listenSocket = socket(PF_INET, SOCK_SEQPACKET, 0)) <= -1)
    { Fatal ( "Unable to create listening socket" ); }
  memset(&AdrServ,0,sizeof AdrServ);
  AdrServ.sin_port = htons(port);
  AdrServ.sin_family = PF_INET;
  inet_aton(listenAddress,&(AdrServ.sin_addr));
  int reuse = 1;
  res = setsockopt ( listenSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof ( reuse ) );
  if ( (res = bind(listenSocket, (struct sockaddr *) &AdrServ, __sockAddrSz__ )) <= -1 )
    { Fatal ("Unable to bind() : result %d, error %d:%s\n", res, errno, strerror (errno) ); }
  if ( (listen(listenSocket, 5)) <= -1 )
    { Fatal ( "Unable to listen()\n" ); }
  Info ( "Server started at (%s:%d)\n", listenAddress, port );
  
  
  mySocket = listenSocket;
  
  pthread_create ( &acceptThread, NULL, 
		   DoolpConnectionTCP_runAcceptThread,
		   (void*) this );
  return true;
}

void * DoolpConnectionTCP_runAcceptThread ( void * arg )
{ return ((DoolpConnectionTCP*) arg)->runAcceptThread ( ); }

void * DoolpConnectionTCP::runAcceptThread ( )
{
  __DOOLP_Log ( "Starting acceptThread for DoolpConnectionTCP %p, Forge %p, sock %d\n",
		this, myForge, mySocket );
  Socket clientSocket;
  
  while (true)
    {
      struct sockaddr_in clientAddr;
      socklen_t clientAddrSz = sizeof (struct sockaddr_in);
      Info ( "DoolpConnectionTCP %p : Waiting Clients\n", this );
      clientSocket = accept ( mySocket, 
			      (struct sockaddr *)&clientAddr, 
			      &clientAddrSz );
      __DOOLP_Log ( "New Connection from %s:%d\n", 
		    inet_ntoa ( clientAddr.sin_addr ),
		    clientAddr.sin_port );
      
      DoolpConnectionTCP * newConnection = newDoolpConnectionTCP ();
      newConnection->mySocket = clientSocket;
      newConnection->name = inet_ntoa ( clientAddr.sin_addr );
      newConnection->port = clientAddr.sin_port;
      __DOOLP_Log ( "Created connection at %p\n", newConnection );
      Info ( "New Client from %s\n", newConnection->name );
      newConnection->status = DoolpConnectionStatus_ToBeHandShaked;
      newConnection->handle ();
    }
  return NULL;
}
#endif
