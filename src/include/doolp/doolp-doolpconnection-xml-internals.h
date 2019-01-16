#ifndef __DOOLP_DOOLPCONNECTIONXML_H
#error Should be in doolp-doolpconnection-xml.h !!!
#endif

#warning DEPRECATED !
/*
 * This defines some internal inline functions
 * Maybe we could push this stuff back to main header.
 */

/*
 * Sections Management (for Write )
 */

#if 0

inline bool DoolpConnectionXML::pushSection ( char * c ) 
{ 
  AssertFatal ( sectionsLevel < sectionsMax,
		"Sections went out of bounds ! Tried to push '%s'(at %p)\n", c, c );
  sections[sectionsLevel++] = c;
  return true; 
}

#define _DCXML_write(...)						\
  {									\
    if ( target != NULL )						\
      {									\
	for ( unsigned int __i = 0 ; __i < sectionsLevel ; __i++ )	\
	  fprintf ( target, "  " );					\
	fprintf ( target, __VA_ARGS__ );				\
	fflush ( target );						\
      }									\
    /*    writeBuffSz += sprintf ( writeBuff + writeBuffSz,		\
	  __VA_ARGS__ );						\
	  __DOOLPXML_Log ( "Writing : writeBuffSz=%d\n", writeBuffSz ); */ \
    __DOOLPXML_Log ( __VA_ARGS__ ); 					\
    Warn ( "ReImplement _DCXML_write !!\n" );				\
  }
#endif
