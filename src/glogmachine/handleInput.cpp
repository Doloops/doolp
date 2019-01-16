#include <glogmachine_config.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glog.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#if 0
#include <handleInput.h>

typedef struct ClientViewDoParseArgs
{
  DMLXParserMMap * parser;
  sem_t doParse;
  bool parseEnd; // Say to DoParse that things are over.
  sem_t parseEndSem; // DoParse replies "yes... ok..."
};

void * ClientViewDoParse ( void * __args )
// This thread makes the conversion between the mmap()
// and the ClientView (and LogView)
// We are aware of clients here.
// Ifever the LogView quits, it could be a
// great idea to notify this thread
// Cause it is no use to continue parsing without 
// Anyone after.
{
  ClientViewDoParseArgs * args = (ClientViewDoParseArgs *) __args;
  DMLXParserMMap * parser = args->parser;
  ClientView * client = NULL;
  Log ( "Running ClientViewDoParse\n" );
  while ( ! (parser->hasWork() ) )
    {
      Log ( "Waiting 'doParse' semaphore\n" );
      sem_wait ( &(args->doParse) );
      Log ( "Recieved 'doParse' semaphore\n" );
    }
  string source;
  int offset;

  parser->checkEventName ( "source" );
  source = parser->getAttr ( "name" );
  offset = parser->getAttrInt ( "offset" );
  Log ( "Source='%s', offset=%d\n", source.c_str(), offset );
  client = getClient ( source, parser );
  if ( client == NULL )
    {
      Warn ( "Given a NULL client. Quitting.\n" );
      sem_post ( &(args->parseEndSem) );
      return NULL;
    }
  while ( true )
    {
      while ( ! (parser->hasWork() ) )
	{
	  if ( args->parseEnd )
	    {
	      Log ( "Things are over, quitting.\n" );
	      client->setDisconnected();
	      sem_post ( &(args->parseEndSem) );
	      return NULL;
	    }
	  sem_wait ( &(args->doParse) );
	}
      try
	{
	  while ( parser->hasWork () )
	    {
	      parser->parse();
	      Log ( "Parsed : %d events, %d end events\n", 
		    parser->getEventsNumber(),
		    parser->getEndEventsNumber() );
	      while ( parser->getEndEventsNumber() > 0 )
		client->parseOneEvent();
	    }
	}
      catch ( DMLXParserException * e )
	{
	  Warn ( "DMLXParserException : '%s'\n",
		 e->getMessage() );
	  return NULL;
	}
      }
  Bug ( "Shall not be here !\n" );
  return NULL;
}

// This prevents from having multiple dumpFiles of same name.
time_t dumpFileLast = 0;
unsigned int dumpFileVersion = 0;

void * handle ( void * args )
// handle() has no idea about Clients.
// Only take streams from socket
// Dump it to the dumpfile
// Keep informed the ClientViewDoParse() using doParse semaphore
// And the DMLXParserMMap::giveNewBytesToRead() method.
//
// doParse send to parsing policy
// when preWindowSize is filled (shall be around 10ko maybe)
// OR when timeout (i.e. select() == 0) ?
// Timeout could be around 5seconds.
{
  Socket sock = (Socket) args;
  int attempt = 0;
  int maxAttempts = 5;
  int preBufferSize = 1024 * 5;
  int parseWindowMaxSize = 1024 * 100; // * 256;
  time_t maxTimeBetweenParse = 4;
  // Create the dump file
  char dumpFile[64];
  time_t now = time(NULL);
  if ( dumpFileLast == now ) // Shall require a mutex here
    dumpFileVersion++;
  else
    dumpFileVersion = 0;
  dumpFileLast = now;
  sprintf ( dumpFile, "logs/%x-%d.log.xml", (int) now, dumpFileVersion );
  int fd = open ( dumpFile, O_CREAT + O_RDWR );
  if ( fd == 0 )
    Bug ( "Could not open log file '%s'\n", dumpFile );
  
  // Create the mmap()
  int pagesz = getpagesize ();
  Log ( "Page Size is '%d'\n", pagesz );
  int mmapsz = pagesz * 1024 * 128;
  void * buffer = mmap ( NULL, mmapsz, PROT_READ + PROT_WRITE,
			 MAP_SHARED, fd, 0 );
  if ( buffer == MAP_FAILED )
    Fatal ( "Could not open mmapped buffer\n" );
  Log ( "Buffer at %p (sz=%d)\n", buffer, mmapsz );

  // Create the DMLXParserMMap based on this buffer
  DMLXParserMMap * parser = new DMLXParserMMap ( buffer, mmapsz, parseWindowMaxSize );
  parser->setMaxEndEventsInNameSpace ( 10 );
  parser->setParseCanFillBufferOnce ( true );
  char *buffRd;
  buffRd = (char*) malloc ( preBufferSize );
  int iter = 0;
  int currRead = 0; // Number of bytes read and not parsed.

  ClientViewDoParseArgs * doArgs = new ClientViewDoParseArgs();
  doArgs->parser = parser;
  sem_init ( &(doArgs->doParse), 0, 0 );
  doArgs->parseEnd = false;
  sem_init ( &(doArgs->parseEndSem), 0, 0 );

  pthread_t thread;
  pthread_create ( &thread, NULL, ClientViewDoParse, (void*) doArgs );

  time_t lastParse = time(NULL);
  while ( true )
    {
      time_t now = time(NULL);
      iter++;
      if ( iter % 100 == 0 )
	Log ( "Iter %d for %d (%d bytes in stock)\n", iter, sock, currRead );
      fd_set fdSet;
      struct timeval TimeToWait;
      TimeToWait.tv_sec = 1;
      TimeToWait.tv_usec = 0;
      FD_ZERO( &fdSet );
      FD_SET( sock , &fdSet );
      int result = select( sock+1, &fdSet, NULL, NULL, &TimeToWait );
      if ( result == 0 )
	{
	  Log ( "Nothing to select() ? attempt=%d\n", attempt );
	  attempt++;
	  if ( attempt == maxAttempts )
	    goto handleEnd;
	  //
	  /*
	  if ( now - lastParse > maxTimeBetweenParse )
	    {
	      currRead = 0;
	      lastParse = now;
	      sem_post ( &(doArgs->doParse) );
	    }
	  */
	  continue;
	}
      if ( result == -1 || ( ! FD_ISSET ( sock, &fdSet ) ) )
	{
	  Log ( "Finished ?\n" );
	  goto handleEnd;
	}
      int rd = read ( sock, buffRd, preBufferSize );
      if ( rd == -1 )
	{
	  Log ( "Error : '%s'\n", strerror ( errno ) );
	  goto handleEnd;
	}
      else if ( rd == 0 )
	{
	  Log ( "Nothing to read ? attempt=%d\n", attempt );
	  attempt++;
	  if ( attempt == maxAttempts )
	    goto handleEnd;
	  sleep(1);
	  continue;
	}
      attempt = 0;
      //      Log ( "Read '%d' bytes\n", rd );
      write ( fd, buffRd, rd );

      parser->giveNewBytesToRead ( rd );

      currRead += rd;
      if ( ( currRead > parseWindowMaxSize ) || ( now - lastParse > maxTimeBetweenParse ) )
	{
	  Log ( "Running doParse, currRead = '%d', time since last Parse = '%d'\n", 
		currRead, (int)(now-lastParse) );
	  currRead = 0;
	  lastParse = now;
	  sem_post ( &(doArgs->doParse) );
	}
    }
 handleEnd:
  Log ( "Finished ? still currRead='%d' in stock\n", currRead );
  doArgs->parseEnd = true;
  sem_post ( &(doArgs->doParse) );
  sem_wait ( &(doArgs->parseEndSem) );
  Log ( "DoParse finished... Quitting.\n" );
  delete ( parser );
  munmap ( buffer, mmapsz );
  close ( fd );
  return NULL;
}


#endif

#if 0
bool handleLoop ( ClientView * client, Socket sock )
{
  int iter = 0;
  while ( true )
    {
      iter++;
      if ( iter % 100 == 0 )
	Log ( "Iter. for %d\n", sock );
      fd_set fdSet;
      struct timeval TimeToWait;
      TimeToWait.tv_sec = 0;
      TimeToWait.tv_usec = 500;
      FD_ZERO( &fdSet );
      FD_SET( sock , &fdSet );
      int result = select( sock+1, &fdSet, NULL, NULL, &TimeToWait );
      if ( result == 0 ) continue;
      if ( result == -1 )
	{
	  Log ( "Finished ?\n" );
	  return true;
	  //	  goto handleEnd;
	}
      Log ( "got result from select : %d\n", result );
      if ( FD_ISSET ( sock, &fdSet ) )
	{
	  Log ( "Got news from '%d'\n", sock );
	  if ( client->handled == false )
	    {
	      sem_post ( &(client->handleEnd ) );
	      //	      mainWindow->setSourceActive ( (Glib::ustring&) source, instanceIndex, false );
	      return false;
	    }
	  //	  mainWindow->updateSource ( (Glib::ustring&) source, instanceIndex );
	  /*
	  if ( ! view->parse () )
	    {
	      Log ( "Parse finished ? (attempt = %d)\n", attempt );
	      attempt ++;
	      if ( attempt == 100 )
		{
		  Log ( "Parse finished !!!!\n" );
		  goto handleEnd;
		}
	      usleep ( 500 );
	    }
	  else
	    {
	      attempt = 0;
	    }
	  */
	}
    }

}



  char dumpFile[64];
  sprintf ( dumpFile, "logs/%d.log.xml", (int) time (NULL) );
  int dump;
  dump = open ( dumpFile, O_CREAT + O_WRONLY );

  if ( dump == -1 )
    fprintf ( stderr, "Could not open dump file '%s'\n", dumpFile );

  fprintf ( stderr, "Initing new DMLXParserSocket\n" );
  DMLXParserSocket * parser = new DMLXParserSocket ( sock );
  fprintf ( stderr, "Inited new DMLXParserSocket\n" );
  if ( dump != -1 )
    parser->dumpToSocket ( dump );

  try
    {
      fprintf ( stderr, "parser at event %s\n", parser->getEvent()->name );
      parser->checkEventName ( "source" );
      source = parser->getAttr ( "name" );
      offset = parser->getAttrInt ( "offset" );
    }
  catch ( DMLXParserException * e )
    {
      fprintf ( stderr, "Got exception '%s'\n", e->getMessage () );
      delete ( e );
      return false;
    }
  char dumpTrueFile[64];
  sprintf ( dumpTrueFile, "logs/%s.%d.log.xml", source.c_str(), offset );
  rename ( dumpFile, dumpTrueFile );

  client = getClient ( source, parser );

  if ( ! handleLoop ( client, sock ) )
    return NULL;

  client->handled = false;
  /*
    TO REIMPLEMENT : 
    view->rebind ( NULL );
    mainWindow->setSourceBinded ( (Glib::ustring&) source, instanceIndex, false );
  */
  sem_post ( &(client->handleEnd ) ); // Not necessary
  
  return NULL;
}


#endif 


