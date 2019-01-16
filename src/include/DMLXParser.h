#ifndef __DMLXPARSER_H
#define __DMLXPARSER_H

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <map>
#include <list>

#include <glog.h>
#include <DMLXKeyword.h>
/*
 * DMLXParser : XML Fast Parser
 * 
 * DMLXParser uses 4 main items to parse :
 * * Read buffer : implementing different sources is implementing the way to fill this buffer.
 * * StoreSpace : each value is stored in this cyclic buffer, and freed in popEvent().
 * * KeywordSpace : the keys of the XML are uniquely stored in this space.
 * * Parsing : this object stores the different states of the parsing automat.
 */ 

#define __DMLX_PARSER_EVENT_STACK_MAX 128

namespace DMLX
{
  class ParserException
  {
    char * message;
    bool fatal;
  public:
    ParserException ( char * _msg ) { message = _msg; fatal = false; }
    ParserException ( char * _msg, bool _fatal ) { message = _msg; fatal = _fatal; }
    inline char * getMessage () const { return message; }
    inline bool isFatal () const { return fatal; }
  };

  class ParserFullSpaceException
  {
 public:
    ParserFullSpaceException() {}
  };
  
  typedef enum ParserState
    {
      parseStateMarkupOutside         = 0,  // Outside any markup
      parseStateMarkupText            = 1,  // Text between markup
      parseStateMarkupTextSymbol      = 2,  // &entity; in text
      parseStateMarkupEnter           = 3,  // At the "<" beginning of markup
      parseStateMarkupName            = 4,  // Name of the markup
      parseStateMarkupInside          = 5,  // Inside of the markup, after the name
      parseStateMarkupAttrName        = 6,  // Attribute name 
      parseStateMarkupAttrBeforeEqual = 7,  // After the attribute name, before equal
      parseStateMarkupAttrEqual       = 8,  // "=" sign
      parseStateMarkupAttrValue       = 9,  // After the '"', reading value till next '"'
      parseStateMarkupAttrValueSymbol = 10, // &entity in value
      parseStateMarkupEnd             = 11, // At the '/' in <markup/>
      parseStateMarkupEndName         = 12, // At the the / in </markup>
      parseStateXMLSignature          = 13, // In the <? .. ?>
    };


  class Attr
  {
  public:
    Keyword * name;
    char * value;
    Attr * next;
  };
  class Event
  {
    char flags;
  public:
    Keyword * name; // Be carefull, name points to text contents in a isText() event.
    Event * next;
    // Could stop here for Text and End Events.
    char * text;
    Attr * first;
    
    inline void __init()
      { flags = 0; name = NULL; next = NULL; first = NULL; }
   
    inline Event() 
      { __init(); }
      // At default construction, Event is a non full markup.
    
    
    inline bool isMarkup() const { return ((flags & 3) == 0); }
    inline bool isEnd() const { return ((flags & 1) == 1); }
    inline bool isText() const { return ((flags & 2) == 2); }
    inline bool isFull() const { return ((flags & 4) == 4); }
    inline char getFlags() const { return flags; }
    
    inline void setMarkup() { flags|=3; flags^=3; }
    inline void setEnd() { flags|=1; }
    inline void setText() { flags|=2; }
    inline void setFull(bool full) { flags|=4; if (!full) flags^=4; }
  };
  
  
  class Parser
  {
  protected:
    Event * firstEvent;
    Event * lastEvent;
    unsigned int eventsNumber;
    unsigned int endEventsNumber;
    
    bool destructorShallFreeBuffer; // Give an opportunity for herited classes to realloc buffer.
    unsigned int bufferMax;
    char *buffer; // warning : +1 to Allow a '\0' in anycase for debug output.
    unsigned int bufferIdx;
    unsigned int bufferSz;
    
    class StoreSpace
    {
    protected:
      unsigned int max;
      char *space;
      unsigned int first;
      unsigned int front;
      unsigned int curLength; // curLength for alloc() extend(char**,char) mechanism
      
    public:
      StoreSpace ( unsigned int _max );
      ~StoreSpace ();
      
      /*
       * Accessors (mostly for logEvents)
       */ 
      inline char * getSpace() const { return space; }
      inline unsigned int getFirst() const { return first; }
      inline unsigned int getFront() const { return front; }
      inline unsigned int getMax() const { return max; }
      /*
       * Methods for parse() and parseBuffer()
       */
      inline unsigned int current () const
	{ 
	  AssertBug ( front < max,
		      "Went out of bounds !!!\n" );
	  return front;
	}
      void * alloc ( unsigned int reqLength );
      void * alloc (); // Empty size
      void extend ( char **s, char c ); // Pushed as a inline in Parser-parse-ng
      void * realloc_begin ( void * old, unsigned int prevLength, unsigned int reqLength );
      bool free ( Event * event ); // Free a whole event
      unsigned int avail ( ); // Available memory.
      unsigned int topointer ( void * ptr ); // Converts a global pointer to an offset in space.
    };
    StoreSpace storeSpace;
  public:
  protected:
    HashTree * hashTree;
    bool canDeleteHashTree;
  public:
    void setHashTree ( HashTree * _hashTree )
    { hashTree = _hashTree; canDeleteHashTree = false; }
    inline HashTree * getHashTree () const { return hashTree; }
    class Parsing // Values for char-by-char reading
    {
    public:
      Parsing ();
      ~Parsing ();
      static const unsigned int eventStackMax = __DMLX_PARSER_EVENT_STACK_MAX;
      unsigned int eventStackLevel;
      Keyword *eventStack[__DMLX_PARSER_EVENT_STACK_MAX];
      unsigned int totalParsed;
      unsigned int totalLinesParsed;
      ParserState state;
      Event * event;
      Attr * lastAttr;
      Keyword *name;
      char nameBuffer[64];
      unsigned int nameBufferSz;
      char *value;
      char *text;
      char **symbolLong;
      char **symbolShort;
      char symbol[64]; // Current symbol
      unsigned int symbolSz;
      
      // for parseBufferBin()
      unsigned int reqLength;
      KeyHash hash;
    //    unsinged int curIdx;
      char valueFlag;
      bool keywordInMarkup;
    };
    Parsing parsing;
    
    Event * newEvent ( bool isEnd );
    Attr * addAttr ( Event * event, 
		     Attr * lastAttr,
		     Keyword * name, 
		     char * value );
    void parseBufferXML () throw (ParserException*, ParserFullSpaceException*);
    void parseBufferBin () throw (ParserException*, ParserFullSpaceException*);
    
    /*
     * Options;
     */
    bool parseKeepAllText;
    bool parseCanFillBufferOnce;
    unsigned int maxEndEventsInStoreSpace; // Maximum end events in queue
    bool parseBin;
 public:
    Parser();
    virtual ~Parser();
    bool doFillBuffer (); // Asks for filling buffer from outside..
    virtual unsigned int fillBuffer ( void * buff, int max_size ) = 0;
    virtual bool canFill () = 0;
    
    void parse ( ) throw (ParserException*, ParserFullSpaceException*);
    bool logEvents ( );
    bool logKeywords () { return hashTree->log(); }
    bool dumpKeywords ( char * fileName ) { return hashTree->dumpKeywords(fileName); }
    /*
     * Options setting.
     */
    void setParseKeepAllText ( bool value );
    void setMaxEndEventsInStoreSpace ( unsigned int number );
    void setParseCanFillBufferOnce ( bool value );
    void setParseBin ( bool value );
    /*
     * Event Handling
     */
    Event * getEvent ();
    bool popEvent ();
    bool popEventEnd ();
    bool popEventAndEnd ();
    
    /*
     * Event Access
     */
    char * getEventName ();
    bool isEventEnd ();
    bool isEventText ();
    bool isEventName ( const char * c );
    void checkEventName ( const char * c );
    bool isEventName ( Keyword& keyword );
    void checkEventName ( Keyword& keyword );
    
    /* 
     * Attribute Access
     */
    char* getText ();
    bool hasAttr ( Keyword& keyword );
    char* getAttr ( Keyword& keyword );
    int getAttrInt ( Keyword& keyword );
    bool getAttrBool ( Keyword& keyword );
    float getAttrFloat ( Keyword& keyword );
    
    bool hasAttr ( const char * name );
    char* getAttr ( const char * name );
    int getAttrInt ( const char * name );
    bool getAttrBool ( const char * name );
    float getAttrFloat ( const char * name );
    
    unsigned int getEventsNumber (); // Fuzzy stupid ?
    inline unsigned int getEndEventsNumber () const
    { return endEventsNumber; }
    bool isFinished ();
  };
  
  class ParserFile : public Parser
  {
    //  int fp;
    FILE * fp;
  public:
    ParserFile( char * file );
    ~ParserFile();
    unsigned int fillBuffer ( void * buff, int max_size );
    bool canFill ();
  };

  class ParserSocket : public Parser
  {
    int sock;
    int dumpsock;
    unsigned int waitTime;
    unsigned int waits;
  public:
    ParserSocket( int _sock );
    ~ParserSocket();
    unsigned int fillBuffer ( void * buff, int max_size );
    bool canFill ();
    
    bool dumpToSocket ( int _dumpsock );
  };
 
  /*
    A custom Parser from mmap() files.
    * Notes : _parseWindow_max shall be totally deprecated ?... (what is THIS for...)
  */
  class ParserMMap : public Parser
  {
    bool dead;
    void * mmap_buff;
    unsigned int mmap_sz; // Total size of the mmap
    unsigned int mmap_cur; // Current position of parsing in mmap
    unsigned int mmap_max;  // Until where I am allowed to read.
    unsigned int parseWindow_max; // The maximum I am allowed to send for parsing.
  public:
    ParserMMap( const void * _mmap_buff, unsigned int _mmap_sz, unsigned int _parseWindow_max );
    bool giveNewBytesToRead(unsigned int bytes);
    unsigned int fillBuffer ( void * buff, int max_size );
    bool canFill ();
    void setDead() { dead = true; }
    bool hasWork();// Do I have work to do...
  };

}; // NameSpace DMLX
#endif // __DMLXPARSER_H
