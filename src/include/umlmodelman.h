#ifndef __UMLMODELMAN_H
#define __UMLMODELMAN_H

#include <string>
#include <list>
#include <map>
#include <vector>

#include <DMLXDocument.h>
#include <DMLXWriter.h>

class UMLStructure;
class UMLAssociation;
class UMLClass;
class UMLModel;

#include <umlmodelman/UMLElement.h>
#include <umlmodelman/UMLPrimitives.h>
#include <umlmodelman/UMLAttribute.h>
#include <umlmodelman/UMLReference.h>
#include <umlmodelman/UMLClass.h>
#include <umlmodelman/UMLModel.h>

#define __UMLAssociation__(__name,__type1,__type2) \
  class __name : public UMLAssociation		   \
  {						   \
  protected:					   \
    bool checkTypes(UMLElement*e1,UMLElement*e2);  \
  public:					   \
    char * __getAssociationName ();		   \
    void add1(UMLElement*from,UMLElement*e);	   \
    void remove1(UMLElement*from,UMLElement*e);	   \
    void add2(UMLElement*from,UMLElement*e);	   \
    void remove2(UMLElement*from,UMLElement*e);	   \
  };

#define _STRINGIFY(_x) #_x
#define STRINGIFY(_x) _STRINGIFY(_x)

#define __UMLAssociation__CODE__(__name,__type1,__type2,__link1,__link2) \
  char * __name::__getAssociationName ()				\
  { return STRINGIFY(__name); }						\
  bool __name::checkTypes(UMLElement*e1,UMLElement*e2)			\
  {									\
    if ( dynamic_cast<__type2*>(e1) == NULL )				\
      {									\
	Warn ( "First element is invalid : name %s, type %s, basetype %s\n", \
	       e1->__getName(), e1->__getType(), e1->__getBaseType() );	\
	return false;							\
      }									\
    if ( dynamic_cast<__type1*>(e2) == NULL )				\
      {									\
	Warn ( "Second element is invalid : name %s, type %s, basetype %s\n", \
	       e2->__getName(), e2->__getType(), e2->__getBaseType() ); \
	return false;							\
      }									\
    return true;							\
  }									\
  void __name::add1(UMLElement*from,UMLElement*e)			\
  {									\
    if (checkTypes(from,e))						\
      {									\
	(dynamic_cast<__type2*>(from))->__link1.add_force		\
	  ( dynamic_cast<__type1*>(e) );				\
      }									\
  }									\
  void __name::remove1(UMLElement*from,UMLElement*e)			\
  {									\
    if (checkTypes(from,e))						\
      {									\
	(dynamic_cast<__type2*>(from))->__link1.remove_force		\
	  ( dynamic_cast<__type1*>(e) );				\
      }									\
  }									\
  void __name::add2(UMLElement*from,UMLElement*e)			\
  {									\
    if (checkTypes(e,from))						\
      {									\
	(dynamic_cast<__type1*>(from))->__link2.add_force		\
	  ( dynamic_cast<__type2*>(e) );				\
      }									\
  }									\
  void __name::remove2(UMLElement*from,UMLElement*e)			\
  {									\
    if (checkTypes(e,from))						\
      {									\
	(dynamic_cast<__type1*>(from))->__link2.remove_force		\
	  ( dynamic_cast<__type2*>(e) );				\
      }									\
  }


#endif // __UMLMODELMAN_H

/*
  void __name::remove1(UMLElement*from,UMLElement*e) { if (checkTypes(from,e)){(dynamic_cast<%s*>(from))->%s.remove_force ( dynamic_cast<%s*>(e) ); }}\n",
	   assoc->getAttribute ( "name" ),ae2_type, getProtectedName(ae1->getAttribute("name")), ae1_type );
  void __name::add2(UMLElement*from,UMLElement*e) { if (checkTypes(e,from)){(dynamic_cast<%s*>(from))->%s.add_force ( dynamic_cast<%s*>(e) ); }}\n",
	   assoc->getAttribute ( "name" ),ae1_type, getProtectedName(ae2->getAttribute("name")), ae2_type );
void __name::remove2(UMLElement*from,UMLElement*e) { if (checkTypes(e,from)){(dynamic_cast<%s*>(from))->%s.remove_force ( dynamic_cast<%s*>(e) ); }}\n",
	   assoc->getAttribute ( "name" ),ae1_type, getProtectedName(ae2->getAttribute("name")), ae2_type );

*/

/*-----------------------------------------------------------------------------------*/
// Garbage
/*-----------------------------------------------------------------------------------*/
#if 0
/*
class Integer : public UMLPrimitive<int>
{
 public:
  Integer() { setValue ( 0 ); }
  Integer(int i) { setValue ( i ); }
  char * __getBaseType() { return "[UMLPrimitive : Integer]"; }
  virtual void parse ( DMLX::Node * node )                 { __value = atoi ( node->getText() ); }
  virtual void parsePlainText ( const char * text )        { __value = atoi ( text ); }
  virtual void write ( DMLX::Writer * writer )             { char b[32]; sprintf ( b, "%d\n", __value ); writer->writeText ( b, false ); }
  virtual bool writeAsAttribute ( DMLX::Writer * writer )  { return writer->writeAttrInt ( __name, __value ); }
};
*/
//class Long : public Integer {};
/*
class String : public UMLElement, public std::string // UMLPrimitive<std::string>
{
 public:
  String() { Log ( "New string at %p\n", this ); }
  ~String() { Log ( "Delete string at %p\n", this ); }
  char * __getBaseType() { return "[UMLPrimitive : String]"; }
  virtual void parse ( DMLX::Node * node )                 { node->log(); assign ( node->getText() ); }
  virtual void parsePlainText ( const char * text )        { assign ( text ); }
  virtual void write ( DMLX::Writer * writer )             { writer->writeText ( (char*)c_str(), true ); writer->writeText ( "\n", false ); }
  virtual bool writeAsAttribute ( DMLX::Writer * writer )  { return writer->writeAttr ( __name, (char*)c_str(), true ); }

  // Accessing operators.
  inline String& operator = ( const std::string & s )      { assign ( s ); return * this; }
};
*/
class Float : public Double {};

template<typename T>
class UMLPrimitive : public UMLElement
{
 protected:
  T __value;
 public:
  virtual char * __getBaseType() { return "[NOT DEFINED ! UMLPrimitive.]"; }
  T& getValue () const { return (T&)__value; }
  void setValue ( T &v ) { __value = v; }
  virtual void parse ( DMLX::Node * node )
  {
    Log ( "Parse primitive.\n" );
  }
  inline UMLPrimitive& operator= (const T& op) { __value = op; return *this; }
  inline UMLPrimitive& operator+= (const T& op) { __value += op; return *this; }
  inline UMLPrimitive& operator-= (const T& op) { __value += op; return *this; }
  inline operator T () { return __value; }
};

// In UMLMultipleAttribute::parse()

	/*
	UMLStructure * s = dynamic_cast<UMLStructure *> (t);
	if ( s != NULL )
	  {
	    subnode = s->parseAsStructure ( subnode );
	  }
	else if ( dynamic_cast<Boolean *> (t)
		  || dynamic_cast<Integer *> (t)
		  || dynamic_cast<String *> (t) )
	  {
	    UMLAttribute<T> at;
	    at.__setModel ( __model );
	    at.parse (subnode);
	    *t = at;
	  }
	else
	  {
	    t->parse ( subnode );
	  }
	*/


#endif


