#include <glog.h>
#include <glogers.h>

GlogRoot::GlogRoot ( char* _progName )
{
  firstGloger = NULL;
  progName = _progName;
  ftime ( &offset );
  for ( int i = 0 ; i < __GLOG_PRIOR_MAX ; i++ )
    showPrior[i] = true;
}

void GlogRoot::__addGlog ( Gloger * gloger )
{
  gloger->nextGloger = firstGloger;
  firstGloger = gloger;
}

void GlogRoot::sendToAll ( GlogItem & item )
{
  if ( firstGloger == NULL )
    { 
      fprintf ( stderr, "GLOG : No glog defined !\n" );
      fprintf ( stderr, "GLOG : You MUST add a Gloger using addGlog()\n" );
      fprintf ( stderr, "GLOG : before any use of logging (be carefull of constructors before main())\n" );
      exit ( -1 );
    }
  for ( Gloger * gloger = firstGloger ; gloger != NULL ; gloger = gloger->nextGloger )
    {
      gloger->send ( item );
      if ( item.priority >= __GLOG_PRIOR_FATAL )
	gloger->flush ();
    }
}

#if 0
void __Glog_SendToAll(GlogItem &glogItem)
{
  if ( __GLOGINFO_CURRENT == NULL )
    { 
      fprintf ( stderr, "GLOG : No glog defined !\n" );
      fprintf ( stderr, "GLOG : You MUST add a GlogInfo using addGlog()\n" );
      fprintf ( stderr, "GLOG : before any use of logging (be carefull of constructors before main())\n" );
      exit ( -1 );
    }
  if ( __GLOGINFO_OFFSET == 0 )
    {
      timeb tp; ftime ( &tp) ;
      __GLOGINFO_OFFSET = tp.time;
    }								
  for ( GlogInfo * _g = __GLOGINFO_CURRENT ; 
	_g != NULL ; _g = _g->nextGlogInfo )
    {
      _g->send ( glogItem );
      if ( glogItem.priority >= __GLOG_PRIOR_FATAL )
	_g->flush ();
    }
}
#endif 

char * __GLOG__PRIOR__STRING[] = 
  { "Log", "Info", "Warn", "Error", "Fatal", "Bug" };

#if 0
int main ( int argc, char ** argv )
{
  newGlog ( new GlogInfo_File ( stdout ) );
  Log ( "test\n" );
  AssertBug ( false, "wrong condition\n" );
  return 0;
}
#endif
#if 0

#endif 
