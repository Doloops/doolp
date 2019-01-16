#ifndef __DOOLP_DOOLPCONNECTIONAGGREGATE_H
#define __DOOLP_DOOLPCONNECTIONAGGREGATE_H

#ifdef __DOOLP_USE_DOOLPCONNECTIONAGGREGATE


#include <doolp/doolp-doolpcallcontext.h>

#include <list>
using namespace std;

/*
 * This is a dummy useless DoolpConnection implementation.
 * It takes several write connections
 * But only one read connection.
 */

class DoolpConnectionAggregate : public DoolpConnection
{
 protected:
  list<DoolpConnection*> writeConnections;
  DoolpConnection * readConnection;

 public:
  DoolpConnectionAggregate ( DoolpForge * myForge ) 
    { 
      readConnection = NULL; 
      __init ( myForge); 
      __DOOLP_Log ( "New Connection Aggregate at %p\n", this );
    }
  bool endConnection () { return true; }

  bool setReadConnection ( DoolpConnection * _read ) 
  { 
    readConnection = _read; 
    distAgentId = _read->distAgentId; 
    //    _read->distAgentId = 0; // 
    _read->disableGetConnectionForAgentId = true;
    __DOOLP_Log ( "distAgentId = '%d'\n", distAgentId );
    __DOOLP_Log ( "Adding connection aggregate '%p' to forge\n", this );
    myForge->addConnection ( this );
    __DOOLP_Log ( "Connection added.\n" );
    return true; 
  }
  bool addWriteConnection ( DoolpConnection * _write )
  { writeConnections.push_back ( _write ); return true; }

#define _testReadConnection() \
  if ( readConnection == NULL ) Bug ( "ReadConnection not defined !\n" )
#define _forAllWriteConnections(__wr__) \
  for ( list<DoolpConnection*>::iterator __wr__ = writeConnections.begin () /*, __wrT__ = *__wr__ */; \
	__wr__ != writeConnections.end () ; __wr__ ++ /*, __wrT__ = *__wr__*/ )
   
  bool tryLockRead () { _testReadConnection(); return readConnection->tryLockRead (); }
  bool lockRead () { _testReadConnection(); return readConnection->lockRead (); }
  bool unlockRead () { _testReadConnection(); return readConnection->unlockRead (); }
  bool isLockRead () { _testReadConnection(); return readConnection->isLockRead (); }

  bool lockWrite () 
  { _forAllWriteConnections(writeConn) if ( ! (*writeConn)->lockWrite() ) return false; return true; }
  bool unlockWrite () 
  { _forAllWriteConnections(writeConn) if ( ! (*writeConn)->unlockWrite() ) return false; return true; }
  bool isLockWrite () { _forAllWriteConnections(writeConn) if ( (*writeConn)->isLockWrite() ) return true; return false; }

  ssize_t Write(const void *buf, size_t nbyte)
  { Bug ( "You should not use this !!!\n" ); }
  ssize_t Read(void *buf, size_t nbyte)
  { Bug ( "You should not use this !!!\n" ); }


#define __readDelegate(_rt,__func) _rt __func ( )		\
  { _testReadConnection(); return readConnection->__func ( ); }
#define __readDelegate1(_rt,__func,_T1) _rt __func ( _T1 _A1 )	\
  { _testReadConnection(); return readConnection->__func ( _A1 ); }
#define __readDelegate2(_rt,__func,_T1,_T2) _rt __func ( _T1 _A1, _T2 _A2 ) \
  { _testReadConnection(); return readConnection->__func ( _A1, _A2 ); }
#define __readDelegate3(_rt,__func,_T1,_T2,_T3) _rt __func ( _T1 _A1, _T2 _A2, _T3 _A3 ) \
  { _testReadConnection(); return readConnection->__func ( _A1, _A2, _A3 ); }

#define __writeDelegate(__func) bool __func ( )				\
  { _forAllWriteConnections(writeConn) if ( ! (*writeConn)->__func () ) return false; return true; }
#define __writeDelegate1(__func,_T1) bool __func ( _T1 _A1 )	\
  { _forAllWriteConnections(writeConn) if ( ! (*writeConn)->__func ( _A1 ) ) return false; return true; }
#define __writeDelegate2(__func,_T1,_T2) bool __func ( _T1 _A1, _T2 _A2 ) \
  { _forAllWriteConnections(writeConn) if ( ! (*writeConn)->__func ( _A1, _A2 ) ) return false; return true; }
  

#define __readFunc(__T) __readDelegate1(bool,Read,__T)
#define __writeFunc(__T) __writeDelegate1(Write,__T)

  __readDelegate2(int,waitRead,int,int)
    __readDelegate(int,waitRead)
    __writeDelegate(flushWrite)
    __readDelegate1(bool,waitSpecificCall,DoolpCall*)

    __writeFunc(int) __writeFunc(int*)
    __readFunc(int*) __readFunc(bool *)
    __writeFunc(DoolpObjectId) __readFunc(DoolpObjectId *)
    __writeFunc(char *) __readFunc(char **)
    __writeFunc(string*) __readFunc(string**)
    __writeFunc(float) __readFunc(float *)
    __writeFunc(DoolpFullContextId *) __readFunc(DoolpFullContextId **)
    __writeDelegate2(Write,DoolpObject *,list<DoolpObjectParamId> *)
    __writeDelegate1(WriteObjectHead, DoolpObject *)
    __readDelegate3( bool, Read, DoolpObject **,bool,bool)

    __writeDelegate1( startNewCall, DoolpCall *)
    __writeDelegate1( startReply, DoolpJob *)
    __writeDelegate( startParamSubSection )
    __readDelegate( bool, readParamSubSection )
    __writeDelegate( endSubSection )
    __readDelegate( bool, readSubSectionEnd )
    __writeDelegate( endMessage )
    __readDelegate( bool, readMessageEnd )

    __writeDelegate1( startList, unsigned int)
    __readDelegate( unsigned int, readList )
    
    __writeDelegate1( setNextBlockIndex, unsigned int)
    __readDelegate( unsigned int, getNextBlockIndex )
    __readDelegate( bool, readDoolpObjectSubSection )

    __writeDelegate2( setDoolpStream, DoolpCallContext *, DoolpStreamIndex)
    __writeDelegate ( leaveDoolpStream )
    __writeDelegate ( endDoolpStream )
    __readDelegate ( bool, readDoolpStreamEnd )
    __readDelegate ( bool, readDoolpStreamSubSection )
#undef __writeFunc
#undef __readFunc  
#undef _testReadConnection
#undef _forAllWriteConnections
};






#endif // __DOOLP_USE_DOOLPCONNECTIONAGGREGATE


#endif // __DOOLP_DOOLPCONNECTIONAGGREGATE_H
