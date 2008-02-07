#
#	SimpleMan - Simple Download Manager
#	Copyright (C) 2007 Roque Pinel, Rio de Janeiro
#	Email simplemandownloader@gmail.com
#
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# src/Makefile.am for the Simple Manager project

INCLUDES = $(GTKMM_CFLAGS)\
           $(GTHREAD_CFLAGS)\
           $(DBUS_GLIB_CFLAGS)
       
LIBS =  $(GTKMM_LIBS)\
        $(GTHREAD_LIBS)\
        $(DBUS_GLIB_LIBS)

bin_PROGRAMS = simpleman

simpleman_SOURCES = sm_util.cpp sm_msg.cpp sm_socket.cpp sm_ftp.cpp sm_http.cpp sm_net.cpp sm_model_c.cpp sm_model_r.cpp sm_model_l.cpp sm_pref.cpp sm_new.cpp sm_service.cpp sm_win.cpp sm_main.cpp

# EOF
