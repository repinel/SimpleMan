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

#ifndef SM_SERVICE_H
#define SM_SERVICE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef ENABLE_DBUS
#include <glib/gerror.h>
#include <glib-object.h>

#define APP_SERVICE_NAME           "org.gnome.simpleman.Application"
#define APP_PATH_NAME              "/org/gnome/simpleman/SimpleMan"
#define APP_INTERFACE_NAME         "org.gnome.simpleman.Application"
#define SIMPLEMAN_TYPE_APPLICATION ( simpleman_service_get_type () )

typedef struct _SimpleManApplication {
	GObject object;
} SimpleManApplication;

typedef struct _SimpleManApplicationClass {
	GObjectClass object_class;
} SimpleManApplicationClass;

gboolean register_service         ( void );
gboolean send_links               ( char ** );
gboolean sm_application_open_link ( SimpleManApplication  *, const char *, const bool, GError ** );

#endif /* ENABLE_DBUS */

#endif /* SM_SERVICE_H */


/* EOF */

