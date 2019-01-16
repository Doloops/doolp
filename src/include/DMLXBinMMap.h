#include <DMLXBin.h>
#include <DMLXKeyword.h>
/*
 * Fast (as fast as I can) binary accessor (not parser)
 * based on a mmap buffer
 */
#include <map>
namespace DMLX
{
  class BinMMap;
  class BinMMap
  {
  protected:
    int fidles;
    void * buffer;
    static const unsigned int bufferMax = 2 << 28;
    KeyHash *itemsToCount;
    unsigned int itemMapperInterval;
    unsigned int itemMapperLast;
    unsigned int lastItemNumber; // Highest item number found...
    std::map<unsigned int, unsigned int> itemMapper;
    void __init();
  public:
    BinMMap ( char * file );
    BinMMap ( int _fidles );
    ~BinMMap ();
    class iterator
    {
      friend class BinMMap;
    protected:
      unsigned int offset;
      unsigned int itemNumber;
      BinMMap * binMMap;
      iterator( BinMMap * _binMMap )
	{ offset = 0; itemNumber = 0 ; binMMap = _binMMap; }
      bool getAttrPos(KeyHash h, unsigned int * pos);
      void setOffset(unsigned int _offset, unsigned int _itemNumber )
      { offset = _offset; itemNumber = _itemNumber; }
    public:
      char * getPos(unsigned int pos)
	{ 
	  return (char*) (((unsigned int)binMMap->buffer) + offset + pos );
	}
      unsigned int getOffset() const { return offset; }
      unsigned int getItemNumber() const { return itemNumber; }
      bool isAtEnd()
      { return ( offset == binMMap->getBufferSize() ); }
      bool isMarkup()
      { return ( (getPos(0))[0] == DMLXBinFlag_Markup ); }
      bool isText()
      { return ( (getPos(0))[0] == DMLXBinFlag_Text ); }
      bool isMarkupEnd()
      { return ( (getPos(0))[0] == DMLXBinFlag_MarkupEnd ); }
      KeyHash getMarkupHash()
      { AssertBug ( isMarkup(), "iterator is not on a Markup !\n" );
	return *((KeyHash*) getPos(DMLXBinFlagSz)); }
      bool hasAttr(KeyHash h);
      unsigned int getAttr(KeyHash h, char * buffer, unsigned int maxLength);
      int getAttrInt(KeyHash h);
      unsigned int getAttrHex(KeyHash h);
      float getAttrFloat(KeyHash h);

      unsigned int getText(char * buff, unsigned int maxLength)
      { return getText(0, buff, maxLength); }
      unsigned int getText(unsigned int pos, char * buff, unsigned int maxLength);

      iterator& operator++();

      iterator operator++(int)
      { iterator __tmp = *this; this->operator++(); // iter++;
	return __tmp; }

    };
    iterator getIterator ()
    { iterator i ( this );
      return i; }
    iterator getIterator ( unsigned int itemNumber );
    unsigned int getBufferSize();
    unsigned int getLastItemNumber () const { return lastItemNumber; }
    bool setItemsToCount ( KeyHash * itemsHashList )
    { itemsToCount = itemsHashList; return true; }
    bool setItemMapperInterval ( unsigned int interval )
    { itemMapperInterval = interval; return true; }
  };

}; // NameSpace DMLX
