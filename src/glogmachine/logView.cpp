#include <glogmachine_config.h>
#include <logView.h>
#include <gdk/gdkkeysyms.h>
#include <gtkmm/accelgroup.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/paned.h>
#include <gtkmm/menu.h>
#include <gtkmm/box.h>
#include <gtkmm/scale.h>
#include <gtk/gtkmain.h>
#include <iostream>
#include <glog.h>

static DMLX::Keyword keyw_program("program");
static DMLX::Keyword keyw_glog("glog");
static DMLX::Keyword keyw_t("t");
static DMLX::Keyword keyw_tm("tm");
static DMLX::Keyword keyw_thread("thread");
static DMLX::Keyword keyw_file("file");
static DMLX::Keyword keyw_func("func");
static DMLX::Keyword keyw_line("line");
static DMLX::Keyword keyw_prior("prior");
static DMLX::Keyword keyw_text("text");
static DMLX::Keyword keyw_id("id");
static DMLX::Keyword keyw_value("value");
static DMLX::Keyword keyw_module("module");
static DMLX::Keyword keyw_offset("offset");

// #define AssertBug(__cond,...) { if ( ! (__cond) ) { fprintf ( stderr, "Assertion false : "/**/#__cond/**/" : "/**/__VA_ARGS__); } }

//LogView::LogView ( DMLXParser * _parser, Glib::ustring _source ) : Gtk::Window(Gtk::WINDOW_TOPLEVEL)




void LogView::init () 
{
  Warn ( "Deprecated ?\n" );
}

extern char * __GLOG__PRIOR__STRING[];

#define txt_buffer_sz 512
static char txt_buffer[txt_buffer_sz];
static char cThread[11];


void LogView::addRecordInTree ( unsigned int recordId, unsigned int sourceId, 
				DMLX::BinMMap::iterator &i, Gtk::TreeModel::Row &row )
{
  row[logTreeColumns->colId] = recordId;
  row[logTreeColumns->colSourceIndex] = sourceId;
  if ( showRelativeTime )
    {
      unsigned int t = i.getAttrInt ( keyw_t.getHash() );
      unsigned int tm = i.getAttrInt ( keyw_tm.getHash() );
      if ( tm >= offset_tm  )
	{
	  row[logTreeColumns->colSec] = t - offset_t;
	  row[logTreeColumns->colUSec] = tm - offset_tm;
	}
      else
	{
	  row[logTreeColumns->colSec] = t - offset_t - 1;
	  row[logTreeColumns->colUSec] = ( 1000 + tm ) - offset_tm;
	}
    }
  else
    {
      row[logTreeColumns->colSec] = i.getAttrInt ( keyw_t.getHash() );
      row[logTreeColumns->colUSec] = i.getAttrInt ( keyw_tm.getHash() );
    }
  sprintf ( cThread, "0x%08x", i.getAttrHex ( keyw_thread.getHash() ) );
  row[logTreeColumns->colThread] = cThread; // r->thread;
  Glib::ustring * file = sourceFiles.get ( i.getAttrHex ( keyw_file.getHash() ) );
  if ( file != NULL )
    row[logTreeColumns->colFile] = *file;
  else
    row[logTreeColumns->colFile] = "Unknown";
  Glib::ustring * func = sourceFuncs.get ( i.getAttrHex ( keyw_func.getHash() ) );
  if ( func != NULL )
    row[logTreeColumns->colFunc] = *func;
  else
    row[logTreeColumns->colFunc] = "Unknown";


  row[logTreeColumns->colLine] = i.getAttrInt ( keyw_line.getHash() ); // r->line;
  row[logTreeColumns->colLevel] = __GLOG__PRIOR__STRING[i.getAttrInt ( keyw_prior.getHash() )];

  unsigned int ln = i.getAttr ( keyw_text.getHash(), txt_buffer, txt_buffer_sz );
  AssertBug ( ln > 0, "No text ?\n" );
  for ( unsigned int i = 0 ; i < ln ; i++ )
    {
      if ( txt_buffer[i] == '\n' )
	txt_buffer[i] = '.';
    }
  row[logTreeColumns->colMessage] = txt_buffer;
}

void LogView::clearLogs ()
{
  logTreeModel->clear ();
}

void LogView::refresh ( bool seekend )
{
  Log ( "Refresh (seekend=%d)\n", seekend );
  unsigned int start = (unsigned int)logViewScale->get_value();

  Gtk::TreeIter iter = logTreeModel->children().begin ();
  if ( ! seekend )
    {
      if ( iter != logTreeModel->children().end() )
	{
	  for ( unsigned int x = (*iter)[logTreeColumns->colId] ; x < start ; x++ )
	    {
	      iter = logTreeModel->erase ( iter );
	      if ( iter == logTreeModel->children().end() )
		break;
	    }
	}
    }
  unsigned int viewed = 0;
  bool skiprecord = false;
  DMLX::BinMMap::iterator biter = binMMap->getIterator ( start );

  Log ( "bufferSize = %d\n", binMMap->getBufferSize() );
  while ( biter.getItemNumber() != start )
    {
      biter++;
      AssertBug ( ! biter.isAtEnd(), "Reached end of file while getting start=%d\n", start );
    }
  for ( ; ! biter.isAtEnd() ; biter ++ )
    {
      if ( biter.isAtEnd() )
	{
	  Warn ( "At end !\n" );
	  break;
	}
      if ( ! biter.isMarkup() )
	continue;
      if ( biter.getMarkupHash () == keyw_module.getHash () )
	{
	  unsigned int mod_ptr = biter.getAttrHex ( keyw_id.getHash() );
	  if ( sourceModules.has ( mod_ptr ) ) continue;
	  char mod[128];
	  biter.getAttr ( keyw_value.getHash(), mod, 128 );
	  Glib::ustring * modS = new Glib::ustring ( mod ); // rec->file );
	  sourceModules.put ( mod_ptr, modS );
	  continue;
	}
      if ( biter.getMarkupHash () == keyw_file.getHash () ) 
	{
	  unsigned int file_ptr = biter.getAttrHex ( keyw_id.getHash() );
	  if ( sourceFiles.has ( file_ptr ) ) continue;
	  char file[128];
	  biter.getAttr ( keyw_value.getHash(), file, 128 );
	  Glib::ustring * fileS = new Glib::ustring ( file ); // rec->file );
	  sourceFiles.put ( file_ptr, fileS );
	  continue;
	}
      if ( biter.getMarkupHash () == keyw_func.getHash () )
	{
	  unsigned int func_ptr = biter.getAttrHex ( keyw_id.getHash() );
	  if ( sourceFuncs.has ( func_ptr ) ) continue;
	  char func[128];
	  biter.getAttr ( keyw_value.getHash(), func, 128 );
	  Glib::ustring * funcS = new Glib::ustring ( func ); // rec->file );
	  sourceFuncs.put ( func_ptr, funcS );
	  continue;
	}
      if ( biter.getMarkupHash () == keyw_program.getHash () )
	{
	  char program[128];
	  biter.getAttr ( keyw_value.getHash(), program, 128 );
	  Log ( "Module name '%s'\n", program );
	  string _title = "LogView : program '";
	  _title += program;
	  _title += "'";
	  set_title( _title );
	  continue;
	}
      if ( biter.getMarkupHash () == keyw_offset.getHash () )
	{
	  offset_t = biter.getAttrInt ( keyw_t.getHash () );
	  offset_tm = biter.getAttrInt ( keyw_tm.getHash () );
	  Log ( "Offset t=%d, tm=%d\n", offset_t, offset_tm );
	  continue;
	}
      skiprecord = false;
      unsigned int r = biter.getItemNumber();
      if ( ! seekend )
	{
	  while ( iter != logTreeModel->children().end() )
	    {
	      unsigned int idx = (*iter)[logTreeColumns->colId];
	      unsigned int sIdx = (*iter)[logTreeColumns->colSourceIndex];
	      if ( ! sourcesChecked[sIdx] )
		{
		  iter = logTreeModel->erase ( iter );
		  continue;
		}
	      if ( idx == r )
		{
		  skiprecord = true;
		  viewed ++;
		  iter++;
		  break;
		}
	      if ( idx > r )
		break;
	      iter++;
	    }
	  if ( skiprecord )
	    {
	      if ( viewed >= maxRecordsViewed )
		break;
	      continue;
	    }
	}
      unsigned int mod_ptr, file_ptr, func_ptr, line;
      mod_ptr = biter.getAttrHex ( keyw_module.getHash() );
      file_ptr = biter.getAttrHex ( keyw_file.getHash() );
      func_ptr = biter.getAttrHex ( keyw_func.getHash() );
      line = biter.getAttrInt ( keyw_line.getHash() );
      
      unsigned int sourceId = sourcesIndex.get ( file_ptr, line, func_ptr );
      if ( sourceId == 0 )
	{
	  sourceId = sourceNextIndex++;
	  sourcesIndex.put ( file_ptr, line, func_ptr, sourceId );
	  addSource ( mod_ptr, file_ptr, line, func_ptr, sourceId );
	  SyncSourceItemAndSourceTreeView ();
	  sourcesChecked[sourceId] = true;
	}
      if ( (!seekend) && sourcesChecked[sourceId] )
	{
	  if ( iter != logTreeModel->children().end() )
	    {
	      AssertBug ( (*iter)[logTreeColumns->colId] > r, "Shall have skipped this record !\n" );
	      iter = logTreeModel->insert ( iter );
	      Gtk::TreeModel::Row row = *iter;
	      addRecordInTree ( r, sourceId, biter, row );
	      iter++;
	    }
	  else
	    {
	      Gtk::TreeModel::Row row = *(logTreeModel->append () );
	      addRecordInTree ( r, sourceId,  biter, row );
	      if ( r > totalRecords )
		{
		  totalRecords = r;
		  logViewScale->set_range ( 0, totalRecords + 1 /*+ maxRecordsViewed*/ );
		  logViewScale->show ();
		}
	    }
	  viewed ++;
	  if ( viewed >= maxRecordsViewed )
	    break;
	}
    }
  // Iterator overhead
  if ( ! seekend )
    {
      if ( ! biter.isAtEnd() ) biter++;
      if ( ! biter.isAtEnd() ) biter++;
      if ( iter != logTreeModel->children().end() )
	{
	  while ( iter != logTreeModel->children().end() )
	    {
	      iter = logTreeModel->erase ( iter );
	    }
	}
      logTreeView->show ();
    }
  else
    {
      Log ( "Seek end.\n" );
      AssertFatal ( biter.isAtEnd(), "Not reached end ?\n" );
      logViewScale->set_range ( 0, binMMap->getLastItemNumber() );
      logViewScale->set_value ( binMMap->getLastItemNumber() - maxRecordsViewed + 1 );
      refresh ( false );
    }
  Log ( "Refresh ok.\n" );
}

void LogView::logTreeRowActivated ( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* /* column */)
{
  Gtk::TreeModel::iterator iter = logTreeModel->get_iter(path);
  if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      // fprintf ( stderr, "Activated Id=%d, sId=%d\n", (int)row[logTreeColumns->colId], (int)row[logTreeColumns->colSourceIndex] );
      // Get the cursor
      // Open a context menu
      logContextMenu->popup (0, gtk_get_current_event_time() );
    }
}

void LogView::sourceTreeRowChanged ( const Gtk::TreeModel::Path& path, const Gtk::TreeModel::iterator& iter)
{
  Gtk::TreeModel::Row row = *iter;
  Glib::ustring colName = row[sourceTreeColumns->colName];
  int index = row[sourceTreeColumns->colId];
  bool checked = row[sourceTreeColumns->colChecked];
  bool dontShow = dontShowRecord;
  dontShowRecord = true;
  if ( index < 2 )
    {
      for ( Gtk::TreeIter iter = row.children ().begin() ; 
	    iter != row.children().end() ; iter ++ )
	{
	  Gtk::TreeModel::Row childrow = *iter;
	  childrow[sourceTreeColumns->colChecked] = checked;
	}
    }
  else
    {
      // if ( (!dontShowRecord) && sourcesChecked[index] != checked )
      if ( sourcesChecked[index] != checked )
	{
	  sourcesChecked[index] = checked;
	  //	  Log ( "%s sourceIndex='%d'\n", checked ? "Showing" : "Hiding", index );
	  // showRecord ( index, checked );
	}
    }
  if ( ! dontShow )
    {
      Log ( "Showtime from '%d', checked=%d\n", index, checked );
      dontShowRecord = false;
      refresh ();
    }
}


void LogView::logTreeUnselectLine ()
{
  Gtk::TreeModel::Row row = *( logTreeView->get_selection()->get_selected() );
  showSource ( row[logTreeColumns->colSourceIndex], false, false, false );
}

void LogView::logTreeUnselectFunc ()
{
  Gtk::TreeModel::Row row = *( logTreeView->get_selection()->get_selected() );
  showSource ( row[logTreeColumns->colSourceIndex], false, false, true );
}

void LogView::logTreeUnselectFile ()
{
  Gtk::TreeModel::Row row = *( logTreeView->get_selection()->get_selected() );
  showSource ( row[logTreeColumns->colSourceIndex], false, true, false );
}

void LogView::logTreeShowRelativeTime ()
{
  if ( logTreeToggleActionShowRelativeTime->get_active() )
    fprintf ( stderr, "Show Relative Time ON\n" );
  else
    fprintf ( stderr, "Show Relative Time OFF\n" );
  if ( showRelativeTime == logTreeToggleActionShowRelativeTime->get_active() )
    {
      Warn ( "logTreeShowRelativeTime : no change !\n" );
      return;
    }
  showRelativeTime = logTreeToggleActionShowRelativeTime->get_active();
  Warn ( "TODO : REIMPLEMENT !\n" );
  for ( Gtk::TreeIter iter = logTreeModel->children().begin () ;
	iter != logTreeModel->children().end () ; iter++ )
    {
      /*
      if ( showRelativeTime )
	{
	  (*iter)[logTreeColumns->colSec] = (*iter)[logTreeColumns->colSec] - timeOffset;
	}
      else
	{
	  (*iter)[logTreeColumns->colSec] = (*iter)[logTreeColumns->colSec] + timeOffset;
	}
	*/
    }  

}


void LogView::logTreeClearLogs ()
{
  fprintf ( stderr, "Clear Logs\n" );
  clearLogs ();
}



void LogView::logViewScale_value_changed()
{
  int val = (int) logViewScale->get_value ();
  Log ( "Value : '%d'\n", val );
  refresh ();
}

void LogView::logViewScale_goto_end()
{
  Log ( "Seeking end...\n" );
  refresh ( true );
}
