typedef bool Boolean;
typedef int Integer;
typedef int Long;
typedef std::string String;
typedef float Double;

class UMLStructure : public UMLElement
{
 protected:
  std::list<UMLElement*> __items;
  void __addItem ( UMLElement * i )
  { __items.push_back ( i ); }
 public:
  virtual bool isStructure() { return true; }
  virtual char * __getBaseType() { return "[NOT DEFINED ! UMLStructure ]"; }
  virtual void parse ( DMLX::Node * n );
  DMLX::Node * parseAsStructure ( DMLX::Node * n); // Returns the last node used for parsing.
  virtual void write ( DMLX::Writer * writer );
};


template<typename Tb> inline DMLX::Node * UMLAttribute_doParse ( UMLModel * _m, 
								 Tb * tb, 
								 DMLX::Node * node ) 
{ 
  //  Log ( "UMLAttribute_doParse<generic> model=%p, tb=%p, node=%p\n", _m, tb, node );
  tb->__setModel(_m); 
  if ( tb->isStructure() )
    {
      UMLStructure * st = (UMLStructure*) tb;
      return UMLAttribute_doParse<UMLStructure> ( _m, st, node );
	// tb->parseAsStructure ( node ); 
    }
  else
    {
      tb->Tb::parse ( node->getFirst() ); 
    }
  return node; 
}

template<typename Tb> void UMLAttribute_doWrite ( Tb * elt, DMLX::Writer * writer ) 
{ 
  if ( elt->isStructure() )
    {
      UMLStructure * st = dynamic_cast<UMLStructure*> ((UMLElement*) elt);
      st->write ( writer );
      return;
    }
  if ( elt->isUMLIdSet() )
    {
      AssertBug ( elt->__getType(), "Attribute has no type ?\n" );
      string xmlizedName2("UML:");
      xmlizedName2+= elt->__getType();
      writer->writeMarkup(xmlizedName2.c_str());
      writer->writeAttr("xmi.idref",elt->getId() );
      writer->writeMarkupEnd();
    }
  else
    {
      elt->write ( writer );
    }
}

template<> DMLX::Node * UMLAttribute_doParse<Boolean> ( UMLModel * _m, Boolean * tb, DMLX::Node * node );
template<> DMLX::Node * UMLAttribute_doParse<Integer> ( UMLModel * _m, Integer * tb, DMLX::Node * node );
template<> DMLX::Node * UMLAttribute_doParse<Double> ( UMLModel * _m, Double * tb, DMLX::Node * node );
template<> DMLX::Node * UMLAttribute_doParse<String> ( UMLModel * _m, String * tb, DMLX::Node * node );
template<> DMLX::Node * UMLAttribute_doParse<UMLStructure> ( UMLModel * _m, UMLStructure * tb, DMLX::Node * node );

template<> void UMLAttribute_doWrite<Boolean> ( Boolean * tb, DMLX::Writer * writer );
template<> void UMLAttribute_doWrite<Integer> ( Integer * tb, DMLX::Writer * writer );
template<> void UMLAttribute_doWrite<Double> ( Double * tb, DMLX::Writer * writer );
template<> void UMLAttribute_doWrite<String> ( String * tb, DMLX::Writer * writer );
template<> void UMLAttribute_doWrite<UMLStructure> ( UMLStructure * tb, DMLX::Writer * writer );



template<typename enumerator>
class UMLEnumeration : public UMLElement
{
 protected:
  enumerator __value;
  std::map<std::string,enumerator> __mapValues;
  std::map<enumerator,std::string> __reverseMap;
  void __addValue(const char * valname,enumerator val)
  { string s(strchr(valname,'_')+1); __mapValues[s] = val; __reverseMap[val] = s; } // Workaround for the unprefix tag...
 public:
  virtual char * __getBaseType()             { return "[NOT DEFINED ! UMLEnumeration ]"; }
  virtual void parse ( DMLX::Node * n )              { Bug ( "Invalid ! (not implemented ?\n" ); }
  virtual void parsePlainText ( const char * text )  { string s(text); __value = __mapValues[s]; }
  virtual bool writeAsAttribute ( DMLX::Writer * writer )
  {
    writer->writeAttr ( __name, (char*) __reverseMap[__value].c_str() );
    return true;
  }
  void setValue ( const enumerator e ) { __value = e; }
};

