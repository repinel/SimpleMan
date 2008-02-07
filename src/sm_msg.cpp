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

#include <iostream>

#include <unistd.h>

#include <gtkmm/stock.h>

#include "sm_msg.h"


SM_Msg::SM_Msg ( Gtk::Window &win, const std::string &title, const std::string &msg, const bool &show_cancel )
	: Gtk::Dialog ( title, win, false, false ),
	  sm_LabelMSG( msg, false )
{
	set_size_request( 400, 120 );
	set_resizable( false );
	
	sm_boxMSG = get_vbox();
	sm_boxMSG->set_homogeneous( false );
	sm_boxMSG->set_spacing( 0 );
	
	sm_boxMSG->pack_start( sm_LabelMSG, Gtk::PACK_SHRINK, 0 );
	
	if( show_cancel )
		add_button( Gtk::Stock::CANCEL, 0 );
	add_button( Gtk::Stock::OK, 1 );

	set_default_response( 1 );	
	activate_default();
}

const int SM_Msg::run_msg ( void )
{
	sm_awser = 1;
	timer_on = true;

	sm_run_timer = Glib::signal_timeout().connect( sigc::mem_fun( *this, &SM_Msg::on_run_timer ), 0 );

	while( timer_on )
		usleep( 50 );

	return sm_awser;
}

bool SM_Msg::on_run_timer ( void )
{
	show_all_children();
	sm_awser = run();

	timer_on = false;
	sm_run_timer.disconnect();

	return true;
}


/* EOF */

