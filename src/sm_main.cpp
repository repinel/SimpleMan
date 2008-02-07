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
#include <gtkmm/main.h>

#include <cstdlib>
#include <string>

#ifdef ENABLE_DBUS
#include "sm_service.h"
#endif /* ENABLE_DBUS */

#include "sm_util.h"
#include "sm_win.h"


SM_Win *ptr_window;

bool check_help( char ** );

int main ( int argc, char **argv )
{
	if( ! Glib::thread_supported() )
		Glib::thread_init();

	Gtk::Main kit( argc, argv );

	if( check_help( argv ) )
		exit( EXIT_SUCCESS );

#ifdef ENABLE_DBUS
	if( ! register_service() )
	{
		send_links( argv );
		exit( EXIT_SUCCESS );
	}
#endif /* ENABLE_DBUS */

	SM_Win window( argv );
	ptr_window = &window;
	
	gdk_threads_enter();
	Gtk::Main::run( window );
	gdk_threads_leave();

	exit( EXIT_SUCCESS );
}

bool check_help( char **sm_argv )
{
	if( sm_argv[1] && ( strcmp( sm_argv[1], "--help" ) == 0 || strcmp( sm_argv[1], "-h" ) == 0 ) )
	{
		SM_Util::msg_out( "SimpleManager v0.1, download manager.\n"
						  "Usage: simpleman [OPTIONS]... [URL]...\n"
						  "\n"
						  "Startup:\n"
						  "   -d,  --default   Use default setting for a new task.\n"
						  "   -u,  --url       Download URL.\n"
						  "   -h,  --help      Print this help.\n"
						  "\n"
						  "Examples:\n"
						  "   simpleman -d -u http://site.simpleman/file\n"
						  "   simpleman    -u http://site.simpleman/file\n"
						  "   simpleman -d -u -u http://site.simpleman/file -u ftp://site.simpleman/file\n"
						  "\n"
						  "Mail bug report and suggestions to <simplemandownloader@gmail.com>\n" );
		return true;
	}
	return false;
}


/* EOF */
