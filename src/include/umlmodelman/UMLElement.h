class UMLId
{
 protected:
  friend class UMLModel;
  unsigned int __id;
  char __toString[32];
  UMLModel * __model;
 public:
  UMLId();
  ~UMLId();
  bool isSet();
  void set ( UMLModel * m, unsigned int id );
  char * toString ( );
};

class UMLElement
{
 protected:
  char * __name;
  char * __type;
  UMLId __id;
  UMLModel * __model;
  UMLElement() { __model = NULL; __name = NULL; __type = NULL; }
 public:
  virtual char * __getBaseType() { return "[NOT DEFINED ! UMLElement.]"; }
  char * getId();
  bool isUMLIdSet() { return __id.isSet(); }
  virtual bool isStructure() { return false; }
  void __setModel ( UMLModel * _m ) { __model = _m; }
  void __setName ( char * _n ) { __name = _n; }
  void __setType ( char * _t ) { __type = _t; }
  UMLModel * __getModel () const { return __model; }
  char * __getName () const { return __name; }
  char * __getType () const { return __type; }

  template<typename T> inline bool isOfType() { return ( dynamic_cast<T*>(this) != NULL ); }
  template<typename T> inline T * toType() { AssertFatal ( isOfType<T>(), "Wrong type.\n" ); return dynamic_cast<T*>(this); }

  virtual void parse ( DMLX::Node * node ) = 0;
  virtual void parsePlainText ( const char * text )
  { Bug ( "Invalid parsing : not implemented for this type '%s' (name '%s')\n",
	  __type, __name ); }
  virtual void write ( DMLX::Writer * writer )
  { Bug ( "Write not implemented : name=%s, type=%s, baseType=%s\n", __name, __type, __getBaseType() ); }
  virtual bool writeAsAttribute ( DMLX::Writer * writer )
  { return false; }
  virtual ~UMLElement() {}
};
