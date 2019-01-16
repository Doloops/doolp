#include <glog.h>
#include <DMLXWriter.h>
#include <stdio.h>
#include <unistd.h>
#include <DMLXBin.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>


#define __DMLX_WRITER_WRITEBUFFER_SIZE 1024
#undef AssertBug
#define AssertBug(...)

#if 1
#undef Log
#define Log(...)
#endif 

void DMLX::Writer::__init()
{
  writeBufferMax = __DMLX_WRITER_WRITEBUFFER_SIZE;
  writeBuffer = (char*) malloc ( __DMLX_WRITER_WRITEBUFFER_SIZE );
  markupLevel = 0;
  markupState = MarkupState_PreClosed;
  writeBufferIdx = 0;
  dumpFidles = -1;
  fidles = -1;
  indentation = true;
  prettyWrite = true;
  __writeBin = false;
  useSend = false;
  writeBinKeywords = false;
  tabs[0] = '\0';
  hasWrittenText = false;
}

DMLX::Writer::Writer ( int _fidles, bool _writeBin )
{
  __init ();
  Log ( "New DMLXWriter, fidles=%d, writeBin=%s\n",
	_fidles, _writeBin ? "true" : "false" );
  fidles = _fidles;
  __writeBin = _writeBin;
}

DMLX::Writer::Writer ( const char * filename )
{
  __init ();
  int fd = creat ( filename, S_IRUSR + S_IWUSR );
  fd = open ( filename, O_WRONLY );
  AssertFatal ( fd != -1, "Could not open file '%s'\n", filename );
  fidles = fd;
  __writeBin = false;
  Log ( "New DMLXWriter, file '%s', fidles=%d\n",
	filename, fd );
}

DMLX::Writer::~Writer ()
{
}

DMLX::WriterException::WriterException ( char * _message )
{ message = _message; }

DMLX::WriterException::~WriterException ()
{}

void DMLX::Writer::setDumpFidles ( int _dumpFidles )
{
  dumpFidles = _dumpFidles;
}

bool DMLX::Writer::flush ()
{
  if ( writeBufferIdx == 0 )
    {
      Log ( "DMLX::Writer : nothing to flush ()\n" );
      return true;
    }
  if ( fidles != -1 )
    {
      if ( useSend )
	{
	  int res = send ( fidles, writeBuffer, writeBufferIdx, MSG_DONTWAIT + MSG_NOSIGNAL );
	  if ( res == -1 )
	    {
	      Warn ( "Could not write : Error %d:%s\n", errno, strerror ( errno ) );
	      throw new WriterException ( "Could not write.\n" );
	    }
	}
      else
	{
	  int res = write ( fidles, writeBuffer, writeBufferIdx );
	  if ( res == -1 )
	    {
	      Warn ( "Could not write : Error %d:%s\n", errno, strerror ( errno ) );
	      throw new WriterException ( "Could not write.\n" );
	    }
	  // fsync ( fidles );
	}
    }
  if ( dumpFidles != -1 )
    {
      fsync ( dumpFidles );
    }
  Log ( "Flushed %d bytes\n", writeBufferIdx );
  writeBufferIdx = 0;
  return true;
}

#define isLockWrite() true

#define __DCXML_RAWWRITE(...)						\
  { AssertBug ( !writeBin(), "Shall not be here !\n" );			\
    AssertBug ( isLockWrite(), "Connection is not locked for writing !\n" ); \
    unsigned int __last = writeBufferIdx;				\
    unsigned int __nb = snprintf ( &(writeBuffer[writeBufferIdx]),	\
				   writeBufferMax - writeBufferIdx,	\
				   __VA_ARGS__ );			\
    AssertBug ( __nb < writeBufferMax, "Too much to write at the same time !\n" ); \
    if ( __nb >= writeBufferMax - writeBufferIdx )			\
      { flush (); __nb = snprintf ( &(writeBuffer[writeBufferIdx]),	\
			  writeBufferMax - writeBufferIdx,		\
			  __VA_ARGS__ ); }				\
    writeBufferIdx += __nb;						\
    if ( dumpFidles != -1 )						\
      write ( dumpFidles, &(writeBuffer[__last]), __nb );		\
    flush ();								\
  }
  
#define __DMLX_WRITEBIN(__buff,__sz)				\
  { AssertBug ( writeBin(), "writeBin() not set !\n" );			\
    Log ( "Writing %d bytes in bin (idx=%d)\n", __sz, writeBufferIdx );	\
    if ( fidles != -1 )							\
      {									\
	if ( writeBufferIdx + __sz >= writeBufferMax )			\
	  flush ();							\
	memcpy ( &(writeBuffer[writeBufferIdx]), __buff, __sz );	\
	writeBufferIdx += __sz;						\
      }									\
    if ( dumpFidles != -1 )						\
      { write ( dumpFidles, __buff, __sz ); /*fsync ( dumpFidles );*/ }	\
  } 

#define __DMLX_WRITEBINKEYWORD(__keyword)				\
  { if ( ! keywordWritten[__keyword.getHash()] )			\
      {									\
	Log ( "Writing keyword '%s' (hash '0x%x'\n", __keyword.getKeyword(), __keyword.getHash() ); \
	__DMLX_WRITEBINFLAG(DMLXBinFlag_Keyword);			\
	DMLX::KeyHash _Khash = __keyword.getHash();			\
	__DMLX_WRITEBIN(&_Khash, DMLXBinHashSz);			\
	DMLXBinTextLength __ln = strlen ( __keyword.getKeyword() );	\
	__DMLX_WRITEBIN(&__ln, DMLXBinTextLengthSz );			\
	__DMLX_WRITEBIN(__keyword.getKeyword(), __ln );			\
	keywordWritten[__keyword.getHash()] = true;			\
      }									\
  }

#define __DMLX_WRITEBINFLAG(__flag)					\
  { DMLXBinFlag _flag = __flag; __DMLX_WRITEBIN(&_flag,DMLXBinFlagSz); }

#define __DMLX_WRITEBINHASH(__flag,__keyword)				\
  { if ( writeBinKeywords ) __DMLX_WRITEBINKEYWORD(__keyword);		\
    __DMLX_WRITEBINFLAG(__flag);					\
    DMLX::KeyHash _hash = __keyword.getHash();				\
    __DMLX_WRITEBIN(&_hash,DMLXBinHashSz); }

bool DMLX::Writer::writeMarkup ( DMLX::Keyword& markupName )
{
  Log ( "Writing Markup '%s'\n", markupName.getKeyword() );
  if ( fidles == -1 )
    return false;
  hasWrittenText = false;
  if ( markupLevel > 0 )
    {
      if ( ! writeBin () &&
	   ( markupState == MarkupState_Openned ) )
	{
	  __DCXML_RAWWRITE( ">" );
	  if ( prettyWrite ) __DCXML_RAWWRITE( "\n" );
	}
    }
  AssertFatal ( markupLevel < markupMax, 
		"Too much levels (%d) : enarge DMLX::Writer::markupMax (%d)\n", 
		markupLevel, markupMax );
  if ( writeBin() )
    {
      __DMLX_WRITEBINHASH ( DMLXBinFlag_Markup, markupName );
    }
  else
    {
      __DCXML_RAWWRITE( "%s<%s", tabs, markupName.getKeyword() );
    }
  markups[markupLevel] = new Keyword ( markupName.getKeyword() );
  for ( int i = 0 ; i <= markupLevel ; i++ )
    {
      Log ( "\tlevel %d : markup '%s'\n", i, markups[i]->getKeyword() );
    }
  markupState = MarkupState_Openned;
  if ( indentation )
    //    tabs[markupLevel] = '\t';
    tabs[markupLevel] = ' ';  
  markupLevel++;
  AssertBug ( markupLevel < markupMax - 1, "Too much markups...\n" );
  tabs[markupLevel] = '\0';
  return true;
}
bool DMLX::Writer::__writeAttr ( DMLX::Keyword& attrName, char * attrValue, bool xmlize )
{
  if ( fidles == -1 )
    return false;
  if ( writeBin() )
    return false;
  AssertBug ( markupState == MarkupState_Openned, "Invalid XML Writing.\n" );
  if ( xmlize )
    {
      char xmlizedValue[128];
      xmlizeString ( attrValue, xmlizedValue, 128 );
      __DCXML_RAWWRITE ( " %s=\"%s\"", attrName.getKeyword(), xmlizedValue );
    }
  else
    {
      __DCXML_RAWWRITE ( " %s=\"%s\"", attrName.getKeyword(), attrValue );
    }
  return true;
}

bool DMLX::Writer::xmlizeString( const char * source, char * target, int maxTargetLength )
{
  int idx = 0, c = 0;
  while ( source[c] != '\0' )
    {
      AssertFatal ( idx < maxTargetLength, "Buffer overflow while XMLizing string.\n" );
      Log ( "XMLize : c=%d, idx=%d, source[c]=%c\n",
	    c, idx, source[c] );
      switch ( source[c] )
	{
	case '<': idx+=sprintf ( target+idx, "&lt;" ); break;
	case '>': idx+=sprintf ( target+idx, "&gt;" ); break;
	case '&': idx+=sprintf ( target+idx, "&amp;" ); break;
	case '"': idx+=sprintf ( target+idx, "&quot;" ); break;
#define __DMLX_WRITER_SPRINTF_AMP_BUG
#ifdef  __DMLX_WRITER_SPRINTF_AMP_BUG
	case '%': target[idx++] = '%'; target[idx++] = '%'; break;
#endif  // __DMLX_WRITER_SPRINTF_AMP_BUG
	default: target[idx++] = source[c];
	}
      c++;
    }
  target[idx] = '\0';
  return true;
}

bool DMLX::Writer::writeAttr ( DMLX::Keyword& attrName, char * attrValue, bool xmlize )
{
  Log ( "Writing Attr '%s'='%s'\n", attrName.getKeyword(), attrValue );
  if ( fidles == -1 )
    return false;
  if ( writeBin() )
    {
      __DMLX_WRITEBINHASH ( DMLXBinFlag_Attribute, attrName );
      __DMLX_WRITEBINFLAG ( DMLXBinFlag_Text  );
      DMLXBinTextLength ln = strlen( attrValue );
      __DMLX_WRITEBIN ( &ln, DMLXBinTextLengthSz );
      __DMLX_WRITEBIN (attrValue, ln );
      return true;
    }
  return __writeAttr ( attrName, attrValue, xmlize );
}


bool DMLX::Writer::writeAttrInt ( DMLX::Keyword& attrName, int value )
{
  Log ( "Writing Attr '%s'='%d'\n", attrName.getKeyword(), value );
  if ( fidles == -1 )
    return false;
  if ( writeBin() )
    {
      __DMLX_WRITEBINHASH ( DMLXBinFlag_Attribute, attrName );
      __DMLX_WRITEBINFLAG ( DMLXBinFlag_Integer  );
      __DMLX_WRITEBIN (&value, DMLXBinIntegerSz );
      return true;
    }
  char text[16]; sprintf ( text, "%d", value );
  return __writeAttr ( attrName, text, false );
}

bool DMLX::Writer::writeAttrHex ( DMLX::Keyword& attrName, int value )
{
  Log ( "Writing Attr '%s'='0x%x'\n", attrName.getKeyword(), value );
  if ( fidles == -1 )
    return false;
  if ( writeBin() )
    {
      __DMLX_WRITEBINHASH ( DMLXBinFlag_Attribute, attrName );
      __DMLX_WRITEBINFLAG ( DMLXBinFlag_Hex  );
      __DMLX_WRITEBIN (&value, DMLXBinHexSz );
      return true;
    }
  char text[16]; sprintf ( text, "0x%x", value );
  return __writeAttr ( attrName, text, false );
}

bool DMLX::Writer::writeAttrFloat ( DMLX::Keyword& attrName, float value )
{
  Log ( "Writing Attr '%s'='%f'\n", attrName.getKeyword(), value );
  if ( fidles == -1 )
    return false;
  if ( writeBin() )
    {
      __DMLX_WRITEBINHASH ( DMLXBinFlag_Attribute, attrName );
      __DMLX_WRITEBINFLAG ( DMLXBinFlag_Float  );
      __DMLX_WRITEBIN (&value, DMLXBinFloatSz );
      return true;
    }
  char text[16]; sprintf ( text, "%f", value );
  return __writeAttr ( attrName, text, false );
}

bool DMLX::Writer::writeText ( char * text, bool xmlize )
{
  Log ( "Writing Text '%s'\n", text );
  if ( fidles == -1 )
    return false;
  hasWrittenText = true;
  AssertBug ( markupLevel > 0, "Invalid XML Writing.\n" );
  if ( writeBin() )
    {
      __DMLX_WRITEBINFLAG ( DMLXBinFlag_Text  );
      DMLXBinTextLength ln = strlen( text );
      __DMLX_WRITEBIN ( &ln, DMLXBinTextLengthSz );
      __DMLX_WRITEBIN ( text, ln );
      return true;
    }
  if ( markupState == MarkupState_Openned )
    {
      __DCXML_RAWWRITE( ">" );
      markupState = MarkupState_PreClosed;
    }
  if ( xmlize )
    {
      char xmlizedText[1024];
      xmlizeString ( text, xmlizedText, 1024 );
      __DCXML_RAWWRITE ( "%s", xmlizedText );
    }
  else
    {
      __DCXML_RAWWRITE ( "%s", text );
    }
  return true;
}
bool DMLX::Writer::writeMarkupEnd ( )
{
  if ( fidles == -1 )
    return false;
  AssertBug ( markupLevel > 0, "No Markup to end !\n" );
  markupLevel--;
  Log ( "Writing markupEnd '%s'\n", markups[markupLevel]->getKeyword() );
  tabs[markupLevel] = '\0';
  if ( writeBin() )
    {
      __DMLX_WRITEBINFLAG ( DMLXBinFlag_MarkupEnd );
      markupState = MarkupState_PreClosed;
      delete markups[markupLevel];
      hasWrittenText = false;
      return true;
    }
  if ( markupState == MarkupState_Openned )
    {
      __DCXML_RAWWRITE ( "/>" );
      if ( prettyWrite ) __DCXML_RAWWRITE( "\n" );
    }
  else
    {
      if ( hasWrittenText )
	{
	  hasWrittenText = false;
	}
      else
	{
	  __DCXML_RAWWRITE ( "%s", tabs );
	}
      __DCXML_RAWWRITE ( "</%s>", markups[markupLevel]->getKeyword() );
      if ( prettyWrite ) __DCXML_RAWWRITE( "\n" );
    }
  markupState = MarkupState_PreClosed;
  delete markups[markupLevel];
  return true;
}

