#include <doolp/doolp-doolpnamingfile.h>
#include <stdio.h>
#include <DMLXParser.h>

Doolp::NamingFile::NamingFile ( char * _fileName )
{
  noflush = false;
  Log ( "New DoolpNamingFile service : _fileName is '%s' (this=%p)\n", _fileName, this );
  fileName = (char*) malloc ( strlen(_fileName) + 1 );
  strcpy ( fileName, _fileName );
  serviceName = "DoolpNaming";
  Log ( "Now filename is at %p\n", fileName );
  Log ( "Now filename content is %s\n", fileName );
  readfile ( );
}

Doolp::Object * Doolp::NamingFile::getServiceAsObject()
{
  Log ( "Getting Service as DoolpObject\n" );
  return dynamic_cast<Object*> (this);
}


bool Doolp::NamingFile::readfile ()
{
  noflush = true;
  Log ( "Reading file '%s'\n", fileName );
  try
    {
      DMLX::ParserFile parser ( fileName );

      parser.checkEventName ( "DoolpNaming" );
      parser.logEvents();
      parser.popEvent();

      while ( ! parser.isEventEnd() )
	{
	  if ( parser.isEventName ( "Context" ) )
	    {
	      string _contextName = parser.getAttr("name");
	      FullContextId fcId;
	      fcId.agentId = parser.getAttrInt("agentId");
	      fcId.contextId = parser.getAttrInt("contextId");
	      fcId.stepping = parser.getAttrInt("stepping");
	      doolpfunclocal(setNamedContext) ( _contextName, &fcId );
	      parser.popEventAndEnd ();
	      continue;
	    }
	  if ( parser.isEventName ( "Object" ) )
	    {
	      string _ctxtName = parser.getAttr ( "contextName" );
	      string _objName = parser.getAttr("name");
	      ObjectId objId = parser.getAttrInt("objectId");
	      doolpfunclocal(setNamedObject) ( _ctxtName, _objName, objId );
	      parser.popEventAndEnd ();
	      continue;
	    }
	  Fatal ( "Invalid event name : '%s'\n", parser.getEvent()->name->getKeyword() );
	}
      Log ( "End of file reading.\n" );
    }
  catch ( DMLX::ParserException * e )
    {
      Warn ( "DMX::ParserException : '%s'\n", e->getMessage() );
    }
  noflush = false;
  return true;
}


bool Doolp::NamingFile::flush()
{
  if ( noflush )
    return true;
  FILE * fp = fopen ( fileName, "w" );
  AssertFatal ( fp != NULL, " could not open naming file '%s'\n", fileName );
  strongmap<string,FullContextId*>::iterator context;
  fprintf ( fp, "<DoolpNaming type=\"XMLFile\" version=\"0\">\n" );
  for ( context = contexts.begin (); context != contexts.end () ; context ++)
    {
      fprintf ( fp, "<Context name=\"%s\" agentId=\"%d\" contextId=\"%d\" stepping=\"%d\"/>\n", 
		context->first.c_str(), logDoolpFullContextId ( context->second ) );
    }
  strongdoublemap<string,string,ObjectId>::iterator object;
  for ( object = objects.begin (); object != objects.end () ; object++ )
    {
      fprintf ( fp, "<Object name=\"%s\" contextName=\"%s\" objectId=\"%d\"/>\n", object.first().c_str(), object.second().c_str(), object.third() );
    }
  fprintf ( fp, "</DoolpNaming>\n" );
  fclose ( fp );
  return true;
}
