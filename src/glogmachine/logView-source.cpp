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

LogView::SourceItem::SourceItem(Glib::ustring &_name, unsigned int _sourceIndex )
{
  son = NULL; next = NULL;
  name = _name;
  sourceIndex = _sourceIndex;
  isNew = true;
  hasNewSons = false;
}

LogView::SourceItem * LogView::getSourceItem ( LogView::SourceItem * father, 
					       Glib::ustring & name, 
					       unsigned int sourceIndex )
// Add to father->son list if not exist, sorted by name.
{
  AssertBug ( father != NULL, "Invalid NULL father.\n" );
  if ( father->son == NULL )
    {
      father->son = new SourceItem ( name, sourceIndex );
      father->hasNewSons = true;
      return father->son;
    }
  SourceItem * item = father->son;
  SourceItem * last = NULL;
  while ( item != NULL )
    {
      if ( item->name == name )
	return item;
      if ( item->name > name )
	{
	  // Inserting between this one.
	  SourceItem * newItem = new SourceItem ( name, sourceIndex );
	  newItem->next = item;
	  if ( last != NULL )
	    last->next = newItem;
	  else
	    father->son = newItem;
	  father->hasNewSons = true;
	  return newItem;
	}
      last = item;
      item = item->next;
    }
  // Appending
  SourceItem * newItem = new SourceItem ( name, sourceIndex );
  last->next = newItem;
  father->hasNewSons = true;
  return newItem;
}

void LogView::logSourceItem ( LogView::SourceItem * item )
{
  Log ( "Item '%s' (isNew=%d,hasNewSons=%d,sId=%d)\n", item->name.c_str(),
	item->isNew, item->hasNewSons, item->sourceIndex );
  if ( item->son != NULL )
    {
      Log ( "Sons of '%s'\n", item->name.c_str() );
      logSourceItem ( item->son );
      Log ( "End of sons of '%s'\n", item->name.c_str() );
    }
  if ( item->next != NULL )
    logSourceItem ( item->next );
}

void LogView::SyncSourceItemAndSourceTreeView ( )
{
  if ( sourceCacheVersion == sourceTreeVersion )
    return;
  SyncSourceItemAndSourceTreeView ( sourceItemRoot->son, sourceTreeModel->children() );  
  sourceTreeVersion = sourceCacheVersion;
  sourceTreeView->show ();
}

void LogView::SyncSourceItemAndSourceTreeView ( LogView::SourceItem * item, const Gtk::TreeModel::Children &children )
// Item is supposed to be the first element of the sons,
// Correlated to the children->begin() iterator
{
  AssertBug ( item != NULL, "Been given a NULL item\n" );
  Gtk::TreeIter treeIter = children.begin();
  while ( item != NULL )
    {
      if ( item->isNew )
	{
	  dontShowRecord = true;
	  treeIter = sourceTreeModel->insert(treeIter);
	  Gtk::TreeModel::Row row = *treeIter;
	  row[sourceTreeColumns->colId] = item->sourceIndex;
	  row[sourceTreeColumns->colName] = item->name;
	  row[sourceTreeColumns->colChecked] = true; // (bool)((*treeIter)[sourceTreeColumns->colChecked]); // true;
	  item->isNew = false;
	  dontShowRecord = false;
	}
      if ( item->son != NULL )
	{
	  //	  AssertBug ( item->son != NULL, "Item said hasNewSons but has no son at all\n" );
	  SyncSourceItemAndSourceTreeView ( (SourceItem*)item->son, treeIter->children() );
	  // item->hasNewSons = false;
	}
      treeIter++;
      item = item->next;
    }
}

void LogView::addSource ( unsigned int mod_ptr, unsigned int file_ptr, unsigned int line, unsigned int func_ptr, unsigned int sourceIndex )
{
  if ( sourceItemRoot == NULL )
    {
      Glib::ustring rootName = "ROOT";
      sourceItemRoot = new SourceItem ( rootName, 0 );
    }
  Glib::ustring * mod = sourceModules.get ( mod_ptr );
  AssertFatal ( mod != NULL, "Could not get mod_ptr '0x%x'\n", mod_ptr );
  Glib::ustring * file = sourceFiles.get ( file_ptr );
  AssertFatal ( file != NULL, "Could not get file_ptr '0x%x'\n", file_ptr );
  Glib::ustring * func = sourceFuncs.get ( func_ptr );
  AssertFatal ( func != NULL, "Could not get func_ptr '0x%x'\n", file_ptr );
  SourceItem * fileItem = getSourceItem ( sourceItemRoot, *mod, 0 );
  unsigned int i = 0;
  while ( true )
    {
      unsigned int j = file->find ( '/', i );
      Glib::ustring nfile(*file,i,j-i);
      fileItem = getSourceItem ( fileItem, nfile, 0 );
      if ( j == file->npos )
	break;
      else
	i = j + 1;
    }
  SourceItem * funcItem = getSourceItem ( fileItem, *func, 1 );
  char lineS[32]; // This is ugly.
  sprintf ( lineS, "Line %d", line );
  Glib::ustring lineStr = lineS;
  SourceItem * lineItem = getSourceItem ( funcItem, lineStr, sourceIndex );
  //  Log ( "fileItem=%p, funcItem=%p, lineItem=%p\n",
  //	fileItem, funcItem, lineItem );
  sourcesChecked[sourceIndex] = true;
  sourceCacheVersion++;
}


bool LogView::showSource ( unsigned int sourceIndex, bool show, bool touchFile, bool touchFunc )
{
  bool found = false;
  for ( Gtk::TreeIter fileIter = sourceTreeModel->children().begin () ;
	fileIter != sourceTreeModel->children().end () ; fileIter ++ )
    {
      for ( Gtk::TreeIter funcIter = fileIter->children().begin () ;
	    funcIter != fileIter->children().end () ; funcIter ++ )
	{
	  for ( Gtk::TreeIter lineIter = funcIter->children().begin () ;
		lineIter != funcIter->children().end () ; lineIter ++ )
	    {
	      if ( (*lineIter)[sourceTreeColumns->colId] == sourceIndex )
		{
		  (*lineIter)[sourceTreeColumns->colChecked] = show;
		  if ( ! touchFile && ! touchFunc )
		    return true;
		  found = true;
		  break;
		}
	    }
	  if ( found && !touchFile && touchFunc )
	    {
	      (*funcIter)[sourceTreeColumns->colChecked] = show;
	      return true;
	    }
	}
      if ( found && touchFile )
	{
	  (*fileIter)[sourceTreeColumns->colChecked] = show;
	  return true;
	}
    }
  return false;
}


void LogView::sourceTreeRowActivated ( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* /* column */)
{
  sourceContextMenu->popup (0, gtk_get_current_event_time() );
}


void LogView::sourceTreeSelectAll ()
{
  for ( Gtk::TreeIter fileIter = sourceTreeModel->children().begin () ;
	fileIter != sourceTreeModel->children().end () ; fileIter ++ )
    (*fileIter)[sourceTreeColumns->colChecked] = true;
}

void LogView::sourceTreeUnselectAll ()
{
  for ( Gtk::TreeIter fileIter = sourceTreeModel->children().begin () ;
	fileIter != sourceTreeModel->children().end () ; fileIter ++ )
    (*fileIter)[sourceTreeColumns->colChecked] = false;
}
