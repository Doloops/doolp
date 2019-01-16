#include <generateDML.h>
#include <crc32.h>
#include <parseCtags.h>
#include <stdio.h>

// TODO : 
// set signature = void when empty.

extern bool Mode_stdRPC;

#undef __DOOLPCC__PARSE__DOOLPABLE_DECLARATION_OLD


char * ctagsGetAttr ( ctagsLine * line, char * name, bool notFoundIsFatal )
{
  list<attrValue>::iterator attr;
  for ( attr = line->aValue->begin() ; attr != line->aValue->end() ; attr ++ )
    {
      if ( strcmp ( name, (*attr).name ) == 0 )
	return (*attr).value;
    }
  if ( ! notFoundIsFatal )
    return NULL;
	
  Warn ( "Not found : %s\n", name );
  Log ( "In CtagsLine : \n" );
  dumpCtagsLine ( stderr, line );
  Fatal ( "Quitting.\n" );
  return NULL;
}

char * ctagsGetAttr ( ctagsLine * line, char * name )
{ 	return ctagsGetAttr ( line, name, true ); }

char * ctagsGetAttr_nonFatal ( ctagsLine * line, char * name )
{	return ctagsGetAttr ( line, name, false ); }


bool parseSignature ( DMLSection * func, char * signature )
{
  Bug ( "DEPRECATED !\n" );
  return false;
}

#define getObjectClass() objectsSection->getNodeWithAttribute ( "name", ctagsGetAttr( &(*line), "class") );


DMLX::Node * generateDML ( list<ctagsLine> * parsedCtags, vector<classHierarchy*> * _hierarchy )
{
  //  DMLSection * metaSection = new DMLSection ( "DoolpObjects" ); // , "generated" );
  DMLX::Node * objectsSection = new DMLX::Node ("DoolpObjects");
  char buffer[256];             
  list<ctagsLine>::iterator line;
  
  // Step 0, Dump
#ifdef __generateDML_dump
  for ( line = parsedCtags->begin() ; line != parsedCtags->end() ; line ++ )
    {
      
      printf ( "--------------\n" );
      printf ( "Item=%s (sourceFile=\"%s\") (code=\"%s\")\n", (*line).item, (*line).sourceFile, (*line).code );
      list<attrValue>::iterator attr;
      for ( attr = (*line).aValue->begin() ; attr != (*line).aValue->end() ; attr ++ )
	printf ( "\t%s = %s\n", (*attr).name, (*attr).value );
    }
  printf ( "****************************************\n" );
#endif // __generateDML_dump
  
  // Step 1, Create Classes
  Log ( "1 - Creating Classes..\n" );
  for ( line = parsedCtags->begin() ; line != parsedCtags->end() ; line ++ )
    {
      if ( strcmp ( ctagsGetAttr ( &(*line), "kind" ), "class" ) == 0 )
	{
	  if ( strstr ( line->item, "::" ) ) continue;
	  
	  classHierarchy* hierarchy = NULL;
	  char * classname = new char[128];
	  classname[0] = '\0';
	  char * _namespace = ctagsGetAttr_nonFatal ( &(*line), "namespace" );
	  if ( _namespace )
	    {
	      strcpy ( classname, ctagsGetAttr ( &(*line), "namespace" ) );
	      strcat ( classname, "::" );
	    }
	  strcat ( classname, line->item );
	  Log ( "\tClass : '%s' (from file '%s')\n", 
		classname, (*line).sourceFile );

	  for ( unsigned int hie = 0 ; hie < _hierarchy->size () ; hie ++ )
	    if ( strcmp ( _hierarchy->at(hie)->className, classname ) == 0 )
	      hierarchy = _hierarchy->at(hie);
	  if ( hierarchy == NULL ) Bug ( "Could not find hierachy for class '%s'\n", classname );
	  if ( hierarchy->baseClass == NULL ) 
	    { 
	      Warn ( "No baseClass for class '%s'\n", classname ); 
	      free ( classname ); 
	      continue; 
	    }
	  Log ( "\t\tClass has baseClass '%s'\n", hierarchy->baseClass->className );

	  DMLX::Node * newClass = new DMLX::Node ( objectsSection, strstr (hierarchy->baseClass->className,"::") +2 );
	  
	  newClass->addAttribute ( "name", classname ); // (*line).item );
	  if ( strstr ( classname, "::" ) )
	    {
	      char * last = classname;
	      while ( strstr ( last, "::" ) != NULL )
		last = strstr ( last, "::" ) + 2;
	      newClass->addAttribute ( "shortName", last );
	    }
	  else
	    {
	      newClass->addAttribute ( "shortName", classname );
	    }
	  for ( list<classHierarchy*>::iterator inherits = hierarchy->inherits->begin () ;
		inherits != hierarchy->inherits->end () ; inherits ++ )
	    {
	      if ( strcmp ( (*inherits)->className, "Doolp::Object" ) == 0 )
		continue;
	      DMLX::Node * secInherits = new DMLX::Node ( newClass, "Inherits" );
	      secInherits->addAttribute ( "class", (*inherits)->className );
	    }
	  (new DMLX::Node ( newClass, "Include" ))->addAttribute ( "header", (*line).sourceFile  );

	  char * nameId = (char*) malloc ( 12 );
	  sprintf ( nameId, "%p", (void*) crc32_get ( classname ) );
	  newClass->addAttribute ( "nameId", nameId );
	}
      
    }	
  
  // Step 2, Add Classes Options
  Log ( "2 - Configuring Classes..\n" );
  for ( line = parsedCtags->begin() ; line != parsedCtags->end() ; line ++ )
    {
      if ( strcmp ( ctagsGetAttr ( &(*line), "kind" ), "prototype" ) == 0 )
	{
	  char * itemName = (*line).item;
	  bool multipleOptions = false;
	  if ( strcmp ( itemName, "DoolpObject_Options" ) == 0 )
	    {
	      multipleOptions = true;
	    }
	  else if ( strcmp ( itemName, "DoolpObject_Option" ) == 0 )
	    {
	      multipleOptions = false;
	    }
	  else
	    {
	      continue;
	    }
	  /*
	  char * options_start = strstr ( (*line).code,"DoolpObject_Option" );
	  if ( options_start == NULL )
	    {
	      Fatal ( "Invalid code for DoolpObject_Options : '%s'\n",
		      (*line).code );
	    }
	  */
	  char * options_start = line->code;
	  options_start = strchr ( options_start, '(' ) + 1;
	  while ( options_start[0] == ' ' )
	    {
	      options_start++;
	    }
	  if ( options_start[0] == '\0' )
	    {
	      Fatal ( "Invalid options..\n" );
	    }
	  char options[128];
	  strcpy ( options, options_start );
	  Log ( "options='%s'\n", options );
	  char *optionsValue = strchr ( options, ',' );
	  optionsValue[0] = '\0';
	  optionsValue++;
	  while ( optionsValue[0] == ' ' )
	    optionsValue++;
	  Log ( "optionsValue = '%s'\n", optionsValue );
	  int ln = strlen ( optionsValue );
	  for ( int i = ln ; i > 0 ; i-- )
	    {
	      if ( optionsValue[i] == ')' )
		{ 
		  optionsValue[i] = '\0'; 
		  break; 
		}
	    }
	  Log ( "optionsValue = '%s'\n", optionsValue );
	  char * optName = (char*) malloc (strlen ( options ) + 1);
	  strcpy ( optName, options );
	  char * optValue = (char*) malloc (strlen ( optionsValue ) + 1);
	  strcpy ( optValue, optionsValue );

	  Log ( "\tClass : '%s' : Option Name '%s', Value '%s'\n", ctagsGetAttr( &(*line), "class"),
		optName, optValue );
	  //	  DMLSection * obj = metaSection->getSectionWithParam ( "name", ctagsGetAttr( &(*line), "class") );
	  DMLX::Node * obj = getObjectClass();
	  if ( obj == NULL ) Fatal ( "Could not get object : %s\n", ctagsGetAttr ( &(*line), "class") ); 
	  if ( multipleOptions )
	    {
	      int nbValues = 0;
	      char * optValue2 = NULL;
	      char * optLabel = NULL;
	      DMLX::Node * opt = obj->addNode ( optName );
	      while ( ( optValue2 = strchr ( optValue, ',' ) ) != NULL )
		{
		  optValue2[0] = '\0';
		  optLabel = (char*) malloc ( 8 );
		  sprintf ( optLabel, "value%d", nbValues );
		  opt->addAttribute ( optLabel, optValue );
		  nbValues ++;
		  optValue = optValue2 + 1;
		}
	      if  ( nbValues == 0 )
		{
		  opt->addAttribute ( "value", optValue );
		}
	      else
		{
		  optLabel = (char*) malloc ( 8 );
		  sprintf ( optLabel, "value%d", nbValues );
		  opt->addAttribute ( optLabel, optValue );
		}
	    }
	  else
	    {
	      obj->addAttribute ( optName, optValue );
	    }
	 
	}
    }

  // Step 3, Add member parameters
  Log ( "3 - Creating Members and links...\n" );
  for ( line = parsedCtags->begin() ; line != parsedCtags->end() ; line ++ )
    {
      if ( strcmp ( ctagsGetAttr ( &(*line), "kind" ), "member" ) == 0 )
	{
	  char * memberNodeName = NULL;
	  char * typeName = NULL;
	  char * linkType = NULL;
	  if ( strstr ( (*line).item, "::") != NULL ) continue;
	  if ( strstr ( (*line).code, "doolpuniquelink" ) != NULL )
	    {
	      Log ( "\tUniqueLink : %s\n", (*line).item );
	      memberNodeName = "Link";
	      typeName = "doolpuniquelink";
	      linkType = "ObjectUniqueLink";
	    }
	  else if ( strstr ( (*line).code, "doolpmultilink" ) != NULL )
	    {
	      Log ( "\tMultiLink : %s\n", (*line).item );
	      memberNodeName = "Link";
	      typeName = "doolpmultilink";
	      linkType = "ObjectMultiLink";
	    }
	  else if ( strstr ( (*line).code, "doolpparam" ) != NULL ) 
	    {
	      Log ( "\tMember : %s\n", (*line).item );
	      memberNodeName = "Parameter";
	      typeName = "doolpparam";
	    }
	  else
	    {
	      continue;
	    }
	  DMLX::Node * obj = getObjectClass();
	  if ( obj == NULL ) Fatal ( "Could not get object : %s\n", ctagsGetAttr ( &(*line), "class") ); 
	  DMLX::Node * member = new DMLX::Node ( obj, memberNodeName );
	  member->addAttribute ( "name", (*line).item );	
	  // Here we must decode the type of param.
	  char * buff, * buffItem;
	  buff = (*line).code + 3;
	  
	  buff = strstr ( buff, typeName );
	  AssertBug ( buff != NULL, "Invalid buff spec.\n" );
	  buff += strlen ( typeName ) + 1;
	  strcpy ( buffer, buff ); // Copy this for modification.
	  buffItem = strstr ( buffer, (*line).item );
	  if ( buffItem == NULL ) Fatal ( "Unexpected end of line or error syntax in param %s\n", (*line).item );
	  buffItem[0] = '\0';
	  
	  char * type = (char *) malloc ( strlen ( buffer ) + 2 + 1 );
	  strcpy ( type, buffer ); 
	  
	  while ( type[0] == ' ' || type[0] == '<' )
	    type++;
	  int i = strlen(type) - 1;
	  while ( type[i] == ' ' || type[i] == '>' )
	    { type[i] = '\0'; i--; }
	  
	  Log ( "\t\tMember is of type '%s'\n", type );
	  
	  member->addAttribute ( "type", type );
	  char * index = (char *) malloc ( 12 );
	  if ( crc32_get ( (*line).item ) == 0 )
	    Bug ( "Arg !!! crc32_get ( %s ) == 0 !!!\n",
		  (*line).item );
	  sprintf ( index, "%p", (void*) crc32_get ( (*line).item ) );
	  
	  member->addAttribute ( "index", index );
	  
	  char * ln = ctagsGetAttr ( &(*line), "line" );
	  Log ( "\t\tMember is at line '%s'\n", ln );
	  
	  member->addAttribute ( "line", ln );

	  if ( linkType != NULL )
	    {
	      member->addAttribute ( "linkType", linkType );
	    }
	}
    }

  Log ( "3b - Configuring Link Reverse..\n" );
  for ( line = parsedCtags->begin() ; line != parsedCtags->end() ; line ++ )
    {
      if ( strcmp ( ctagsGetAttr ( &(*line), "kind" ), "prototype" ) == 0 )
	{
	  char * itemName = (*line).item;
	  if ( strcmp ( itemName, "DoolpObjectLink_setReverse" ) == 0 )
	    {
	      char * _opt = strchr ( line->code, '(' ) + 1;
	      char * opt = (char*) malloc ( strlen ( _opt ) + 1 );
	      strcpy ( opt, _opt );
	      strchr(opt, ')' )[0] = '\0';
	      while ( opt[0] == ' ' ) opt++;
	      char * opt2 = strchr(opt, ',' );
	      opt2[0] = '\0'; opt2++;
	      for ( char * o = opt2 - 1 ; *o == ' ' ; o-- )
		*o = '\0';
	      while ( opt2[0] == ' ' ) opt2++;
	      for ( char * o = strchr ( opt2, '\0' ) - 1 ; *o == ' ' ; o-- )
		*o = '\0';
	      Log ( "Reverse : obj '%s', link '%s', reverse '%s'\n",
		    ctagsGetAttr( &(*line), "class"),
		    opt, opt2 );
	      DMLX::Node * obj = getObjectClass();
	      Log ( "obj=%p\n", obj );
	      DMLX::Node * link = obj->getNodeWithAttribute ( "name", opt );
	      AssertFatal ( link != NULL, "Could not get link '%s'\n", opt );
	      link->addAttribute ( "reverse", opt2 );
	    }
	}
    }
  // Step 4, Add object slots
  Log ( "4 - Creating DoolpSlots..\n" );
  for ( line = parsedCtags->begin() ; line != parsedCtags->end() ; line ++ )
    {
      if ( strcmp ( ctagsGetAttr ( &(*line), "kind" ), "member" ) == 0 )
	{
	  if ( strstr ( (*line).item, "::") != NULL ) continue;
	  if ( strstr ( (*line).code, "doolpfunc" ) == NULL ) continue;
	  Log ( "\tDoolpFunc Slot : %s\n", (*line).item );
	  
	  char * buff;
	  buff = (*line).code + 3;
	  
	  buff = strstr ( buff, "doolpfunc" );
	  buff += 8 + 1;
	  char signature[128];
	  strcpy ( signature, strchr( buff, '<' ) + 1 );
	  int nbLevs = 1;
	  char * c = signature;
	  while ( *c != '\0' )
	    {
	      switch ( *c )
		{ case '<' : nbLevs++; break;
		case '>' : nbLevs--; break;
		}
	      if ( nbLevs == 0 )
		{
		  *c = '\0';
		  break;
		}
	      c++;
	    }
	  c--;
	  while ( *c == ' ' && c > signature )
	    { *c = '\0' ; c-- ; }
	  Log ( "\t\tRaw signature : '%s'\n", signature );
	  
	  //	  DMLSection * obj = metaSection->getSectionWithParam ( "name", ctagsGetAttr( &(*line), "class") );
	  DMLX::Node * obj = getObjectClass();
	  if ( obj == NULL ) Fatal ( "Could not get object : %s\n", ctagsGetAttr ( &(*line), "class") ); 

	  //	  DMLSection * newSlot = obj->addSection ( "Slot" );
	  //	  newSlot->addParam ( "name", (*line).item );
	  DMLX::Node * newSlot = new DMLX::Node ( obj, "Slot" );
	  newSlot->addAttribute ( "name", (*line).item );

	  //	  char * signature_copy = (char*) malloc ( strlen ( signature ) + 1 );
	  //	  strcpy ( signature_copy, signature );
	  //	  newSlot->addParam ( "signature", signature_copy );
	  newSlot->addAttribute ( "signature", signature );
	  
	  char * index_str = (char *) malloc ( 12 );
	  unsigned int index = crc32_get ( signature );
	  index += crc32_get ( (*line).item );
	  if ( index == 0 )
	    Bug ( "Arg !! crc32_get ( \"%s\" ) == 0 !\n",
		  signature );
	  sprintf ( index_str, "0x%x", index );
	  //	  newSlot->addParam ( "index", index_str );
	  newSlot->addAttribute ( "index", index_str );

	  Log ( "\t\tSlot has rpcId '%s'\n", index_str );
	  char * ln = ctagsGetAttr ( &(*line), "line" );
	  Log ( "\t\tSlot is at line '%s'\n", ln );
	  newSlot->addAttribute ( "line", ln ); // newSlot->addParam ( "line", ln );
	}
      
    }

  // Step 5, add Slot helpers
  Log ( "5 - Creating DoolpSlot Helpers..\n" );
  for ( line = parsedCtags->begin() ; line != parsedCtags->end() ; line ++ )
    {
      if ( strcmp ( ctagsGetAttr ( &(*line), "kind" ), "prototype" ) == 0
	   || strcmp ( ctagsGetAttr ( &(*line), "kind" ), "function" ) == 0 )
	{
	  if ( strstr ( line->item, "::" ) != NULL )
	    continue;
	  if ( strstr ( line->code, "doolpfunclocal" ) != NULL )
	    {
	      Log ( "\tSlot '%s' has a doolpfunclocal helper\n", line->item );
	      DMLX::Node * obj = getObjectClass();
	      if ( obj == NULL ) Fatal ( "Could not get object : %s\n", ctagsGetAttr ( &(*line), "class") ); 

	      //	      DMLSection * obj = metaSection->getSectionWithParam ( "name",
	      //								    ctagsGetAttr( &(*line), "class") );
	      //	      DMLSection * helper = obj->addSection ( "SlotHelper" );
	      DMLX::Node * helper = new DMLX::Node ( obj, "SlotHelper" );
	      helper->addAttribute ( "name", line->item );
	      helper->addAttribute ( "type", "local" );
	      //	      helper->addParam ( "name", line->item );
	      //	      helper->addParam ( "type", "local" );
	      continue;
	    }
	  if ( strstr ( line->code, "doolpfunchelper" ) != NULL )
	    {
	      char helpertype[128],*slotname;
	      Log ( "\tCode : '%s'\n", strstr(line->code,"doolpfunchelper") + 15 );
	      sscanf ( strstr(line->code,"doolpfunchelper") + 15, "(%s)", helpertype );
	      strchr(helpertype,')')[0] = '\0';
	      slotname = strchr(helpertype, ',');
	      slotname[0] = '\0';
	      slotname++;

	      Log ( "\tSlot '%s' has a helper function '%s'\n", slotname, helpertype );
	      char *__helpertype = (char*) malloc ( strlen ( helpertype ) + 1);
	      strcpy (__helpertype, helpertype );
	      char *__slotname = (char*) malloc ( strlen ( slotname ) + 1);
	      strcpy (__slotname, slotname );


	      DMLX::Node * obj = getObjectClass();
	      if ( obj == NULL ) Fatal ( "Could not get object : %s\n", ctagsGetAttr ( &(*line), "class") ); 

	      //	      DMLSection * obj = metaSection->getSectionWithParam ( "name",
	      //							    ctagsGetAttr( &(*line), "class") );
	      //	      DMLSection * helper = obj->addSection ( "SlotHelper" );

	      DMLX::Node * helper = new DMLX::Node ( obj, "SlotHelper" );
	      helper->addAttribute ( "name", __slotname );
	      helper->addAttribute ( "type", __helpertype );

	      //	      helper->addParam ( "name", __slotname );
	      //	      helper->addParam ( "type", __helpertype );
	      continue;
	    }
	}
    }

  // Step 6, extra exceptions
  Log ( "6 - Creating Extra Exceptions..\n" );
  for ( line = parsedCtags->begin() ; line != parsedCtags->end() ; line ++ )
    {
      if ( ( strcmp ( ctagsGetAttr ( &(*line), "kind" ), "prototype" ) == 0 )
	   && ( strcmp ( line->item, "__Doolp_DoolpException" ) == 0 ) )
	{
	  char * nm = (char*) malloc ( 64 );
	  strncpy ( nm, strchr(line->code,'(') + 1, 64 );
	  strchr ( nm, ',') [0] = '\0';
	  Log ( "\tNew Exception '%s'\n", nm );
	  //	  DMLSection * newClass = metaSection->addSection ( new DMLSection ( "Exception" ) );
	  DMLX::Node * exception = new DMLX::Node ( objectsSection, "Exception" );
	  char * idx = (char*) malloc (16);
	  sprintf ( idx, "0x%x", crc32_get ( nm ) );
	  exception->addAttribute ( "name", nm );
	  exception->addAttribute ( "index", idx );

	  char * nameId = (char*) malloc ( 12 );
	  sprintf ( nameId, "%p", (void*) crc32_get ( nm ) );
	  exception->addAttribute ( "nameId", nameId );

	  //	  newClass->addParam ( "name", nm );
	  //	  newClass->addParam ( "index", idx );
	  char * sourceFile = (char *) malloc ( strlen ( (*line).sourceFile ) + 2 + 1 );
	  sprintf ( sourceFile, "%s", (*line).sourceFile );
	  exception->addNode ( "Include" )->addAttribute ( "header", sourceFile );
	  //	  newClass->addSection ( "Include" )->addParam ( "header", sourceFile );
	}
    }
  Log ( "DML generation done\n" );
  return objectsSection;
  //  return metaSection;
}

#if 0
bool generateDML_Ids ( DMLSection * metaSection )
{
  list<DMLSection *>::iterator object;
  char buffer[512];
  for ( object = metaSection->getSections()->begin() ; object != metaSection->getSections()->end() ; object++ )
    {
      // object name numbering
      char * nameId = (char*) malloc ( 12 );
      sprintf ( nameId, "%p", (void*) crc32_get ( (*object)->getParam("name") ) );
      (*object)->addParam ( "nameId", nameId );
      // functions numbering
      list<DMLSection *>::iterator func;
      for ( func = (*object)->getSections()->begin () ; func != (*object)->getSections()->end () ; func++ )
	{
	  if ( strcmp ( (*func)->getName(), "Function" ) != 0 ) continue;
	  char * functionId = (char*) malloc ( 12 );
	  char * functionType = (char*) malloc ( 6 );
	  sprintf ( buffer, "%s__%s__%s",
		    (*object)->getParam("name"),
		    (*func)->getParam("name"),
		    (*func)->getParam ( "parameters" ) );
	  sprintf ( functionId, "%p", (void*) crc32_get ( buffer ) );
	  memcpy ( functionType, functionId + 2, 4 );
	  functionType[4] = '\0';
	  (*func)->addParam ( "functionId", functionId );
	  (*func)->addParam ( "type", functionType );
	  char * functionRoot = (char *) malloc ( 128 );
	  sprintf ( functionRoot, "%s__%s__%s",
		    (*object)->getParam("name"), (*func)->getParam("name"), functionType );
	  (*func)->addParam ( "functionRoot", functionRoot );
	}
    }

  return true;
}
#endif

#if 0
bool markIsImplemented ( DMLSection * metaSection, char * className, char * funcName, char * type )
{
  Bug ( "DEPRECATED !!\n" );
  list<DMLSection *>::iterator clss;
  for ( clss = metaSection->getSections()->begin () ; clss != metaSection->getSections()->end () ; clss++ )
    if ( strcmp ( (*clss)->getParam("name"), className ) == 0 )
      {
	list<DMLSection *>::iterator func;
	for ( func = (*clss)->getSections()->begin () ; func != (*clss)->getSections()->end () ; func ++ )
	  if ( ( strcmp ( (*func)->getName(), "Function" ) == 0 )
	       && ( strcmp ( (*func)->getParam("name"), funcName ) == 0 ) )
	    {		
	      // (*func)->addParam ( "isImplementedLocally", "true" );
	      (*func)->setParam ( "isImplementedLocally", "true"); // ->setValue ( "true" );
	      (*func)->setParam ( "localName", (*func)->getParam("name") );
	      (*func)->setParam ( "mustGenerateStrap", "false"); // ->setValue ( "false" );
	      return true;
	    }
      }
  Log ( "Error, could not find function.\n" );
  return false;
}

bool checkCode ( DMLSection * metaSection, list<ctagsLine> * parsedCtags )
{
  char buff[256];
  char * className, * funcName;
  list<ctagsLine>::iterator line;
  for ( line = parsedCtags->begin () ; line != parsedCtags->end () ; line ++ )
    {
      strcpy ( buff, (*line).item );
      if ( ( funcName = strstr ( buff, "::" ) ) == NULL ) continue;
      *(funcName) = '\0';
      funcName += 2;
      className = buff;
      Log ( "Class '%s' : Func '%s' is implemented.\n", className, funcName );
      markIsImplemented ( metaSection, className, funcName, NULL );
    }
  return true;
}

#endif
