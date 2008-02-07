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

#ifdef ENABLE_DBUS

#include <dbus/dbus-glib-bindings.h>

#include "sm_util.h"
#include "sm_win.h"
#include "sm_service.h"
#include "sm_service_dbus.h"


extern SM_Win *ptr_window;

G_DEFINE_TYPE( SimpleManApplication, simpleman_service, G_TYPE_OBJECT );

	/* try to register the service, if true, so is the first time */	
gboolean register_service ( void )
{
	DBusGConnection *connection;
	DBusGProxy *proxy;
	GError *error = NULL;
	guint request_result;
	SimpleManApplication *app;

	connection = dbus_g_bus_get ( DBUS_BUS_STARTER, &error );
	if ( connection == NULL )
	{
		SM_Util::msg_err( "Error: DBus service connection." );
		g_warning ( "Error: DBus service connection." );
		g_error_free ( error );
		return FALSE;
	}
	proxy = dbus_g_proxy_new_for_name (connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
	
	if ( ! org_freedesktop_DBus_request_name ( proxy, APP_SERVICE_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE, &request_result, &error ) )
	{
		SM_Util::msg_err( "Error: DBus service request name." );
		g_warning ( "Error: DBus service request name." );
		g_clear_error ( &error );
	}
	if ( request_result == DBUS_REQUEST_NAME_REPLY_EXISTS )
		return FALSE;

	app = reinterpret_cast< SimpleManApplication * > ( g_object_new ( SIMPLEMAN_TYPE_APPLICATION, NULL ) );
	dbus_g_object_type_install_info ( SIMPLEMAN_TYPE_APPLICATION, &dbus_glib_sm_service_object_info );

	dbus_g_connection_register_g_object ( connection, APP_PATH_NAME, G_OBJECT( app ) );

	return TRUE;
}

static void simpleman_service_class_init ( SimpleManApplicationClass *simpleman_class )
{
	dbus_g_object_type_install_info ( G_TYPE_FROM_CLASS ( simpleman_class ), &dbus_glib_sm_service_object_info );
}

static void simpleman_service_init ( SimpleManApplication *simpleman_object )
{
}	

gboolean sm_application_open_link ( SimpleManApplication *app, const char *sm_link, const bool is_default, GError **error )
{
	ptr_window->set_default( is_default );
	ptr_window->download_link( sm_link );

	return TRUE;
}

gboolean send_links ( char **sm_argv )
{
	GError *error = NULL;
	DBusGConnection *connection;
	gboolean result = FALSE;
	DBusGProxy *remote_object;

	int first = 1;
	bool is_default = false;
	bool url = false;

	connection = dbus_g_bus_get ( DBUS_BUS_STARTER, &error );
	if ( connection == NULL )
	{
		g_warning ( error->message );
		g_error_free ( error );	
		return FALSE;
	}
	remote_object = dbus_g_proxy_new_for_name ( connection, APP_SERVICE_NAME, APP_PATH_NAME, APP_INTERFACE_NAME );

	
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
			url = false;
			if ( ! dbus_g_proxy_call ( remote_object, "OpenLink", &error, G_TYPE_STRING,
									   sm_argv[i], G_TYPE_BOOLEAN, is_default, G_TYPE_INVALID, G_TYPE_INVALID ) )
			{
				g_warning ( error->message );
				g_clear_error ( &error );
				continue;
			}
		}
		else if( strcmp( sm_argv[i], "--url" ) == 0 || strcmp( sm_argv[i], "-u" ) == 0 )
		{
			url = true;
			continue;
		}
		result = TRUE;
	}

	return result;
}
#endif /* ENABLE_DBUS */


/* EOF */


