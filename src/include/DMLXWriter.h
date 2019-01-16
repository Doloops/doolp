#ifndef __DML_DMLXWRITER_H
#define __DML_DMLXWRITER_H

#include <DMLXKeyword.h>
#include <map>

namespace DMLX
{
  class Writer
  {
  protected:
    int fidles;
    /*
     * Writing Characteristics
     */
    char *writeBuffer;
    unsigned int writeBufferIdx;
    unsigned int writeBufferMax;

    static const unsigned int markupMax = 128;
    typedef enum MarkupState  { MarkupState_Openned,
				MarkupState_PreClosed };
    MarkupState markupState;
    Keyword * markups[markupMax];
    char tabs[markupMax]; // For identation
    unsigned int markupLevel;
    bool hasWrittenText;
    /*
     * Options
     */
    bool __writeBin;
    bool indentation;
    bool useSend;
    int dumpFidles;
    bool writeBinKeywords;
    bool prettyWrite;
    std::map<KeyHash, bool> keywordWritten;

    inline bool writeBin() const { return __writeBin; }

    static bool xmlizeString ( const char * source, char * target, int maxTargetLength );
    bool __writeAttr ( Keyword & attrName, char * attrValue, bool xmlize );
    void __init();
  public:
    Writer ( int _fidles, bool _writeBin );
    Writer ( const char * filename );
    ~Writer ();

    void setDumpFidles ( int _dumpFidles );
    void setIndentation ( bool value ) { indentation = value; }
    void setWriteBinKeywords ( bool value ) { writeBinKeywords = value; }
    void setUseSend ( bool value ) { useSend = value; }

    bool writeMarkup ( Keyword & markupName );
    bool writeAttr ( Keyword & attrName, char * attrValue, bool xmlize );
    bool writeAttrInt ( Keyword & attrName, int value );
    bool writeAttrHex ( Keyword & attrName, int value );
    bool writeAttrFloat ( Keyword & attrName, float value );
    bool writeText ( char * text, bool xmlize );
    bool writeMarkupEnd ( );

    bool writeMarkup ( const char * sMarkupName )
    { Keyword k(sMarkupName); return writeMarkup ( k ); }
    bool writeAttr ( const char * sAttrName, char * attrValue, bool xmlize = true )
    { Keyword k(sAttrName); return writeAttr ( k, attrValue, xmlize ); }
    bool writeAttrInt ( const char * sAttrName, int value )
    { Keyword k(sAttrName); return writeAttrInt ( k, value ); }
    bool writeAttrHex ( const char * sAttrName, int value )
    { Keyword k(sAttrName); return writeAttrHex ( k, value ); }
    bool writeAttrFloat ( const char * sAttrName, float value )
    { Keyword k(sAttrName); return writeAttrFloat ( k, value ); }
    
    bool flush();
    
  };


  class WriterException
  {
  protected:
    char * message;
  public:
    WriterException ( char * _message );
    ~WriterException ();
    inline char * getMessage () const { return message; }
  };

}; // Namespace DMLX

#endif // __DML_DMLXWRITER_H
