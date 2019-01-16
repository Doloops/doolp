#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <dirent.h>
#include <stdio.h>
#include <fcntl.h>

#include <doolp/doolp-doolppersistance-ftree.h>
#include <doolp/doolp-doolpobjectcache.h>
#include <doolp/doolp-doolpobjectbuffer.h>
#include <doolp/doolp-doolpexceptions.h>

Doolp::PersistanceFTree::PersistanceFTree ( char * _ftreeRoot )
{
  __DOOLP_Log ( "New DoolpPersistanceFTree with root='%s'\n", _ftreeRoot );
  serviceName = "DoolpPersistance";
  ftreeRoot = (char*) malloc ( strlen ( _ftreeRoot ) + 1 );
  strcpy ( ftreeRoot, _ftreeRoot );
  setOptions ();
}


Doolp::FullContextId * Doolp::PersistanceFTree::doolpfunclocal(getNewContext) ( bool force )
{
  FullContextId * newC = getForge()->getNewFullContextId ();
  return newC;
}

bool Doolp::PersistanceFTree::doolpfunclocal(commit) ( Doolp::FullContextId * fullContextId, 
						       Doolp::Stream<Doolp::ObjectId> * deletedObjects,
						       Doolp::Stream<Doolp::Object*> * objects )
{
  __DOOLP_Log ( "Commiting ! fullContextId=[%d,%d,%d]\n",
		logDoolpFullContextId(fullContextId) );
  AssertFatal ( fullContextId->agentId == getOwnerAgentId(), " This commit does not concern my contexts !\n" );
  while ( ! ( objects->isFinished () 
	      && deletedObjects->isFinished() ) )
    {
      if ( deletedObjects->canPop () )
	{
	  ObjectId objId;
	  deletedObjects->pop ( objId );
	  __DOOLP_Log ( "deleted object : '%d'\n",
			objId );
	}
      if ( objects->isFinished () )
	continue;
      Object * obj;
      objects->pop ( obj );
      __DOOLP_Log ( "commit objId='%d' nameId='%x'\n", obj->getObjectId(), obj->getNameId() );
      AssertFatal ( obj != NULL, " Could not get object\n" );
      AssertFatal ( obj->isObjectBuffer(), " This is NOT a Doolp::ObjectBuffer !\n" );
      if ( ! writeObject ( fullContextId, (ObjectBuffer*) obj ) )
	{
	  Warn ( "Could not write object\n" );
	  return false;
	}
	   
    }
  return true;
}

bool Doolp::PersistanceFTree::writeObject ( Doolp::FullContextId * fullContextId, Doolp::ObjectBuffer * objBuff )
{
#define _checkDir(_path) \
  if ( stat(_path, &st ) != 0 ) \
    {								\
      if ( mkdir ( _path, S_IRUSR + S_IWUSR + S_IXUSR ) != 0 )	\
	{ Fatal ( "Could not create dir '%s'\n", _path ); }	\
    }
#define _writeToFile(_fileName,...)					\
  sprintf ( pathfile, _fileName );					\
  fp = fopen ( path, "w" );						\
  AssertFatal ( fp != NULL, " Could not open '%s' for write\n", path );	\
  fprintf ( fp, __VA_ARGS__ );						\
  fclose ( fp );
  

  struct stat st;
  FILE * fp;
  char path[256];
  unsigned int idx = 0;
  char * pathfile;

  _checkDir ( ftreeRoot );
  sprintf ( path, "%s/objects", ftreeRoot ); // /objects/%d.%d.%d/0x%x" );
  _checkDir ( path );
  sprintf ( path, "%s/objects/%d.%d.%d", ftreeRoot, logDoolpFullContextId ( fullContextId ) );
  _checkDir ( path );
  sprintf ( path, "%s/objects/%d.%d.%d/0x%x/", ftreeRoot, logDoolpFullContextId ( fullContextId ), objBuff->getObjectId() );
  _checkDir ( path );

  idx = strlen ( path );
  pathfile = &(path[idx]);
  sprintf ( pathfile, "nameId" );

  _writeToFile ( "nameId", "0x%x", objBuff->getNameId () );
  _writeToFile ( "ownerAgentId", "%d", objBuff->getOwnerAgentId () );
  sprintf ( pathfile, "params/" );
  _checkDir ( path );

  idx = strlen ( path );
  pathfile = &(path[idx]);

  map<ObjectParamId, ObjectBufferParam*>::iterator param;
  for ( param = objBuff->params.begin () ; param != objBuff->params.end ();
	param ++ )
    {
      //      char paramFile[32];
      sprintf ( pathfile, "0x%x", param->first );
      AssertFatal ( param->second != NULL, " no paramBuffer !\n" );
      AssertFatal ( param->second->buffer != NULL, " paramBuffer empty !\n" );
      //      _writeToFile ( paramFile, (char*) ( param->second->buffer ) );
      int fd = creat ( path, S_IRUSR | S_IWUSR ); close ( fd );
      fd = open ( path, O_TRUNC | O_WRONLY );
      write ( fd, &(param->second->type), sizeof (int) );
      write ( fd, &(param->second->size), sizeof (int) );
      write ( fd, param->second->buffer, param->second->size );
      close ( fd );
    }
  return true;
#undef _writeToFile
#undef _checkDir
}



Doolp::Object * Doolp::PersistanceFTree::doolpfunclocal(retrieveObject) ( Doolp::FullContextId * fullContextId, 
									  Doolp::ObjectId objId )
{
#define _checkDir(_path)					\
  if ( stat(_path, &st ) != 0 )					\
    {								\
      Warn ( "Dir '%s' does not exist\n", _path );		\
      throw new NoObjectFound ();				\
    }
#define _readFromFile(_fileName,...)					\
  sprintf ( pathfile, _fileName );					\
  fp = fopen ( path, "r" );						\
  AssertFatal ( fp != NULL, " Could open file '%s' for read\n", path );	\
  fscanf ( fp, __VA_ARGS__ );						\
  fclose ( fp );
  
  struct stat st;
  FILE * fp;
  char path[256];
  unsigned int idx = 0;
  char * pathfile;

  _checkDir ( ftreeRoot );
  sprintf ( path, "%s/objects", ftreeRoot ); // /objects/%d.%d.%d/0x%x" );
  _checkDir ( path );
  sprintf ( path, "%s/objects/%d.%d.%d", ftreeRoot, logDoolpFullContextId ( fullContextId ) );
  _checkDir ( path );
  sprintf ( path, "%s/objects/%d.%d.%d/0x%x/", ftreeRoot, logDoolpFullContextId ( fullContextId ), objId );
  _checkDir ( path );

  idx = strlen ( path );
  pathfile = &(path[idx]);

  ObjectNameId _nameId;
  AgentId _ownerId;
  
  _readFromFile ( "nameId", "0x%x", &_nameId );
  _readFromFile ( "ownerAgentId", "%d", &_ownerId );
  
  __DOOLP_Log ( "nameId=0x%x, ownerAgentId=%d\n", _nameId, _ownerId );
  
  ObjectBuffer * buff = new ObjectBuffer ( objId, _nameId );
  //  buff->getObjectId() = objId;
  buff->setOwner ( _ownerId );

  sprintf ( pathfile, "params/" );
  _checkDir ( path );
  idx = strlen ( path );
  pathfile = &(path[idx]);

  DIR * params = opendir ( path );
  AssertFatal ( params != NULL, "Could not open dir '%s'\n", path );
  struct dirent * dr;
  while ( ( dr = readdir ( params ) ) != NULL )
    {
      if ( dr->d_name[0] == '.' ) continue;
      __DOOLP_Log ( "Found paramId '%s'\n", dr->d_name );
      ObjectParamId paramId;
      sscanf ( dr->d_name, "0x%x", &paramId );
      __DOOLP_Log ( "paramId=0x%x\n", paramId );
      ObjectBufferParam * pb = new ObjectBufferParam();

      sprintf ( pathfile, dr->d_name );				       
      int fd = open ( path , O_RDONLY );
      AssertFatal ( fd != 0, " Could open file '%s' for read\n", path );
      read ( fd, &(pb->type), sizeof(int) );
      read ( fd, &(pb->size), sizeof(int) );
      Log ( "Read paramId 0x%x : type=%d, size=%d\n", 
	    paramId, pb->type, pb->size );
      if ( pb->size == 0 )
	Warn ( "Read size for param 0x%x is 0 !\n", paramId );
      pb->buffer = malloc ( pb->size );
      read ( fd, pb->buffer, pb->size );
      close ( fd );
      buff->params[paramId] = pb;
    }
  closedir ( params );

  buff->setTTL ( getForge()->options.jobObjectTTL );
  getForge()->getObjectCache()->add ( buff, fullContextId );
  getForge()->getObjectCache()->notifyContextEnd ( fullContextId ); // Must do this
  return (Object*) buff;
#undef _readFromFile
#undef _checkDir
}

