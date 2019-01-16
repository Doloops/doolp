#include <stdio.h>
#include <stdlib.h>
#include <glog.h>
#include <glogers.h>
#include <fcntl.h>
#include <unistd.h>

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

setGlog ( "glogshowbin" );

int main ( int argc, char ** argv )
{
  GlogInfo * gi = new GlogInfo_File ( stdout );
  addGlog ( gi );
  if ( argc != 2 )
    {
      fprintf ( stderr, "Error : argc shall be equal to 2\n" );
      exit ( -1 );
    }
  int fd = open ( argv[1], O_RDONLY );
  if ( fd == -1 )
    {
      fprintf ( stderr, "Error : could not open '%s' for read\n", argv[1] );
      exit ( -1 );
    }
  //  FILE * fp = fopen ( argv[1], "r" );
  int brsize = sizeof ( GlogBinRecord );
  Log ( "glogshowbin : brsize=%d\n", brsize );
  GlogBinRecord br;
  GlogItem ir;
  ftime ( &(ir.now) ); // Set timezone, ...
  ir.file = br.file;
  ir.function = br.function;
  //  ir.text = br.text;
  unsigned int offset = 0;
  int byteoffset = 0;
  while ( true )
    {
      int res = read ( fd, &br, brsize );
      if ( res < brsize )
	{
	  fprintf ( stderr, "Incomplete read : quitting (byteoffset=%d).\n", byteoffset );
	  close ( fd );
	  exit ( -1);
	}
      byteoffset += brsize;
      ir.priority = br.priority;
      if ( offset == 0 )
	{
	  offset = br.t;
	  __GLOGINFO_OFFSET = offset;
	}
      ir.now.time = br.t;
      ir.now.millitm = br.tm;
      ir.thread = br.thread;
      ir.line = br.line;
      memcpy ( ir.text, br.text, __GLOG_GLOGITEM_MAXTEXTSIZE );
      br.file[__GLOG_GLOGITEM_MAXFILESIZE-1] = '\0';;
      br.function[__GLOG_GLOGITEM_MAXFUNCSIZE-1] = '\0';

      ir.text[__GLOG_GLOGITEM_MAXTEXTSIZE-1] = '\0';
      //      fprintf ( stderr, "file_ptr=0x%x, func_ptr=0x%x\n",
      //		br.file_ptr, br.func_ptr );
      gi->send ( ir );
    }
}
