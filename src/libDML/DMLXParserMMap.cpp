#include <DMLXParser.h>

DMLX::ParserMMap::ParserMMap( const void * _mmap_buff, unsigned int _mmap_sz, unsigned int _parseWindow_max )
{
  dead = false;
  mmap_buff = (void*)_mmap_buff;
  mmap_sz = _mmap_sz;
  parseWindow_max = _parseWindow_max;
  mmap_cur = 0;
  mmap_max = 0;
  free ( buffer ); // Do not need this anymore...
  buffer = NULL;
  destructorShallFreeBuffer = false; // because fillBuffer() fakes buffer.
  bufferMax = parseWindow_max;
}

bool DMLX::ParserMMap::giveNewBytesToRead(unsigned int bytes)
{
  AssertBug ( mmap_max + bytes <= mmap_sz, "Gave a mmap_max out of bounds !\n" );
  mmap_max += bytes;
  return true;
}
unsigned int DMLX::ParserMMap::fillBuffer ( void * buff, int max_size )
{
  // In fact we do not use the buff and max_size...
  // We directly interfere with buffer, bufferIdx, bufferSz
  if ( mmap_cur == mmap_max )
    {
      Warn ( "Nothing to read..\n" );
    }
  buffer = (char*) ((unsigned int) mmap_buff + mmap_cur );
  unsigned int rd = mmap_max - mmap_cur;
  if ( rd > parseWindow_max )
    rd = parseWindow_max;
  mmap_cur += rd;
  Log ( "Filled buffer : gave '%d' bytes\n", rd );
  return rd;
}
bool DMLX::ParserMMap::canFill ()
{
  return (! dead);
}

bool DMLX::ParserMMap::hasWork() 
{ 
  return ( (mmap_cur < mmap_max) || ( endEventsNumber > 0 ) );
}
