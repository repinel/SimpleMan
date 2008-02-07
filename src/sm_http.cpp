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

#include <fstream>
#include <time.h>

#include <unistd.h>

#include "sm_util.h"
#include "sm_http.h"

#define MAXRECV 1024


SM_HTTP::SM_HTTP ( void )
{
		/* setting */
	start = true;
	set_speedLimit( 0 );
}


	/* initial functions */

const bool SM_HTTP::configure ( std::string host, int port )
{
	if( ! SM_Socket::create() )
	{
		SM_Util::msg_err( "Error: could not create socket client." );
		return false;
	}
	if( ! SM_Socket::connect( host, port ) )
	{
		SM_Util::msg_err( "Error: could not bind to port." );
		return false;
	}
	return true;
}


	/* set functions */

const bool SM_HTTP::set_stop ( void )
{
	if( ! start )
		return false;
	start = false;
	return true;
}

const bool SM_HTTP::set_speedLimit ( const int &new_speedLimit )
{
	sm_speedLimit = new_speedLimit;

	return true;
}


	/* get functions */

const std::string SM_HTTP::get_head ( const std::string& fileName )
{
	std::string http_cod ( "HEAD " + fileName + " HTTP/1.0\r\nConnection: Keep-Alive\n\n" );

	if( ! SM_Socket::send( http_cod ) )
	{
		return "";
	}
	
	char buf[MAXRECV + 1];
	int recv_return = SM_Socket::recv ( buf, sizeof( buf ) );
	if( recv_return <= 0 )
	{
		return "";
	}

	if(recv_return <= MAXRECV )
		buf[recv_return] = '\0';
	SM_Util::msg_out( buf );

	return buf;
}

const bool SM_HTTP::get_file ( SM_TASK_DATA *ptr_taskData, const std::string fileName, const std::string filePath, const int bytes, const bool newFile )
{
		/* return 206 */
	std::string http_cod = "GET " + fileName + " HTTP/1.0\r\nRange: bytes=-" + SM_Util::itoa( bytes )  + "\n\n";

	SM_Util::msg_out( http_cod );

	if( ! SM_Socket::send( http_cod ) )
	{
		SM_Util::msg_err( "Error: could not send request to server." );
		return false;
	}
	std::ofstream *outFile;
		
	if( newFile )
		outFile = new std::ofstream( filePath.c_str(), std::ios::trunc | std::ios::binary );
	else
		outFile = new std::ofstream( filePath.c_str(), std::ios::app | std::ios::binary );

	if( ! outFile )
	{
		SM_Util::msg_err( "Error: could not open file to write." );
		outFile->close();	
		delete outFile;
		return false;
	}

	time_t before, after;
	time( &before );

	char buf[MAXRECV + 1];
	int recv_return = SM_Socket::recv( buf, MAXRECV );
	if ( recv_return == -1 )
	{
		SM_Util::msg_err( "Error: could not receive from server." );
		outFile->close();	
		delete outFile;
		return false;
	}
	bool option = false;
	int i;
	for( i = 0; i < recv_return - 3; i++ )
	{
		if( buf[i] == '\r' && buf[i+1] == '\n' && buf[i+2] == '\r' && buf[i+3] == '\n' )
		{			
			i += 4;
			option = true;
			break;
		}
	}
	int percentage, total_recv = 0, time_pas, time_lef, speed;
	total_recv += recv_return;

		/* refresh information */
	Glib::Mutex mutex;
	{
		Glib::Mutex::Lock lock ( mutex );

		ptr_taskData->completed += recv_return - i;
		time( &after );

		time_pas = after - before;
		if( time_pas == 0 )
			speed = 0;
		else
			speed =  total_recv / time_pas;
		if( speed == 0 )
			time_lef = 0;
		else
			time_lef = ( ptr_taskData->size - ptr_taskData->completed ) / speed;
		ptr_taskData->speed = speed / 1024;
		if( time_lef >= 0 )
			ptr_taskData->time = SM_Util::show_time( time_lef );
		if( ptr_taskData->completed == 0 || ptr_taskData->size == 0 )
			percentage = 0;
		else
			percentage = ( ( ( double ) ptr_taskData->completed ) / ptr_taskData->size ) * 100;
		if( percentage <= 100 )
			ptr_taskData->percentage = percentage;
		else
			ptr_taskData->percentage = 100;
	}

	while ( recv_return > 0 && start )
	{
		if( option )
		{
			outFile->write( buf+i, recv_return-i );
			option = false;
		}
		else
			outFile->write( buf, recv_return );

		active_speedLimit( speed, total_recv, after - before );

		recv_return = SM_Socket::recv( buf, MAXRECV );
		if( recv_return == -1 )
		{
			SM_Util::msg_err( "Error: could not receive from server." );
			outFile->close();	
			delete outFile;
			return false;
		}
		total_recv += recv_return;

			/* refresh information */
		{
			Glib::Mutex::Lock lock ( mutex );

			ptr_taskData->completed += recv_return;
			time( &after );
			time_pas = after - before;
			if( time_pas == 0 )
				speed = 0;
			else
				speed =  total_recv / time_pas;
			if( speed == 0 )
				time_lef = 0;
			else
				time_lef = ( ptr_taskData->size - ptr_taskData->completed ) / speed;
			ptr_taskData->speed = speed / 1024;

			if( time_lef >= 0 )
				ptr_taskData->time = SM_Util::show_time( time_lef );

			percentage = ( ( ( double ) ptr_taskData->completed ) / ptr_taskData->size ) * 100;
			if( percentage <= 100 )
				ptr_taskData->percentage = percentage;
			else
				ptr_taskData->percentage = 100;
		}
	}
	outFile->close();	
	delete outFile;
	return true;
}


	/* active functions */

const bool SM_HTTP::active_speedLimit ( const int &speed, const int &total_recv, const int &time_pass )
{
	if( sm_speedLimit == 0 || speed <= ( sm_speedLimit * 1024 ) )
		return true;

	int delay = ( float ) total_recv / sm_speedLimit;

	if( delay > time_pass )
		usleep( delay );

	return true;
}


/* EOF */

