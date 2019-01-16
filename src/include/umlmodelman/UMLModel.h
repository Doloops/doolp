#include <strongmap.h>

class UMLModel
{
 protected:
  std::map<std::string, UMLClassConstructor> __classesMap;
  void __addClass(char * _name, UMLClassConstructor c );
  std::list<UMLClass *> __classes;
  std::map<string,UMLClass*> __referencedClasses;
  std::list<UMLId*> __umlIds;
  unsigned int nextId;
  strongdoublemap<UMLClass *,std::string,std::list<UMLClass*>*> __writtenAssociations;
  void clearUMLIds();
  void clearAssociations();
 public:
  UMLModel ();
  void provideId ( UMLId & id );
  void releaseId ( UMLId & id );
  UMLClass * __classFromNode ( DMLX::Node * cl );
  void parse ( DMLX::Node * content );
  void parse ( DMLX::Document * doc );
  void write ( DMLX::Writer * writer );

  void importXMI ( const char * filename );
  void exportXMI ( const char * filename );
  void log ();

  void setAssociationWritten ( UMLClass * class1, UMLClass * class2, UMLAssociation * assoc );
  bool isAssociationWritten ( UMLClass * class1, UMLClass * class2, UMLAssociation * assoc );

  template<typename T>
    T * getElement ( char * name )
    {
      std::list<UMLClass *>::iterator cl;
      for ( cl = __classes.begin () ; cl != __classes.end () ; cl++ )
	{
	  if ( strcmp ( (*cl)->__getName(), name ) == 0 )
	    return dynamic_cast<T*>(*cl);
	}
      Warn ( "Could not get element named '%s'\n", name );
      return NULL;
    }
};
