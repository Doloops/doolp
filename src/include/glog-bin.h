#ifndef __GLOG_BIN_H
#define __GLOG_BIN_H

#include <glogers.h>

typedef struct
{
  unsigned int priority;
  unsigned int t;
  unsigned int tm;
  unsigned int thread;
  unsigned int file_ptr;
  char file[__GLOG_GLOGITEM_MAXFILESIZE];
  unsigned int func_ptr;
  char function[__GLOG_GLOGITEM_MAXFUNCSIZE];
  unsigned int line;
  char text[__GLOG_GLOGITEM_MAXTEXTSIZE];
} GlogBinRecord;

#define __GLOG_GLOGBINRECORD_SIZE (sizeof(GlogBinRecord))

#endif // __GLOG_BIN_H
