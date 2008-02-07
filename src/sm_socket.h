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

#ifndef SM_SOCKET_H
#define SM_SOCKET_H

#include <string>

#include <arpa/inet.h>


class SM_Socket
{
  public:

	SM_Socket ( void );
	virtual ~SM_Socket ( void );

		/* intial functions */

	const bool create  ( void );
	const bool connect ( const std::string host, const int port );
	const bool close   ( void );

		/* data functions */
		
	const bool send             ( const std::string ) const;
	const int  recv             ( char *, const int ) const;
	void       set_non_blocking ( const bool );

		/* get functions */
	
	const int get_sock ( void ) const;
	
		/* check functions */

	const bool is_valid ( void ) const;
	
  private:
	int         sm_sock;
	sockaddr_in sm_addr;
};


#endif /* SM_SOCKET_H */

/* EOF */


