#ifdef __DML_DMLWRITER_CPP__
#include "DML.h"

#define _printtab for ( int __i = 0 ; __i < tab ; __i++) fprintf ( fp, "\t" )

bool DMLSection::write ( int tab, FILE * fp )
{
	_printtab;
	fprintf ( fp, "%s %s\n", type, name );
	_printtab;
	fprintf ( fp, "{\n" );
	list<DMLParam*>::iterator param;
	for ( param = params.begin ()  ; param != params.end () ; param ++ )
		(*param)->write ( tab + 1, fp );
	list<DMLSection*>::iterator section;
	for ( section = sections.begin ()  ; section != sections.end () ; section ++ )
		(*section)->write ( tab + 1, fp );
	_printtab;
	fprintf ( fp, "}\n" );

	return true;
}


bool DMLParam::write ( int tab, FILE * fp )
{
	_printtab;
	if ( type != NULL ) fprintf ( fp, "%s ", type );
	if ( name == NULL ) Bug ( "Name has not been set for this parameter.\n" );
	fprintf ( fp, name );
	if ( soleValue != NULL ) fprintf ( fp, " = %s", soleValue );
	fprintf ( fp, ";\n");
	return true;
}
#endif 
