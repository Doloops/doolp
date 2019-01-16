#ifndef __DML_DMLTRANSFORM_H
#define __DML_DMLTRANSFORM_H

#ifndef __DML_H
#error Shall be included in DML.h file directly.
#endif 

/*
  pre-parse using :
  opcodes are :
  // Replacements
  @name
  @type
  @father.name
  @father.type
  @('value')
  @(father.'value')
  // Conditions
  @if
  @else
  @endif
  // Loops
  @forAllSections
  @forSections ( )
  @.
  // Evals
  ==
  !=
  &&
  ||
  // Others
  [raw] // raw data.
*/

class DMLTransformTocken
{
 public:
  unsigned char opcode;
  char * value;
  DMLTransformTocken * left, *right;
};
class DMLTransformLine
{
 public:
  unsigned char opcode;
  DMLTransformTocken * precond;
  DMLTransformTocken * line;
  DMLTransformLine * next;
  DMLTransformLine * jmpTrue, * jmpFalse;
};

class DMLTransformSection
{
 public:
  char * name;
  list<char *> lines;
  // DMLTransformLine * firstLine;
  // DMLTransformSection * nextSection; // by name...
  DMLTransformSection::DMLTransformSection ( char * _n )
    { name = _n; }
};

class DMLTransform
{
 public:
  map<char *, DMLTransformSection*> sections;
  DMLTransformSection * getSection ( char * name ) // A RECODER !!!
    {
      for ( map<char *,DMLTransformSection*>::iterator sec = sections.begin ();
	    sec != sections.end (); sec ++ )
	{
	  if ( strncmp ( sec->first, name , 256 ) == 0 )
	    return sec->second;
	}
      Bug ( "Could not find section '%s'\n", name );
      return NULL;
    }
};








#endif // __DML_DMLTRANSFORM_H
