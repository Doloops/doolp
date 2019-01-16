#ifndef __DOOLP_DOOLPFORGESCHEDULER_H
#define __DOOLP_DOOLPFORGESCHEDULER_H

#include <doolp/doolp-doolpclasses.h>
#include <doolp/doolp-doolplock.h>
#include <time.h>
#include <pthread.h>
#include <list>
#include <strongmap.h>
using namespace std;

/*
 * Usage : add inherited objects of SchedItem
 * runSchedItem semantics : 
 * * a true return value means Scheduler can delete item
 * * a false one means Scheduler CAN NOT delete this item.
 */
namespace Doolp
{
  class ForgeScheduler
  {
    friend void * __doolpforgescheduler__start(void * arg);
  public:
    class SchedItem
    {
    public:
      virtual ~SchedItem() {}
      virtual bool runSchedItem(time_t when) = 0;
    };
  protected:
    bool threadStart();
    pthread_t thread;
    class SchedList
    {
    public:
      SchedList ( time_t _when) { when = _when; next = NULL; }
      time_t when;
      list<SchedItem*> itemList;
      SchedList * next;
    };
    SchedList * first;
    SchedList * last;
    Lock myLock;
    inline void lock() { myLock.lock(); }
    inline bool trylock() { return myLock.trylock(); }
    inline void unlock() { myLock.unlock(); }

  public:
    ForgeScheduler::ForgeScheduler();

    bool start();
    bool add( time_t when, SchedItem * item );
    bool log();
  };
}; // NameSpace Doolp
#endif // __DOOLP_DOOLPFORGESCHEDULER_H
