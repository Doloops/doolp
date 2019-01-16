class UMLVAttribute
{
 protected:
  char * __origin;
 public:
  void __setOrigin ( char * o ) { __origin = o; }
  virtual void __set ( UMLModel * m, char * n, char * t );
};

template <typename T>
class UMLAttribute : public UMLVAttribute, public T
{
 public:
  UMLAttribute () {  }
  T& get () { return (T&)(*this); }
  virtual void parse(  DMLX::Node * node ) 
  {
    AssertBug ( __model != NULL, "Model not set : __name=%s, __type=%s, __baseType=%s\n", __name, __type, __getBaseType() );
    UMLAttribute_doParse<T> ( __model, dynamic_cast<T*>(this), node );
  }
  virtual void parsePlainText ( const char * text )
  {
    this->T::parsePlainText ( text );
  }
  virtual void write ( DMLX::Writer * writer )
  {
    AssertBug ( __name != NULL, "No Name Set !\n" );
    AssertBug ( __origin != NULL, "NO ORIGIN SET !\n" );
    string xmlizedName("UML:");
    xmlizedName += __origin;
    xmlizedName += ".";
    xmlizedName += __name;
    writer->writeMarkup(xmlizedName.c_str());
    if ( isUMLIdSet() )
      {
	AssertBug ( __getType(), "Attribute has no type ?\n" );
	string xmlizedName2("UML:");
	xmlizedName2+= __getType();
	writer->writeMarkup(xmlizedName2.c_str());
	writer->writeAttr("xmi.idref",getId() );
	writer->writeMarkupEnd();
      }
    else
      {
	this->T::write ( writer );
      }
    writer->writeMarkupEnd();
  }
};

class UMLAttribute<Boolean> : public UMLVAttribute, public UMLElement
{
 protected:
  bool __value;
 public:
  char * __getBaseType() { return "[UMLPrimitive : Boolean]"; }
  UMLAttribute() { __value = false; }
  virtual void parse ( DMLX::Node * node ) { node->log(); Bug ( "NOT IMPLEMENTED.\n" ); }
  virtual void parsePlainText ( const char * text )  { if ( strcmp ( text, "false" ) == 0 ) __value = false; else if ( strcmp ( text, "true" ) == 0 ) __value = true;
						       else Fatal ( "Invalid boolean value '%s'\n", text ); }
  virtual void write ( DMLX::Writer * writer )  { writer->writeText ( (char*) (__value ? "true" : "false"), true ); }
  virtual bool writeAsAttribute ( DMLX::Writer * writer ) { return writer->writeAttr ( __name, (char*) (__value ? "true" : "false") ); }
};

class UMLAttribute<Integer> : public UMLVAttribute, public UMLElement
{
 protected:
  int __value;
 public:
  char * __getBaseType() { return "[UMLPrimitive : Integer]"; }
  virtual void parse ( DMLX::Node * node )                 { UMLAttribute_doParse<Integer> ( NULL, &__value, node); }
  virtual void parsePlainText ( const char * text )        { __value = atoi ( text ); }
  virtual void write ( DMLX::Writer * writer )             { Bug ( "NOT IMPLEMENTED!\n" ); }
  virtual bool writeAsAttribute ( DMLX::Writer * writer )  { return writer->writeAttrInt ( __name, __value ); }
  UMLAttribute& operator= ( int i ) { __value = i; return *this; }
};

class UMLAttribute<Double> : public UMLVAttribute, public UMLElement
{
 protected:
  float __value;
 public:
  char * __getBaseType() { return "[UMLPrimitive : Double]"; }
  virtual void parse ( DMLX::Node * node )                 { UMLAttribute_doParse<Double> ( NULL, &__value, node); }
  virtual void parsePlainText ( const char * text )        { __value = atof ( text ); }
  virtual void write ( DMLX::Writer * writer )             { char b[32]; sprintf ( b, "%f\n", __value ); writer->writeText ( b, false ); }
  virtual bool writeAsAttribute ( DMLX::Writer * writer )  { return writer->writeAttrFloat ( __name, __value ); }
  UMLAttribute& operator= ( float f ) { __value = f; return *this; }
};

class UMLAttribute<String> : public String, public UMLVAttribute, public UMLElement
{
 public:
  UMLAttribute & operator= ( std::string s )               { assign ( s ); return *this; }
  UMLAttribute & operator= ( String & s )                  { assign ( s ); return *this; }
  UMLAttribute & operator= ( UMLAttribute<String> & s )    { assign ( s ); return *this; }
  char * __getBaseType() { return "[UMLPrimitive : String]"; }
  virtual void parse ( DMLX::Node * node )                 { node->log(); assign ( node->getText() ); }
  virtual void parsePlainText ( const char * text )        { assign ( text ); }
  virtual void write ( DMLX::Writer * writer )             { writer->writeText ( (char*)c_str(), true ); writer->writeText ( "\n", false ); }
  virtual bool writeAsAttribute ( DMLX::Writer * writer )  { return writer->writeAttr ( __name, (char*)c_str(), true ); }
};


template <int min, int max, bool ordered, bool unknown, typename T>
  class UMLMultipleAttribute : public UMLVAttribute, public UMLElement
{
 protected:
  typedef typename std::list<T*>::iterator elementierator;
  std::list<T*> __elements;
 public:
  virtual char * __getBaseType() { return "[UMLMultipleAttribute]"; }
  UMLMultipleAttribute() { }

  virtual void parse(  DMLX::Node * node ) 
  {
    __elements.clear ();
    for ( DMLX::Node * subnode = node->getFirst(); 
	  subnode != NULL ; subnode = subnode->getNext() )
      {
	T * t = new T();
	subnode = UMLAttribute_doParse<T> ( __model, t, subnode );
	__elements.push_back (t);
      }
  }
  virtual void parsePlainText ( const char * text )        { Bug ( "Can not parsePlainText when attribute is multiple ! (can I ?)\n" ); }
  virtual bool writeAsAttribute ( DMLX::Writer * writer )  { return false; }
  virtual void write ( DMLX::Writer * writer )
  {
    if ( __elements.size() == 0 )
      return;
    AssertBug ( __name != NULL, "No Name Set !\n" );
    AssertBug ( __origin != NULL, "NO ORIGIN SET !\n" );
    string xmlizedName("UML:");
    xmlizedName += __origin;
    xmlizedName += ".";
    xmlizedName += __name;
    writer->writeMarkup(xmlizedName.c_str());
    for ( elementierator iter = __elements.begin () ; iter != __elements.end() ; iter++ )
      {
	UMLAttribute_doWrite<T> ( *iter, writer );
      }
    writer->writeMarkupEnd();
  }
};
