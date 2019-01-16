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
#include <gtkmm/button.h>
#include <gtk/gtkmain.h>
#include <iostream>
#include <glog.h>

static DMLX::Keyword keyw_glog("glog");
DMLX::KeyHash hashList[] = { keyw_glog.getHash(), 0x0 };

LogView::LogView ( char * sourceFile, Glib::ustring _source )
{  
  maxRecordsViewed = 100;

  binMMap = new DMLX::BinMMap ( sourceFile );
  binMMap->setItemsToCount ( hashList );
  binMMap->setItemMapperInterval ( maxRecordsViewed );
  totalRecords = 0;
  source = _source;
  sourceNextIndex = 2; // 0 is for files, 1 is for funcs
  nextRecordId = 1;
  dontShowRecord = false;
  showRelativeTime = true;
  sourceItemRoot = NULL;
  sourceCacheVersion = 0;
  sourceTreeVersion = 0;
  maxRecordsInCache = 10000;

  Gtk::Window *logview = this;
  logview->resize ( 1024, 400 );
  gmm_data = new GlademmData(get_accel_group());
  
  // Menus
  refActionGroup = Gtk::ActionGroup::create();
  refActionGroup->add( Gtk::Action::create("ContextMenu", "Context Menu") );

  refActionGroup->add( Gtk::Action::create("LogContextUnselectLine", "Hide Line"),
		       sigc::mem_fun(*this, &LogView::logTreeUnselectLine ) );
  refActionGroup->add( Gtk::Action::create("LogContextUnselectFunc", "Hide Function"),
		       sigc::mem_fun(*this, &LogView::logTreeUnselectFunc ) );
  refActionGroup->add( Gtk::Action::create("LogContextUnselectFile", "Hide File"),
		       sigc::mem_fun(*this, &LogView::logTreeUnselectFile ) );
  logTreeToggleActionShowRelativeTime = 
    Gtk::ToggleAction::create("LogContextShowRelativeTime", "Show Relative Time");
  logTreeToggleActionShowRelativeTime->set_active ( showRelativeTime );
  refActionGroup->add( logTreeToggleActionShowRelativeTime, 
		       sigc::mem_fun(*this, &LogView::logTreeShowRelativeTime) );
  refActionGroup->add( Gtk::Action::create("LogContextClearLogs", "Clear Logs"),
		       sigc::mem_fun(*this, &LogView::logTreeClearLogs ) );

  refActionGroup->add( Gtk::Action::create("SourceContextSelectAll", "Show All"),
		       sigc::mem_fun(*this, &LogView::sourceTreeSelectAll ) );
  refActionGroup->add( Gtk::Action::create("SourceContextUnselectAll", "Hide All"),
		       sigc::mem_fun(*this, &LogView::sourceTreeUnselectAll ) );


  refUIManager = Gtk::UIManager::create();
  refUIManager->insert_action_group(refActionGroup);
  add_accel_group(refUIManager->get_accel_group());
  try
    {
      Glib::ustring ui_info = 
        "<ui>"
        "  <popup name='LogPopupMenu'>"
        "    <menuitem action='LogContextUnselectLine'/>"
        "    <menuitem action='LogContextUnselectFunc'/>"
        "    <menuitem action='LogContextUnselectFile'/>"
        "    <separator/>"
        "    <menuitem action='LogContextShowRelativeTime'/>"
        "    <separator/>"
        "    <menuitem action='LogContextClearLogs'/>"
        "  </popup>"
        "  <popup name='SourcePopupMenu'>"
        "    <menuitem action='SourceContextSelectAll'/>"
        "    <menuitem action='SourceContextUnselectAll'/>"      
        "  </popup>"
        "</ui>";
        
      refUIManager->add_ui_from_string(ui_info);
    }
  catch(const Glib::Error& ex)
    {
      std::cerr << "building menus failed: " <<  ex.what();
    }

  logContextMenu = dynamic_cast<Gtk::Menu*>( refUIManager->get_widget("/LogPopupMenu") ); 
  if(!logContextMenu)
    {
      std::cerr << "menu not found";
      exit (-1);
    }
  sourceContextMenu = dynamic_cast<Gtk::Menu*>( refUIManager->get_widget("/SourcePopupMenu") ); 
  if(!sourceContextMenu)
    {
      std::cerr << "menu not found";
      exit (-1);
    }
  
  sourceTreeColumns = new SourceTreeColumns ();
  sourceTreeModel = Gtk::TreeStore::create ( *sourceTreeColumns );
  sourceTreeView = Gtk::manage(new class Gtk::TreeView());
  sourceTreeView->set_model ( sourceTreeModel );
  sourceTreeView->set_flags(Gtk::CAN_FOCUS);
  sourceTreeView->set_headers_visible(true);
  sourceTreeView->set_rules_hint(false);
  sourceTreeView->set_reorderable(false);
  sourceTreeView->set_enable_search(true);
  sourceTreeView->append_column("Location", sourceTreeColumns->colName);
  sourceTreeView->append_column_editable("Show", sourceTreeColumns->colChecked);
  sourceTreeView->append_column("sourceId", sourceTreeColumns->colId);
  sourceTreeView->get_column(0)->set_resizable (false);
  sourceTreeView->get_column(1)->set_resizable (false);
  sourceTreeView->get_column(2)->set_resizable (true);
  
  sourceTreeView->get_column(1)->set_sizing ( Gtk::TREE_VIEW_COLUMN_FIXED );
  sourceTreeView->get_column(1)->set_fixed_width ( 25 );

  threadTreeColumns = new ThreadTreeColumns ();
  threadTreeModel = Gtk::TreeStore::create ( *threadTreeColumns );
  threadTreeView = Gtk::manage(new class Gtk::TreeView());
  threadTreeView->set_model ( threadTreeModel );
  threadTreeView->set_flags(Gtk::CAN_FOCUS);
  threadTreeView->set_headers_visible(true);
  threadTreeView->set_rules_hint(false);
  threadTreeView->set_reorderable(false);
  threadTreeView->set_enable_search(true);
  threadTreeView->append_column("Location", threadTreeColumns->colName);
  threadTreeView->append_column_editable("Checked", threadTreeColumns->colChecked);
  
  logTreeColumns = new LogTreeColumns ();
  logTreeModel = Gtk::TreeStore::create ( *logTreeColumns );
  logTreeView = Gtk::manage(new class Gtk::TreeView());
  logTreeView->set_model ( logTreeModel );
  logTreeView->set_flags(Gtk::CAN_FOCUS);
  logTreeView->set_headers_visible(true);
  logTreeView->set_rules_hint(true);
  logTreeView->set_reorderable(false);
  logTreeView->set_enable_search(true);

  
  logTreeView->append_column("sId", logTreeColumns->colSourceIndex);
  logTreeView->append_column("Id", logTreeColumns->colId);
  
  logTreeView->append_column("S", logTreeColumns->colSec);
  logTreeView->append_column("uS", logTreeColumns->colUSec);
  logTreeView->append_column("Thread", logTreeColumns->colThread);
  logTreeView->append_column("File", logTreeColumns->colFile);
  logTreeView->append_column("Line", logTreeColumns->colLine);
  logTreeView->append_column("Func", logTreeColumns->colFunc);   
  logTreeView->append_column("Level", logTreeColumns->colLevel);   
  logTreeView->append_column("Message", logTreeColumns->colMessage);   

  logTreeView->signal_row_activated().connect(sigc::mem_fun(*this, &LogView::logTreeRowActivated));


  Gtk::ScrolledWindow *scrolledwindow1 = Gtk::manage(new class Gtk::ScrolledWindow());
  Gtk::ScrolledWindow *scrolledwindow2 = Gtk::manage(new class Gtk::ScrolledWindow());
  Gtk::HPaned *hpaned1 = Gtk::manage(new class Gtk::HPaned());
  Gtk::ScrolledWindow *scrolledwindow3 = Gtk::manage(new class Gtk::ScrolledWindow());
  Gtk::VPaned *vpaned1 = Gtk::manage(new class Gtk::VPaned());
  Gtk::HBox *hbox1 = Gtk::manage(new class Gtk::HBox(false, 0));
  Gtk::VBox *vbox1 = Gtk::manage(new class Gtk::VBox(false, 0));
  Gtk::Button *logViewGotoEnd = Gtk::manage(new class Gtk::Button("End"));
  logViewGotoEnd->signal_clicked().connect(sigc::mem_fun(*this, &LogView::logViewScale_goto_end));
  logViewScale = Gtk::manage(new class Gtk::VScale(0.0, 0.0, (double) maxRecordsViewed ) );
  logViewScale->signal_value_changed().connect(sigc::mem_fun(*this, &LogView::logViewScale_value_changed));
  logViewScale->set_draw_value ( false );
  logViewScale->set_digits ( 0 );
  logViewScale->set_increments ( 1, (double) maxRecordsViewed );

  scrolledwindow1->set_flags(Gtk::CAN_FOCUS);
  scrolledwindow1->set_shadow_type(Gtk::SHADOW_IN);
  scrolledwindow1->set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
  scrolledwindow1->property_window_placement().set_value(Gtk::CORNER_TOP_LEFT);
  scrolledwindow1->add(*sourceTreeView);
  scrolledwindow2->set_flags(Gtk::CAN_FOCUS);
  scrolledwindow2->set_shadow_type(Gtk::SHADOW_IN);
  scrolledwindow2->set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
  scrolledwindow2->property_window_placement().set_value(Gtk::CORNER_TOP_LEFT);
  scrolledwindow2->add(*threadTreeView);

  hpaned1->set_flags(Gtk::CAN_FOCUS);
  hpaned1->set_position(400);
  hpaned1->pack1(*scrolledwindow1, Gtk::SHRINK);
  hpaned1->pack2(*scrolledwindow2, Gtk::EXPAND|Gtk::SHRINK);
  scrolledwindow3->set_flags(Gtk::CAN_FOCUS);
  scrolledwindow3->set_shadow_type(Gtk::SHADOW_IN);
  scrolledwindow3->set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
  scrolledwindow3->property_window_placement().set_value(Gtk::CORNER_TOP_LEFT);
  scrolledwindow3->add(*logTreeView);
  logViewScale->set_digits(0);
  vbox1->pack_start(*logViewScale, true, true, 0);
  vbox1->pack_start(*logViewGotoEnd, false, false, 5 );
  hbox1->pack_start(*scrolledwindow3, true, true, 0);
  hbox1->pack_start(*vbox1, false, false, 10 );

  vpaned1->set_size_request(-1,242);
  vpaned1->set_flags(Gtk::CAN_FOCUS);
  vpaned1->set_position(200);
  vpaned1->pack1(*hpaned1, Gtk::SHRINK);
  //  vpaned1->pack2(*scrolledwindow3, Gtk::EXPAND|Gtk::SHRINK);
  vpaned1->pack2(*hbox1, Gtk::EXPAND|Gtk::SHRINK);
  logview->set_title("Unknown");
  logview->set_modal(false);
  logview->property_window_position().set_value(Gtk::WIN_POS_NONE);
  logview->set_resizable(true);
  logview->property_destroy_with_parent().set_value(false);
  logview->add(*vpaned1);
  sourceTreeView->show();
  scrolledwindow1->show();
  threadTreeView->show();
  scrolledwindow2->show();
  logViewScale->show();
  logTreeView->show();
  hpaned1->show();
  logViewGotoEnd->show();
  vbox1->show();
  scrolledwindow3->show();
  hbox1->show();
  vpaned1->show();
  //  if ( parser != NULL )
  //    logview->show();
   
  Pango::FontDescription font_desc("times 8" ); // = msgRender->property_font_desc().get_value();
  for ( int i = 0 ; i < 8 ; i++ )
    {
      ((Gtk::CellRendererText *) logTreeView->get_column_cell_renderer ( i ))->property_font_desc().set_value(font_desc);
      logTreeView->get_column(i)->set_resizable ();
    }
  ((Gtk::CellRendererText *) sourceTreeView->get_column_cell_renderer ( 0 ))->property_font_desc().set_value(font_desc);

  sourceTreeModel->signal_row_changed().connect ( sigc::mem_fun (*this, &LogView::sourceTreeRowChanged ) );
  sourceTreeView->signal_row_activated().connect ( sigc::mem_fun (*this, &LogView::sourceTreeRowActivated ) );


  refresh ();
  /*
  if ( parser != NULL )
    setBindedState (true);
  else
    setBindedState (false);
  */
}

LogView::~LogView()
{  
  //  removeLogView ( this );
  delete gmm_data;
  delete sourceTreeColumns;
  delete sourceTreeView;
  //  delete sourceTreeModel;
  delete threadTreeColumns;
  delete threadTreeView;
  //  delete threadTreeModel;
  delete logTreeColumns;
  delete logTreeView;
  delete binMMap;
  //  delete logTreeModel;
  //  delete parser;
}


bool LogView::on_delete_event ( GdkEventAny * event )
  /*
   * Deletion event for the LogView window.
   */
{
  //  fprintf ( stderr, "Q !\n" );
  Log ( "Quitting LogView\n" );
  //  hide ();
  delete ( this );
  exit ( 0 );
  //  removeLogView ( this );
  return true;
}
