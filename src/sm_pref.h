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

#ifndef SM_PREF_H
#define SM_PREF_H

#include <gtkmm/alignment.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/combobox.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/window.h>

#include <string>

#include "sm_struct.h"
#include "sm_model_c.h"


class SM_Pref : public Gtk::Dialog
{

 public:
	SM_Pref ( Gtk::Window &, SM_CONFIG_DATA & );
	virtual ~SM_Pref ( void ) { };
	
		/* get functions */

	const std::string get_dir         ( void ) const;	
	const int         get_tasksLimit  ( void ) const;
	const int         get_speedLimit  ( void ) const;
	const std::string get_pass        ( void ) const;
	const bool        get_start       ( void ) const;
	
 private:
 
 		/* events */
 
	virtual void on_button_folder_preferences ( void );
	virtual void on_combo_changed             ( void );

		/* config */

	int sm_tasksLimit;

		/* widgets */

	Gtk::VBox *sm_preferencesBox;
	Gtk::VBox  sm_boxTasks;

	Gtk::HBox  sm_boxPath;
	Gtk::HBox  sm_boxTasksLimit;
	Gtk::HBox  sm_boxSpeedLimit;
	Gtk::HBox  sm_boxPass;
	Gtk::HBox  sm_boxRadio;

	Gtk::Frame sm_FrameTasks;
	Gtk::Frame sm_FrameConnection;
	Gtk::Frame sm_FrameFTP;
	Gtk::Frame sm_FrameStartOptions;
	
	Gtk::Label sm_LabelPath;
	Gtk::Label sm_LabelTasksLimit;
	Gtk::Label sm_LabelSpeedLimit;
	Gtk::Label sm_LabelPass;

	Gtk::Button sm_Button_Folder;	

	SM_Model_C                   sm_Columns_C;
	Gtk::ComboBox                sm_Combo;
	Glib::RefPtr<Gtk::ListStore> sm_refTreeModel_C;

	Gtk::Entry sm_EntryPath;
	Gtk::Entry sm_EntrySpeedLimit;
	Gtk::Entry sm_EntryPass;

	Gtk::RadioButton        sm_RadioButtonOn;
	Gtk::RadioButton        sm_RadioButtonOff;
	Gtk::RadioButton::Group sm_group;

	Gtk::Alignment          sm_AlignmentRadio;
};


#endif /* SM_PREF_H */


/* EOF */


