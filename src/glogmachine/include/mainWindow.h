#ifndef _MAINWINDOW_HH
#  define _MAINWINDOW_HH

#include <strongmap.h>
#include <list>
#include <map>


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


#include <gtkmm/window.h>

class MainWindow : public Gtk::Window
{
  GlademmData *gmm_data;
protected:
  Gtk::Menu * sourceContextMenu;
  Glib::RefPtr<Gtk::UIManager> refUIManager;
  Glib::RefPtr<Gtk::ActionGroup> refActionGroup;

  class SourceTreeColumns : public Gtk::TreeModelColumnRecord
  {
  public:
    SourceTreeColumns ()
    { add(colId); add(colSource); 
      add(colStart); add (colLast);
      add(colBinded); add(colActive); }
    Gtk::TreeModelColumn<int> colId;
    Gtk::TreeModelColumn<Glib::ustring> colSource;
    Gtk::TreeModelColumn<Glib::ustring> colStart;
    Gtk::TreeModelColumn<Glib::ustring> colLast;
    Gtk::TreeModelColumn<bool> colBinded;
    Gtk::TreeModelColumn<bool> colActive;
  };
  SourceTreeColumns * sourceTreeColumns;
  Gtk::TreeView * sourceTreeView;
  Glib::RefPtr<Gtk::TreeStore> sourceTreeModel;

  bool on_delete_event ( GdkEventAny * event );
  Gtk::TreeModel::Row findRow ( Glib::ustring & source );
  Gtk::TreeModel::Row findRow ( Glib::ustring & source, int index );
  bool hasRow ( Glib::ustring & source );

public:
  int sourceNextIndex;
  MainWindow();
  ~MainWindow();

  int addSource ( Glib::ustring& source );
  bool updateSource ( Glib::ustring& source, int index );
  
  bool setSourceBinded ( Glib::ustring& source, int index, bool binded );
  bool setSourceActive ( Glib::ustring& source, int index, bool active );

};



#endif // _MAINWINDOW_HH
