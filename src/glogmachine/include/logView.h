#ifndef _LOGVIEW_HH
#  define _LOGVIEW_HH

#if !defined(GLADEMM_DATA)
#define GLADEMM_DATA 
#include <gtkmm/accelgroup.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/menu.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/toggleaction.h>
using namespace std;

typedef int Socket;

class GlademmData
{  
        
        Glib::RefPtr<Gtk::AccelGroup> accgrp;
public:
        
        GlademmData(Glib::RefPtr<Gtk::AccelGroup> ag) : accgrp(ag)
        {  
        }
        
        Glib::RefPtr<Gtk::AccelGroup>  getAccelGroup()
        {  return accgrp;
        }
};
#endif //GLADEMM_DATA

#include <sys/mman.h>
// #include <glog-bin.h>
// #include <DMLXParser.h>
#include <DMLXBinMMap.h>
#include <strongmap.h>
#include <list>
#include <map>
#include <gtkmm/window.h>
#include <gtkmm/scale.h>

/*
 * TODO :
 * Use hash for the sourcesIndex
 *
 * Add a parallel cache to SourceTreeModel : SourceItems...
 */


// Update the SourceItems algorithm
// Start with root SourceItem * item
// Find the good item for level1

// Update the SourceTreeModel algorithm
// Start with root SourceItem * item, and root SourceTreeModel * row.
// If item->isNew : insert row, set item->isNew = false
// If item->hasNewSons :
// * If row has no children, add children.
// * Run algo with root SourceItem * item = item->son, root row = row.children().begin()
// Set item->hasNewSons = false
// Run algo with item->next and row++;

typedef unsigned int SourceHash;

class LogView : public Gtk::Window // : public window1_glade
{ 
  GlademmData *gmm_data;
protected:
  string source;
  Gtk::Menu * sourceContextMenu;
  Gtk::Menu * logContextMenu;
  Glib::RefPtr<Gtk::UIManager> refUIManager;
  Glib::RefPtr<Gtk::ActionGroup> refActionGroup; //  = Gtk::ActionGroup::create();
  Glib::RefPtr<Gtk::ToggleAction> logTreeToggleActionShowRelativeTime;

  class SourceTreeColumns : public Gtk::TreeModelColumnRecord
  {
  public:
    SourceTreeColumns ()
    { add(colId); add(colName); add ( colChecked); }
    Gtk::TreeModelColumn<unsigned int> colId;
    Gtk::TreeModelColumn<Glib::ustring> colName;
    Gtk::TreeModelColumn<bool> colChecked;
  };
  SourceTreeColumns * sourceTreeColumns;
  Gtk::TreeView * sourceTreeView;
  Glib::RefPtr<Gtk::TreeStore> sourceTreeModel;
  strongtriplemap<unsigned int, unsigned int, unsigned int, unsigned int> sourcesIndex;
  unsigned int sourceNextIndex;
  std::map<unsigned int, bool> sourcesChecked;
  strongmap<unsigned int,Glib::ustring *> sourceModules;
  strongmap<unsigned int,Glib::ustring *> sourceFiles;
  strongmap<unsigned int,Glib::ustring *> sourceFuncs;

  class ThreadTreeColumns : public SourceTreeColumns {};
  ThreadTreeColumns * threadTreeColumns;
  Gtk::TreeView * threadTreeView;
  Glib::RefPtr<Gtk::TreeStore> threadTreeModel;

  class LogTreeColumns : public Gtk::TreeModelColumnRecord
  {
  public:
    LogTreeColumns ()
    { add(colId); add ( colSourceIndex); add(colSec); add(colUSec); add(colThread); 
      add(colFile); add(colLine); add(colFunc); add(colLevel); add(colMessage); }
    Gtk::TreeModelColumn<unsigned int> colId;
    Gtk::TreeModelColumn<unsigned int> colSourceIndex;
    Gtk::TreeModelColumn<int> colSec;
    Gtk::TreeModelColumn<int> colUSec;
    Gtk::TreeModelColumn<Glib::ustring> colThread;
    Gtk::TreeModelColumn<Glib::ustring> colFile;
    Gtk::TreeModelColumn<int> colLine;
    Gtk::TreeModelColumn<Glib::ustring> colFunc;
    Gtk::TreeModelColumn<Glib::ustring> colLevel;
    Gtk::TreeModelColumn<Glib::ustring> colMessage;
  };
  LogTreeColumns * logTreeColumns;
  Gtk::TreeView * logTreeView;
  Glib::RefPtr<Gtk::TreeStore> logTreeModel;
  Gtk::VScale * logViewScale;
  void logViewScale_value_changed();
  void logViewScale_goto_end();
  class SourceItem
  {
  public:
    SourceItem(Glib::ustring &_name, unsigned int _sourceIndex );   
    SourceItem * son;
    SourceItem * next;
    Glib::ustring name;
    unsigned int sourceIndex;
    bool isNew; // Myself not added in SourceTreeModel
    bool hasNewSons; // Sons or descendant not yet updated in SourceTreeModel..
  };
  SourceItem * sourceItemRoot;
  SourceItem * getSourceItem ( SourceItem * father, Glib::ustring & name, unsigned int sourceIndex );
  void logSourceItem ( SourceItem * item );
  void addSource ( unsigned int module, unsigned int file, unsigned int line, unsigned int func, unsigned int sourceIndex );
  void addRecordInTree ( unsigned int recordId, unsigned int sourceId, DMLX::BinMMap::iterator &i, Gtk::TreeModel::Row &row );
  void SyncSourceItemAndSourceTreeView ( LogView::SourceItem * item, const Gtk::TreeModel::Children &children );
  void SyncSourceItemAndSourceTreeView ();
  unsigned int sourceCacheVersion;
  unsigned int sourceTreeVersion;

  unsigned int nextRecordId;

  unsigned int maxRecordsInCache;
  unsigned int maxRecordsViewed;

  unsigned int offset_t;
  unsigned int offset_tm;
  bool showRelativeTime;

  DMLX::BinMMap * binMMap;
  unsigned int totalRecords;
public:
  LogView(char * sourceFile, Glib::ustring _source );
  ~LogView();
  void init ();
  void refresh ( bool seekend = false );
  void clearLogs ();
protected:
  bool on_delete_event ( GdkEventAny * event );

  bool dontShowRecord; // Used when adding a source, do not add record.
  void showRecord ( unsigned int sourceIndex, bool show );
  bool showSource ( unsigned int sourceIndex, bool show, bool touchFile, bool touchFunc );
  int compareRecords ( const Gtk::TreeModel::iterator& iter1, const Gtk::TreeModel::iterator& iter2 );
  
  // Signal Callbacks
  
  void logTreeRowActivated ( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* /* column */);
  void logTreeUnselectLine ();
  void logTreeUnselectFunc ();
  void logTreeUnselectFile ();
  void logTreeShowRelativeTime ();
  void logTreeClearLogs ();

  void sourceTreeRowChanged ( const Gtk::TreeModel::Path& path, const Gtk::TreeModel::iterator& iter);
  void sourceTreeRowActivated ( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* /* column */);
  void sourceTreeSelectAll ();
  void sourceTreeUnselectAll ();
};

bool removeLogView ( LogView * logView ); // Implemented in glogmachine.cc

#endif
