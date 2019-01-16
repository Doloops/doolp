#ifndef __GLOGMACHINE_HANDLEINPUT_H
#define __GLOGMACHINE_HANDLEINPUT_H


#if 0
#include <semaphore.h>
#include <string>
#include <DMLXParser.h>
using namespace std;
// #include <gtkmm/main.h>
// #include "logView.hh"

typedef int Socket;

class LogView;

class ClientView
{
public:
  ClientView() {}
  LogView * logView;
  bool handled;
  string * source;
  sem_t handleEnd;
  sem_t doParse;
  int instanceIndex; // in mainWindow
  void parseOneEvent(); // Strap to logView.
  void setDisconnected(); // Mark this client disconnected.
  DMLXParserMMap * parser;
};

bool handleLoop ( ClientView *, Socket );
void * handle ( void * args );

ClientView * getClient ( string& source, DMLXParserMMap * parser );

#endif

#endif // __GLOGMACHINE_HANDLEINPUT_H
