/*
	SimpleMan - Simple Download Manager
	Copyright (C) 2007 Roque Pinel, Rio de Janeiro
	Email simplemandownloader@gmail.com
*/
/*
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef SM_WIN_H
#define SM_WIN_H

#include <gtkmm/actiongroup.h>
#include <gtkmm/box.h>
#include <gtkmm/cellrendererprogress.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menubar.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/statusicon.h>
#include <gtkmm/toggleaction.h>
#include <gtkmm/treeselection.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/window.h>

#include <vector>

#include "sm_struct.h"
#include "sm_store.h"
#include "sm_model_l.h"
#include "sm_model_r.h"

#define MAX_TASKS 10


class SM_Net;

class SM_Win : public Gtk::Window
{
  public:

	SM_Win ( char ** );
	virtual ~SM_Win ( void );

		/* open cmd links */

	void download_link ( const std::string & );

		/* set functions */

	const bool set_default ( const bool & );

  private:

		/* build functions */
		
	void build_menu_bar    ( void );
	void build_tree_store  ( void );
	void build_list_store  ( void );
	void build_status_bar  ( void );
	void build_status_icon ( void );
	
		/* load functions */

	void load_config  ( void );
	void load_history ( void );
		
		/* save functions */
		
	void save_config  ( void );
	void save_history ( void );

		/* open cmd link */

	bool on_cmd_link  ( void );
	
		/* menubar events */

	virtual void on_menu_file_quit            ( void );
	virtual void on_menu_view_status          ( void );
	virtual void on_menu_view_preferences     ( void );
	virtual void on_menu_task_new             ( void );
	virtual void on_menu_task_resume          ( void );
	virtual void on_menu_task_pause           ( void );
	virtual void on_menu_task_resume_all      ( void );
	virtual void on_menu_task_pause_all       ( void );
	virtual void on_menu_task_up              ( void );
	virtual void on_menu_task_down            ( void );
	virtual void on_menu_task_remove          ( void );
	virtual void on_menu_help_about           ( void );
	
		/* group panel events */
		
	virtual void on_treeview_cursor_changed   ( void );

		/* status icon events */

	virtual void on_status_icon_activate      ( void );

		/* paused - completed - failed */

	virtual void on_download_paused    ( SM_Net * );
	virtual void on_download_completed ( SM_Net * );
	virtual void on_download_failed    ( SM_Net * );

		/* action functions */

	void  define_home                  ( void );

	void  start                        ( SM_TASK_DATA * );
	void  refresh_downloads            ( int );

	void  connect_list_timer           ( void );
	void  disconnect_list_timer        ( void );

		/* list timer event */

	bool   on_list_timer      ( void );

		
		/* data */

	bool update_list;
		
	SM_CONFIG_DATA sm_config;

	SM_Store < SM_TASK_DATA >      sm_downloadsStore;
	SM_Store < SM_COMPLETED_DATA > sm_completedsStore;
	SM_Store < SM_FAILED_DATA >    sm_failedsStore;
	SM_Store < SM_TASK_DATA >      sm_queueStore;

		/* tasks threads */

	SM_Net      *sm_thread[MAX_TASKS];
	int          sm_threadCount;

		/* widgets */

	Gtk::VBox   sm_mainBox;
	
	Gtk::HPaned sm_HPaned;
	Gtk::VPaned	sm_VPanedR;
	
		/* menubar and toolbar */

	Glib::RefPtr< Gtk::ActionGroup >  sm_refActionGroup;
	Glib::RefPtr< Gtk::UIManager >    sm_refUIManager;
	Glib::RefPtr< Gtk::ToggleAction > sm_refStatusToggle;
	
		/* tree panel - left */

	SM_Model_L                         sm_tableModel_L;
	Gtk::ScrolledWindow                sm_ScrolledWindow_L;
	Gtk::TreeView                      sm_TreeView_L;
	Glib::RefPtr< Gtk::TreeStore >     sm_refTreeModel_L;
	Glib::RefPtr< Gtk::TreeSelection > sm_refTreeSelection_L;
	
		/* list panel - right */

	SM_Model_R                         sm_Columns_R1;
	Gtk::ScrolledWindow                sm_ScrolledWindow_R1;
	Gtk::TreeView                      sm_TreeView_R1;
	Glib::RefPtr< Gtk::ListStore >     sm_refTreeModel_R1;
	Glib::RefPtr< Gtk::TreeSelection > sm_refTreeSelection_R1;
	sigc::connection                   sm_list_timer;

	int sm_listGroup;
	
		/* status bar */

	Gtk::Statusbar   sm_statusBar;
	Gtk::Label       sm_speedStatus;
	sigc::connection sm_statusBar_timer;
	
		/* xpm icon */

	Glib::RefPtr< Gdk::Pixbuf >      sm_refPixbuf_winIcon;
	
		/* system tray icon */

	Glib::RefPtr <Gtk::StatusIcon >  sm_status_icon;

		/* open cmd */

	bool               is_cmd;
	bool               is_default;
	std::string        sm_link;
	sigc::connection   sm_cmd_connect;
	char             **sm_argv;

		/* local information */
	std::string        sm_homePath;
};

#endif /* SM_WIN_H */

/* EOF */
