#include <DMLXKeyword.h>

DMLX::HashTree::~HashTree()
{
  //  if ( root != NULL )
  //    delete root;
}

DMLX::HashTree::HashNode::~HashNode()
{
  AssertBug ( first != NULL, "Node has no keyword item !\n" );
  if ( left != NULL )
    {
      delete ( left );
    }
  if ( right != NULL )
    {
      delete ( right );
    }
  KeywordItem * item = first, * next;
  while ( item != NULL )
    {
      next = item->next;
      Log ( "deleting hash='0x%x' key='%s'(%p)\n", hash, item->keyword->getKeyword(), item->keyword );
      delete ( item );
      item = next;
    }
}

bool DMLX::HashTree::log()
// Optimization craziness : do not use recursive calls !
// Use a static-sized depth and stack (both nodes and states)
// States : 0 : must visit left, 1 : must visit right, 2 : finished.
{
  static const unsigned int maxDepth = 32;
  unsigned int depth = 0;
  HashNode * nodeStack[maxDepth];
  HashNode * node = root;
  HashNode::KeywordItem * item;
  char stackState[32];
  //  char tabs[32];
  stackState[0] = 0;
  unsigned int lastDepth = 0;
  unsigned int totalNodes = 0;
  if ( node == NULL )
    {
      Warn ( "No keywords !\n" );
      return false;
    }
  while ( true )
    {
      AssertBug ( node != NULL, "Bug, shall not have a NULL node.\n" );
      /*
	for ( unsigned int i = 0 ; i < depth ; i ++ )
	tabs[i] = '\t';
	tabs[depth] = '\0';
	Log ( "%sAt node=%p(l%p,r%p), hash='0x%x', depth=%d, state=%d\n",
	tabs, node, node->left, node->right, node->hash, depth, stackState[depth] );
      */
      if ( depth > lastDepth )
	lastDepth = depth;

      if ( stackState[depth] == 0 )
	{
	  totalNodes++;
	  nodeStack[depth] = node;
	  // Visiting left
	  stackState[depth] = 1; 
	  if ( node->left != NULL )
	    {
	      node = node->left;
	      depth++;
	      stackState[depth] = 0;
	    }
	  continue;
	}
      else if ( stackState[depth] == 1 )
	{
	  item = node->first;
	  while ( item != NULL )
	    {
	      Log ( "Item '%p' : Hash '0x%08x' : Keyword '%s'\n",
		    item, node->hash, item->keyword->getKeyword() );
	      item = item->next;
	    }

	  // Visiting right
	  stackState[depth] = 2;
	  if ( node->right != NULL )
	    {
	      node = node->right;
	      depth++;
	      stackState[depth] = 0;
	    }
	  continue;
	}
      else if ( stackState[depth] == 2 )
	{
	  if ( depth == 0 )
	    break;
	  depth--;
	  node = nodeStack[depth];
	}
    }
  Log ( "Total depth is %d, Total Nodes %d\n",
	lastDepth, totalNodes );
  return true;
}


bool DMLX::HashTree::dumpKeywords( char * fileName )
// Optimization craziness : do not use recursive calls !
// Use a static-sized depth and stack (both nodes and states)
// States : 0 : must visit left, 1 : must visit right, 2 : finished.
{
  FILE * fp = fopen ( fileName, "w" );
  if ( fp == NULL )
    {
      Error ( "Could not open dump file '%s'\n", fileName );
      return false;
    }
  static const unsigned int maxDepth = 32;
  unsigned int depth = 0;
  HashNode * nodeStack[maxDepth];
  HashNode * node = root;
  HashNode::KeywordItem * item;
  char stackState[32];
  stackState[0] = 0;
  if ( node == NULL )
    {
      Warn ( "No keywords !\n" );
      return false;
    }
  while ( true )
    {
      AssertBug ( node != NULL, "Bug, shall not have a NULL node.\n" );
      if ( stackState[depth] == 0 )
	{
	  nodeStack[depth] = node;
	  // Visiting left
	  stackState[depth] = 1; 
	  if ( node->left != NULL )
	    {
	      node = node->left;
	      depth++;
	      stackState[depth] = 0;
	    }
	  continue;
	}
      else if ( stackState[depth] == 1 )
	{
	  item = node->first;
	  while ( item != NULL )
	    {
	      //	      Log ( "Hash '0x%08x' : Keyword '%s'\n",
	      //		    node->hash, item->keyword->getKeyword() );
	      fprintf ( fp, "%s\n", item->keyword->getKeyword () );
	      item = item->next;
	    }
	  // Visiting right
	  stackState[depth] = 2;
	  if ( node->right != NULL )
	    {
	      node = node->right;
	      depth++;
	      stackState[depth] = 0;
	    }
	  continue;
	}
      else if ( stackState[depth] == 2 )
	{
	  if ( depth == 0 )
	    break;
	  depth--;
	  node = nodeStack[depth];
	}
    }
  return true;
}

void DMLX::HashTree::HashNode::fillKeywordList ( std::list<DMLX::Keyword*> * l )
{
  for ( KeywordItem * item = first ; item != NULL ; item = item->next )
    {
      l->push_back ( item->keyword );
    }
  if ( left != NULL )
    left->fillKeywordList ( l );
  if ( right != NULL )
    right->fillKeywordList ( l );
}

std::list<DMLX::Keyword*> * DMLX::HashTree::getKeywordList ()
{
  std::list<DMLX::Keyword*> * l = new std::list<DMLX::Keyword*>;
  if ( root != NULL )
    root->fillKeywordList ( l );
  return l;
}
