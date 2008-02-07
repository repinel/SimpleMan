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

#ifndef SM_HTTP_H
#define SM_HTTP_H

#include <string>

#include "sm_struct.h"
#include "sm_socket.h"


class SM_HTTP : private SM_Socket
{
  public:

 	SM_HTTP ( void );
	virtual ~SM_HTTP ( void ) { };
	
		/* initial functions */
	
	const bool        configure         ( std::string host, int port );
	
		/* set functions*/
		
	const bool        set_stop          ( void );
	const bool        set_speedLimit    ( const int & );
	
		/* get functions*/
	
	const std::string get_head          ( const std::string & );
	const bool        get_file          ( SM_TASK_DATA *, const std::string, const std::string, const int, const bool );
	
  private:

		/* active functions */

	const bool        active_speedLimit ( const int &, const int &, const int & );
 	
 	bool start;

	int sm_speedLimit;
};


#endif /* SM_HTTP_H */


/* EOF */


