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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <glibmm/thread.h>
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>

#include <fstream>

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "sm_pref.h"
#include "sm_new.h"
#include "sm_msg.h"
#include "sm_net.h"
#include "sm_util.h"
#include "sm_win.h"

#include "icons/sm_icon320.xpm"

#define SIZE_X 960
#define SIZE_Y 400

#define LIST_TIMER 300
#define CMD_TIMER  150

#define TASK_PAUSED   -1
#define TASK_ENQUEUED -2

#define OPTION_NOT_ASKED -1
#define OPTION_RESUME     0
#define OPTION_REPLACE    1

#define RUNNING_GROUP   0
#define COMPLETED_GROUP 1
#define FAILED_GROUP    2

#define CONFIG_FILE_NAME  "simpleMan_config"
#define HISTORY_FILE_NAME "simpleMan_history"
#define LOG_FILE_NAME     "simpleMan.log"

#ifdef VERSION
#define SIMPLEMAN_VERSION "Version "VERSION
#else
#define SIMPLEMAN_VERSION ""
#endif /* VERSION */

#ifndef PACKAGE_BUGREPORT
#define PACKAGE_BUGREPORT "simplemandownloader@gmail.com"
#endif /* PACKAGE_BUGREPORT */


SM_Win::SM_Win ( char **argv )
	: update_list( false ),
	  sm_threadCount ( 0 ),
	  sm_mainBox (false, 0),
	  is_cmd( false ),
	  is_default( false ),
	  sm_argv( argv )
{
		/* setting window properties */

	set_title( "Simple Manager" );
	set_default_size( SIZE_X, SIZE_Y );

	sm_refPixbuf_winIcon = Gdk::Pixbuf::create_from_xpm_data( sm_icon320_xpm );
	set_icon( sm_refPixbuf_winIcon );

		/* setting variables */

	sm_listGroup = RUNNING_GROUP;

	define_home();

		/* setting log */
	SM_Util::set_logPath( sm_homePath + LOG_FILE_NAME );
	SM_Util::check_log();
	
	load_config();

	for( int i = 0; i < MAX_TASKS; i++ )
		sm_thread[i] = NULL;

		/* adding widgets */
	
	add( sm_mainBox );

		/* building */

	build_menu_bar();
	
	sm_mainBox.pack_start( sm_HPaned );
	sm_HPaned.set_position( 123 );	
	build_tree_store();
	sm_HPaned.pack2( sm_VPanedR, false, false );
	build_list_store();
	load_history();
	
	sm_statusBar.set_size_request( -1, 25 );
	build_status_bar();
	sm_mainBox.pack_start( sm_statusBar, Gtk::PACK_SHRINK, 0 );
	sm_refStatusToggle->set_active();
	
	build_status_icon();

		/* cmd timer */
	sm_cmd_connect = Glib::signal_timeout().connect( sigc::mem_fun( *this, &SM_Win::on_cmd_link ), CMD_TIMER );

		/* list timer */
	connect_list_timer();

		/* show widgets */
	show_all_children();
}

SM_Win::~SM_Win ( void )
{
	save_history();
	
	for ( int i = 0; i < MAX_TASKS; i++ )
	{
		if ( sm_thread[i] && sm_thread[i]->isRunning() )
			delete sm_thread[i];
	}
}


	/* build functions */
		
void SM_Win::build_menu_bar   ( void )
{
		/* create actions for menus and toolbars */

	sm_refActionGroup = Gtk::ActionGroup::create();
	
		/* file menu */

	sm_refActionGroup->add( Gtk::Action::create( "FileMenu", "File" ) );
	sm_refActionGroup->add( Gtk::Action::create( "FileQuit", Gtk::Stock::QUIT ),
		sigc::mem_fun( *this, &SM_Win::on_menu_file_quit ) );

		/* view menu */

	sm_refActionGroup->add( Gtk::Action::create( "ViewMenu", "View" ) );	
	sm_refStatusToggle = Gtk::ToggleAction::create( "ViewStatus", "Show Status" );
	sm_refActionGroup->add( sm_refStatusToggle,
		sigc::mem_fun( *this, &SM_Win::on_menu_view_status ) );
	sm_refActionGroup->add(Gtk::Action::create( "ViewPreferences", Gtk::Stock::PREFERENCES ), Gtk::AccelKey( "<control><alt>P" ),
		sigc::mem_fun( *this, &SM_Win::on_menu_view_preferences ) );

		/* task menu */

	sm_refActionGroup->add( Gtk::Action::create( "TaskMenu", "Task" ) );
	sm_refActionGroup->add( Gtk::Action::create( "TaskNew", Gtk::Stock::NEW ), Gtk::AccelKey( "<control>N") ,
		sigc::mem_fun( *this, &SM_Win::on_menu_task_new ) );
	sm_refActionGroup->add( Gtk::Action::create( "TaskResume", Gtk::Stock::MEDIA_PLAY ), Gtk::AccelKey( "<control>X" ),
		sigc::mem_fun ( *this, &SM_Win::on_menu_task_resume ) );
	sm_refActionGroup->add( Gtk::Action::create( "TaskPause", Gtk::Stock::MEDIA_PAUSE ), Gtk::AccelKey( "<control>C" ),
		sigc::mem_fun( *this, &SM_Win::on_menu_task_pause ) );
	sm_refActionGroup->add(Gtk::Action::create( "TaskResumeAll", "Play All" ), Gtk::AccelKey( "<control><alt>X" ),
		sigc::mem_fun( *this, &SM_Win::on_menu_task_resume_all ) );
	sm_refActionGroup->add(Gtk::Action::create( "TaskPauseAll", "Pause All" ), Gtk::AccelKey( "<control><alt>C" ),
		sigc::mem_fun( *this, &SM_Win::on_menu_task_pause_all ) );
	sm_refActionGroup->add(Gtk::Action::create( "TaskUp", Gtk::Stock::GO_UP ),
		sigc::mem_fun( *this, &SM_Win::on_menu_task_up ) );
	sm_refActionGroup->add(Gtk::Action::create( "TaskDown", Gtk::Stock::GO_DOWN ),
		sigc::mem_fun( *this, &SM_Win::on_menu_task_down ) );
	sm_refActionGroup->add(Gtk::Action::create( "TaskRemove", Gtk::Stock::REMOVE ),
		sigc::mem_fun( *this, &SM_Win::on_menu_task_remove ) );

		/* help menu */

	sm_refActionGroup->add( Gtk::Action::create("HelpMenu", "Help" ) );
	sm_refActionGroup->add( Gtk::Action::create("HelpAbout", Gtk::Stock::ABOUT ),
		sigc::mem_fun( *this, &SM_Win::on_menu_help_about ) );
		
	sm_refUIManager = Gtk::UIManager::create();
	sm_refUIManager->insert_action_group( sm_refActionGroup );
	
	add_accel_group( sm_refUIManager->get_accel_group() );
	
	Glib::ustring ui_info = 
		"<ui>"
		"  <menubar name='MenuBar'>"
		"    <menu action='FileMenu'>"
		"      <menuitem action='FileQuit'/>"
		"    </menu>"
		"    <menu action='ViewMenu'>"
		"      <menuitem action='ViewStatus'/>"
		"      <separator/>"
		"      <menuitem action='ViewPreferences'/>"
		"    </menu>"
		"    <menu action='TaskMenu'>"
		"      <menuitem action='TaskNew'/>"
		"      <menuitem action='TaskResume'/>"
		"      <menuitem action='TaskPause'/>"
		"      <separator/>"
		"      <menuitem action='TaskResumeAll'/>"
		"      <menuitem action='TaskPauseAll'/>"
		"      <separator/>"
		"      <menuitem action='TaskUp'/>"
		"      <menuitem action='TaskDown'/>"
		"      <separator/>"
		"      <menuitem action='TaskRemove'/>"
		"    </menu>"
		"    <menu action='HelpMenu'>"
		"      <menuitem action='HelpAbout'/>"
		"    </menu>"
		"  </menubar>"
		"  <toolbar  name='ToolBar'>"
		"    <toolitem action='TaskNew'/>"
		"      <separator/>"
		"    <toolitem action='TaskResume'/>"
		"    <toolitem action='TaskPause'/>"
		"      <separator/>"
		"    <toolitem action='TaskUp'/>"
		"    <toolitem action='TaskDown'/>"
		"      <separator/>"
		"    <toolitem action='TaskRemove'/>"
		"  </toolbar>"
		"</ui>";

#ifdef GLIBMM_EXCEPTIONS_ENABLED
	try
	{      
		sm_refUIManager->add_ui_from_string( ui_info );
	}
	catch( const Glib::Error& ex )
	{
		SM_Util::msg_err( "Error: building menus failed: " +  ex.what() );
	}
#else
	std::auto_ptr< Glib::Error > ex;
	sm_refUIManager->add_ui_from_string( ui_info, ex );
	if( ex.get() )
	{ 
		SM_Util::msg_err( "Error: building menus failed: " +  ex.what() );
	}
#endif /* GLIBMM_EXCEPTIONS_ENABLED */

	Gtk::Widget* pMenubar = sm_refUIManager->get_widget("/MenuBar");
	if( pMenubar )
		sm_mainBox.pack_start( *pMenubar, Gtk::PACK_SHRINK );

	Gtk::Widget* pToolbar = sm_refUIManager->get_widget( "/ToolBar" ) ;
	if( pToolbar )
		sm_mainBox.pack_start( *pToolbar, Gtk::PACK_SHRINK );
}

void SM_Win::build_tree_store ( void )
{
		/* add the TreeView, inside a ScrolledWindow, with the button underneath */

	sm_ScrolledWindow_L.add( sm_TreeView_L );

		/* only show the scrollbars when they are necessary */

	sm_ScrolledWindow_L.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

	sm_HPaned.pack1( sm_ScrolledWindow_L, false, false );

		/* create the Tree model */

	sm_refTreeModel_L = Gtk::TreeStore::create( sm_tableModel_L );
	sm_TreeView_L.set_model( sm_refTreeModel_L );

		/* fill the TreeView's model */

	Gtk::TreeModel::Row row = *( sm_refTreeModel_L->append() );
	row[sm_tableModel_L.sm_col_name] = "SimpleMan";

	Gtk::TreeModel::Row childrow = *( sm_refTreeModel_L->append( row.children() ) );
	childrow[sm_tableModel_L.sm_col_name] = "Running";

	childrow = *( sm_refTreeModel_L->append( row.children() ) );
	childrow[sm_tableModel_L.sm_col_name] = "Completed";

	childrow = *( sm_refTreeModel_L->append( row.children() ) );
	childrow[sm_tableModel_L.sm_col_name] = "Failed";

		/* add the TreeView's view columns */

	sm_TreeView_L.append_column("", sm_tableModel_L.sm_col_name);
	
	sm_TreeView_L.expand_all();
	sm_TreeView_L.set_size_request( 123, 150 );

		/* select running row */

	Gtk::TreeModel::Path path( "0:0" );
	sm_TreeView_L.set_cursor( path );

		/* create the Select Tree */

	sm_refTreeSelection_L = sm_TreeView_L.get_selection();
	sm_refTreeSelection_L->set_mode( Gtk::SELECTION_SINGLE );

		/* connect signal */

	sm_TreeView_L.signal_cursor_changed().connect( sigc::mem_fun( *this, &SM_Win::on_treeview_cursor_changed ) );
}

void SM_Win::build_list_store ( void )
{
		/* add the TreeView, inside a ScrolledWindow, with the button underneath */

	sm_ScrolledWindow_R1.add( sm_TreeView_R1 );

		/* only show the scrollbars when they are necessary */

	sm_ScrolledWindow_R1.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

	sm_VPanedR.pack1( sm_ScrolledWindow_R1, false, false );
	
		/* add the TreeView's view columns */

	sm_TreeView_R1.append_column( "ID", sm_Columns_R1.sm_col_id );
	sm_TreeView_R1.append_column( "File Name", sm_Columns_R1.sm_col_name );
	sm_TreeView_R1.append_column( "Status", sm_Columns_R1.sm_col_status );
	sm_TreeView_R1.append_column( "Size", sm_Columns_R1.sm_col_size );
	sm_TreeView_R1.append_column( "Completed", sm_Columns_R1.sm_col_completed );

		/* display a progress bar instead of a decimal number */

	Gtk::CellRendererProgress* cell = new Gtk::CellRendererProgress;
	int column_num = sm_TreeView_R1.append_column( "Progress", *cell );
	Gtk::TreeViewColumn* pColumn = sm_TreeView_R1.get_column( column_num - 1 );
	if( pColumn )
	{
#ifdef GLIBMM_PROPERTIES_ENABLED
		pColumn->add_attribute( cell->property_value(), sm_Columns_R1.sm_col_percentage );
#else
		pColumn->add_attribute( *cell, "value", sm_Columns_R1.sm_col_percentage );
#endif /* GLIBMM_PROPERTIES_ENABLED */
	}

	sm_TreeView_R1.append_column( "Speed", sm_Columns_R1.sm_col_speed );
	sm_TreeView_R1.append_column( "Time Left", sm_Columns_R1.sm_col_time );
	column_num = sm_TreeView_R1.append_column( "URL", sm_Columns_R1.sm_col_url );

	for( int i = 0; i < column_num; i++ )
	{
		Gtk::TreeView::Column* pColumn = sm_TreeView_R1.get_column( i );
		pColumn->set_reorderable();
		pColumn->set_resizable();
		pColumn->set_clickable();
		pColumn->set_sizing( Gtk::TREE_VIEW_COLUMN_AUTOSIZE );

		switch( i )
		{
				/* id */
			case 0:
				pColumn->set_min_width( 30 );
				pColumn->set_sort_order( Gtk::SORT_ASCENDING );
				break;
				/* name */
			case 1:
				pColumn->set_min_width( 230 );
				break;
				/* status */
			case 2:
				pColumn->set_min_width( 90 );
				break;
				/* size */
			case 3:
				pColumn->set_min_width( 90 );
				break;
				/* completed */
			case 4:
				pColumn->set_min_width( 90 );
				break;
				/* percentage */
			case 5:
				pColumn->set_min_width( 90 );
				break;
				/* speed */
			case 6:
				pColumn->set_min_width( 70 );
				break;
				/* time */
			case 7:
				pColumn->set_min_width( 70 );
				break;
				/* url */
			case 8:
				pColumn->set_min_width( 300 );
				break;
			default:
				break;
		}
	}

		/* create the tree model */

	sm_refTreeModel_R1 = Gtk::ListStore::create( sm_Columns_R1 );
	sm_TreeView_R1.set_model( sm_refTreeModel_R1 );
	sm_TreeView_R1.set_size_request( 123, 150 );
	
		/* create the select tree */

	sm_refTreeSelection_R1 = sm_TreeView_R1.get_selection();
	sm_refTreeSelection_R1->set_mode( Gtk::SELECTION_SINGLE );
}

void SM_Win::build_status_bar ( void )
{
	sm_statusBar.pack_start( sm_speedStatus, Gtk::PACK_SHRINK, 0 );

	sm_speedStatus.set_label( "Speed: 0 KB/s" );
}

void SM_Win::build_status_icon ( void )
{
	sm_status_icon = Gtk::StatusIcon::create( sm_refPixbuf_winIcon );
	sm_status_icon->set_tooltip(" SimpleMan Downloader" );

	sm_status_icon->signal_activate().connect( sigc::mem_fun( *this, &SM_Win::on_status_icon_activate ) );
}


	/* load functions */

void SM_Win::load_config (void)
{
	if( access( ( sm_homePath + CONFIG_FILE_NAME ).c_str(), R_OK ) != -1 )
	{
		std::ifstream inFile ( ( sm_homePath + CONFIG_FILE_NAME ).c_str(), std::ios::binary );
		bool tab = true;
		while( inFile && !inFile.eof() )
		{
			std::string line;
			getline(inFile, line);
			unsigned int posA;
			if( tab && line.find( "[preferences]" ) != std::string::npos )
				tab = false;
			
			else if( ! tab )
			{
				if( ( posA = line.find( "default_path=" ) ) != std::string::npos )
				{
					posA += 13;
					sm_config.defaultPath = line.substr(posA, line.length());
				}
				else if( (posA = line.find( "tasks_limit=" ) ) != std::string::npos )
				{
					posA += 12;
					sm_config.tasksLimit = atoi( ( line.substr( posA, line.length() ) ).c_str() );
				}
				else if( ( posA = line.find( "speed_limit=" ) ) != std::string::npos )
				{
					posA += 12;
					sm_config.speedLimit = atoi( ( line.substr( posA, line.length() ) ).c_str() );
				}
				else if( ( posA = line.find( "default_pass=" ) ) != std::string::npos )
				{
					posA += 13;
					sm_config.defaultPass = line.substr(posA, line.length());
				}
				else if( ( posA = line.find( "start_now=" ) ) != std::string::npos )
				{
					posA += 10;
					sm_config.startNow = atoi( ( line.substr( posA, line.length() ) ).c_str() );
					tab = true;
				}
			}
		}
		inFile.close();
	}
	else
	{
		sm_config.defaultPath = sm_homePath;
		sm_config.tasksLimit  = 0;
		sm_config.speedLimit  = 0;
		sm_config.startNow    = true;
		sm_config.defaultPass = PACKAGE_BUGREPORT;
	}
}

void SM_Win::load_history ( void )
{
	if( access( ( sm_homePath + HISTORY_FILE_NAME ).c_str(), R_OK ) == -1 )
	{
		return;
	}

	std::ifstream inFile ( ( sm_homePath + HISTORY_FILE_NAME ).c_str(), std::ios::binary );
	bool tab = true;
	Gtk::TreeModel::Row row;
	SM_TASK_DATA *sm_download;

	while( inFile && ! inFile.eof() )
	{
		std::string line;
		getline( inFile, line );
		unsigned int posA;
		if( tab && line[0] == '[' )
		{
			Gtk::TreeModel::iterator iter = sm_refTreeModel_R1->append();
			row = *( iter );
			sm_download = new SM_TASK_DATA;
			tab = false;
		}
		else if( ! tab )
		{
			if( ( posA = line.find( "name=" ) ) != std::string::npos )
			{
				posA += 5;
				row[sm_Columns_R1.sm_col_name] = line.substr( posA, line.length() );
				sm_download->name = line.substr( posA, line.length() );
			}
			else if( ( posA = line.find( "size=" ) ) != std::string::npos )
			{
				posA += 5;
				row[sm_Columns_R1.sm_col_size] = line.substr( posA, line.length() );
				sm_download->size = atoi( ( line.substr( posA, line.length() ) ).c_str() );
			}
			else if( ( posA = line.find( "completed=" ) ) != std::string::npos )
			{
				posA += 10;
				row[sm_Columns_R1.sm_col_completed] = line.substr( posA, line.length() );
				sm_download->completed = atoi( ( line.substr( posA, line.length() ) ).c_str() );
			}
			else if( ( posA = line.find( "url=" ) ) != std::string::npos )
			{
				posA += 4;
				row[sm_Columns_R1.sm_col_url] = line.substr( posA, line.length() );
				sm_download->url = line.substr( posA, line.length() );
			}
			else if( ( posA = line.find( "user=" ) ) != std::string::npos )
			{
				posA += 5;
				sm_download->user = line.substr( posA, line.length() );
			}
			else if( ( posA = line.find( "pass=" ) ) != std::string::npos )
			{
				posA += 5;
				sm_download->pass = line.substr( posA, line.length() );
			}
			else if( ( posA = line.find( "path=" ) ) != std::string::npos )
			{
				posA += 5;
				sm_download->t = TASK_PAUSED;
				sm_download->id = sm_downloadsStore.get_size();
				sm_download->speed = 0;
				sm_download->dir = line.substr( posA, line.length() );
				sm_download->listPath = SM_Util::itoa( sm_downloadsStore.get_size() ) + ":0";

				row[sm_Columns_R1.sm_col_id]        = sm_downloadsStore.get_size();				
				row[sm_Columns_R1.sm_col_status]    = "paused";

				sm_download->percentage = 0;
				sm_download->option = OPTION_NOT_ASKED;
				if( sm_download->size > 0 )
				{
					sm_download->percentage = ( ( ( double ) sm_download->completed ) / sm_download->size ) * 100;
					sm_download->option = OPTION_RESUME;
				}					

				if( sm_download->percentage >= 100 )
					sm_download->percentage = 100;
				row[sm_Columns_R1.sm_col_percentage] = sm_download->percentage;

				row[sm_Columns_R1.sm_col_speed] = "";
				row[sm_Columns_R1.sm_col_time]  = "";

				sm_downloadsStore.push_back( sm_download );
				tab = true;
			}
		}
	}
	inFile.close();
}

	
	/* save functions */

void SM_Win::save_config ( void )
{
	if( access( sm_homePath.c_str(), W_OK ) == -1 )
	{
		if( mkdir( sm_homePath.c_str(), O_CREAT ) == -1 )
		{
			SM_Util::msg_err( "Error: could not access config directory." );
			return;
		}
		else
			chmod( sm_homePath.c_str(), S_IWUSR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );
	}

	std::ofstream outFile ( ( sm_homePath + CONFIG_FILE_NAME ).c_str(), std::ios::trunc | std::ios::binary );
	if( outFile )
	{
		outFile << "[preferences]" << std::endl;
		outFile << "default_path=" << sm_config.defaultPath << std::endl;
		outFile << "tasks_limit="  << sm_config.tasksLimit  << std::endl;
		outFile << "speed_limit="  << sm_config.speedLimit  << std::endl;
		outFile << "default_pass=" << sm_config.defaultPass << std::endl;
		outFile << "start_now="    << sm_config.startNow    << std::endl;
	}
	outFile.close();
}

void SM_Win::save_history ( void )
{
	if( access( sm_homePath.c_str(), W_OK ) == -1 )
	{
		if( mkdir( sm_homePath.c_str(), O_CREAT ) == -1 )
		{
			SM_Util::msg_err( "Error: could not access config directory." );
			return;
		}
		else
			chmod( sm_homePath.c_str(), S_IWUSR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );
	}

	std::ofstream outFile ( ( sm_homePath + HISTORY_FILE_NAME ).c_str(), std::ios::trunc | std::ios::binary );

	struct SM_TASK_STORE < SM_TASK_DATA > *temp = sm_downloadsStore.get_first();
	for( int i = 0; temp; i++, temp = temp->next )
	{
		SM_TASK_DATA *sm_download = temp->data;
		outFile << "[" << i << "]" << std::endl;
		outFile << "name="         << sm_download->name      << std::endl;
		outFile << "size="         << sm_download->size      << std::endl;
		outFile << "completed="    << sm_download->completed << std::endl;
		outFile << "url="          << sm_download->url       << std::endl;
		outFile << "user="         << sm_download->user      << std::endl;
		outFile << "pass="         << sm_download->pass      << std::endl;
		outFile << "path="         << sm_download->dir       << std::endl << std::endl;
	}
	outFile.close();
}


	/* open cmd link */

void SM_Win::download_link ( const std::string &new_link )
{
	sm_link = new_link;
	is_cmd = true;
	on_menu_task_new();
}

bool SM_Win::on_cmd_link ( void )
{
	int first = 1;
	bool url = false;
	sm_cmd_connect.disconnect();

	if( sm_argv[first] )
	{
		if( strcmp( sm_argv[first], "--default" ) == 0 || strcmp( sm_argv[first], "-d" ) == 0 )
		{
			first = 2;
			is_default = true;
		}
	}
	for( int i = first; sm_argv[i]; i++ )
	{
		if( url )
		{
			download_link( sm_argv[i] );
			url = false;
		}
		else if( strcmp( sm_argv[i], "--url" ) == 0 || strcmp( sm_argv[i], "-u" ) == 0 )
		{
			url = true;
			continue;
		}
	}
	if( url )
		return false;
	is_default = false;
	return true;
}


	/* set functions */

const bool SM_Win::set_default ( const bool &new_is_default )
{
	is_default = new_is_default;
	return true;
}


	/* menubar events */

void SM_Win::on_menu_file_quit ( void )
{
	hide();
}

void SM_Win::on_menu_view_status ( void )
{
	if( sm_refStatusToggle->get_active() )
	{
		if( sm_statusBar.get_parent() == NULL )
			sm_mainBox.pack_start( sm_statusBar, Gtk::PACK_SHRINK, 0 );
	}
	else
	{
		if( sm_statusBar.get_parent() != NULL )
			sm_mainBox.remove( sm_statusBar );
	}
}

void SM_Win::on_menu_view_preferences ( void )
{
	SM_Pref *sm_preferences = new SM_Pref( *this, sm_config );
	if( sm_preferences->run() == 1 )
	{
		std::string defaultPath = sm_preferences->get_dir();
		sm_config.tasksLimit  = sm_preferences->get_tasksLimit();
		sm_config.speedLimit  = sm_preferences->get_speedLimit();
		sm_config.defaultPass = sm_preferences->get_pass();
		sm_config.startNow    = sm_preferences->get_start();

		if( sm_config.speedLimit != 0 )
		{
			for ( int i = 0; i < MAX_TASKS; i++ )
			{
				if ( sm_thread[i] && sm_thread[i]->isRunning() )
					sm_thread[i]->set_speedLimit( sm_config.speedLimit / sm_threadCount );
			}
		}
		if( ( access( defaultPath.c_str(), R_OK ) ) == -1 )
		{
			std::string title = "Directory Error";
			std::string msg   = "Directory does not exist,\n"
								"would like to create?";

			SM_Msg *sm_msg = new SM_Msg( *this, title, msg, true );
			int awser = sm_msg->run_msg();
			delete sm_msg;

			if( awser )
			{
				if( mkdir( defaultPath.c_str(), 0766 ) == 0 )
				{
					sm_config.defaultPath = defaultPath;

					SM_Util::msg_out( "Preferences: directory created." );
				}
			}
		}
		else
		{
			DIR *dir_ptr = opendir ( defaultPath.c_str() );
			if( dir_ptr )
			{
				( void ) closedir ( dir_ptr );
				sm_config.defaultPath = defaultPath;
			}
			else
			{
				std::string title = "Directory Error";
				std::string msg   = "Could not change\n"
									"directory.";
				SM_Msg *sm_msg = new SM_Msg( *this, title, msg, false );
				sm_msg->run_msg();
				delete sm_msg;
			}
		}
		save_config();
	}
	delete sm_preferences;
}

void SM_Win::on_menu_task_new ( void )
{
	SM_New *sm_new = new SM_New( *this, sm_config );

	if( is_cmd )
	{
		sm_new->set_url( sm_link );
		is_cmd = false;
	}

	if( is_default || sm_new->run() == 1 )
	{
		std::string dir = sm_config.defaultPath;
		bool startNow = sm_new->get_start();

		if( ( access( ( sm_new->get_dir() ).c_str(), R_OK ) ) == -1 )
		{
			std::string title = "Directory Error";
			std::string msg   = "Directory does not exist,\n"
								"would like to create?";

			SM_Msg *sm_msg = new SM_Msg( *this, title, msg, true );
			int awser = sm_msg->run_msg();
			delete sm_msg;

			if( awser )
			{
				if( mkdir( ( sm_new->get_dir() ).c_str(), 0766 ) == 0 )
				{
					dir= sm_new->get_dir();
					SM_Util::msg_out( "New Task: directory created." );
				}
			}
		}
		else
		{
			DIR *dir_ptr = opendir ( ( sm_new->get_dir() ).c_str() );
			if( dir_ptr )
			{
				( void ) closedir ( dir_ptr );
				dir = sm_new->get_dir();
			}
			else
			{
				std::string title = "Directory Error";
				std::string msg   = "Could not change\n"
									"directory.";
				SM_Msg *sm_msg = new SM_Msg( *this, title, msg, false );
				sm_msg->run_msg();
				delete sm_msg;
			}
		}

		SM_TASK_DATA *sm_download = new SM_TASK_DATA;
		sm_download->t          = TASK_PAUSED;
		sm_download->option     = OPTION_NOT_ASKED;
		sm_download->id         = sm_downloadsStore.get_size();
		sm_download->name       = sm_new->get_name();
		sm_download->size       = 0;
		sm_download->completed  = 0;
		sm_download->percentage = 0;
		sm_download->speed      = 0;
		sm_download->time       = "";
		sm_download->url        = sm_new->get_url();
		sm_download->user       = sm_new->get_user();
		sm_download->pass       = sm_new->get_pass();
		sm_download->dir        = dir;
		sm_download->listPath   = SM_Util::itoa( sm_downloadsStore.get_size() ) + ":0";

		if( sm_listGroup == RUNNING_GROUP )
		{
			Gtk::TreeModel::Row row              = *( sm_refTreeModel_R1->append() );
			row[sm_Columns_R1.sm_col_id]         = sm_downloadsStore.get_size();
			row[sm_Columns_R1.sm_col_name]       = sm_download->name;
			row[sm_Columns_R1.sm_col_status]     = "paused";
			row[sm_Columns_R1.sm_col_size]       = "";
			row[sm_Columns_R1.sm_col_completed]  = "";
			row[sm_Columns_R1.sm_col_percentage] = 0;
			row[sm_Columns_R1.sm_col_speed]      = "";
			row[sm_Columns_R1.sm_col_time]       = "";
			row[sm_Columns_R1.sm_col_url]        = sm_download->url;
		}

		sm_downloadsStore.push_back( sm_download );

		if( startNow )
			start( sm_download );
		else
			save_history();
	}
	delete sm_new;
}

void SM_Win::on_menu_task_resume ( void )
{
	Glib::RefPtr< Gtk::TreeModel > sm_refTreeMode = sm_TreeView_R1.get_model();
			
	if( sm_listGroup == RUNNING_GROUP )
	{
		Gtk::TreeModel::iterator iter = sm_refTreeSelection_R1->get_selected();
		if( iter )
		{
			Gtk::TreeModel::Row row = *iter;
			SM_TASK_DATA *sm_download = sm_downloadsStore.get_data( row[sm_Columns_R1.sm_col_id] );

			if( sm_download->t == TASK_PAUSED )
				start( sm_download );
		}
	}
}

void SM_Win::on_menu_task_pause ( void )
{
	Glib::RefPtr< Gtk::TreeModel > sm_refTreeMode = sm_TreeView_R1.get_model();
			
	if( sm_listGroup == RUNNING_GROUP )
	{
		bool change = false;
		Gtk::TreeModel::iterator iter = sm_refTreeSelection_R1->get_selected();
		if( iter )
		{
			Gtk::TreeModel::Row row = *iter;
			SM_TASK_DATA *sm_download = sm_downloadsStore.get_data( row[sm_Columns_R1.sm_col_id] );

				/* try to pause a enqueued item */
			if( sm_download->t == TASK_ENQUEUED )
			{
				struct SM_TASK_STORE < SM_TASK_DATA > *temp = sm_queueStore.get_first();
				for( int i = 0; temp; i++, temp = temp->next )
				{
					SM_TASK_DATA *download = temp->data;
					if( download->id  == sm_download->id )
					{
						download->t = TASK_PAUSED;
						row[sm_Columns_R1.sm_col_status] = "paused";
						sm_queueStore.erase( i );
						if( ! change )
							change = true;
						break;
					}
				}
			}
				/* try to pause a running item */
			else if( sm_download->t != TASK_PAUSED )
			{
				int t = sm_download->t;
				sm_download->t = TASK_PAUSED;

				sm_thread[t]->set_threadID( MAX_TASKS );
				sm_thread[t]->stop();

				sm_download->completed = sm_thread[t]->get_comSize();

				if( ! change )
					change = true;
			}
			if( change )
				save_history();
		}
	}
}

void SM_Win::on_menu_task_resume_all ( void )
{
	struct SM_TASK_STORE < SM_TASK_DATA > *temp = sm_downloadsStore.get_first();
	while( temp )
	{
		SM_TASK_DATA *sm_download = temp->data;
		temp = temp->next;

		if( sm_download->t == TASK_PAUSED )
			start( sm_download );
	}
}

void SM_Win::on_menu_task_pause_all ( void )
{
	if( sm_threadCount == 0 )
		return;
	bool change = false;

		/* try to pause a enqueued item */

	struct SM_TASK_STORE < SM_TASK_DATA > *temp = sm_queueStore.get_first();
	for( int i = 0; temp; i++, temp = temp->next )
	{
		SM_TASK_DATA *sm_download = temp->data;
		sm_download->t = TASK_PAUSED;

		Gtk::TreeModel::Path path ( sm_download->listPath );
		Gtk::TreeModel::iterator iter = sm_refTreeModel_R1->get_iter( path );
		Gtk::TreeModel::Row row = *iter;

		row[sm_Columns_R1.sm_col_status] = "paused";

		if( ! change )
			change = true;
	}
	if( ! sm_queueStore.clear() )
		SM_Util::msg_err( "Error: could not clear task queue." );

		/* try to pause a running item */

	temp = sm_downloadsStore.get_first();
	while( temp )
	{
		SM_TASK_DATA *sm_download = temp->data;
		temp = temp->next;
		if( sm_download->t != TASK_PAUSED && sm_download->t != TASK_ENQUEUED )
		{
			int t = sm_download->t;
			sm_download->t = TASK_PAUSED;

			sm_thread[t]->set_threadID( MAX_TASKS );
			sm_thread[t]->stop();

			sm_download->completed = sm_thread[t]->get_comSize();

			if( ! change )
				change = true;
		}
	}
	if( change )
		save_history();
}

void SM_Win::on_menu_task_up ( void )
{
	Glib::RefPtr<Gtk::TreeModel> sm_refTreeMode = sm_TreeView_R1.get_model();
			
	if( sm_listGroup == RUNNING_GROUP )
	{
		Gtk::TreeModel::iterator iter1 = sm_refTreeSelection_R1->get_selected();
		if( iter1 )
		{
			Gtk::TreeModel::Row row1 = *iter1;
			int id = row1[sm_Columns_R1.sm_col_id];
			Gtk::TreeModel::iterator iter2 = iter1;
			iter2--;
			if( iter2 && id > 0 )
			{
				Gtk::TreeModel::Row row2 = *iter2;

				SM_TASK_DATA *download1 = sm_downloadsStore.get_data( id );
				SM_TASK_DATA *download2 = sm_downloadsStore.get_data( row2[sm_Columns_R1.sm_col_id] );

				download1->id = download2->id;
				download2->id = id;

				if( download1->t != TASK_PAUSED && download1->t != TASK_ENQUEUED )
					sm_thread[download1->t]->set_id( download1->id );
				if( download2->t != TASK_PAUSED && download2->t != TASK_ENQUEUED )
					sm_thread[download2->t]->set_id( download2->id );

				std::string pathString = download1->listPath;
				download1->listPath = download2->listPath;
				download2->listPath = pathString;

				sm_downloadsStore.swap( download1->id, download2->id );

					/* change postions */

				sm_refTreeModel_R1->iter_swap( iter1, iter2 );

					/* change id */

				row1[sm_Columns_R1.sm_col_id] = download1->id;
				row2[sm_Columns_R1.sm_col_id] = download2->id;

					/* swap status: enqueued <-> running */
				if( download1->t == TASK_ENQUEUED && download2->t != TASK_PAUSED && download2->t != TASK_ENQUEUED )
				{
						/* insert the running item after the first enqueued item, so as to be executed imidiately */
					sm_queueStore.insert( 1, download2 );

					int t = download2->t;
					download2->t = TASK_ENQUEUED;

					sm_thread[t]->set_threadID( MAX_TASKS );
					sm_thread[t]->stop();

					download2->completed = sm_thread[t]->get_comSize();
				}
				save_history();
			}
		}
	}
}

void SM_Win::on_menu_task_down ( void )
{
	Glib::RefPtr<Gtk::TreeModel> sm_refTreeMode = sm_TreeView_R1.get_model();
			
	if( sm_listGroup == RUNNING_GROUP )
	{
		Gtk::TreeModel::iterator iter1 = sm_refTreeSelection_R1->get_selected();
		if( iter1 )
		{
			Gtk::TreeModel::Row row1 = *iter1;
			int id = row1[sm_Columns_R1.sm_col_id];
			Gtk::TreeModel::iterator iter2 = iter1;
			iter2++;
			if( iter2 )
			{
				Gtk::TreeModel::Row row2 = *iter2;

				SM_TASK_DATA *download1 = sm_downloadsStore.get_data( id );
				SM_TASK_DATA *download2 = sm_downloadsStore.get_data( row2[sm_Columns_R1.sm_col_id] );

				download1->id = download2->id;
				download2->id = id;

				if( download1->t != TASK_PAUSED && download1->t != TASK_ENQUEUED )
					sm_thread[download1->t]->set_id( download1->id );
				if( download2->t != TASK_PAUSED && download2->t != TASK_ENQUEUED )
					sm_thread[download2->t]->set_id( download2->id );

				std::string pathString = download1->listPath;
				download1->listPath = download2->listPath;
				download2->listPath = pathString;

				sm_downloadsStore.swap( download1->id, download2->id );

					/* change postions */

				sm_refTreeModel_R1->iter_swap( iter1, iter2 );

					/* change id */

				row1[sm_Columns_R1.sm_col_id] = download1->id;
				row2[sm_Columns_R1.sm_col_id] = download2->id;


					/* swap status: running <-> enqueued */
				if( download1->t != TASK_PAUSED && download1->t != TASK_ENQUEUED && download2->t == TASK_ENQUEUED )
				{
						/* insert the running item after the first enqueued item, so as to be executed imidiately */
					sm_queueStore.insert( 1, download1 );

					int t = download1->t;
					download1->t = TASK_ENQUEUED;

					sm_thread[t]->set_threadID( MAX_TASKS );
					sm_thread[t]->stop();

					download1->completed = sm_thread[t]->get_comSize();
				}
				save_history();
			}
		}
	}
}

void SM_Win::on_menu_task_remove ( void )
{
	Gtk::TreeModel::iterator iter = sm_refTreeSelection_R1->get_selected();
	if( ! iter )
		return;
	Gtk::TreeModel::Row row = *iter;
	int id = row[sm_Columns_R1.sm_col_id];

	Glib::RefPtr<Gtk::TreeModel> sm_refTreeMode = sm_TreeView_R1.get_model();
	if( sm_listGroup == RUNNING_GROUP )
	{
		SM_TASK_DATA *sm_download = sm_downloadsStore.get_data( id );

		if( sm_download->t != TASK_PAUSED )
		{
			std::string title = "Remove Alert";
			std::string msg = "You must stop the download\n"
						    	  "before remove it";
			SM_Msg *sm_msg = new SM_Msg( *this, title, msg, false );
			sm_msg->run_msg();
			delete sm_msg;
		}
		else
		{
			sm_refTreeModel_R1->erase( iter );
			refresh_downloads( id );
			sm_downloadsStore.erase( id );
			delete sm_download;
			save_history();
		}
	}
	else if( sm_listGroup == COMPLETED_GROUP )
	{
		sm_refTreeModel_R1->erase( iter );

		std::string listPath;
		struct SM_TASK_STORE < SM_COMPLETED_DATA > *temp = sm_completedsStore.get_first();
		for( int i = 0; temp; i++, temp = temp->next )
		{
			SM_COMPLETED_DATA *sm_completed = temp->data;
			if( i == id )
				listPath = sm_completed->listPath;
			else if( i > id )
			{
				std::string temp = sm_completed->listPath;
				sm_completed->listPath = listPath;
				listPath = temp;
			}
		}
		sm_completedsStore.erase( id );
		update_list = true;
	}
	else if( sm_listGroup == FAILED_GROUP )
	{
		sm_refTreeModel_R1->erase( iter );

		std::string listPath;
		struct SM_TASK_STORE < SM_FAILED_DATA > *temp = sm_failedsStore.get_first();
		for( int i = 0; temp; i++, temp = temp->next )
		{
			SM_FAILED_DATA *sm_failed = temp->data;
			if( i == id )
				listPath = sm_failed->listPath;
			else if( i > id )
			{
				std::string temp = sm_failed->listPath;
				sm_failed->listPath = listPath;
				listPath = temp;
			}
		}
		sm_failedsStore.erase( id );
		update_list = true;
	}
}

void SM_Win::on_menu_help_about ( void )
{
	Gtk::Dialog sm_about ( "About", *this, false, false );
	std::string sm_msg = "                      "SIMPLEMAN_VERSION"\n"
						"Simple Manager - Download Manager\n"
						"Developed Using C++ and GTKmm\n"
						"Copyright ©2007 Roque Pinel\n"
						PACKAGE_BUGREPORT;
	Gtk::Label sm_Label ( sm_msg, false );
	Gtk::Image sm_logo ( sm_refPixbuf_winIcon );
	Gtk::VBox *sm_aboutBox = sm_about.get_vbox();
	sm_aboutBox->set_homogeneous( false );
	sm_aboutBox->set_spacing( 0 );
	sm_aboutBox->pack_start( sm_logo, Gtk::PACK_SHRINK, 0 );
	sm_aboutBox->pack_start( sm_Label, Gtk::PACK_SHRINK, 0 );
	sm_about.add_button( Gtk::Stock::CLOSE, 1 );
	sm_about.set_default_response( 1 );	
	sm_about.activate_default();
	sm_about.show_all_children();
	sm_about.run();
}


	/* treeview events */

void SM_Win::on_treeview_cursor_changed ( void )
{
	Gtk::TreeModel::iterator iter = sm_refTreeSelection_L->get_selected();
	if(iter)
	{
		Gtk::TreeModel::Row row = *iter;
		Glib::ustring tabName = row[sm_tableModel_L.sm_col_name];
		if( tabName.compare( "Running" ) == 0 )
		{
			sm_listGroup = RUNNING_GROUP;
			update_list = true;
		}
		else if ( tabName.compare( "Completed" ) == 0 )
		{
			sm_listGroup = COMPLETED_GROUP;
			update_list = true;
		}
		else if ( tabName.compare( "Failed" ) == 0 )
		{
			sm_listGroup = FAILED_GROUP;
			update_list = true;
		}
	}
}


	/* status icon events */

void SM_Win::on_status_icon_activate ( void )
{
	if( is_visible() )
	{
//		SM_Util::msg_out( "Window on." );
	}
	else
	{
//		SM_Util::msg_out( "Window off." );
	}
}


	/* list timer event */

bool SM_Win::on_list_timer ( void )
{
	if( sm_listGroup == RUNNING_GROUP )
	{
		if( update_list )
		{
			sm_refTreeModel_R1->clear();
			struct SM_TASK_STORE < SM_TASK_DATA > *temp = sm_downloadsStore.get_first();

			while( temp )
			{
				SM_TASK_DATA *sm_download = temp->data;
				temp = temp->next;

				Gtk::TreeModel::Row row              = *( sm_refTreeModel_R1->append() );
				row[sm_Columns_R1.sm_col_id]         = sm_download->id;
				row[sm_Columns_R1.sm_col_name]       = sm_download->name;
				if( sm_download->t == TASK_PAUSED )
					row[sm_Columns_R1.sm_col_status] = "paused";
				else if ( sm_download->t == TASK_ENQUEUED )
					row[sm_Columns_R1.sm_col_status] = "enqueued";
				else
					row[sm_Columns_R1.sm_col_status] = "running";
				row[sm_Columns_R1.sm_col_size]       = SM_Util::show_size( sm_download->size );
				row[sm_Columns_R1.sm_col_completed]  = SM_Util::show_size( sm_download->completed );
				row[sm_Columns_R1.sm_col_percentage] = sm_download->percentage;
				if( sm_download->t == TASK_PAUSED || sm_download->t == TASK_ENQUEUED )
				{
					row[sm_Columns_R1.sm_col_speed]  = "";
					row[sm_Columns_R1.sm_col_time]   = "";
				}
				else
				{
					row[sm_Columns_R1.sm_col_speed]  = SM_Util::show_speed( sm_download->speed );
					row[sm_Columns_R1.sm_col_time]   = sm_download->time;
				}
				row[sm_Columns_R1.sm_col_url]        = sm_download->url;				
			}
			update_list = false;
		}
		else
		{
			for( int i = 0; i < MAX_TASKS; i++ )
			{
				if( sm_thread[i] && sm_thread[i]->isRunning() && sm_listGroup == RUNNING_GROUP )
				{
					Gtk::TreeModel::Path path ( sm_thread[i]->get_listPath() );
					Gtk::TreeModel::iterator iter = sm_refTreeModel_R1->get_iter( path );
					Gtk::TreeModel::Row row       = *iter;

					row[sm_Columns_R1.sm_col_status]     = "running";
					row[sm_Columns_R1.sm_col_size]       = SM_Util::show_size( sm_thread[i]->get_totalSize() );
					row[sm_Columns_R1.sm_col_completed]  = SM_Util::show_size( sm_thread[i]->get_comSize() );
					row[sm_Columns_R1.sm_col_percentage] = sm_thread[i]->get_percentage();
					row[sm_Columns_R1.sm_col_speed]      = SM_Util::show_speed( sm_thread[i]->get_speed() );
					row[sm_Columns_R1.sm_col_time]       = sm_thread[i]->get_time();
				}
			}
		}
	}
	else if( sm_listGroup == COMPLETED_GROUP && update_list )
	{
		sm_refTreeModel_R1->clear();
		struct SM_TASK_STORE < SM_COMPLETED_DATA > *temp = sm_completedsStore.get_first();
		for( int i = 0; temp; i++, temp = temp->next )
		{
			SM_COMPLETED_DATA *sm_completed = temp->data;

			Gtk::TreeModel::Row row              = *( sm_refTreeModel_R1->append() );
			row[sm_Columns_R1.sm_col_id]         = i;
			row[sm_Columns_R1.sm_col_name]       = sm_completed->name;
			row[sm_Columns_R1.sm_col_status]     = "completed";
			row[sm_Columns_R1.sm_col_size]       = sm_completed->size;
			row[sm_Columns_R1.sm_col_completed]  = sm_completed->size;
			row[sm_Columns_R1.sm_col_percentage] = 100;
			row[sm_Columns_R1.sm_col_speed]      = "";
			row[sm_Columns_R1.sm_col_time]       = "";
			row[sm_Columns_R1.sm_col_url]        = sm_completed->url;
		}
		update_list = false;
	}
	else if( sm_listGroup == FAILED_GROUP && update_list )
	{
		sm_refTreeModel_R1->clear();
		struct SM_TASK_STORE < SM_FAILED_DATA > *temp = sm_failedsStore.get_first();
		for( int i = 0; temp; i++, temp = temp->next )
		{
			SM_FAILED_DATA *sm_failed = temp->data;

			Gtk::TreeModel::Row row              = *( sm_refTreeModel_R1->append() );
			row[sm_Columns_R1.sm_col_id]         = i;
			row[sm_Columns_R1.sm_col_name]       = sm_failed->name;
			row[sm_Columns_R1.sm_col_status]     = "failed";
			row[sm_Columns_R1.sm_col_size]       = sm_failed->size;
			row[sm_Columns_R1.sm_col_completed]  = sm_failed->completed;
			row[sm_Columns_R1.sm_col_percentage] = sm_failed->percentage;
			row[sm_Columns_R1.sm_col_speed]      = "";
			row[sm_Columns_R1.sm_col_time]       = "";
			row[sm_Columns_R1.sm_col_url]        = sm_failed->url;
		}
		update_list = false;
	}

		/* refresh statusbar */

	int totalSpeed = 0;
	for( int i = 0; i < MAX_TASKS; i++ )
	{
		if( sm_thread[i] && sm_thread[i]->isRunning() )
			totalSpeed += sm_thread[i]->get_speed();
	}

	sm_speedStatus.set_label( "Speed: " + SM_Util::itoa( totalSpeed ) + " KB/s" );

	return true;
}


	/* file event: paused | completed | failed */

void SM_Win::on_download_paused ( SM_Net *sm_net )
{
	SM_Util::msg_out( "PAUSED 1-3" );

	sm_threadCount--;
	update_list = true;

		/* running a enqueued item */
	if( sm_queueStore.get_size() > 0 )
	{
		SM_Util::msg_out( "PAUSED 2-3" );

		SM_TASK_DATA *download = sm_queueStore.pop_front();
		start( download );
	}
	SM_Util::msg_out( "PAUSED 3-3" );
}

void SM_Win::on_download_completed ( SM_Net *sm_net )
{
	SM_Util::msg_out( "COMPLETED 1-4" );

	SM_COMPLETED_DATA *sm_completed = new SM_COMPLETED_DATA;

	Glib::Mutex mutex;
	{
		Glib::Mutex::Lock lock ( mutex );

		sm_completed->name     = sm_net->get_name();
		sm_completed->size     = SM_Util::show_size( sm_net->get_totalSize() );
		sm_completed->url      = sm_net->get_url();
		sm_completed->listPath = SM_Util::itoa( sm_completedsStore.get_size() ) + ":0";
		sm_completedsStore.push_back( sm_completed );

		refresh_downloads( sm_net->get_id() );
		
		int id = sm_net->get_id();
		SM_TASK_DATA *sm_download = sm_downloadsStore.get_data( id );
		delete sm_download;
		sm_downloadsStore.erase( id );

		sm_net->set_threadID( MAX_TASKS );
		sm_threadCount--;

		update_list = true;
	}
	save_history();

	SM_Util::msg_out( "COMPLETED 2-4" );

		/* running a enqueued item */
	if( sm_queueStore.get_size() > 0 )
	{
		SM_Util::msg_out( "COMPLETED 3-4" );

		SM_TASK_DATA *sm_download = sm_queueStore.pop_front();
		start( sm_download );
	}
	SM_Util::msg_out( "COMPLETED 4-4" );
}

void SM_Win::on_download_failed ( SM_Net *sm_net )
{
	SM_Util::msg_out( "FAILED 1-3" );

	SM_FAILED_DATA *sm_failed = new SM_FAILED_DATA;

	Glib::Mutex mutex;
	{
		Glib::Mutex::Lock lock ( mutex );

		sm_failed->name       = sm_net->get_name();
		sm_failed->size       = SM_Util::show_size( sm_net->get_totalSize() );
		sm_failed->completed  = SM_Util::show_size( sm_net->get_comSize() );
		sm_failed->percentage = sm_net->get_percentage();
		sm_failed->url        = sm_net->get_url();
		sm_failed->listPath   = SM_Util::itoa( sm_failedsStore.get_size() ) + ":0";
		sm_failedsStore.push_back( sm_failed );

		refresh_downloads( sm_net->get_id() );

		int id = sm_net->get_id();
		SM_TASK_DATA *sm_download = sm_downloadsStore.get_data( id );
		delete sm_download;
		sm_downloadsStore.erase( id );

		sm_net->set_threadID( MAX_TASKS );
		sm_threadCount--;

		update_list = true;
	}
	save_history();

	SM_Util::msg_out( "FAILED 2-3" );

		/* running a enqueued item */
	if( sm_queueStore.get_size() > 0 )
	{
		SM_TASK_DATA *sm_download = sm_queueStore.pop_front();
		start( sm_download );
	}
	SM_Util::msg_out( "FAILED 3-3" );
}


	/* actions functions */

void SM_Win::define_home ( void )
{
	char * home;
	home = getenv ("HOME");

	sm_homePath = "";
	if( home )
	{
		sm_homePath.append( home );
		sm_homePath.append( "/.simpleman/" );
	}
	else
		sm_homePath = "";
}

void SM_Win::start ( SM_TASK_DATA *sm_download )
{
	if( sm_threadCount <= sm_config.tasksLimit )
	{
		int t = MAX_TASKS;
		for( int i = 0; i < MAX_TASKS; i++ )
		{
			if( sm_thread[i] == NULL )
			{
				t = i;
				break;
			}
			else if( sm_thread[i]->get_threadID() == MAX_TASKS )
			{
				t = i;
				delete sm_thread[i];
				sm_thread[i] = NULL;
				break;
			}
		}
		if( t != MAX_TASKS )
		{
			SM_Util::msg_out( "RUNNING" );

			SM_Net *sm_net = new SM_Net( this, sm_download );

			Glib::Mutex mutex;
			{
				Glib::Mutex::Lock lock ( mutex );
				if( ! sm_net->create( sm_download->url ) )
				{
					on_download_failed( sm_net );
					return;
				}
				sm_net->set_path( sm_download->dir + "/" + sm_download->name );

				sm_download->t = t;
				sm_thread[t] = sm_net;
				sm_thread[t]->set_threadID( t );
				sm_thread[t]->set_speedLimit( sm_config.speedLimit / ( sm_threadCount + 1 ) );
				sm_thread[t]->signal_paused().connect( sigc::mem_fun( *this, &SM_Win::on_download_paused ) );
				sm_thread[t]->signal_completed().connect( sigc::mem_fun( *this, &SM_Win::on_download_completed ) );
				sm_thread[t]->signal_failed().connect( sigc::mem_fun( *this, &SM_Win::on_download_failed ) );
				sm_thread[t]->start();
				sm_threadCount++;
			}
		}
		else
		{
			SM_Util::msg_out( "ENQUEUED 1" );

			sm_download->t = TASK_ENQUEUED;
			sm_queueStore.push_back( sm_download );
			update_list = true;
		}
	}
	else
	{
		SM_Util::msg_out( "ENQUEUED 2" );

		sm_download->t = TASK_ENQUEUED;
		sm_queueStore.push_back( sm_download );
		update_list = true;
	}
}

void SM_Win::refresh_downloads ( int id )
{
	std::string listPath;
	struct SM_TASK_STORE < SM_TASK_DATA > *temp = sm_downloadsStore.get_first();
	for( int i = 0; temp; i++, temp = temp->next )
	{
		SM_TASK_DATA *sm_downloads = temp->data;
		if( i == id )
			listPath = sm_downloads->listPath;
		else if( i > id )
		{
			std::string temp = sm_downloads->listPath;
			sm_downloads->id = id;
			sm_downloads->listPath = listPath;
			listPath = temp;
			id++;
		}
	}
}

void SM_Win::connect_list_timer ( void )
{
	if( ! sm_list_timer.connected() )
		sm_list_timer = Glib::signal_timeout().connect( sigc::mem_fun( *this, &SM_Win::on_list_timer ), LIST_TIMER );
}

void SM_Win::disconnect_list_timer ( void )
{
	if( sm_list_timer.connected() )
		sm_list_timer.disconnect();
}



/* EOF */
