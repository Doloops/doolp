#ifndef __DOOLP_DOOLPLOCK_H
#define __DOOLP_DOOLPLOCK_H

#include <doolp/doolp-doolpclasses.h>
#include <pthread.h>

/*
 * pthread-based lock.
 */

namespace Doolp
{
  class Lock
  {
  protected:
    pthread_mutex_t __lock;
    pthread_t lockedBy;
    char * lockName;
  public:
    Lock() { pthread_mutex_init ( &__lock, NULL ); lockedBy = 0; lockName = "(noname)"; }
    ~Lock() { }
    void setName ( char * _name ) { lockName = _name; }


    void lock () 
    {
      if ( lockedBy == 0 )
	Log ( "%s : Not locked..\n", lockName );
      if ( pthread_mutex_trylock ( &__lock ) != 0 )
	{
	  AssertBug ( lockedBy != pthread_self(), "%s : locked by current thread : 0x%x !\n", lockName, (int)pthread_self() );
	  if ( lockedBy == 0)
	    Warn ( "%s : trylock failed but not locked by anyone !\n", lockName );
	  Log ( "%s : trylock failed : locked by 0x%x\n", lockName, (int)lockedBy );
	  pthread_mutex_lock ( &__lock );
	}
      lockedBy = pthread_self();
      Log ( "%s : Locked by 0x%x !\n", lockName, (int)pthread_self() );
    }
    bool trylock ()
    {
      if ( lockedBy == 0 )
	Log ( "%s : Not locked..\n", lockName );
      if ( pthread_mutex_trylock ( &__lock ) != 0 )
	{
	  AssertBug ( lockedBy != pthread_self(), "%s : locked by current thread : 0x%x !\n", lockName, (int)pthread_self() );
	  if ( lockedBy == 0)
	    Warn ( "%s : trylock failed but not locked by anyone !\n", lockName );
	  Log ( "%s : trylock failed : locked by 0x%x\n", lockName, (int)lockedBy );
	  return false;
	}
      lockedBy = pthread_self();
      Log ( "%s : Locked by 0x%x !\n", lockName, (int)pthread_self() );
      return true;
    }
    void unlock ()
    {
      AssertBug ( lockedBy != 0, "%s : Not locked !\n", lockName );
      AssertBug ( lockedBy == pthread_self(), 
		  "%s : Not locked by the good thread. Locked by 0x%x, unlock by 0x%x\n",
		  lockName, (int)lockedBy, (int)pthread_self() );
      Log ( "%s : Unlocked by 0x%x !\n", lockName, (int)pthread_self() );
      lockedBy = 0;
      pthread_mutex_unlock ( &__lock );
    }

  };

}; // NameSpace Doolp








#endif // __DOOLP_DOOLPLOCK_H
