#include <doolp/doolp-doolpconnection-xml.h>

// TODO : forward from XML to any kind of DoolpConnection

bool Doolp::ConnectionXML::Write ( DMLX::Event* event )
{
  AssertBug ( event != NULL, "Can not write a null event !\n" );
  //  char buff[128];
  if ( event->isEnd() )
    {
      writer->writeMarkupEnd ();
      return true;
    }
  if ( event->isText() )
    {
      writer->writeText ( event->text, true );
      return true;
    }
  writer->writeMarkup ( *(event->name) );
  for ( DMLX::Attr * attr = event->first;
	attr != NULL ; attr = attr->next )
    {
      //      xmlizeString ( attr->value, buff, 128 );
      __DOOLPXML_FWD_Log ( "\tAttr '%s'='%s'\n",
			   attr->name->getKeyword(), attr->value );
      writer->writeAttr ( *(attr->name), attr->value, true );
    }
  return true;
}

bool Doolp::ConnectionXML::forwardMessage ( Connection * __conn )
{
  ConnectionXML * conn = (ConnectionXML *) __conn;
  // Be carefull, we can start forward after the beginning of the message.
  AssertBug ( parser != NULL, "No parser defined.\n" );
  __DOOLPXML_FWD_Log ( "******** fwd : %d Events in Pipe *********\n", parser->getEventsNumber() );
  if ( conn == NULL )
    __DOOLPXML_FWD_Log ( "Forward to NULL (trash)\n" );
  AssertBug ( parser->getEventsNumber () > 0, "Empty pipe, nothing to forward !\n" );
  if ( conn != NULL )
    {
      if ( ! conn->isLockWrite () )
	conn->lockWrite ( );
    }
  while ( true ) // parser->getEventsNumber () > 0 )
    {
      DMLX::Event * event = parser->getEvent ();
      __DOOLPXML_FWD_Log ( "now eventSize = %d, event %s (end %d)\n", 
			   parser->getEventsNumber(), event->name->getKeyword(),
			   event->isEnd() );
      
      if ( conn != NULL )
	conn->Write ( event );

      if ( event->isEnd() && ( strcmp ( event->name->getKeyword(), "DoolpMessage" ) == 0 ) )
	{
	  if ( conn != NULL )
	    conn->unlockWrite ();
	  parser->popEvent ();
	  __DOOLPXML_FWD_Log ( "Forward of message is finished !\n" );
	  return true;
	}
      parser->popEvent ();
    }
  Bug ( "Shall not be here !\n" );
  return false;
};

