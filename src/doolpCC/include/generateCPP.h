#ifndef __DOOLPCC_GENERATECPP_H
#define __DOOLPCC_GENERATECPP_H


/*
  void generateObjects ( char * inputFile, char * outputFile );
  void generateDoolpInfo ( char * inputFile, char * outputFile );
*/
#define writeLine(...) { fprintf( fp_out, __VA_ARGS__ ); fflush ( fp_out ); }
#define writeComment(...) { fprintf ( fp_out, "/*\n" ); writeLine ( __VA_ARGS__ ); fprintf ( fp_out,"*/\n" ); fflush (fp_out); }

#define addInclude(__what__) writeLine ( "#include \"%s\"\n\n", __what__ )
#define addStdInclude(__what__) writeLine ( "#include <%s>\n\n", __what__ )


#define forIterSections(__iter__,__section__) \
  for ( DMLSectionIter __iter__  = (__section__)->sections.begin () ;	\
	__iter__ != (__section__)->sections.end   () ;			\
	__iter__ ++ )

#define forIterSectionsOfType(__iter__,__section__,__type__)  \
  forIterSections(__iter__,__section__) {			      \
    if ( ! (*(__iter__))->isOfType( __type__ ) ) continue; 
    
#define endFor }

bool __wrStrippedParams ( FILE * fp_out, char * fmt, DMLSection * _func );
#define _wrStrippedParams(...) __wrStrippedParams ( fp_out, __VA_ARGS__)

bool __wrStreamConstructions ( FILE * fp_out, char * fmt, DMLSection * _func );
#define _wrStreamConstructions(...) __wrStreamConstructions ( fp_out, __VA_ARGS__)


bool generateObjects ( DMLSection * metaSection, FILE * fp_out );

bool generateDoolpInfo ( DMLSection * metaSection, FILE * fp_out );

#endif // __DOOLPCC_GENERATECPP_H
