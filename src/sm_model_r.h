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

#ifndef SM_MODEL_R_H
#define SM_MODEL_R_H

#include <gtkmm/treemodel.h>


class SM_Model_R : public Gtk::TreeModel::ColumnRecord
{
 public:

	SM_Model_R ( void );
    virtual ~SM_Model_R ( void ) {};
    
	Gtk::TreeModelColumn<unsigned int>  sm_col_id;
	Gtk::TreeModelColumn<Glib::ustring> sm_col_name;
	Gtk::TreeModelColumn<Glib::ustring> sm_col_status;	
	Gtk::TreeModelColumn<Glib::ustring> sm_col_size;
	Gtk::TreeModelColumn<Glib::ustring> sm_col_completed;
	Gtk::TreeModelColumn<Glib::ustring> sm_col_speed;
	Gtk::TreeModelColumn<unsigned int>  sm_col_percentage;
	Gtk::TreeModelColumn<Glib::ustring> sm_col_time;
	Gtk::TreeModelColumn<Glib::ustring> sm_col_url;
};


#endif /* SM_MODEL_R_H */

/* EOF */
