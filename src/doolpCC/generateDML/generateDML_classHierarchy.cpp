#include <generateDML.h>

extern bool Mode_stdRPC;

//classHierarchy nullHierarchy = { "(none)", NULL, NULL };

classHierarchy * _findBaseClass ( classHierarchy * _class )
{
  classHierarchy * _baseClass;
  if ( _class->baseClass != NULL ) return _class->baseClass;
  for ( list<classHierarchy*>::iterator hier = _class->inherits->begin () ;
	hier != _class->inherits->end () ; hier++ )
    {
      if ( (*hier)->baseClass != NULL )
	_baseClass = (*hier)->baseClass;
      else
	_baseClass = _findBaseClass ( *hier );
      if ( _baseClass == NULL ) Bug ( "Shall not retrieve a NULL baseClass here\n" );
      if ( _class->baseClass != NULL && ( _class->baseClass != _baseClass ) )
	Fatal ( "Class '%s' has duplicate base Classes : '%s' and '%s'\n",
		_class->className, _class->baseClass->className, _baseClass->className );
      _class->baseClass = _baseClass;
    }
  if ( _class->baseClass == NULL )
    {
      Warn ( "Could not get baseClass for class '%s', which inherits :\n", _class->className );
      if ( _class->inherits->size () == 0 )
	Warn ( "\t(no inheritance)\n" );
      for ( list<classHierarchy*>::iterator it = _class->inherits->begin ();
	    it != _class->inherits->end () ; it++ )
	{
	  Error ( "\t%s\n", (*it)->className );
	}
      Warn ( "Nothing will be generated for this class.\n" );
      //      _class->baseClass = &nullHierarchy;
      //      return &nullHierarchy;
      return NULL;
    }
  return _class->baseClass;
}

#define __Classes__MAX 256
vector<classHierarchy*> * 
generateDML_buildClassHierarchy ( list<ctagsLine> * parsedCtags, list<char *> * baseClasses )
{
#define __addClass__(__className__)					\
  { 	classHierarchy* cl = (classHierarchy*) malloc ( sizeof ( classHierarchy ) ); \
    cl->className = __className__;					\
    cl->inherits = new list<classHierarchy*>;				\
    cl->baseClass = NULL;						\
    classes->push_back(cl); classesNb++; }

  // FyI : classesBaseIndex is the number of baseClasses.
#define __addLink__(_classIdx, _fatherName)				\
  /*Log ( "_classIdx=%d, _fatherName='%s'\n", _classIdx, _fatherName );	*/ \
    for ( int _i = 0 ; _i < classesNb ; _i++)				\
      if ( strcmp ( classes->at(_i)->className, _fatherName ) == 0 )	\
	{ if ( _i < classesBaseIndex )					\
	    { if ( classes->at(_classIdx)->baseClass != NULL )		\
		Fatal ( "Duplicate base class for class '%s'\n", classes->at(_classIdx)->className ); \
	      classes->at(_classIdx)->baseClass = classes->at(_i); }	\
	  /*Log ( "Class '%d' inherits '%d'\n", _classIdx, _i );*/	\
	  classes->at(_classIdx)->inherits->push_back( classes->at(_i) ); }

  vector<classHierarchy*> * classes = new vector<classHierarchy*>;

  int classesNb = 0;
  char buffer[512];
  // classHierarchy * classes[__Classes__MAX];
  int classesLinks[__Classes__MAX][__Classes__MAX];
  memset ( classes, 0, sizeof ( char* ) * __Classes__MAX );
  memset ( classesLinks, 0, sizeof (int) * __Classes__MAX * __Classes__MAX );
	
  // Step 1, add all classes in the classes vector.
  Log ( "Class Hierarchy | Step 1 : Building the classes vector\n" );
  list<char *>::iterator baseClass;
  list<ctagsLine>::iterator line;
  for ( baseClass = baseClasses->begin () ; baseClass != baseClasses->end () ; baseClass++)
    __addClass__ (*baseClass)
      int classesBaseIndex = classesNb; 
  for ( line = parsedCtags->begin () ; line != parsedCtags->end () ; line ++ )
    if ( strcmp ( ctagsGetAttr ( &(*line), "kind" ), "class" ) == 0 ) 
      { 
	if ( strstr ( line->item, "::" ) != NULL ) continue;
	char * classname = line->item;
	if ( ctagsGetAttr_nonFatal ( &(*line), "namespace" ) )
	  {
	    classname = new char[128];
	    strcpy ( classname, ctagsGetAttr ( &(*line), "namespace" ) );
	    strcat ( classname, "::" );
	    strcat ( classname, line->item );
	  }
	Log ( "\tNew Class '%s'\n", classname );
	__addClass__ ( classname ); 
      }
  // fprintf ( stderr, "classesNb = %d\n", classesNb );
  
  Log ( "Class Hierarchy | Step 2 : Parsing 'inherits' Fields\n" );
  // Step 2, parse inherits
  int currentClass = classesBaseIndex;
  for ( line = parsedCtags->begin () ; line != parsedCtags->end () ; line ++ )
    if ( strcmp ( ctagsGetAttr ( &(*line), "kind" ), "class" ) == 0 ) 
      {
	if ( strstr ( line->item, "::" ) != NULL ) continue;
	char * inherits = ctagsGetAttr_nonFatal ( &(*line), "inherits" );
	if ( inherits == NULL ) 
	  { Log ( "\tClass '%s' inherits nothing.\n", line->item ); continue; }

	char * classname = line->item;
	char * _namespace = ctagsGetAttr_nonFatal ( &(*line), "namespace" );
	if ( _namespace )
	  {
	    classname = new char[128];
	    strcpy ( classname, ctagsGetAttr ( &(*line), "namespace" ) );
	    strcat ( classname, "::" );
	    strcat ( classname, line->item );
	  }
	Log ( "\tClass '%s' inherits '%s'\n", classname, inherits );
	strcpy ( buffer, inherits );	
	char * last = buffer;
	char * cur;
	char father[64];
	while ( ( cur = strstr ( last, "," ) ) != NULL )
	  {
	    *cur = '\0';
	    father[0] = '\0';
	    if ( _namespace )
	      {
		strcpy ( father, _namespace );
		strcat ( father, "::" );
	      }
	    strcat ( father, last );
	    __addLink__ ( currentClass, father );
	    last = cur + 1;
	  }
	father[0] = '\0';
	if ( _namespace )
	  {
	    strcpy ( father, _namespace );
	    strcat ( father, "::" );
	  }
	strcat ( father, last );
	__addLink__ ( currentClass, father );	
	currentClass++;
      }

  Log ( "Class Hierarchy | Step 3 : Resolving Inheritances\n" );
  // Step 3, propagate baseClass names.
  for ( int cl = classesBaseIndex ; cl < classesNb ; cl++ )
    if ( classes->at(cl)->baseClass == NULL )
      {
	_findBaseClass ( classes->at(cl) );
      }	

  // DUMP
#define __generateDML_dump
#ifdef __generateDML_dump
  for ( int i = 0 ; i < classesNb ; i++ )
    {
      Log ( "Class %d:'%s' - %d inheritance(s)\n", 
	       i, classes->at(i)->className, classes->at(i)->inherits->size() );
      for ( list<classHierarchy*>::iterator hier = classes->at(i)->inherits->begin () ;
	    hier != classes->at(i)->inherits->end () ; hier++ )
	Log ( "\tInherits '%s'\n", (*hier)->className );
    }
#endif //  __generateDML_dump
  return classes;
#undef __addClass__
#undef __addLink__
}
