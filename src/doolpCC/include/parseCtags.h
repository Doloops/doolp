#ifndef __DOOLPPREPROC_PARSECTAGS_H
#define __DOOLPPREPROC_PARSECTAGS_H

#include <DML.h>
#include <glog.h>

#include <list>

typedef struct attrValue
{
	char * name;
	char * value;
};

typedef struct ctagsLine
{
	char * item;
	char * sourceFile;
	char * code;
	list<attrValue> * aValue;
};

list<ctagsLine> * parseCtags ( FILE * input );
// list<ctagsLine> * parseCtags ( char ** files, int filesnb );
list<ctagsLine> * parseCtags ( list<char *> *files );

bool dumpCtagsLine ( FILE * out, ctagsLine * line );
bool dumpCtags ( FILE * out, list<ctagsLine> * lines );

#endif // __DOOLPPREPROC_PARSECTAGS_H
