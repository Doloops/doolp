#include <doolp/doolp-doolpforgescheduler.h>

Doolp::ForgeScheduler::ForgeScheduler()
{
  first = NULL;
  last = NULL;
  //  pthread_mutex_init ( &__lock, NULL );
  myLock.setName ( "ForgeScheduler Lock" );
}

void * Doolp::__doolpforgescheduler__start(void * arg)
{
  ((Doolp::ForgeScheduler*)arg)->threadStart();
  return NULL;
}

bool Doolp::ForgeScheduler::threadStart()
{
  Log ( "Starting DoolpForgeScheduler\n" );
  
  while ( true )
    {
      time_t now = time ( NULL );
      lock();
      if ( ( first == NULL )
	   || ( first->when > now ) )
	{ unlock(); sleep(1); continue; }

      SchedList * lst = first;
      AssertBug ( lst->itemList.size() > 0, "SchedList has an empty itemList..\n" );
      time_t when = lst->when;
      unlock();
      while ( true ) 
	{
	  lock();
	  if ( lst->itemList.size() == 0 )
	    { unlock(); break; }
	  SchedItem * item = lst->itemList.front();
	  lst->itemList.pop_front();
	  unlock();
	  Log ( "Running action for item=%p, when=%d\n",
		item, (int) when );
	  if ( item->runSchedItem ( when ) )
	    {
	      delete ( item );
	    }
	}
      lock();
      first = lst->next;
      if ( first == NULL )
	last = NULL;
      delete ( lst );
      unlock();
    }
  return true;
}

bool Doolp::ForgeScheduler::start()
{
  pthread_create ( &thread, NULL, __doolpforgescheduler__start, (void*)this );
  pthread_detach ( thread );
  return true;
}

bool Doolp::ForgeScheduler::add( time_t when, SchedItem * item )
{
  lock();
  SchedList * lst;
  if ( first == NULL )
    {
      AssertBug ( last == NULL, "First is null but not last ?\n" );
      lst = new SchedList (when);
      first = lst;
      last = lst;
    }
  else
    {
      SchedList * bak = NULL;
      lst = first;
      while ( lst != NULL )
	{
	  if ( lst->when == when )
	    break;
	  if ( lst->when > when )
	    {
	      if ( bak == NULL )
		{
		  lst = new SchedList ( when );
		  lst->next = first;
		  first = lst;
		  break;
		}
	      else
		{
		  bak->next = new SchedList (when);
		  bak->next->next = lst;
		  lst = bak->next;
		  break;
		}
	    }
	  bak = lst;
	  lst = bak->next;
	}
      if ( lst == NULL ) // Adding at end
	{ 
	  AssertBug ( last != NULL, "At end, but last == NULL ?\n" );
	  last->next = new SchedList (when);
	  last = last->next;
	  lst = last;
	}
    }
  AssertBug ( lst != NULL, "Bug : could not create or find the good SchedList\n" );
  lst->itemList.push_back ( item );
  unlock();
  return true;
}


