#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <glogers.h>
#include <DMLXBin.h>
#include <DMLXKeyword.h>

static DMLX::Keyword keyw_program("program");
static DMLX::Keyword keyw_offset("offset");
static DMLX::Keyword keyw_glog("glog");
static DMLX::Keyword keyw_t("t");
static DMLX::Keyword keyw_tm("tm");
static DMLX::Keyword keyw_thread("thread");
static DMLX::Keyword keyw_module("module");
static DMLX::Keyword keyw_file("file");
static DMLX::Keyword keyw_func("func");
static DMLX::Keyword keyw_line("line");
static DMLX::Keyword keyw_prior("prior");
static DMLX::Keyword keyw_text("text");
static DMLX::Keyword keyw_id("id");
static DMLX::Keyword keyw_value("value");

#define __write(__buff,__sz) write ( fidles, __buff, __sz )


Gloger_DMLXBinFile::Gloger_DMLXBinFile ( char * file )
{
  creat ( file, S_IRUSR | S_IWUSR );
  fidles = open ( file, O_WRONLY | O_TRUNC );
  pthread_mutex_init ( &_lock, NULL );
  pthread_mutex_lock ( &_lock );
  __DMLXBIN_writeMarkup (__write, keyw_program );
  __DMLXBIN_writeMarkupAttr (__write, keyw_value,__GLOG__ROOT__.getProgName() );
  __DMLXBIN_writeMarkupEnd (__write);
  unsigned int offset_t = (unsigned int)__GLOG__ROOT__.getOffset().time;
  unsigned int offset_tm = (unsigned int)__GLOG__ROOT__.getOffset().millitm;
  __DMLXBIN_writeMarkup (__write, keyw_offset );
  __DMLXBIN_writeMarkupAttrInt (__write,keyw_t,offset_t);
  __DMLXBIN_writeMarkupAttrInt (__write,keyw_tm,offset_tm);
  __DMLXBIN_writeMarkupEnd (__write);
  pthread_mutex_unlock ( &_lock );
}

bool Gloger_DMLXBinFile::send ( GlogItem &item )
{
  pthread_mutex_lock ( &_lock );
  unsigned int module = (unsigned int) item.module;
  unsigned int file = (unsigned int) item.file;
  unsigned int func = (unsigned int) item.function;
  if ( ( module != 0x0 ) && ( !sentModules[module]) )
    {
      sentModules[module] = true;
      __DMLXBIN_writeMarkup(__write,keyw_module);
      __DMLXBIN_writeMarkupAttrHex(__write,keyw_id,module);
      __DMLXBIN_writeMarkupAttr(__write,keyw_value,item.module);
      __DMLXBIN_writeMarkupEnd(__write);
    }
  if ( ! sentFiles[file] )
    {
      sentFiles[file] = true;
      __DMLXBIN_writeMarkup(__write,keyw_file);
      __DMLXBIN_writeMarkupAttrHex(__write,keyw_id,file);
      __DMLXBIN_writeMarkupAttr(__write,keyw_value,item.file);
      __DMLXBIN_writeMarkupEnd(__write);
    }
  if ( ! sentFuncs[func] )
    {
      sentFuncs[func] = true;
      __DMLXBIN_writeMarkup(__write,keyw_func);
      __DMLXBIN_writeMarkupAttrHex(__write,keyw_id,func);
      __DMLXBIN_writeMarkupAttr(__write,keyw_value,item.function);
      __DMLXBIN_writeMarkupEnd(__write);
    }
  __DMLXBIN_writeMarkup(__write, keyw_glog);
  unsigned int prior = item.priority;
  __DMLXBIN_writeMarkupAttrInt(__write,keyw_prior,prior);
  unsigned int t = item.now.time;
  unsigned int tm = item.now.millitm;
  __DMLXBIN_writeMarkupAttrInt(__write,keyw_t,t);
  __DMLXBIN_writeMarkupAttrInt(__write,keyw_tm,tm);
  __DMLXBIN_writeMarkupAttrHex(__write,keyw_thread,item.thread);
  
  if ( module != 0x0 )
    {
      __DMLXBIN_writeMarkupAttrHex(__write,keyw_module,module);
    }
  
  __DMLXBIN_writeMarkupAttrHex(__write,keyw_file, file);
  __DMLXBIN_writeMarkupAttrHex(__write,keyw_func, func);
  __DMLXBIN_writeMarkupAttrInt(__write,keyw_line, item.line);
  __DMLXBIN_writeMarkupAttr(__write,keyw_text, item.text);
  __DMLXBIN_writeMarkupEnd(__write);
  pthread_mutex_unlock ( &_lock );
  //  flush ();
  return true;
}

bool Gloger_DMLXBinFile::flush ()
{
  fsync ( fidles );
  return true;
}
