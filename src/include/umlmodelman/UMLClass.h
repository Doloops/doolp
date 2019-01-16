class UMLClass : public UMLElement
{
 protected:
  std::map<std::string, UMLVAttribute *> __attributesMap;
  std::map<std::string, UMLVReference *> __referencesMap;

  void __addAttribute ( UMLVAttribute * e, char * _name, char * _type );
  void __addReference ( UMLVReference * r, char * _name, char * _type, UMLAssociation * _assoc );
 public:
  virtual char * __getBaseType() { return "[NOT DEFINED : UMLClass ?]"; }

  UMLElement * resolveElement ( char * elementName );
  void parse ( DMLX::Node * cl );
  void write ( DMLX::Writer * writer );
  virtual void log ();
};

typedef UMLClass * (*UMLClassConstructor) ( );
