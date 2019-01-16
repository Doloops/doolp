// generated 2005/1/28 17:08:01 CET by forgoto@doloop.(none)
// using glademm V2.0.0.1
//
// newer (non customized) versions of this file go to glogmachine.cc_new

// This file is for your program, I won't touch it again!

#include <glogmachine_config.h>
#include <sigc++/sigc++.h>
#include <gtkmm/main.h>

#include <DMLXParser.h>
#include <pthread.h>
#include <semaphore.h>
#include <list>
#include <map>
using namespace std;
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include <mainWindow.h>
#include <logView.h>
#include <handleInput.h>


// #define Log(...) fprintf(stderr,__VA_ARGS__)
// #define Fatal(...) { fprintf(stderr,__VA_ARGS__); exit (-1); }
#include <glogers.h>
setGlog ( "glogmachine" );

Gtk::Main * m;
MainWindow * mainWindow;

#if 0


std::map<string, ClientView *> sourcesViews;

int addClientSource ( Glib::ustring& source )
{ return mainWindow->addSource ( source ); }

void * accept ( void * args )
{
  Socket listenSocket;
  struct sockaddr_in AdrServ;
  int res;
  char * listenAddress = "0.0.0.0";
  int listenPort = 13807;
  int __sockAddrSz__ = sizeof ( struct sockaddr );
  if ( ( listenSocket = socket ( PF_INET, SOCK_STREAM, 0 ) ) <= -1 )
    Fatal ( "Unable to create listening socket" );
  memset(&AdrServ,0,sizeof AdrServ);
  AdrServ.sin_port = htons(listenPort);
  AdrServ.sin_family = PF_INET;
  inet_aton(listenAddress,&(AdrServ.sin_addr));
  int reuse = 1;
  res = setsockopt ( listenSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof ( reuse ) );
  if ( (res = bind(listenSocket, (struct sockaddr *) &AdrServ, __sockAddrSz__ )) <= -1 )
    { Fatal ("Unable to bind() : result %d, error %d:%s\n", res, errno, strerror (errno) ); }
  if ( (listen(listenSocket, 5)) <= -1 )
    { Fatal ( "Unable to listen()\n" ); }
  Log ( "Server started at (%s:%d)\n", listenAddress, listenPort );
  while (true)
    {
      struct sockaddr_in clientAddr;
      socklen_t clientAddrSz = sizeof (struct sockaddr_in);
      Socket clientSocket = accept ( listenSocket, (struct sockaddr *)&clientAddr, 
				     &clientAddrSz );
      Log ( "New connection from %s:%d\n",
	    inet_ntoa ( clientAddr.sin_addr ),
	    clientAddr.sin_port );
      pthread_t handlethread;
      pthread_create ( &handlethread, NULL, handle, (void*)clientSocket );
      pthread_detach ( handlethread );
    }
}

ClientView * getClient ( string& source, DMLXParserMMap * parser )
{
#warning TODO : cleanup this function.
  ClientView * client;
  LogView * view;
  Log ( "Source is '%s'\n", source.c_str () );
  //  int instanceIndex = mainWindow->addSource ( (Glib::ustring&) source );
  int instanceIndex = addClientSource ( (Glib::ustring&) source );
  if ( sourcesViews[source] == NULL )
    {
      Log ( "New LogView for source '%s'\n", source.c_str () );
      view = new LogView ( parser, source );
      view->init ();
      Log ( "View inited.\n" );
      client = new ClientView ();
      client->logView = view;
      client->parser = parser;
      client->handled = true;
      client->source = new string ( source );
      client->instanceIndex = instanceIndex;
      sem_init ( &(client->handleEnd), 0, 0 );
      sem_init ( &(client->doParse), 0, 0 );
      sourcesViews[source] = client;
    }
  else
    {
      client = sourcesViews[source];
      if ( client->handled )
	{
	  Log ( "Source '%s' still handled !\n", source.c_str() );
	  // delete ( parser );
	  return NULL;
	}
      view = client->logView;
      Log ( "Re-using existing View at '%p'\n", view );
      client->handled = true;
      client->parser = parser;
      client->instanceIndex = instanceIndex;
      sem_init ( &(client->handleEnd), 0, 0 );
      sem_init ( &(client->doParse), 0, 0 );
      view->clearLogs ();
      view->rebind ( parser );
      view->init ();
    }
  if ( view == NULL )
    { Warn ( "No view defined for this source '%s'\n", source.c_str() ); exit (-1); }
  return client;
}

void ClientView::parseOneEvent()
{
  logView->parseOneEvent();
}

void ClientView::setDisconnected()
{
  handled = false;
  logView->setBindedState ( false );
  logView->rebind ( NULL );
}

bool removeLogView ( LogView * logView )
{
  //std::map<string, LogView *> sourcesViews;
  for ( std::map<string, ClientView *>::iterator iter = sourcesViews.begin () ;
	iter != sourcesViews.end () ; iter ++ )
    {
      if ( iter->second->logView == logView )
	{
	  ClientView * client = iter->second;
	  if ( client->handled )
	    {
	      client->handled = false;
	      fprintf ( stderr, "Waiting for handle thread to stop.\n" );
	      sem_wait ( &(client->handleEnd) );
	      fprintf ( stderr, "Handle thread stopped.\n" );
	    }
	  delete ( client->source );
	  delete ( client );
	  sourcesViews.erase ( iter );
	  return true;
	}
    }
  Bug ( "LogView not found.\n" );
  return false;
}
#endif
int main(int argc, char **argv)
{  
  addGlog ( new Gloger_File ( stderr ) );
  AssertFatal ( argc == 2, "Syntax : glogmachine [logfile.dmlx]\n" );
  m = new Gtk::Main (&argc, &argv);
  Glib::ustring g = "Unknown";
  LogView *logview = new class LogView( argv[1], g );
  //  logview->iconify();
  //  logview->hide ();

  // logview->init ();
  //logview->iconify ();
  //  mainWindow = new class MainWindow ();
  //  pthread_t acceptThread;
  //  pthread_create ( &acceptThread, NULL, accept, NULL );

  m->run ( *logview ); // Shall run the main window here.
  // m->run ( *mainWindow );
  return 0;
}
