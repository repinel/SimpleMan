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

#ifndef SM_MSG_H
#define SM_MSG_H

#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>

#include <string>


class SM_Msg : public Gtk::Dialog
{
  public:

	SM_Msg ( Gtk::Window &, const std::string &, const std::string &, const bool & );

		/* run function */

	const int run_msg ( void );
	
  private:

		/* timer event */

	bool on_run_timer ( void );

		/* data */

	int  sm_awser;
	bool timer_on;

	sigc::connection sm_run_timer;

		/* widgets */

	Gtk::VBox  *sm_boxMSG;
	
	Gtk::Label  sm_LabelMSG;
};


#endif /* SM_MSG_H */


/* EOF */

