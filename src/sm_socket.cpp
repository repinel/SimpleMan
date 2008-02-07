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

#include <fcntl.h>
#include <netdb.h>

#include "sm_socket.h"

#define MAXRECV 1024


SM_Socket::SM_Socket ( void )
	: sm_sock ( -1 )
{
	memset ( &sm_addr, 0, sizeof( sm_addr ) );
}

SM_Socket::~SM_Socket ( void )
{
	if( is_valid() )
    	::close ( sm_sock );
}


	/* intial functions */

const bool SM_Socket::create ( void )
{
	sm_sock = socket ( AF_INET, SOCK_STREAM, 0 );

	if( ! is_valid() )
		return false;

		/* TIME_WAIT - argh */
	int on = 1;
	if( setsockopt ( sm_sock, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof( on ) ) == -1 )
		return false;

		/* secs timeout */
	struct timeval timeout;
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;

	if( setsockopt( sm_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof( timeout ) ) == -1)
		return false;

	return true;
}

const bool SM_Socket::connect ( const std::string host, const int port )
{
	if( ! is_valid() )
		return false;

	sm_addr.sin_family = AF_INET;
	sm_addr.sin_port = htons (port);

	struct hostent *hostName = gethostbyname( host.c_str() );
	if( hostName == NULL )
		return false;

	sm_addr.sin_addr.s_addr = *( ( unsigned long * ) hostName->h_addr );

	int status = ::connect ( sm_sock, ( struct sockaddr * ) &sm_addr, sizeof ( sm_addr ) );
	if( status == 0 )
		return true;

	return false;
}

const bool SM_Socket::close ( void )
{
	if( ! is_valid() )
		return false;

	::close ( sm_sock );
	return true;
}


	/* data functions */

const bool SM_Socket::send ( const std::string s ) const
{
	int status = ::send ( sm_sock, s.c_str(), s.size(), MSG_NOSIGNAL );
	if( status == -1 )
		return false;
	else
		return true;
}

const int SM_Socket::recv ( char *buf, const int bufSize ) const
{
	int recv_return = ::recv ( sm_sock, buf, bufSize, 0 );
	
	return recv_return;
}

void SM_Socket::set_non_blocking ( const bool b )
{
	int opts;

	opts = fcntl ( sm_sock, F_GETFL );

	if ( opts < 0 )
		return;

	if ( b )
		opts = ( opts | O_NONBLOCK );
	else
		opts = ( opts & ~O_NONBLOCK );

	fcntl ( sm_sock, F_SETFL,opts );

}


	/* get functions */
	
const int SM_Socket::get_sock ( void ) const
{
	return sm_sock;
}

	
	/* check functions */

const bool SM_Socket::is_valid ( void ) const
{
	return sm_sock != -1; 
}


/* EOF */

