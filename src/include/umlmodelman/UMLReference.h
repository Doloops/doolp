class UMLVReference : public UMLElement
{
 protected:
  char * __origin;
  UMLAssociation * association;
  UMLClass * baseClass;
 public:
  virtual void parse ( DMLX::Node * node ) = 0;
  virtual void write ( DMLX::Writer * writer ) = 0;
  void __setOrigin ( char * o ) { __origin = o; }
  void __setAssociation ( UMLAssociation * a ) { association = a; }
  void __setBaseClass ( UMLClass * c ) { baseClass = c; }
  void add_otherside ( int numberAE, UMLElement * t );
  void remove_otherside ( int numberAE, UMLElement * t );
};

class UMLAssociation
{
 public:
  virtual char * __getAssociationName () = 0;
  virtual void add1 ( UMLElement * from, UMLElement * e ) = 0;
  virtual void remove1 ( UMLElement * from, UMLElement * e ) = 0;
  virtual void add2 ( UMLElement * from, UMLElement * e ) = 0;
  virtual void remove2 ( UMLElement * from, UMLElement * e ) = 0;
};

template<typename T>
class UMLReference : public UMLVReference
{
 protected:
  std::list<T *> __references;
 public:
  typedef typename std::list<T *>::iterator iterator;
  iterator begin () { return __references.begin(); }
  iterator end () { return __references.end(); }
  unsigned int size () const { return __references.size(); }
  T * front () { AssertBug ( size() > 0, "Empty reference.\n" ); return __references.front(); }
  T * back () { AssertBug ( size() > 0, "Empty reference.\n" ); return __references.back(); }
};

template <int min, int max, bool ordered, bool unknown, bool isMaster, int numberAE, typename T>
  // numberAE is the number in AssociationEnd
  class UMLRealReference : public UMLReference<T>
{
 protected:
  typedef typename std::list<T *>::iterator refiterator; // Internal use
  void checkBounds () 
  {
    AssertFatal ( (int)__references.size() >= min, "Not enough elements in Reference '%s' (class '%s') : Minimum is %d\n",
		  __name, baseClass->__getName(), min );
    AssertFatal ( (max == -1) || ((int) __references.size() <= max), "Too much elements in Reference '%s' (class '%s') : Maximum is %d\n",
		  __name, baseClass->__getName(), max );
  }
 public:
  UMLRealReference() 
    { 
      association = NULL; baseClass = NULL; 
    }
  ~UMLRealReference() 
    {
      for ( refiterator i = begin () ; i != end() ; i++ )
	{
	  remove_otherside ( numberAE, (UMLElement*)(*i) );
	}
    }
  inline operator T* ()
    {
      AssertBug ( max == 1, "Invalid operation : reference is a multiple reference.\n" );
      AssertBug ( __references.size() <= 1, "Too many references : max == 1, but has %d references.\n",
		  __references.size() );
      if ( __references.size() != 1 )
	return NULL;
      return (*__references.begin() );
    }
  inline UMLRealReference& operator= ( T * t )
    {
      __references.clear(); 
      add ( t );
      return *this;
    }
  inline UMLRealReference& operator+= ( T * t )
    {
      add ( t );
      return *this;
    }
  inline UMLRealReference& operator-= ( T * t )
    {
      remove ( t );
      return *this;
    }
  inline T* get ( unsigned int n )
    {
      AssertFatal ( (int)n < size(), "Out of bounds : asked for element '%d', but only '%d' elements\n",
		    n, size() );
      unsigned int cur = 0;
      for ( refiterator i = begin() ; i != end() ; i++ )
	{
	  if ( cur == n )
	    return *i;
	  cur++;
	}
      Fatal ( "Out of bounds : asked for element '%d', but only '%d' elements\n",
	      n, size() );
    }
  virtual char * __getBaseType() { return "[UMLReference]"; }
  void add_force ( T * t ) 
  { 
    AssertBug ( (max == -1) || ((int)__references.size() < max), "Too much elements in Reference '%s' (class '%s') : Maximum is %d\n",
		__name, baseClass->__getName(), max );
    __references.push_back ( t ); 
  }
  void remove_force ( T * t ) { __references.remove ( t ); }
  void add ( T * t )
  {
    if ( __model == NULL && baseClass->__getModel () != NULL )
      __model = baseClass->__getModel();
    AssertFatal ( __model != NULL || t->__getModel() != NULL,
		  "None of the elements have a model set. Reference at least one in a model-aware element\n" );
    if ( __model == NULL ) { __model == t->__getModel(); baseClass->__setModel ( __model ); }
    else t->__setModel( __model );
    add_force ( t );
    add_otherside ( numberAE, t );
  }
  void remove ( T * t )
  {
    remove_force ( t );
    remove_otherside ( numberAE, t );
  }
  template<class T_target>
    T_target * getElement ( std::string name ) // Here we assert that T has a name...
    {
      for ( refiterator r = begin() ; r != end() ; r++ )
	{
	  if ( (*r)->name == name )
	    {
	      T_target * t = dynamic_cast<T_target*> (*r);
	      AssertFatal ( t != NULL, "Found an object named '%s', but not of the good type.\n",
			    name.c_str() );
	      return t;
	    }
	}
      Warn ( "Could not find element with name '%s'\n", name.c_str() );
      return NULL;
    }
  virtual void parse ( DMLX::Node * item )
  {
    AssertBug ( __model != NULL, "Model not set.\n" );
    for ( DMLX::Node * node = item->getFirst() ;
	  node != NULL ; node = node->getNext() )
      {
	add ( dynamic_cast<T*> (__model->__classFromNode ( node ) ) );
      }
  }
  virtual void write ( DMLX::Writer * writer ) 
  {
    checkBounds ();
    if ( ! isMaster )
      return;
    if ( __references.size() == 0 )
      return;
    AssertBug ( __name != NULL, "No Name Set !\n" );
    AssertBug ( __origin != NULL, "NO ORIGIN SET !\n" );
    string xmlizedName("UML:");
    xmlizedName += __origin;
    xmlizedName += ".";
    xmlizedName += __name;
    writer->writeMarkup(xmlizedName.c_str());
    for ( refiterator ref = __references.begin() ; ref != __references.end() ; ref ++ )
      {
	if ( __model->isAssociationWritten ( baseClass, *ref, association ) )
	  continue;
	__model->setAssociationWritten ( *ref, baseClass, association );
	if ( (*ref)->isUMLIdSet() )
	  {
	    AssertBug ( (*ref)->__getType(), "Reference has no name ?\n" );
	    string xmlizedName2("UML:");
	    xmlizedName2+= (*ref)->__getType();
	    writer->writeMarkup(xmlizedName2.c_str());
	    writer->writeAttr("xmi.idref",(*ref)->getId() );
	    writer->writeMarkupEnd();
	  }
	else
	  {
	    (*ref)->write ( writer );
	  }
      }
    writer->writeMarkupEnd();
  }  


};
