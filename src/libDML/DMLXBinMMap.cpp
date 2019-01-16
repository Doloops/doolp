#include <DMLXBinMMap.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#if 1
#undef Log
#define Log(...)
#endif

void DMLX::BinMMap::__init ()
{
  itemsToCount = NULL;
  itemMapperInterval = 0;
  itemMapperLast = 0;
  lastItemNumber = 0;
}

DMLX::BinMMap::BinMMap ( char * file )
{
  __init ();
  fidles = open ( file, O_RDONLY );
  AssertFatal ( fidles != -1, "Could not open file '%s'\n", file );
  buffer = mmap ( NULL, bufferMax, PROT_READ, MAP_SHARED, fidles, 0 );
  AssertFatal ( buffer != MAP_FAILED, "Could not mmap file '%s'.\n", file );
  Log ( "Buffer at '%p'\n", buffer );
  Info ( "Starting binMMap for file '%s'\n", file );
}

DMLX::BinMMap::BinMMap ( int _fidles )
{
  __init ();
  fidles = _fidles;
  AssertFatal ( fidles != -1, "Could not create mmap for fidles '%d'\n", fidles );
  buffer = mmap ( NULL, bufferMax, PROT_READ, MAP_SHARED, fidles, 0 );
  AssertFatal ( buffer != MAP_FAILED, "Could not mmap fidles '%d'.\n", fidles );
  Log ( "Buffer at '%p'\n", buffer );
  Info ( "Starting binMMap for fidles '%d'\n", fidles );
}

DMLX::BinMMap::~BinMMap ()
{
  Info ( "Quitting binMMap\n" );
  if ( fidles != -1 )
    {
      munmap ( buffer, bufferMax );
      close ( fidles );
    }
}

unsigned int DMLX::BinMMap::getBufferSize()
{
  struct stat st;
  int res = fstat ( fidles, &st );
  AssertBug ( res != -1, "Could not fstat.\n" );
  //  Log ( "Stats : file size is '%d'\n", (unsigned int)st.st_size );
  return (unsigned int)st.st_size;
}

bool DMLX::BinMMap::iterator::getAttrPos(KeyHash h, unsigned int * pos)
{
  unsigned int ln = DMLXBinFlagSz + DMLXBinHashSz; // Markup flag and hash
  while ( true )
    {
      DMLXBinFlag flag = getPos(ln)[0];
      if ( flag != DMLXBinFlag_Attribute )
	{
	  *pos = ln;
	  return false;
	}
      //      Log ( "Attr '0x%x'\n", *((KeyHash*)getPos(ln+DMLXBinFlagSz)) );
      if ( h == *((KeyHash*)getPos(ln+DMLXBinFlagSz)) )
	{
	  *pos = ln;
	  return true;
	}
	//	return ln;
      ln += DMLXBinFlagSz + DMLXBinHashSz; // Attribute flag and hash
      switch ( getPos(ln)[0] )
	{
	case DMLXBinFlag_Integer: ln += DMLXBinFlagSz + DMLXBinIntegerSz; break;
	case DMLXBinFlag_Hex: ln+= DMLXBinFlagSz + DMLXBinHexSz; break;
	case DMLXBinFlag_Float: ln+= DMLXBinFlagSz + DMLXBinFloatSz; break;
	case DMLXBinFlag_Text:
	  {
	    DMLXBinTextLength lnT = *((DMLXBinTextLength*) getPos(ln + DMLXBinFlagSz));
	    ln += DMLXBinFlagSz + DMLXBinTextLengthSz + lnT;
	    break;
	  }
	default:
	  Bug ( "Unexpected flag '%d'\n", getPos(ln)[0]);
	}  
    }
  return false;
}

DMLX::BinMMap::iterator& DMLX::BinMMap::iterator::operator++()
{
  // Evaluate item length...
  Log ( "At iter(%p) offset '%d'\n", this, offset );
  AssertBug ( ! isAtEnd(), "iterator at end !\n" );
  if ( isMarkupEnd() )
    {
      offset += DMLXBinFlagSz;
      return *this;
    }
  else if ( isText() )
    {
      DMLXBinTextLength ln = *((DMLXBinTextLength*) getPos(DMLXBinFlagSz));
      offset += DMLXBinFlagSz + DMLXBinTextLengthSz + ln;
      return *this;
    }
  else if ( isMarkup() )
    {
      bool newItem = false;
      Log ( "markupHash '0x%x'\n", getMarkupHash() );
      if ( binMMap->itemsToCount == NULL )
	{
	  //	  itemNumber++;
	  newItem = true;
	}
      else
	{
	  for ( int h = 0 ; binMMap->itemsToCount[h] != 0x0 ; h++ )
	    {
	      Log ( "Comp to '0x%x'\n", binMMap->itemsToCount[h] );
	      if ( binMMap->itemsToCount[h] == getMarkupHash() )
		{
		  newItem = true; break;
		}
	    }
	}
      if ( newItem )
	{
	  Log ( "newItem..%d\n", itemNumber );
	  if ( ( binMMap->itemMapperInterval > 0 )
	       && ( itemNumber % binMMap->itemMapperInterval == 0 ) )
	    {
	      Log ( "Setting offset '0x%x' as itemNumber '%d'\n",
		    offset, itemNumber )
	      binMMap->itemMapper[itemNumber] = offset;
	      if ( itemNumber > binMMap->itemMapperLast )
		binMMap->itemMapperLast = itemNumber;
	    }
	  if ( binMMap->lastItemNumber < itemNumber )
	    binMMap->lastItemNumber = itemNumber;
	  itemNumber++;
	}
      unsigned int ln = 0;
      bool res = getAttrPos ( 0x00, &ln );
      AssertBug ( !res, "We should not have found this key : 0x00...\n" );
      //      Log ( "Total item length : '%d'\n", ln );
      offset += ln;
      return *this;
    }
  Bug ( "Invalid Markup !\n" );
}

bool DMLX::BinMMap::iterator::hasAttr(KeyHash h)
{
  AssertFatal ( isMarkup(), "Not on a markup event !\n" );
  unsigned int ln;
  return getAttrPos ( h, &ln );
}

unsigned int DMLX::BinMMap::iterator::getAttr(KeyHash h, char * buffer, unsigned int maxLength)
{
  unsigned int pos = 0;
  if ( ! getAttrPos(h, &pos) )
    {
      Fatal ( "Could not get attr '0x%x'\n", h );
    }
  return getText ( pos + DMLXBinFlagSz + DMLXBinHashSz, buffer, maxLength );
}

int DMLX::BinMMap::iterator::getAttrInt(KeyHash h)
{
  unsigned int pos = 0;
  if ( ! getAttrPos(h, &pos) )
    {
      Fatal ( "Could not get attr '0x%x'\n", h );
    }
  AssertFatal ( getPos(pos + DMLXBinFlagSz + DMLXBinHashSz)[0] == DMLXBinFlag_Integer,
		"Not an Integer attribute\n" );
  return *((int*)getPos(pos + DMLXBinFlagSz + DMLXBinHashSz + DMLXBinFlagSz ));
}

unsigned int DMLX::BinMMap::iterator::getAttrHex(KeyHash h)
{
  unsigned int pos = 0;
  if ( ! getAttrPos(h, &pos) )
    {
      Fatal ( "Could not get attr '0x%x'\n", h );
    }
  AssertFatal ( getPos(pos + DMLXBinFlagSz + DMLXBinHashSz)[0] == DMLXBinFlag_Hex,
		"Not an Hex attribute\n" );
  return *((unsigned int*)getPos(pos + DMLXBinFlagSz + DMLXBinHashSz + DMLXBinFlagSz ));
}


unsigned int DMLX::BinMMap::iterator::getText(unsigned int pos,char * buff, unsigned int maxLength)
{
  //  AssertFatal ( isText(), "Not on a markup text event !\n" );
  AssertFatal ( (getPos(pos))[0] == DMLXBinFlag_Text, "Not on a markup text !\n" );
  DMLXBinTextLength ln = *((DMLXBinTextLength*) getPos(pos + DMLXBinFlagSz));
  //  if ( ln 
  AssertFatal ( ln < maxLength, "Insufficient buffer size %d !\n", maxLength );
  memcpy ( buff, getPos ( pos + DMLXBinFlagSz + DMLXBinTextLengthSz ), ln );
  buff[ln] = '\0';
  return (unsigned int)ln;
}


DMLX::BinMMap::iterator DMLX::BinMMap::getIterator ( unsigned int itemNumber )
{
  if ( itemNumber == 0 )
    {
      return getIterator ();
    }
  AssertBug ( itemMapperInterval > 0, "itemMapperInterval not set !\n" );
  unsigned int baseItemNumber = ((unsigned int) itemNumber / itemMapperInterval) * itemMapperInterval;
  Log ( "itemNumber=%d, baseItemNumber=%d, itemMapperLast=%d, itemMapperInterval=%d\n", 
	itemNumber, baseItemNumber, 
	itemMapperLast, itemMapperInterval );
  if ( itemMapperLast == 0 )
    return getIterator ();
  iterator iter ( this );
  if ( baseItemNumber > itemMapperLast )
    {
      iter.setOffset ( itemMapper[itemMapperLast], itemMapperLast );
      return iter;
    }
  AssertBug ( itemMapper[baseItemNumber] > 0, "itemMapper gave a zero as offset !\n" );
  Log ( "Offset '%d'(0x%x)\n", itemMapper[baseItemNumber], itemMapper[baseItemNumber] );
  iter.setOffset ( itemMapper[baseItemNumber], baseItemNumber );
  while ( iter.getItemNumber() != itemNumber )
    iter.operator++();
  return iter;
}


#ifdef __DMLXBINMMAP_TEST
#include <glogers.h>
setGlog ( "DMLXBinMMap-test" );

DMLX::Keyword keyw_glog("glog");
DMLX::Keyword keyw_t("t");
DMLX::Keyword keyw_tm("tm");
DMLX::Keyword keyw_thread("thread");
DMLX::Keyword keyw_file("file");
DMLX::Keyword keyw_func("func");
DMLX::Keyword keyw_line("line");
DMLX::Keyword keyw_prior("prior");
DMLX::Keyword keyw_text("text");
DMLX::Keyword keyw_id("id");
DMLX::Keyword keyw_value("value");


int main ( int argc, char ** argv )
{
  addGlog ( new GlogInfo_File ( stdout ) );

  AssertBug ( argc == 2, "Wrong number of arguments.\n" );
  DMLX::BinMMap binMMap ( argv[1] );
  DMLX::KeyHash hashList[] =
    { keyw_glog.getHash(), 0x0 };
  binMMap.setItemsToCount ( hashList );
  binMMap.setItemMapperInterval ( 1000 );
  DMLX::BinMMap::iterator iter = binMMap.getIterator();

  char buffer[2048];
  Info ( "bufferSize=%d(0x%x)\n", binMMap.getBufferSize(), binMMap.getBufferSize() );

  while ( ! iter.isAtEnd() )
    {
      //      Log ( "---------------------------- Offset=%d(0x%x)\n", iter.getOffset(), iter.getOffset());
      //      char c = iter.getPos(0) [0];
      //      Log ( "Flag '%d'\n", c );
      if ( iter.isMarkup() )
	{
	  //	  Log ( "isMarkup : itemNumber=%d\n", iter.getItemNumber() );
	  if ( iter.getMarkupHash() == keyw_file.getHash() )
	    {
	      unsigned int res = iter.getAttr( keyw_value.getHash(), buffer, 2048 );
	      Log ( "File='%s', id=0x%x\n", buffer, iter.getAttrHex( keyw_id.getHash() ) );
	    }
	  else if ( iter.getMarkupHash() == keyw_func.getHash() )
	    {
	      unsigned int res = iter.getAttr( keyw_value.getHash(), buffer, 2048 );
	      Log ( "Func='%s', id=0x%x\n", buffer, iter.getAttrHex( keyw_id.getHash() ) );
	    }
	  else if ( iter.getMarkupHash() == keyw_glog.getHash() )
	    {
	      Log ( "prior=%d, t=%d, tm=%d, thread=0x%x, file=0x%x, func=0x%x, line=%d\n",
		    iter.getAttrInt ( keyw_prior.getHash() ),
		    iter.getAttrInt ( keyw_t.getHash() ),
		    iter.getAttrInt ( keyw_tm.getHash() ),
		    iter.getAttrHex ( keyw_thread.getHash() ),
		    iter.getAttrHex ( keyw_file.getHash() ),
		    iter.getAttrHex ( keyw_func.getHash() ),
		    iter.getAttrInt ( keyw_line.getHash() ) );
	    }
	}
      else if ( iter.isText() )
	{
	  iter.getText(buffer, 2048 );
	  Log ( "Text <<%s>>\n", buffer ) ;
	}
      else if ( iter.isMarkupEnd() )
	{

	}
      else
	{
	  Fatal ( "Unexpected flag...\n" );
	}
      
      iter++;
    }
  Info ( "Finished parsing..(max iter=%d)\n", iter.getItemNumber() );
  DMLX::BinMMap::iterator iter2 = binMMap.getIterator( 14625 );
  Info ( "iter itemNumber=%d\n", iter2.getItemNumber() );
}
#endif
