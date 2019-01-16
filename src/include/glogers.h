#ifndef __GLOG__GLOG__GLOGERS__H
#define __GLOG__GLOG__GLOGERS__H

#include <glog.h>
#include <pthread.h>

#include <map>
/*
 * GlogInfo Senders
 */

#define setGlog(__prog_name__)				\
  GlogRoot __GLOG__ROOT__(__prog_name__);

#define addGlog(__glog__)			\
  __GLOG__ROOT__.__addGlog (__glog__)

#define showGlogPrior(__Prior__,__show__) \
  __GLOG__ROOT__.setPriorShown(__Prior__,__show__)

class Gloger_File : public Gloger
{
  FILE * fp;
 public:
  Gloger_File ( FILE * _fp ) { fp = _fp; }
  Gloger_File ( char * name );
  bool send ( GlogItem &item );
  bool flush ();
};

class Gloger_XMLFile : public Gloger
{
  FILE * fp;
 public:
  Gloger_XMLFile ( FILE * _fp ) { fp = _fp; }
  Gloger_XMLFile ( char * name );
  bool send ( GlogItem &item );
  bool flush ();
};

class Gloger_XMLSock : public Gloger
{
 protected:
  int sock;
  char writeBuff[1024];
  pthread_mutex_t mutex;
 public:
  Gloger_XMLSock ( char * sourceName, char * address, unsigned int port );

  bool send ( GlogItem &item );
  bool flush ();
};

class Gloger_DMLXBinFile : public Gloger
{
 protected:
  int fidles;
  pthread_mutex_t _lock;
  std::map<unsigned int, bool> sentModules;
  std::map<unsigned int, bool> sentFiles;
  std::map<unsigned int, bool> sentFuncs;
 public:
  Gloger_DMLXBinFile ( char * file );
  bool send ( GlogItem &item );
  bool flush ();
};


#define __GLOG_GLOGITEM_MAXFILESIZE 64
#define __GLOG_GLOGITEM_MAXFUNCSIZE 32
class Gloger_BinFile : public Gloger // THIS IS NOW DEPRECATED : Gloger_DMLXBinFile has better space saving...
{
  int fd;
  pthread_mutex_t _lock;
 public:
  Gloger_BinFile ( int _fd ) { fd = _fd; pthread_mutex_init ( &_lock, NULL );}
  Gloger_BinFile ( char * name );
  bool send ( GlogItem &item );
  bool flush ();
};


#endif // __GLOG__GLOG__GLOGERS__H
