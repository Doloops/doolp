#include <glogers.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
extern char * __GLOG__PRIOR__STRING[];


/*
 * Standard File printing
 */

Gloger_File::Gloger_File ( char * name )
{
  fp = fopen ( name, "w" );
  if ( fp == NULL )
    { fprintf ( stderr, "Can not open log file.\n" ); exit ( -1); }
}
bool Gloger_File::send ( GlogItem &item )
{
  int tm = (int)(item.now.millitm - __GLOG__ROOT__.getOffset().millitm);
  int t = (int)(item.now.time - __GLOG__ROOT__.getOffset().time);
  if ( tm < 0 )
    { tm += 1000; t += 1; }
  fprintf ( fp, "[%d:%d][%x][%s:%s:%u:%s] %s : ",
	    t, tm, item.thread, 
	    item.module != NULL ? item.module : "*", item.file, item.line, item.function,
	    __GLOG__PRIOR__STRING[item.priority] );
  fprintf ( fp, item.text );
  fflush ( fp );
  return true;
}

bool Gloger_File::flush () 
{ fflush (fp); return true; }

Gloger_XMLFile::Gloger_XMLFile ( char * name )
{
  fp = fopen ( name, "w" );
  if ( fp == NULL )
    { fprintf ( stderr, "Can not open XML log file.\n" ); exit ( -1); }
}
bool Gloger_XMLFile::send ( GlogItem &item )
{
  fprintf ( fp, "<glog t=%d tm=%d thread=\"0x%x\" file=\"%s\" line=%d function=\"%s\" prior=\"%s\">",
	    item.now.time, item.now.millitm,
	    item.thread, item.file, item.line, item.function, 
	    __GLOG__PRIOR__STRING[item.priority] );
  fprintf ( fp, item.text );
  fprintf ( fp, "</glog>" );
  fflush ( fp );
  return true;
}
bool Gloger_XMLFile::flush ()
 { fflush ( fp ); return true; }


#if 0 // DEPRECATED !!!
Gloger_XMLSock::Gloger_XMLSock ( char * sourceName, char * host, unsigned int port )
{
  fprintf ( stderr, "GLOG : Connecting XML Socket to : %s:%d\n", host, port );
  pthread_mutex_init ( &mutex, NULL );
  int res;
  struct sockaddr_in AdrServ;
  int __sockAddrSz__ = sizeof ( struct sockaddr );
  if (  (sock = socket(PF_INET, SOCK_STREAM, 0)) <= -1)
    { fprintf ( stderr, "GLOG : Unable to create socket for Gloger_XMLSOCK." ); }
  memset(&AdrServ,0,sizeof AdrServ);
  AdrServ.sin_port = htons(port);
  AdrServ.sin_family = PF_INET;
  inet_aton(host,&(AdrServ.sin_addr));
  if ( (res = connect(sock, (struct sockaddr *) &AdrServ, __sockAddrSz__ )) <= -1 )
    { fprintf ( stderr, "GLOG : Unable to connect XML Socket : error %s\n", strerror(errno) ); }
  char buffer[64];
  sprintf ( buffer, "<?xml version=\"1.0\"?>\n" );
  write ( sock, buffer, strlen ( buffer ) );
  sprintf ( buffer, "<source name=\"%s\" offset=\"%d\" modulename=\"%s\"/>\n", 
	    sourceName, 
	    (int) __GLOGINFO_OFFSET,
	    __GLOGINFO_MODULENAME );
  write ( sock, buffer, strlen ( buffer ) );
}

bool Gloger_XMLSock::send ( GlogItem &item )
{
  if ( sock == 0 )
    return false;
  pthread_mutex_lock ( &mutex );
  int idx = 0;
  idx += sprintf ( writeBuff+idx, 
		   "<glog t=\"%d\" ut=\"%d\" thread=\"0x%x\" file=\"%s\" "
		   "line=\"%d\" function=\"%s\" prior=\"%s\">",
		   (int)item.now.time, (int)item.now.millitm,
		   (int)item.thread, item.file, item.line, item.function, 
		   __GLOG__PRIOR__STRING[item.priority] );
  //  idx += sprintf ( writeBuff, item.text );
  int c = 0;
  while ( item.text[c] != '\0' )
    {
      switch ( item.text[c] )
	{
	case '<': idx+=sprintf ( writeBuff+idx, "&lt;" ); break;
	case '>': idx+=sprintf ( writeBuff+idx, "&gt;" ); break;
	case '&': idx+=sprintf ( writeBuff+idx, "&amp;" ); break;
	case '"': idx+=sprintf ( writeBuff+idx, "&quot;" ); break;
	default: writeBuff[idx++] = item.text[c];
	}
      c++;
    }
  idx += sprintf ( writeBuff+idx, "</glog>" );
  //  write ( sock, writeBuff, idx );
  int res = ::send ( sock, writeBuff, idx, MSG_DONTWAIT + MSG_NOSIGNAL );
  fsync ( sock );
  if ( res == -1 )
    {
      fprintf ( stderr, "GLOG : XMLSOCK : res=%d, errno=%d:%s\n", res,
		errno, strerror ( errno ) );
      sock = 0;
    }
  pthread_mutex_unlock ( &mutex );
  return true;
}

bool Gloger_XMLSock::flush ()
{
  return true;
}
#endif

#if 0 // DEPRECATED
Gloger_BinFile::Gloger_BinFile ( char * name )
{
  creat ( name, S_IRUSR | S_IWUSR );
  fd = open ( name, O_WRONLY | O_TRUNC );
  pthread_mutex_init ( &_lock, NULL );
}

bool Gloger_BinFile::send ( GlogItem &item )
{
  char c[__GLOG_GLOGITEM_MAXTEXTSIZE];
  memset ( c, 0, __GLOG_GLOGITEM_MAXTEXTSIZE );
  pthread_mutex_lock ( &_lock );
  unsigned int prior = item.priority;
  write ( fd, &prior, 4 );
  unsigned int t = item.now.time; //  - __GLOGINFO_OFFSET;
  write ( fd, &t, 4 );
  unsigned int tm = item.now.millitm;
  write ( fd, &tm, 4 );
  write ( fd, &item.thread, 4 );
  unsigned int file_ptr = (unsigned int)item.file;
  write ( fd, &file_ptr, 4 );
  int i = 0;
  i = strlen ( item.file ) + 1;
  write ( fd, item.file, i ); // __GLOG_GLOGITEM_MAXFILESIZE );
  write ( fd, c, __GLOG_GLOGITEM_MAXFILESIZE - i );
  unsigned int func_ptr = (unsigned int)item.function;
  write ( fd, &func_ptr, 4 );
  i = strlen ( item.function ) + 1;
  write ( fd, item.function, i ); // __GLOG_GLOGITEM_MAXFUNCSIZE );
  write ( fd, c, __GLOG_GLOGITEM_MAXFUNCSIZE - i );
  write ( fd, &item.line, 4 );
  i = strlen ( item.text ) + 1;
  write ( fd, item.text, i ); // __GLOG_GLOGITEM_MAXTEXTSIZE );
  write ( fd, c, __GLOG_GLOGITEM_MAXTEXTSIZE - i );
  pthread_mutex_unlock ( &_lock );
  return true;
}

bool Gloger_BinFile::flush ()
{
  fsync ( fd );
  return true;
}
#endif
