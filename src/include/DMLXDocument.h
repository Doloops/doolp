#ifndef _DMLXDOCUMENT_H_
#define _DMLXDOCUMENT_H_

#include <DMLXKeyword.h>
#include <DMLXParser.h>
#include <DMLXWriter.h>

#include <list>
#include <string>
#include <strongmap.h>

namespace DMLX
{
  class Node;
  class Document;
	
  class Attribute
  {
    friend class Node;
  protected:
    Keyword * name;
    char * value;
    Attribute * next;
  public:
    Attribute ( Keyword * _name, const char * _value );
    Attribute ( const char * _name, const char * _value );
    void setNext ( Attribute * _a ) { next = _a; }
    Attribute * getNext ( ) { return next; }
    char * getName () { return name->getKeyword(); }
    KeyHash getHash () { return name->getHash(); }
    char * getValue () { return value; }
    void write(Writer & writer );
    void log();
  };
	
  class Node
  {	
    friend class Document;
  protected:
    Node * father;
    bool _isText;
    Keyword * name;
    char * text;
    Node * first, * last, * next;
    Node * reference;
    Attribute * attrfirst, * attrlast;
    void setNext ( Node * n ) { next = n; }
    void __init() { name = NULL; father = NULL; _isText = false; text = NULL; first = last = next = NULL; attrfirst = attrlast = NULL; reference = NULL; }
    void buildReferences ( Document& doc, Keyword& keyw_id );
    void resolveReferences ( Document& doc, Keyword& keyw_idref );
  public:	
    Node ( Keyword * _name );
    Node ( Keyword & _name );
    Node ( const char * _name );

    Node ( Node * _father, Keyword * _name );
    Node ( Node * _father, const char * _name );

    Node ( const char * _text, bool cpy );
    Node ( Node * _father, const char * _text, bool cpy );
		  
    bool isText() const { return _isText; }
    bool isNode() const { return !_isText; }

    bool isName ( Keyword & k ) const { return isText() ? false : name->isEqual ( k ); }
    bool isName ( const char * wk ) const { Keyword k(wk); return isName (k); }
    bool isReference () { return ( reference != NULL ); }
    Node * getReference() { AssertFatal ( isReference(), "not a reference..\n" ); return reference; }

    void addNode ( Node * n );
    Node * addNode ( Keyword & k )
    { Node * n = new Node ( k ); addNode ( n ); return n; }
    Node * addNode ( const char * _name )
    { Node * n = new Node ( _name ); addNode ( n ); return n; }
    void addAttribute ( Attribute * a );
    void addAttribute ( Keyword * k, const char * text )
    { addAttribute ( new Attribute ( k, text ) ); }
    void addAttribute ( Keyword & k, const char * text )
    { addAttribute ( new Attribute ( &k, text ) ); }
    void addAttribute ( const char * name, const char * text )
    { addAttribute ( new Attribute ( name, text ) ); }
    
    Node * getFather() { return father; }
    char * getText() { AssertBug ( _isText, "Not a text Node\n" ); return text; }
    char * getName() { AssertBug ( !_isText, "Is a text Node\n" ); return name->getKeyword(); }
    KeyHash getHash() { AssertBug ( !_isText, "Is a text Node\n" ); return name->getHash(); }
    Node * getFirst () { return first; }
    Node * getNext() { return next; }
    Attribute * getFirstAttribute () { return attrfirst; }
    std::list<Node *> * getNodeList ( DMLX::Keyword & k );
    std::list<Node *> * getNodeList ( const char * wk ) { Keyword k(wk); return getNodeList (k); }

    void popFirst() { if ( first ) first = first->getNext(); }
		
    char * getAttribute ( DMLX::Keyword & k );
    char * getAttribute ( const char * wk ) { Keyword k(wk); return getAttribute (k); }
    Node * getNode ( DMLX::Keyword & k );
    Node * getNode ( const char * wk ) { Keyword k(wk); return getNode (k); }
    Node * getNodeWithAttribute ( DMLX::Keyword & attribute, const char * value );
    Node * getNodeWithAttribute ( const char * attribute, const char * value )
      { DMLX::Keyword k(attribute); return getNodeWithAttribute(k,value); }

    void write(Writer & writer );

    char * getValueOf ( const char * selection );
    bool checkCondition ( const char * cond );		
    void transform(int fd, Node * xslStylesheet, Node * xslTemplate);

    void log();
  };
	
  class Document
  {
  protected:
    Node * first, * last;
    HashTree * hashTree;
    strongmap <std::string,Node *> references;
    friend class Node;

    void addReference ( const char * ref, Node * node );
    Node * getReference ( const char * ref );
  public:
    Document() { first = last = NULL; hashTree = new HashTree(); }
    void addNode ( Node * n ); // { nodes.push_back ( n ); }
    Node * getFirst () { return first; }
    Node * getNode ( DMLX::Keyword & k );
    Node * getNode ( const char * wk ) { Keyword k(wk); return getNode (k); }
    std::list<Node *> * getNodeList ( DMLX::Keyword & k );

    HashTree * getHashTree() { return hashTree; }
		
    void buildFromParser ( DMLX::Parser & parser );
    void write(Writer & writer );
    void transform(int fd, Document * xsl);
    void log();

    void buildReferences ( Keyword& keyw_id, Keyword& keyw_idref );
    void buildReferences ( const char * wk_id, const char * wk_idref )
    { Keyword keyw_id(wk_id); Keyword keyw_idref(wk_idref);
      buildReferences ( keyw_id, keyw_idref ); }

  };
}; // NameSpace DMLX

#endif //_DMLXDOCUMENT_H_
