#ifndef __DOOLP_DOOLPPUBSUB_FILTER_H
#define __DOOLP_DOOLPPUBSUB_FILTER_H

#include <doolp/doolp-doolpclasses.h>
#include <DMLXParser.h>

#include <list>

namespace Doolp
{
  class PubSubFilter
  {
  protected:
    static PubSubFilter * buildFilter ( DMLX::Parser& );
  public:
    virtual ~PubSubFilter () {}
    virtual bool filter ( Object * obj ) = 0;
    virtual string * toString () = 0;
    static PubSubFilter * buildFilter ( string & subscription );
  };

  template <typename T_type>
    class PubSubFilterAccessor
    {
    public:
      virtual ~PubSubFilterAccessor () {}
      virtual T_type getValue ( Object * obj ) = 0;
      virtual string * toString () = 0;
    };

  class PubSubFilterAccessorObjectId : public PubSubFilterAccessor<ObjectNameId>
  {
  public:
    ObjectId getValue ( Object * obj ) { return obj->getObjectId(); }
    string * toString() { string * s = new string( "objectId" ); return s; }
  };

  class PubSubFilterAccessorObjectNameId : public PubSubFilterAccessor<ObjectNameId>
  {
  public:
    ObjectNameId getValue ( Object * obj ) { return obj->getNameId(); }
    string * toString() { string * s = new string( "nameId" ); return s; }
  };

  template<typename T_Param_type>
    class PubSubFilterAccessorParameter : public PubSubFilterAccessor<T_Param_type>
  {
    ObjectParamId paramId;
  public:
    PubSubFilterAccessorParameter( ObjectParamId _paramId ) { paramId = _paramId; }
    T_Param_type getValue ( Object * obj );
    string * toString() { string * s = new string("parameter nameId=\"0x"); *s+= paramId ; *s+= "\""; return s; }
  };

  class PubSubFilterAnd : public PubSubFilter
  {
    typedef std::list<PubSubFilter *> FilterList;
    FilterList filters;
  public:
    PubSubFilterAnd ( ) { }
    void add ( PubSubFilter * f ) { filters.push_back ( f ); }
    ~PubSubFilterAnd () { for ( FilterList::iterator f = filters.begin() ; f != filters.end () ; f++ ) delete ( *f); }
    bool filter ( Object * obj ) 
    { for ( FilterList::iterator f = filters.begin() ; f != filters.end () ; f++ )
	if ( ! (*f)->filter(obj) )
	  return false;
      return true; }
    string * toString () { string *s = new string("<and>"); 
      for ( FilterList::iterator f = filters.begin() ; f != filters.end () ; f++ )
	{ string * sf = (*f)->toString(); *s+=*sf; delete ( sf ); }
      *s+="</and>"; return s; }
  };

  class PubSubFilterOr : public PubSubFilter
  {
    typedef std::list<PubSubFilter *> FilterList;
    FilterList filters;
  public:
    PubSubFilterOr ( ) { }
    void add ( PubSubFilter * f ) { filters.push_back ( f ); }
    ~PubSubFilterOr () { for ( FilterList::iterator f = filters.begin() ; f != filters.end () ; f++ ) delete ( *f); }
    bool filter ( Object * obj )
    { for ( FilterList::iterator f = filters.begin() ; f != filters.end () ; f++ )
	if ( (*f)->filter(obj) )
	  return true;
      return false; }
    string * toString () { string *s = new string("<or>"); 
      for ( FilterList::iterator f = filters.begin() ; f != filters.end () ; f++ )
	{ string * sf = (*f)->toString(); *s+=*sf; delete ( sf ); }
      *s+="</or>"; return s; }
  };

  template<typename T_type>
    class PubSubFilterEqual : public PubSubFilter
  {
    PubSubFilterAccessor<T_type> * accessor;
    T_type value;
  public:
    PubSubFilterEqual ( PubSubFilterAccessor<T_type> * _accessor, T_type _value ) { accessor = _accessor; value = _value; }
    ~PubSubFilterEqual () { delete ( accessor ); }
    bool filter ( Object * obj );
    string * toString () { string *s = new string("<"); 
      string * a = accessor->toString(); *s += *a; delete ( a );
      *s += " equals "; /**s += value;*/ *s += "/>"; return s; }
  };

#if 0
  template <typename T_type>
    class PubSubFilterIsOf : public PubSubFilter
    {
      PubSubFilterAccessor<T_type> * accessor;
      list<T_type> * lst;
    public:
      typedef typename list<T_type>::iterator list_iterator;
      PubSubFilterIsOf ( PubSubFilterAccessor<T_type> * _accessor, list<T_type> * _lst ) { accessor = _accessor; lst = _lst; }
      ~PubSubFilterIsOf () { delete ( accessor ); delete ( lst ); }
      bool filter ( Object * obj ) 
      { 
	T_type value = accessor->getValue ( obj );
	for ( list_iterator iter = lst->begin () ; iter != lst->end () ; iter ++ )
	  {
	    if ( value == *iter )
	      return true;
	  }
	return false;
      }
      bool toString () { return "NOT IMPLEMENTED"; }
    };
#endif

};





#endif // __DOOLP_DOOLPPUBSUB_FILTER_H
