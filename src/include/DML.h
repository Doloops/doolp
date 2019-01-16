#ifndef __DML_H
#define __DML_H

/*
  Shall remove all char*, replaced with strings
*/


/* Debbie Meta Language */
// #include <vector>
#include <list>
#include <map>
#include "glog.h"
using namespace std;
// class DMLParam;
class DMLSection;

// Upgrade to XML Syntax definition.

// #define __DML_EXTENSIVE_DEBUG
#ifdef __DML_EXTENSIVE_DEBUG
bool dumpWords ( list<char *> * words );
#else
#define dumpWords(words)
#endif

#include <DMLTransform.h>

/*
class DMLParam
{
 private:
  // vector<char *> value;
  char * name;
  char * value;
 public:
  DMLParam::DMLParam ( char * _name ) { name = _name; value = NULL; }
  DMLParam::DMLParam ( char * _name, char * value ) { name = _name, value = _value; }
  
};
*/
  // char * name;
  // 	char * type; // Deprecated ?
	// list<char *> options;
  /*
	DMLParam::DMLParam ( char * _name ) { name = _name; type = NULL ; soleValue = NULL; }
	DMLParam::DMLParam ( char * _name, char * _value ) { name = _name; type = NULL ; soleValue = _value; }
	DMLParam::DMLParam ( char * _type, char * _name, char * _value ) 
	  { type = _type; name = _name; soleValue = _value; }
	  */
	/*
	DMLParam::DMLParam ( char * _type, vector<char *> _options, char * _name, vector<char *> _value )
		{ type = _type ; options = _options ; name = _name ; value = _value ; }
		*/
  /*
	void DMLParam::setValue ( char * _value )
	{ soleValue = _value; }
	char * DMLParam::getValue () { return soleValue; }
	bool DMLParam::write ( int tab, FILE * fp );
  */
	/*
	void DMLParam::setValue ( vector<char*> &_value )
	{ value = _value; }
	*/
	/*
	char * DMLParam::getValue ( )
	  { if ( value.size() == 0 ) Fatal ( "No value for parameter \"%s\"\n", name );
	    if ( value.size() > 1 ) 
	      { 
		Error ( "Dumping values of parameter \"%s\"\n", name );
		dumpWords ( &value );
		Fatal ( "Too much values for parameter \"%s\"\n", name );
	      }
	    return value[0]; }
	    */


typedef map<char *, char*> DMLSectionParams;

class DMLSection
{
protected:
  // bool DMLSection::write ( int tab, FILE * fp );
  
  char * name;
  DMLSection * father;
  bool textSection;
  
  DMLSectionParams params;
  list<DMLSection*> sections;

	// char * type;
	// list<char*> options;
	// list<DMLParam*> params;
	// map<char*, char*> params;

public:	

	DMLSection::DMLSection ( char * _name )	
	  { 
	    textSection = false;
	    name = _name; 
	    father = NULL; 
	  }
	DMLSection::DMLSection ( char * text, char * encoding )
	  {
	    textSection = true;
	    name = "Text";
	    father = NULL;
	    addParam ( "(text)", text );
	  }
	
	bool isTextSection () { return textSection; }
	bool setTextSection () { textSection = true; return true;}

	bool addParam ( char * paramName ) 
	{ return addParam ( paramName, NULL ); }
	
	bool addParam ( char * paramName, char * paramValue )
	{ params.insert ( make_pair ( paramName, paramValue ) ); 
	  return true; 
	}
	bool setParam ( char * paramName, char * paramValue );
	char * getName () { return name; }
	char * getParam ( char * paramName );
	
	// { return params[paramName]; }

	DMLSection * getSection ( char * sectionName );
	DMLSection * getSectionWithParam ( char * param, char * value );

	list<DMLSection*> * getSections ( ) { return &sections; }
	DMLSection * addSection ( DMLSection * section )
	{
	  sections.push_back ( section );
	  return section;
	}
	bool addTextSection ( char * text, char * encoding )
	{
	  return addSection ( new DMLSection ( text, encoding ) );
	  //	  DMLSection * textSection = new DMLSection ( "(text)" );
	}
	DMLSection * addSection ( char * sectionName )
	  { 
	    DMLSection * section = new DMLSection ( sectionName ); 
	    addSection ( section ); return section; 
	  }


	//	bool DMLSection::write ( FILE * fp )
	//	{ return write ( 0, fp ); }

	

 protected:
	bool DMLSection::transformLine ( char * to, char * from );
	bool DMLSection::transform ( FILE * fp_out, DMLTransform * trans, char * line, char * precond );
	bool DMLSection::transform ( FILE * fp_out, DMLTransform * trans, DMLTransformSection * sec, char * precond );
 public:
	bool DMLSection::transform ( FILE * fp_out, DMLTransform * trans );

 protected:
	bool writeXML ( FILE * fp, int tab );

 public:
	bool writeXML ( FILE * fp );
};

typedef list<DMLSection *>::iterator DMLSectionIter;

// DMLSection * readDMLSection ( FILE * fp );
DMLSection * readDMLSectionXML ( FILE * fp );

DMLTransform * readDMLTransform ( FILE * fp );

// Internal Stuff


char readWords ( FILE * fp, list<char *> * words, 
		 const char * separators, 
		 const char * explicitSeparators,
		 const char * terminators, 
		 const char * avoids );
char seekTo ( FILE * fp, const char * to );
char seekTo ( FILE * fp, const char * to, char ** text );
#endif  // __DML_H


#if 0

// Garbage

	// DMLSection::DMLSection ( char * _type, char * _name )	
	//		{ type = _type ; name = _name; father = NULL; }

	// DMLParam * DMLSection::getParam ( char * paramName );

	//	bool DMLSection::isOfType ( char * _type )
	// { return ( strcmp ( type, _type ) == 0 ) ; }
	/*
	
	bool DMLSection::addParam ( char * paramName ) 
		{ return addParam ( new DMLParam ( paramName ) ); }
	DMLParam * DMLSection::addParam ( DMLParam * param ) 
		{ params.push_back ( param ); return param; }
	DMLParam * DMLSection::addParam ( char * paramName, char * paramValue )	
		{ return addParam ( new DMLParam ( paramName, paramValue ) ); }	
	*/

	/*
	DMLSection * DMLSection::addSection ( char * sectionType, char * sectionName )
		{ DMLSection * section = new DMLSection ( sectionType, sectionName ); addSection ( section ); return section; }
	*/
	DMLSection * DMLSection::addSection ( DMLSection * section )
		{ sections.push_back ( section ); section->father = this; return section; }

	DMLSection * DMLSection::getFather ( ) { return father; }
	/*
	char * DMLSection::getParamValue ( char * paramName ) 
	{ 
		DMLParam * param = getParam ( paramName );
		if ( param )
		  return param->getValue();//param->value[0];
		else
		  return NULL;
	}
	*/

	/*
	char * DMLSection::getVal ( char * param )
	  { return getParamValue ( param ); }
	bool DMLSection::isVal( char * param, char * value )
	  { return ( strcmp ( getVal ( param ), value ) == 0 ); }
	bool DMLSection::getParamValueBool ( char * paramName )
	{
		char * val = getParamValue ( paramName );
		while ( val[0] == ' ' ) val ++;
		if ( !val )
			{ Error ( "Could not find parameter : %s\n", paramName ); 
			  return false; }
		Log ( "paramName \'%s\', val \'%s\' (bool %s)\n", paramName, val,
		      strncmp ( val, "true", 4) == 0 ? "true" : "false" );
		return ( strncmp ( val, "true", 4) == 0);	
	}
	*/
/*	
	bool DMLSection::getBool ( char * par )
	{ return getParamValueBool ( par ); }
*/
#endif
