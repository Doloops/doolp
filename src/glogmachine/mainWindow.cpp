#include <glogmachine_config.h>
#include <mainWindow.h>
#include <gdk/gdkkeysyms.h>
#include <gtkmm/accelgroup.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/paned.h>
#include <gtkmm/menu.h>
#include <gtk/gtkmain.h>
#include <iostream>
#include <string.h>
#include <time.h>

MainWindow::MainWindow ()
{
  gmm_data = new GlademmData(get_accel_group());

  sourceNextIndex = 1;

  sourceTreeColumns = new SourceTreeColumns ();
  sourceTreeModel = Gtk::TreeStore::create ( *sourceTreeColumns );
  sourceTreeView = Gtk::manage(new class Gtk::TreeView());
  sourceTreeView->set_model ( sourceTreeModel );
  sourceTreeView->set_flags(Gtk::CAN_FOCUS);
  sourceTreeView->set_headers_visible(true);
  sourceTreeView->set_rules_hint(false);
  sourceTreeView->set_reorderable(false);
  sourceTreeView->set_enable_search(true);

  sourceTreeView->append_column("Source", sourceTreeColumns->colSource);
  sourceTreeView->append_column("Start", sourceTreeColumns->colStart);
  sourceTreeView->append_column("Last", sourceTreeColumns->colLast);
  sourceTreeView->append_column("Connected", sourceTreeColumns->colBinded);
  //  sourceTreeView->append_column("Active", sourceTreeColumns->colActive);

  Gtk::ScrolledWindow *scrolledwindow1 = Gtk::manage(new class Gtk::ScrolledWindow());
  scrolledwindow1->set_flags(Gtk::CAN_FOCUS);
  scrolledwindow1->set_shadow_type(Gtk::SHADOW_IN);
  scrolledwindow1->set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
  scrolledwindow1->property_window_placement().set_value(Gtk::CORNER_TOP_LEFT);
  scrolledwindow1->add(*sourceTreeView);


  add (*scrolledwindow1 );
  set_title ( "GlogMachine" );
  resize ( 600, 300 );

  sourceTreeView->show ();
  scrolledwindow1->show ();
  show ();

}

MainWindow::~MainWindow ()
{

}


bool MainWindow::on_delete_event ( GdkEventAny * event )
{
  fprintf ( stderr, "Main Window died !\n" );
  exit (-1); // Brute force quitting ?
  return true;
}


int MainWindow::addSource ( Glib::ustring& source )
{
  Gtk::TreeModel::Row row;
  if ( !hasRow(source) )
    {
      fprintf ( stderr, "***** New source '%s'\n", source.c_str() );
      //      row = 
      Gtk::TreeModel::Row father = *(sourceTreeModel->append() ); // findRow(source);
      father[sourceTreeColumns->colSource] = source;
      row = *(sourceTreeModel->append(father.children()));
      
    }
  else
    {
      fprintf ( stderr, "***** Source with son '%s'\n", source.c_str() );
      Gtk::TreeModel::Row father = findRow(source);
      row = *(sourceTreeModel->append(father.children()));
      
    }
  row[sourceTreeColumns->colSource] = "New Instance";
  row[sourceTreeColumns->colId] = sourceNextIndex++;
  char start_str[64];
  //  sprintf ( start_str, "%d", start );
  time_t start = time ( NULL );
  ctime_r ( &start, start_str );
  int lgt = strlen ( start_str );
  start_str[lgt-1] = '\0';
  
  row[sourceTreeColumns->colStart] = start_str;
  row[sourceTreeColumns->colLast] = start_str;  
  row[sourceTreeColumns->colBinded] = true;
  row[sourceTreeColumns->colActive] = true;
  sourceTreeView->show ();
  return (int) row[sourceTreeColumns->colId];
  //  return true;
}

Gtk::TreeModel::Row MainWindow::findRow ( Glib::ustring & source )
{
  for ( Gtk::TreeIter iter = sourceTreeModel->children().begin () ;
	iter != sourceTreeModel->children().end () ; iter ++ )
    {
      if ( (*iter)[sourceTreeColumns->colSource] == source )
	return *iter;
    }
  fprintf ( stderr, "Source not found : '%s'\n", source.c_str() );
  exit (-1);
}

Gtk::TreeModel::Row MainWindow::findRow ( Glib::ustring & source, int index )
{
  Gtk::TreeModel::Row sourceRow = findRow ( source );
  for ( Gtk::TreeIter iter = sourceRow->children().begin () ;
	iter != sourceRow->children().end () ; iter ++ )
    {
      if ( (*iter)[sourceTreeColumns->colId] == index )
	return *iter;
    }
  fprintf ( stderr, "Source not found : '%s' with index '%d'\n", source.c_str(), index );
  exit (-1);
}


bool MainWindow::hasRow ( Glib::ustring & source )
{
  for ( Gtk::TreeIter iter = sourceTreeModel->children().begin () ;
	iter != sourceTreeModel->children().end () ; iter ++ )
    {
      if ( (*iter)[sourceTreeColumns->colSource] == source )
	return true;
    }
  return false;
}


bool MainWindow::updateSource ( Glib::ustring& source, int index )
{
  time_t update = time ( NULL );
  char update_str[64];
  ctime_r ( &update, update_str );
  int lgt = strlen ( update_str );
  update_str[lgt-1] = '\0';

  (findRow (source,index))[sourceTreeColumns->colLast] = update_str;
  return true;
}

bool MainWindow::setSourceBinded ( Glib::ustring& source, int index, bool binded )
{
  (findRow (source,index))[sourceTreeColumns->colBinded] = binded;
  return true;
}
bool MainWindow::setSourceActive ( Glib::ustring& source, int index, bool active )
{
  (findRow (source,index))[sourceTreeColumns->colActive] = active;
  return true;
}

