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

#ifndef SM_NEW
#define SM_NEW

#include <gtkmm/alignment.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/window.h>

#include <string>

#include "sm_struct.h"


class SM_New : public Gtk::Dialog
{

  public:
	SM_New ( Gtk::Window &, const SM_CONFIG_DATA & );
	virtual ~SM_New ( void ) {};
	
		/* set functions */
	
	void set_url   ( const std::string & );
	
		/* get functions */
	
	const std::string get_url   ( void ) const;
	const std::string get_dir   ( void ) const;
	const std::string get_name  ( void ) const;
	const std::string get_user  ( void ) const;
	const std::string get_pass  ( void ) const;
	const bool        get_start ( void ) const;

  private:

		/* set functions */

	void set_start ( const bool & );
 
 		/* events */
 
	virtual void on_changed_url       ( void );
	virtual void on_button_folder_new ( void );

		/* ftp */

	const std::string &sm_defaultPass;

		/* widgets */

	Gtk::VBox *sm_newBox;
	Gtk::VBox  sm_boxFrameFile;
	Gtk::VBox  sm_boxFrameFTP;

	Gtk::HBox sm_boxURL;
	Gtk::HBox sm_boxDir;
	Gtk::HBox sm_boxName;
	Gtk::HBox sm_boxUser;
	Gtk::HBox sm_boxPass;
	Gtk::HBox sm_boxRadio;

	Gtk::Frame sm_FrameFile;
	Gtk::Frame sm_FrameFTP;
	Gtk::Frame sm_FrameStartOptions;

	Gtk::Label sm_LabelURL;
	Gtk::Label sm_LabelDir;
	Gtk::Label sm_LabelName;
	Gtk::Label sm_LabelUser;
	Gtk::Label sm_LabelPass;

	Gtk::Entry sm_EntryURL;
	Gtk::Entry sm_EntryDir;
	Gtk::Entry sm_EntryName;
	Gtk::Entry sm_EntryUser;
	Gtk::Entry sm_EntryPass;

	Gtk::Button sm_Button_Folder;

	Gtk::RadioButton        sm_RadioButtonOn;
	Gtk::RadioButton        sm_RadioButtonOff;
	Gtk::RadioButton::Group sm_group;

	Gtk::Alignment          sm_AlignmentRadio;
};


#endif /* SM_NEW */

/* EOF */


