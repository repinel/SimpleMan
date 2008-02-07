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

#ifndef SM_FTP_H
#define SM_FTP_H

#include <string>

#include "sm_struct.h"
#include "sm_socket.h"


class SM_FTP : private SM_Socket
{
  public:

 	SM_FTP ( void );
	virtual ~SM_FTP ( void ) { };

		/* initial functions */
	
	const bool configure      ( const std::string &, const int & );
	const bool user_conf      ( const std::string &, const std::string & );
	
		/* set functions*/

	const bool set_type       ( const bool & );
		
	const bool set_stop       ( void );

	const bool set_speedLimit ( const int & );

		/* get functions */

	const long get_size       ( const std::string & );
	const bool get_file       ( SM_TASK_DATA *, const std::string, const std::string, const int, const bool );

		/* final functions */

	const bool quit           ( void );	
	
  private:

		/* initial functions */

	const bool  pasv_mode  ( std::string &, int & );
	SM_Socket * open_pasv  ( const std::string &, const int & );

	const bool  rest       ( const int & );
	const bool  retr       ( const std::string & );

		/* get functions */

	const std::string get_cod    ( int & );

		/* final functions */

	const bool  abort      ( void );

		/* active functions */

	const bool active_speedLimit ( const int &, const int &, const int & );


	std::string user;
	std::string pass;
 
	bool start;

	int sm_speedLimit;
};


#endif /* SM_FTP_H */


/* EOF */
