#include <DMLXDocument.h>

#if 0
#undef Log
#define Log(...)
#endif

#define __DMLX_DOCUMENT_COPY_VALUES

#ifdef __DMLX_DOCUMENT_COPY_VALUES
#define __setValue(_v) value = new char[strlen(_v)+1]; strcpy ( value, _v );
#define __setText(_t,_b) if ( _b ) { text = new char[strlen(_t)+1]; strcpy ( text, _t ); } else { text = (char*)_t; }
#else
#define __setValue(_v) value = (char*)_v;
#define __setText(_t,_b) text = (char*)_t;
#endif
DMLX::Attribute::Attribute ( Keyword * _name, const char * _value )
{ name = _name; __setValue(_value); next = NULL; }

DMLX::Attribute::Attribute ( const char * _name, const char * _value )
{ Keyword * k = new Keyword ( _name ); name = k;
  __setValue(_value); next = NULL;}



DMLX::Node::Node ( Keyword & _name ) 
{ __init(); name = &_name; }
DMLX::Node::Node ( Keyword * _name ) 
{ __init(); name = _name; }
DMLX::Node::Node ( const char * _name )
{ __init(); Keyword * k = new Keyword ( _name ); name = k; Log ( "New node '%s'\n", _name );
}

DMLX::Node::Node ( Node * _father, Keyword * _name ) 
{ __init(); father = _father; name = _name; 
  if ( father != NULL ) father->addNode(this); }
DMLX::Node::Node ( Node * _father, const char * _name ) 
{ __init(); father = _father; Keyword * k = new Keyword ( _name ); name = k;  Log ( "New node '%s', father %p\n", _name, father );
  if ( father != NULL ) father->addNode(this); }

DMLX::Node::Node ( const char * _text, bool cpy ) 
{ __init(); _isText = true;  __setText(_text,cpy); }
DMLX::Node::Node ( Node * _father, const char * _text, bool cpy ) 
{ __init(); father = _father; _isText = true; __setText(_text,cpy); 
  if ( father != NULL ) father->addNode(this); }

void DMLX::Node::addNode ( Node * n ) 
{
  if ( first == NULL )
    {
      AssertBug ( last == NULL, "first null but not last.\n" );
      first = last = n;      
      return;
    }
  last->setNext ( n );
  last = n;
  n->father = this;
}

void DMLX::Node::addAttribute ( DMLX::Attribute * a )
{
  if ( attrfirst == NULL )
    {
      AssertBug ( attrlast == NULL, "attrfirst is null, but not attrlast" );
      attrfirst = attrlast = a;
      return;
    }
  attrlast->setNext ( a );
  attrlast = a;
}

void DMLX::Document::addNode ( Node * n ) 
{
  if ( first == NULL )
    {
      AssertBug ( last == NULL, "first null but not last.\n" );
      first = last = n;      
      return;
    }
  last->setNext ( n );
  last = n;
}

void DMLX::Attribute::write ( DMLX::Writer &writer )
{
  writer.writeAttr ( *name, getValue(), true );
}

void DMLX::Node::write ( DMLX::Writer &writer )
{
  if ( isText() )
    {
      if ( getText() == NULL )
	{
	  Warn ( "Null Text.\n" );
	  return;
	}
      Log ( "Writing text (at '%p')\n", getText() );
      writer.writeText ( getText(), true );
      return; 
    }
  writer.writeMarkup ( *name );
  
  for ( Attribute * attr = attrfirst; attr != NULL ; attr = attr->getNext() )
    attr->write ( writer );
  for ( Node * node = first ; node != NULL ; node = node->getNext() )
    node->write ( writer );
  writer.writeMarkupEnd ();
}

void DMLX::Document::write ( DMLX::Writer &writer )
{
  for ( Node * node = first ; node != NULL ; node = node->getNext() )
    {
      node->write ( writer );
    }
  writer.flush ();
}


void DMLX::Attribute::log ()
{
  Log ( "\tAttr '%s'='%s'\n", name->getKeyword(), getValue() );
}

void DMLX::Node::log ()
{
  if ( isText() )
    {
      if ( getText() == NULL )
	{
	  Warn ( "Null Text.\n" );
	  return;
	}
      Log ( "Text node (%d bytes) : %s\n", strlen ( getText() ), getText() );
      return; 
    }
  Log ( "At Node '%s' [%p]\n", name->getKeyword(), this );
  Attribute * last = NULL;
  for ( Attribute * attr = attrfirst; attr != NULL ; attr = attr->getNext() )
    {
      AssertBug ( last != attr, "Cycling ! last=%p, attr=%p\n", last, attr );
      last = attr;
      Log ( "At attr %p\n", attr );
      attr->log ();
    }
  for ( Node * node = first ; node != NULL ; node = node->getNext() )
    {
      node->log ();
    }
}

void DMLX::Document::log ()
{
  for ( Node * node = first ; node != NULL ; node = node->getNext() )
    {
      node->log ();
    }
}

char * DMLX::Node::getAttribute ( DMLX::Keyword & k )
{
  for ( Attribute * attr = attrfirst ; attr != NULL ; attr = attr->getNext() )
    if ( attr->name->isEqual ( k ) )
      return attr->getValue ();
      
  //  Warn ( "Could not get attr with key '%s'\n", k.getKeyword () );
  return NULL;
}

DMLX::Node * DMLX::Node::getNode ( DMLX::Keyword & k )
{
  for ( Node * n = first; n != NULL ; n = n->getNext() )
    if ( n->isName (k) )
      return n;
  Warn ( "Could not get node with key '%s'\n",
	 k.getKeyword () );
  return NULL;
}

DMLX::Node * DMLX::Node::getNodeWithAttribute ( DMLX::Keyword & attribute, const char * value )
{
  if ( first == NULL ) Log ( "No nodes at all !\n" );
  for ( Node * n = first; n != NULL ; n = n->getNext() )
    {
      if ( n->isText() ) continue;
      Log ( "At node %p : %s\n", n, n->getName () );
      Log ( "Attr '%s'\n", n->getAttribute ( attribute ) );
      char * attr = n->getAttribute ( attribute );
      if ( attr == NULL )
	continue;
      if ( strcmp ( attr, value ) == 0 )
	return n;
    }
  Warn ( "Could not get node with attribute '%s' and value '%s'\n",
	 attribute.getKeyword (), value );
  return NULL;
}



DMLX::Node * DMLX::Document::getNode ( DMLX::Keyword & k )
{
  for ( Node * node = first ; node != NULL ; node = node->getNext() )
    {
      if ( node->isName ( k ) )
	return node;
    }
  Warn ( "Could not get node with key '%s'\n",
	 k.getKeyword () );
  return NULL;
}

std::list<DMLX::Node *> * DMLX::Node::getNodeList ( DMLX::Keyword & k )
{
  std::list<DMLX::Node *> * lst = new std::list<DMLX::Node *>;	
  for ( Node * node = first ; node != NULL ; node = node->getNext() )
    {
      if ( node->isName ( k ) )
	lst->push_back ( node );
    }
  return lst;
}

std::list<DMLX::Node *> * DMLX::Document::getNodeList ( DMLX::Keyword & k )
{
  std::list<DMLX::Node *> * lst = new std::list<DMLX::Node *>;	
  for ( Node * node = first ; node != NULL ; node = node->getNext() )
    {
      if ( node->isName ( k ) )
	lst->push_back ( node );
    }
  return lst;
}

#define __Log(...)
void DMLX::Document::buildFromParser ( DMLX::Parser & parser )
{
  //  parser.setParseCanFillBufferOnce ( true );
#define __currentFather ( nodeStack.size() == 0 ? NULL : nodeStack.back () )
  parser.setHashTree ( hashTree );
  bool lastIsText = false;
  try
    {
      std::list<Node*> nodeStack;
      Event * event;
      while ( ( event = parser.getEvent() ) != NULL )
	{
	  Node * node = NULL;
	  if ( event->isEnd() )
	    {
	      lastIsText = false;
	      __Log ( "End '%s'\n", parser.getEventName () );
	      AssertBug ( nodeStack.size() > 0 );
	      nodeStack.pop_back();
	      parser.popEventEnd();
	      continue;
	    }
	  else if ( event->isMarkup () )
	    {
	      lastIsText = false;
	      node = new Node ( __currentFather, event->name );
	      __Log ( "Markup '%s'\n", parser.getEventName () );
	      Attr * attr = event->first;
	      while ( attr != NULL )
		{
		  node->addAttribute ( new Attribute ( attr->name, attr->value ) );
		  attr = attr->next;	
		}
	    }
	  else if ( event->isText () )
	    {
	      if ( lastIsText == true )
		{
		  __Log ( "Two consecutive texts ! event=%p\n", event );
		}
	      lastIsText = true;
	      __Log ( "Original text at ('%p')\n", event->text );
	      node = new Node ( __currentFather, event->text, true );
	    }
	  else
	    Bug ( "Unkown event type...\n" );

	  if ( nodeStack.size () == 0 )
	    addNode ( node );
	  if ( event->isMarkup() )
	    nodeStack.push_back ( node );
				
	  parser.popEvent();
	}	
    }
  catch ( ParserException * e )
    {
      __Log ( "Got exception '%s'\n", e->getMessage() );		
    }	
}




/*
 * References related functions
 */
void DMLX::Document::addReference ( const char * ref, Node * node )
{
  //  Log ( "New reference '%s'\n", ref );
  string sref(ref);
  references.put ( sref, node );
}

DMLX::Node * DMLX::Document::getReference ( const char * ref )
{
  string sref(ref);
  AssertFatal ( references.get ( sref ) != NULL, "Could not resolve reference !\n" );
  return references.get ( sref );
}

void DMLX::Node::buildReferences ( Document& doc, Keyword& keyw_id )
{
  char * ref = getAttribute ( keyw_id );
  if ( ref != NULL )
    {
      doc.addReference ( ref, this );
    }
  if ( first )
    first->buildReferences ( doc, keyw_id );
  if ( next )
    next->buildReferences ( doc, keyw_id );
}

void DMLX::Node::resolveReferences ( Document& doc, Keyword& keyw_idref )
{
  char * ref = getAttribute ( keyw_idref );
  if ( ref != NULL )
    {
      reference = doc.getReference ( ref );
    }
  if ( first )
    first->resolveReferences ( doc, keyw_idref );
  if ( next )
    next->resolveReferences ( doc, keyw_idref );
}

void DMLX::Document::buildReferences ( Keyword& keyw_id, Keyword& keyw_idref )
{
  if ( first )
    {
      first->buildReferences ( *this, keyw_id );
      first->resolveReferences ( *this, keyw_idref );
    }
}

