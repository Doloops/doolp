/*
 * Glog is the Ultimate Logger for C and C++
 *
 * Usage : see glogers.h for initialization
 * Use Log, Info, Warn, Error, Fatal, Bug as printf() functions
 * Fatal and Bug will exit the program with exit(-1);
 * AssertFatal and AssertError will exit(-1) if the condition specified as first argument fails
 *
 * Notes : maybe we could use the va_list in _Glog instead of sprintf (delegate printing to GlogInfo...)
 */

#ifndef __GLOG__GLOG__GLOG__H
#define __GLOG__GLOG__GLOG__H
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/timeb.h>
#include <pthread.h>

#define __GLOG_PRIOR_LOG 0
#define __GLOG_PRIOR_INFO 1
#define __GLOG_PRIOR_WARN 2
#define __GLOG_PRIOR_ERROR 3
#define __GLOG_PRIOR_FATAL 4
#define __GLOG_PRIOR_BUG 5

#define __GLOG_PRIOR_MAX 6

#define __GLOG_GLOGITEM_MAXTEXTSIZE 512

#define __GLOG__STRINGIFY__Z(__x) #__x
#define __GLOG__STRINGIFY__(__x) __GLOG__STRINGIFY__Z(__x)

#ifdef __GLOG__MODULE__
static char * __glog_module__ = __GLOG__STRINGIFY__(__GLOG__MODULE__);
#else
static char * __glog_module__ = NULL;
#endif

class GlogItem
{
 public:
  unsigned char priority;
  timeb now;
  pthread_t thread;
  char * module;
  char * file;
  char * function;
  unsigned int line;
  char text[__GLOG_GLOGITEM_MAXTEXTSIZE];
};

class Gloger
{
 public:
  virtual ~Gloger() { fprintf ( stderr, "End of GlogInfo"); }
  virtual bool send ( GlogItem &item ) = 0;
  virtual bool flush () = 0;
  Gloger * nextGloger;
};

class GlogRoot
{
 protected:
  Gloger * firstGloger;
  char * progName;
  timeb offset;
  bool showPrior[__GLOG_PRIOR_MAX];
 public:
  GlogRoot(char * _progName);
  void __addGlog ( Gloger * gloger );
  void sendToAll ( GlogItem & item );
  
  // Accessors...
  timeb & getOffset() { return offset; }
  char * getProgName() const { return progName; }
  void setPriorShown(unsigned int prior, bool shown) { showPrior[prior] = shown; }
  bool isPriorShown(unsigned int prior) const { return showPrior[prior]; }
};
extern GlogRoot __GLOG__ROOT__;

#define _GLog(__Prior, ...)						\
  {									\
    if ( __GLOG__ROOT__.isPriorShown(__Prior)  )			\
      {									\
	GlogItem __glog__item__;					\
	__glog__item__.priority = __Prior;				\
	ftime ( &(__glog__item__.now) );				\
	__glog__item__.thread = pthread_self ();			\
	__glog__item__.module = __glog_module__;			\
	__glog__item__.file = (char*)__FILE__;				\
	__glog__item__.function = (char*)__FUNCTION__;			\
	__glog__item__.line = __LINE__;					\
	int __glog__res__ = snprintf ( __glog__item__.text,		\
				       __GLOG_GLOGITEM_MAXTEXTSIZE,	\
				       __VA_ARGS__ );			\
	if ( __glog__res__ >= __GLOG_GLOGITEM_MAXTEXTSIZE )		\
	  fprintf ( stderr,						\
		    "GLOG : Message too long (%d bytes), truncated.\n",	\
		    __glog__res__ );					\
	__GLOG__ROOT__.sendToAll ( __glog__item__ );			\
      }									\
  }


#ifdef LOG //__GLOG_SHOW_LOG
#define Log(...)   { _GLog ( __GLOG_PRIOR_LOG, __VA_ARGS__ );   }
#else
#define Log(...)
#endif
#define Info(...)  { _GLog ( __GLOG_PRIOR_INFO, __VA_ARGS__ );  }
#define Warn(...)  { _GLog ( __GLOG_PRIOR_WARN, __VA_ARGS__ );  }
#define Error(...) { _GLog ( __GLOG_PRIOR_ERROR, __VA_ARGS__ ); }
#define Fatal(...) { _GLog ( __GLOG_PRIOR_FATAL, __VA_ARGS__ ); exit (-1); }

#define Bug(...) { _GLog ( __GLOG_PRIOR_BUG, __VA_ARGS__ ); exit (-1); }

 
#define LogErrNo { _GLog ( __GLOG_PRIOR_INFO, "Errno : %d:%s\n", errno, strerror(errno) ); }


#define AssertFatal(condition,...) { if ( ! ( condition ) ) { Fatal ( "Assertion false : "/**/#condition/**/" : "/**/__VA_ARGS__ ); } }

#define AssertBug(condition,...) { if ( ! ( condition ) ) { Bug ( "Assertion false : "/**/#condition/**/" : "/**/__VA_ARGS__ ); } }

#endif // __GLOG__GLOG__GLOG__H
