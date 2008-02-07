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

#include <gtkmm/stock.h>

#include "sm_pref.h"
#include "sm_util.h"

#define SIZE_X 500
#define SIZE_Y 260


SM_Pref::SM_Pref ( Gtk::Window &win, SM_CONFIG_DATA &sm_config )
	: Gtk::Dialog ( "Preferences", win, false, false ),
	  sm_boxTasks( false, 0 ),
	  sm_boxPath( false, 0 ),
	  sm_boxTasksLimit( false, 0 ),
	  sm_boxSpeedLimit( false, 0 ),
	  sm_boxPass( false, 0 ),
	  sm_boxRadio( false, 0 ),
	  sm_LabelPath( "  Default Save Path: ", false ),
	  sm_LabelTasksLimit( "  Tasks Limit:                                     "
						  "                                                   ", false ),
	  sm_LabelSpeedLimit( "  Speed Limit (KB/s) [0 = unlimit]:            "
						  "                                  ", false ),
	  sm_LabelPass( "  Default Anonymous Pass: ", false ),
	  sm_Button_Folder ( "..." ),
	  sm_RadioButtonOn( "On" ),
	  sm_RadioButtonOff( "Off" ),
	  sm_group ( sm_RadioButtonOn.get_group() ),
	  sm_AlignmentRadio( Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0 )
{
	set_size_request( SIZE_X, SIZE_Y );
	set_resizable ( false );

	sm_preferencesBox = get_vbox();
	sm_preferencesBox->set_homogeneous( false );
	sm_preferencesBox->set_spacing( 0 );

		/* Tasks */

	sm_FrameTasks.set_label("Tasks" );
	sm_FrameTasks.set_label_align( Gtk::ALIGN_CENTER, Gtk::ALIGN_TOP );
	sm_FrameTasks.set_shadow_type( Gtk::SHADOW_ETCHED_OUT );	
	sm_preferencesBox->pack_start( sm_FrameTasks, Gtk::PACK_SHRINK, 0 );

	sm_EntryPath.set_text( sm_config.defaultPath );
	sm_EntryPath.set_width_chars( 40 );
	sm_EntryPath.set_activates_default( true );
	sm_Button_Folder.signal_clicked().connect( sigc::mem_fun( *this, &SM_Pref::on_button_folder_preferences ) );
	sm_boxPath.pack_start( sm_LabelPath, Gtk::PACK_SHRINK, 0 );
	sm_boxPath.pack_start( sm_EntryPath, Gtk::PACK_SHRINK, 0 );
	sm_boxPath.pack_start( sm_Button_Folder, Gtk::PACK_SHRINK, 0 );
	
		/* create the TreeModel: */
	sm_refTreeModel_C = Gtk::ListStore::create( sm_Columns_C );
	sm_Combo.set_model( sm_refTreeModel_C );
		/* fill the ComboBox's TreeModel */
	for( int i = 1; i <= 10; i++ )
	{
		Gtk::TreeModel::Row row = *( sm_refTreeModel_C->append() );
		row[sm_Columns_C.sm_col_id] = i;
	}
	sm_tasksLimit = sm_config.tasksLimit;
	sm_Combo.set_active( sm_tasksLimit );

		/* add the model columns to the ComboBox */
	sm_Combo.pack_start( sm_Columns_C.sm_col_id );
	sm_Combo.signal_changed().connect( sigc::mem_fun( *this, &SM_Pref::on_combo_changed ) );
	sm_boxTasksLimit.pack_start( sm_LabelTasksLimit, Gtk::PACK_SHRINK, 0 );
	sm_boxTasksLimit.pack_start( sm_Combo, Gtk::PACK_SHRINK, 0 );
	
	sm_FrameTasks.add( sm_boxTasks );
	sm_boxTasks.pack_start( sm_boxPath, Gtk::PACK_SHRINK, 0 );
	sm_boxTasks.pack_start( sm_boxTasksLimit, Gtk::PACK_SHRINK, 0 );

		/* Connection */
	
	sm_FrameConnection.set_label( "Connection");
	sm_FrameConnection.set_label_align(  Gtk::ALIGN_CENTER, Gtk::ALIGN_TOP  );
	sm_FrameConnection.set_shadow_type(Gtk::SHADOW_ETCHED_OUT);	
	sm_preferencesBox->pack_start( sm_FrameConnection, Gtk::PACK_SHRINK, 3 );
	
	sm_EntrySpeedLimit.set_text( SM_Util::itoa( sm_config.speedLimit ) );
	sm_EntrySpeedLimit.set_width_chars( 10 );
	sm_EntrySpeedLimit.set_activates_default( true );
	sm_boxSpeedLimit.pack_start( sm_LabelSpeedLimit, Gtk::PACK_SHRINK, 0 );
	sm_boxSpeedLimit.pack_start( sm_EntrySpeedLimit, Gtk::PACK_SHRINK, 0 );	
	sm_FrameConnection.add( sm_boxSpeedLimit );

		/* FTP Options */

	sm_FrameFTP.set_label( "FTP Options" );
	sm_FrameFTP.set_label_align( Gtk::ALIGN_CENTER, Gtk::ALIGN_TOP );
	sm_FrameFTP.set_shadow_type( Gtk::SHADOW_ETCHED_OUT );
	sm_preferencesBox->pack_start( sm_FrameFTP, Gtk::PACK_SHRINK, 3 );

	sm_EntryPass.set_text( sm_config.defaultPass );
	sm_EntryPass.set_width_chars( 38 );
	sm_EntryPass.set_activates_default( true );
	sm_boxPass.pack_start( sm_LabelPass, Gtk::PACK_SHRINK, 0 );
	sm_boxPass.pack_start( sm_EntryPass, Gtk::PACK_SHRINK, 0 );
	sm_FrameFTP.add( sm_boxPass );

		/* Default 'Start Now */

	sm_FrameStartOptions.set_label( "Default 'Start Now'" );
	sm_FrameStartOptions.set_label_align( Gtk::ALIGN_CENTER, Gtk::ALIGN_TOP );
	sm_FrameStartOptions.set_shadow_type( Gtk::SHADOW_ETCHED_OUT );
	sm_preferencesBox->pack_start( sm_FrameStartOptions, Gtk::PACK_SHRINK, 3 );

	sm_RadioButtonOff.set_group( sm_group );
	sm_AlignmentRadio.add( sm_boxRadio );
	sm_boxRadio.pack_start( sm_RadioButtonOn, Gtk::PACK_SHRINK, 0 );
	sm_boxRadio.pack_start( sm_RadioButtonOff, Gtk::PACK_SHRINK, 0 );
	sm_FrameStartOptions.add( sm_AlignmentRadio );

	if( sm_config.startNow )
		sm_RadioButtonOn.set_active();
	else
		sm_RadioButtonOff.set_active();

		/* buttons */

	add_button( Gtk::Stock::CANCEL, 0 );
	add_button( Gtk::Stock::OK, 1 );

	set_default_response( 1 );	
	activate_default();

	show_all_children();
}


	/* events */

void SM_Pref::on_button_folder_preferences ( void )
{
	Gtk::FileChooserDialog sm_folderDialog( "Please choose a folder", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER );
	sm_folderDialog.set_transient_for( *this );
    
		/* add response buttons the the dialog */
	sm_folderDialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
	sm_folderDialog.add_button( "Select", Gtk::RESPONSE_OK );
	
	int result = sm_folderDialog.run();

		/* handle the response */
	if ( result == Gtk::RESPONSE_OK )
	{
		sm_EntryPath.set_text( sm_folderDialog.get_filename() );
	}
}

void SM_Pref::on_combo_changed ( void )
{
	Gtk::TreeModel::iterator iter = sm_Combo.get_active();
	if( iter )
	{ 
		Gtk::TreeModel::Row row = *iter;
		if( row )
		{ 
			sm_tasksLimit = row[sm_Columns_C.sm_col_id] - 1;
		}
	}
}


	/* get functions */

const std::string SM_Pref::get_dir ( void ) const
{
	return sm_EntryPath.get_text();
}

const int SM_Pref::get_tasksLimit ( void ) const
{
	return sm_tasksLimit;
}

const int SM_Pref::get_speedLimit ( void ) const
{
	int var = atoi( ( sm_EntrySpeedLimit.get_text() ).c_str() );
	
	return var;
}

const std::string SM_Pref::get_pass ( void ) const
{
	return sm_EntryPass.get_text();
}

const bool SM_Pref::get_start ( void ) const
{
	return sm_RadioButtonOn.get_active();
}


/* EOF */

