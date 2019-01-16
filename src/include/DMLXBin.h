#ifndef __DML_DMLXBIN_H
#define __DML_DMLXBIN_H

/*
 * Binary format for XML, using known hash
 */

typedef unsigned char DMLXBinFlag;
#define DMLXBinFlagSz (sizeof(DMLXBinFlag))
typedef unsigned int DMLXBinHash;
#define DMLXBinHashSz (sizeof(DMLXBinHash))

typedef unsigned short int DMLXBinTextLength;
#define DMLXBinTextLengthSz (sizeof(DMLXBinTextLength))
#define DMLXBinIntegerSz (sizeof(unsigned int))
#define DMLXBinHexSz (sizeof(unsigned int))
#define DMLXBinFloatSz (sizeof(float))


#define DMLXBinFlag_Markup    1
#define DMLXBinFlag_Attribute 2
#define DMLXBinFlag_MarkupEnd 3
#define DMLXBinFlag_Keyword   4
#define DMLXBinFlag_Text      5
#define DMLXBinFlag_Integer   6
#define DMLXBinFlag_Hex       7
#define DMLXBinFlag_Float     8

inline DMLXBinHash DMLXBinGetHash (const char* str) // based on the RSHash algorithm
{
  
  DMLXBinHash b = 378551;
  DMLXBinHash a = 63689;
  DMLXBinHash hash = 0;
  for(unsigned int i = 0; str[i] != '\0'; i++)
    {
      hash = hash*a+str[i];
      a = a*b;
    }
  return (hash & 0x7FFFFFFF);
};


// Set of macros for DMLXBin writing
// The __WRITE function shall take two arguments : a void* as buffer, and an unsigned int as size of buffer

// __flag is a char (1 byte)
// __keyword is a & DMLX::Keyword (reference)
// __text is char *

#define __DMLXBIN_writeFlag(__WRITE,__flag) { DMLXBinFlag __f = __flag; __WRITE ( &__f, DMLXBinFlagSz ); }
#define __DMLXBIN_writeHash(__WRITE,__hash) { DMLXBinHash __h = __hash; __WRITE ( &__h, DMLXBinHashSz ); }

#define __DMLXBIN_writeMarkup(__WRITE,__keyword)			\
  { __DMLXBIN_writeFlag (__WRITE, DMLXBinFlag_Markup);			\
    __DMLXBIN_writeHash (__WRITE, __keyword.getHash() ); }
#define __DMLXBIN_writeMarkupKeyword(__WRITE,__keyword)			\
  { __DMLXBIN_writeFlag (__WRITE, DMLXBinFlag_Keyword);			\
    __DMLXBIN_writeHash ( __keyword.getHash() );			\
    DMLXBinTextLength __ln = (DMLXBinTextLength) ( strlen ( __keyword.getKeyword() ) ); \
    __WRITE ( &__ln, DMLXBinTextLengthSz ); __WRITE ( __keyword.getKeyword(), ln ); }
    
#define __DMLXBIN_writeMarkupAttr(__WRITE,__keyword,__text)	\
  { __DMLXBIN_writeFlag (__WRITE, DMLXBinFlag_Attribute );	\
    __DMLXBIN_writeHash (__WRITE, __keyword.getHash() );	\
    __DMLXBIN_writeMarkupText (__WRITE, __text );}
#define __DMLXBIN_writeMarkupAttrInt(__WRITE,__keyword,__value) \
  { __DMLXBIN_writeFlag (__WRITE, DMLXBinFlag_Attribute );	\
    __DMLXBIN_writeHash (__WRITE, __keyword.getHash() );	\
    __DMLXBIN_writeFlag (__WRITE, DMLXBinFlag_Integer );	\
    __WRITE(&__value, DMLXBinIntegerSz ); }
#define __DMLXBIN_writeMarkupAttrHex(__WRITE,__keyword,__value) \
  { __DMLXBIN_writeFlag (__WRITE, DMLXBinFlag_Attribute );	\
    __DMLXBIN_writeHash (__WRITE, __keyword.getHash() );	\
    __DMLXBIN_writeFlag (__WRITE, DMLXBinFlag_Hex );		\
    __WRITE(&__value, DMLXBinHexSz ); }

#define __DMLXBIN_writeMarkupAttrFloat(__WRITE,__keyword,__value) \
  { __DMLXBIN_writeFlag (__WRITE, DMLXBinFlag_Attribute );	  \
    __DMLXBIN_writeHash (__WRITE, __keyword.getHash() );	  \
    __DMLXBIN_writeFlag (__WRITE, DMLXBinFlag_Float );		  \
    __WRITE(&__value, DMLXBinFloatSz ); }


#define __DMLXBIN_writeMarkupText(__WRITE,__text)			\
  { __DMLXBIN_writeFlag (__WRITE, DMLXBinFlag_Text );			\
    DMLXBinTextLength __ln = strlen ( __text );				\
    __WRITE ( &__ln, DMLXBinTextLengthSz ); __WRITE ( __text, __ln ); }

#define __DMLXBIN_writeMarkupEnd(__WRITE) { __DMLXBIN_writeFlag (__WRITE, DMLXBinFlag_MarkupEnd ); }

#endif // __DML_DMLXBIN_H
