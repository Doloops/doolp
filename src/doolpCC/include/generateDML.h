#ifndef __DOOLPPREPROC_GENERATEDML_H
#define __DOOLPPREPROC_GENERATEDML_H

#include <DMLXDocument.h>
#include <parseCtags.h>

#include <list>
#include <vector>

typedef struct classHierarchy
{
	char * className;
	list<classHierarchy*> * inherits;
	classHierarchy * baseClass;
};

vector<classHierarchy*> * 
	generateDML_buildClassHierarchy ( list<ctagsLine> * parsedCtags, list<char *> * baseClasses );

// Build up XML from the Headers files
DMLX::Node * generateDML ( list<ctagsLine> * parsedCtags, vector<classHierarchy*> * hierarchy );

// Refine the XML data with CPP files.
bool checkCode ( DMLSection * metaSection, list<ctagsLine> * parsedCtags );

bool generateDML_Ids ( DMLSection * metaSection );


char * ctagsGetAttr ( ctagsLine * line, char * name, bool notFoundIsFatal );
char * ctagsGetAttr ( ctagsLine * line, char * name );
char * ctagsGetAttr_nonFatal ( ctagsLine * line, char * name );

#endif // __DOOLPPREPROC_GENERATEDML_H
