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
#include "sm_ftp.h"

#define MAXRECV 1024


SM_FTP::SM_FTP ( )
{
		/* setting */
	start = true;
	set_speedLimit( 0 );
}


	/* initial functions */

const bool SM_FTP::configure ( const std::string &host, const int &port )
{
	if( ! SM_Socket::create() )
	{
		SM_Util::msg_err( "Error: could not create socket client." );
//		SM_Util::show_msg( "Socket Error", "Could not create\nsocket client.", false );
		return false;
	}

	if( ! SM_Socket::connect( host, port ) )
	{
		SM_Util::msg_err( "Error: could not bind to port." );
//		SM_Util::show_msg( "Socket Error", "Could not bind\nto port.", false );
		return false;
	}

	int cod;
	get_cod( cod );
	if( cod != 220 )
		return false;

	return true;
}

const bool SM_FTP::user_conf ( const std::string &user, const std::string &pass )
{
	if( ! SM_Socket::send( "USER " + user + "\r\n" ) )
		return false;

	SM_Util::msg_out( "USER " + user );

	int cod;
	get_cod( cod );
	if( cod != 331 )
		return false;

	if( ! SM_Socket::send( "PASS " + pass + "\r\n" ) )
		return false;

	SM_Util::msg_out( "PASS " + pass );

	get_cod( cod );
	if( cod != 230 )
		return false;

	return true;
}

const bool SM_FTP::pasv_mode ( std::string &ip, int &port )
{
	if( ! SM_Socket::send( "PASV\r\n" ) )
		return false;

#ifdef DEBUG
	SM_Util::msg_out( "PASV" );
#endif /* DEBUG */

	int cod;
	std::string ret_cod = get_cod( cod );
	if( cod != 227 )
		return false;

	std::string ip_port = "";
	unsigned int posA = ret_cod.find("(");
	unsigned int posB = ret_cod.find(")");

	if( posA == std::string::npos || posB == std::string::npos )
		return false;
	posA++;
	ret_cod = ret_cod.substr( posA, posB - posA );

	int n = 0, p = 0;
	char token[6][6];
	for( unsigned int i = 0; i < ret_cod.length(); i++ )
	{
		if( ret_cod[i] == ',' )
		{
			token[n][p] = '\0';
			n++;
			p = 0;
		}
		else
		{
			token[n][p] = ret_cod[i];
			p++;
		}
	}
	token[n][p] = '\0';

	for( int i = 0; i < 4; i++ )
	{
		ip.append( token[i] );
		if( i < 3 )
			ip.append( "." );
	}
	port = atoi( token[4] ) * 256 + atoi( token[5] );

	return true;
}

SM_Socket * SM_FTP::open_pasv ( const std::string &host, const int &port )
{
	SM_Socket *pasv_socket = new SM_Socket();

	if( ! pasv_socket->create() )
	{
		SM_Util::msg_err( "Could not create client socket." );
		return NULL;
	}

	if( ! pasv_socket->connect( host, port ) )
	{
		SM_Util::msg_err( "Could not bind to port." );
		return NULL;
	}

	return pasv_socket;
}

const bool SM_FTP::rest ( const int &position )
{
	if( ! SM_Socket::send( "REST " + SM_Util::itoa( position ) + "\r\n" ) )
		return false;

	SM_Util::msg_out( "REST " + SM_Util::itoa( position ) );

	int cod;
	get_cod( cod );
	if( cod != 350 )
		return false;

	return true;
}

const bool SM_FTP::retr ( const std::string &fileName )
{
	if( ! SM_Socket::send( "RETR " + fileName + "\r\n" ) )
		return false;

	SM_Util::msg_out( "RETR " + fileName );

	int cod;
	get_cod( cod );
	if( cod != 150 )
		return false;

	return true;
}

	/* set functions */

const bool SM_FTP::set_type  ( const bool &binary )
{
	std::string ftp_code;

	if( binary )
		ftp_code = "TYPE I\r\n";
	else
		ftp_code = "TYPE A\r\n";

	if( ! SM_Socket::send( ftp_code ) )
		return false;

	SM_Util::msg_out( ftp_code );
	
	int cod;
	get_cod( cod );
	if( cod != 200 )
		return false;

	return true;
}

const bool SM_FTP::set_stop ( void )
{
	if( ! start )
		return false;
	start = false;
	return true;
}

const bool SM_FTP::set_speedLimit ( const int &new_speedLimit )
{
	sm_speedLimit = new_speedLimit;

	return true;
}


	/* get functions */

const std::string SM_FTP::get_cod ( int &cod )
{
	bool first_msg = true;
	std::string msg = "";

	char buf[MAXRECV + 1];
	int recv_return = SM_Socket::recv ( buf, MAXRECV );

	cod = 0;

	while( recv_return > 0 )
	{
		if( first_msg )
		{
			char c = buf[3];
			buf[3] = '\0';
			cod = atoi( buf );
			buf[3] = c;
			first_msg = false;
		}
		int pos = 0;
		for( int i = 0; i < recv_return - 1; i++)
		{
				/* one line */
			if( buf[i] == '\r' && buf[i + 1] )
			{
				std::string line = buf;
				line = line.substr( pos, i - pos );

				if( line[0] >= '0' && line[0] <= '9' 
					&& line[1] >= '0' && line[1] <= '9' 
					&& line[2] >= '0' && line[2] <= '9' 
					&& line[3] == ' ' )
				{
					if( recv_return < MAXRECV )
						buf[recv_return] = '\0';
					else
						buf[MAXRECV - 1] = '\0';
					msg.append( buf );

					SM_Util::msg_out( msg );

					return msg;
				}
				pos = i + 2;
			}
		}
		if( recv_return < MAXRECV )
			buf[recv_return] = '\0';
		else
			buf[MAXRECV - 1] = '\0';
		msg.append( buf );

		recv_return = SM_Socket::recv ( buf, MAXRECV );
	}
	return msg;
}

const long  SM_FTP::get_size  ( const std::string & fileName )
{
	if( ! SM_Socket::send( "SIZE " + fileName + "\r\n" ) )
		return -1;

	SM_Util::msg_out( "SIZE " + fileName );

	int cod;
	std::string ret_cod = get_cod( cod );
	if( cod != 213 )
		return false;
	
	long size = atoi( ( ret_cod.substr( 4, ret_cod.length() - 4 ) ).c_str() );

	return size;
}

const bool SM_FTP::get_file ( SM_TASK_DATA *ptr_taskData, const std::string fileName, const std::string filePath, const int bytes, const bool newFile )
{
	std::string pasv_ip;
	int pasv_port;

	if( ! pasv_mode( pasv_ip, pasv_port ) )
		return false;

	SM_Socket *pasv_socket = open_pasv( pasv_ip, pasv_port );

	if( ! pasv_socket )
		return false;

	if( ! rest( bytes ) )
		return false;

	if( ! retr( fileName ) )
		return false;

	std::ofstream *outFile;
		
	if( newFile )
		outFile = new std::ofstream( filePath.c_str(), std::ios::trunc | std::ios::binary );
	else
		outFile = new std::ofstream( filePath.c_str(), std::ios::app | std::ios::binary );

	if( ! outFile )
	{
		SM_Util::msg_err( "Could not open file to write." );
		delete pasv_socket;
		return false;
	}

	time_t before, after;
	time( &before );

	char buf[MAXRECV + 1];
	int recv_return = pasv_socket->recv( buf, MAXRECV );
	if ( recv_return == -1 )
	{
		SM_Util::msg_err( "Could not receive from server." );
		outFile->close();	
		delete outFile;
		delete pasv_socket;
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
			ptr_taskData->time = SM_Util::show_time(time_lef);
		if( ptr_taskData->completed == 0 || ptr_taskData->size == 0 )
			percentage = 0;
		else
			percentage = ( ( ( double ) ptr_taskData->completed ) / ptr_taskData->size ) * 100;
		if(percentage <= 100)
			ptr_taskData->percentage = percentage;
		else
			ptr_taskData->percentage = 100;
	}

	if( ! start )
	{
		if( ! abort() )
		{
			SM_Util::msg_err( "Could not abort connection." );
			outFile->close();	
			delete outFile;
			delete pasv_socket;
			return false;
		}
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

		recv_return = pasv_socket->recv( buf, MAXRECV );
		if( recv_return == -1 )
		{
			SM_Util::msg_err( "Could not receive from server." );
			outFile->close();	
			delete outFile;
			delete pasv_socket;
			return false;
		}
		total_recv += recv_return;

			/* refresh information */

		Glib::Mutex mutex;
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

		if( ! start )
		{
			if( ! abort() )
			{
				SM_Util::msg_err( "Could not abort connection." );
				outFile->close();
				delete outFile;
				delete pasv_socket;
				return false;
			}
			break;
		}
	}
	outFile->close();	
	delete outFile;
	delete pasv_socket;

	int cod;
	get_cod( cod );

		/* 226 -> successed */ 
	if( cod == 226 )
	{
		if( ptr_taskData->size > 0 )
			ptr_taskData->completed = ptr_taskData->size;

		if( ! quit() )
		{
			SM_Util::msg_err( "Error: could not quit connection." );
			return false;
		}
		return true;
	}
		/* 426 -> aborted | 200 -> successful */
	else if( cod ==  426 || cod == 200)
		return true;

	return false;
}


	/* final functions */

const bool SM_FTP::abort ( void )
{
	if( ! SM_Socket::send( "ABOR\r\n" ) )
		return false;

	SM_Util::msg_out( "ABOR" );

	return true;
}

const bool SM_FTP::quit ( void )
{
	if( ! SM_Socket::send( "QUIT\r\n" ) )
		return false;

	SM_Util::msg_out( "QUIT" );
	
	int cod;
	get_cod( cod );
	if( cod != 221 )
		return false;

	return true;
}


	/* active functions */

const bool SM_FTP::active_speedLimit ( const int &speed, const int &total_recv, const int &time_pass )
{
	if( sm_speedLimit == 0 || speed <= ( sm_speedLimit * 1024 ) )
		return true;

	int delay = ( float ) total_recv / sm_speedLimit;

	if( delay > time_pass )
		usleep( delay );

	return true;
}


/* EOF */

