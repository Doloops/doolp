#ifndef __DML_DMLXKEYWORD_H
#define __DML_DMLXKEYWORD_H

#include <stdlib.h>
#include <string.h>
#include <glog.h>
#include <list>

namespace DMLX
{
  typedef unsigned int KeyHash;
  class Keyword
  {
  protected:
    char * keyword;
    KeyHash hash;
  public:
    inline static KeyHash rsHash(char* str) // based on the RSHash algorithm
      {
	KeyHash b = 378551;
	KeyHash a = 63689;
	KeyHash hash = 0;
	for(unsigned int i = 0; str[i] != '\0'; i++)
	  {
	    hash = hash*a+str[i];
	    a = a*b;
	  }
	return (hash & 0x7FFFFFFF);
      };
    inline Keyword () 
      { keyword = NULL; hash = 0; }
    inline Keyword ( const char * _keyword )
    {
      set ( _keyword );
    }
    inline void set ( const char * _keyword )
    {
      unsigned int ln = strlen (_keyword ) + 1; 
      keyword = (char*) malloc (ln); 
      memcpy ( keyword, _keyword, ln); 	
      hash = rsHash ( keyword );
    }
    ~Keyword () {}
    inline KeyHash getHash () const { return hash; }
    inline char * getKeyword () const { return keyword; }
    inline bool isEqual ( Keyword & keyword )
      { return isEqual ( keyword.hash, keyword.keyword ); }
    
    inline bool isEqual ( Keyword * keyword )
      { return isEqual ( keyword->hash, keyword->keyword ); }
    inline bool isEqual ( char * aKeyword )
      { return ( strcmp ( keyword, aKeyword ) == 0 ); }
    inline bool isEqual ( KeyHash aHash, char * aKeyword )
      {
		if ( aHash != hash )
		  return false;
		return isEqual ( aKeyword );
      }
  };



  class HashTree
  {
  public:
    class HashNode
    {
    public:
      class KeywordItem
      {
      public:
	Keyword * keyword;
	inline KeywordItem ( char * _keyword )
	  { keyword = new Keyword ( _keyword ); next = NULL; }
	inline KeywordItem ( Keyword * _keyword ) 
	  { keyword = _keyword; next = NULL; }
	inline ~KeywordItem ( )
	  { delete (keyword); }
	KeywordItem * next;
      };
      KeyHash hash;
      KeywordItem * first;
      KeywordItem * last;
      HashNode * left;
      HashNode * right;
      
      inline HashNode ( KeyHash _hash )
	{ hash = _hash; left = NULL; right = NULL; 
	  first = NULL; last = NULL; }
      ~HashNode();
      
      inline Keyword * getUnique ( KeyHash _hash )
	{
	  if ( first == NULL 
	       || last == NULL )
	    Bug ( "Can not find hash '0x%x' : empty hash\n", _hash );
	  AssertBug ( hash == _hash, "Shall be in the correct node !n" );
	  AssertFatal ( first == last, "Multiple keywords for hash '0x%x'\n", hash );
	  return first->keyword;
	}
      inline Keyword * getUnique ( char * keyword )
	{
	  KeywordItem * item = first;
	  while ( item != NULL )
	    { if ( item->keyword->isEqual ( keyword ) )
		return item->keyword; 
	      item = item->next;
	    }
	  if ( first == NULL )
	    {
	      first = last = new KeywordItem ( keyword );
	      return first->keyword;
	    }
	  last->next = new KeywordItem ( keyword );
	  last = last->next;
	  return last->keyword;
	}
      inline bool add ( Keyword * keyword )
	{
	  for ( KeywordItem * item = first ; item != NULL ; item = item->next )
	    {
	      if ( item->keyword->isEqual ( keyword ) )
		return false;
	    }
	  if ( last != NULL )
	    {
	      last->next = new KeywordItem ( keyword );
	      last = last->next;
	    }
	  else
	    {
	      first = last = new KeywordItem ( keyword );
	    }
	  return true;
	}
      void fillKeywordList ( std::list<DMLX::Keyword*> * l );
    };
    HashNode * root;
    HashTree() { root = NULL; }
    ~HashTree();

    inline HashNode * getNode ( KeyHash hash )
      {
	HashNode * node;
	if ( root == NULL )
	  { root = node = new HashNode ( hash ); }
	else
	  {
	    node = root;
	    while ( node->hash != hash )
	      {
		if ( node->hash > hash )
		  if ( node->left == NULL )
		    { node->left = new HashNode(hash); return node->left; }
		  else
		    { node = node->left; continue; }
		if ( node->right == NULL )
		  { node->right = new HashNode(hash); return node->right; }
		node = node->right;
	      }
	  }
	return node;
      }
    inline bool add ( Keyword * keyword )
    { return getNode ( keyword->getHash() )->add ( keyword ); }
    inline Keyword * getUnique(char * keyword)
      {
	if ( keyword[0] == '\0' )
	  { Bug ( "Invalid keyword '%s' (shall throw exception here\n", keyword ); }
	KeyHash hash = Keyword::rsHash ( keyword );
	return getNode(hash)->getUnique(keyword);
      }
    inline Keyword * getUnique(KeyHash hash)
      { return getNode ( hash )->getUnique ( hash ); }

    bool log();
    bool dumpKeywords ( char * fileName );
    std::list<Keyword*> * getKeywordList ();
  };
}; // NameSpace DMLX



#endif // __DML_DMLXKEYWORD_H
