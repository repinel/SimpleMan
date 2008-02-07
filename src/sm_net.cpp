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

#include <cstdlib>
#include <fstream>

#include <unistd.h>

#include "sm_util.h"
#include "sm_msg.h"
#include "sm_socket.h"
#include "sm_http.h"
#include "sm_ftp.h"
#include "sm_win.h"
#include "sm_net.h"

#define OPTION_NOT_ASKED -1
#define OPTION_RESUME     0
#define OPTION_REPLACE    1


SM_Net::SM_Net ( SM_Win *new_win, SM_TASK_DATA *new_download )
	: sm_download( new_download ),
	  sm_win( new_win )
{
	changeURL = false;
	running = false;
	donwload = false;

	sm_httpSocket = NULL;
	sm_ftpSocket  = NULL;

	sm_speedLimit = 0;
}

SM_Net::~SM_Net ( void )
{

}


	/* initial functions */

const bool SM_Net::create ( const std::string &new_url )
{
	if( ! check_url( new_url ) )
	{
		SM_Util::msg_err( "Error: could not identify URL." );
		show_error( "URL Check", "Could not identify URL." , false );
		return false;
	}
	int find_return = new_url.find( "http://" );

		/* http */
	if( find_return == 0 )
		sm_protocol = 0;
	else
	{
		find_return = new_url.find( "ftp://" );

			/* ftp */
		if( find_return == 0 )
			sm_protocol = 1;
		else
		{
			SM_Util::msg_err( "Error: network protocol not supported." );
			show_error( "URL Check", "Network protocol\nnot supported." , false );
			sm_protocol = 2;
			return false;
		}
	}
	if ( ! set_url( new_url ) )
	{
		SM_Util::msg_err( "Error: could not define url." );
		show_error( "URL Check", "Could not define url." , false );
		return false;
	}
	if ( ! set_port( new_url ) )
	{
		SM_Util::msg_err( "Error: could not define port." );
		show_error( "Port Check", "Could not define port." , false );
		return false;
	}
	if ( ! set_address( new_url ) )
	{
		SM_Util::msg_err( "Error: could not define address." );
		show_error( "Address Check", "Could not define address." , false );
		return false;
	}
	if ( ! set_fileName( new_url ) )
	{
		SM_Util::msg_err( "Error: could not define file name." );
		show_error( "URL File Check", "Could not define\nfile name." , false );
		return false;
	}
	return true;
}


	/* Set Functions */

const bool SM_Net::set_threadID ( const int &new_threadID )
{
	sm_threadID = new_threadID;

	return true;
}

const bool SM_Net::set_id ( const int &new_id )
{
	sm_download->id = new_id;

	return true;
}

const bool SM_Net::set_comSize ( const int &new_comSize )
{
	sm_download->completed = new_comSize;

	return true;
}

const bool SM_Net::set_totalSize ( const int &new_totalSize )
{
	sm_download->size = new_totalSize;
	return true;
}

const bool SM_Net::set_totalSize ( const std::string &head_text )
{
		/* HTTP */
	if( isHTTP() )
	{
		int posA = head_text.find( "Length:" ) + 8;	
		int posB = head_text.find( "Keep-A" ) - 3;
		set_totalSize ( atoi( ( head_text.substr( posA, posB-posA+1 ) ).c_str() ) );

		SM_Util::msg_out( "TotalSize: " + head_text.substr( posA, posB-posA+1 ) );

		return true;
	}
		/* FTP */
	else if( isFTP() )
	{
		set_totalSize( 0 );
		return true;
	}
	
	set_totalSize( 0 );
	return false;
}

const bool SM_Net::set_url ( const std::string &new_url )
{
	sm_download->url = new_url;

	SM_Util::msg_out( "URL: " + get_url() );

	return true;
}

const bool SM_Net::set_port ( const std::string &new_url )
{
	if( ! isSupported () )
		return false;

	unsigned int posA = new_url.find( "//" ) + 2;
	unsigned int posB = new_url.find( "/", posA );

	if( posB == std::string::npos )
	{
		if( isHTTP() )
			sm_port = 80;
		else if( isFTP() )
			sm_port = 21;
	}
	else
	{
		std::string temp = new_url.substr( posA, posB - posA );
		posA = temp.find( ":" );

		if( posA == std::string::npos )
		{
			if( isHTTP() )
				sm_port = 80;
			else if( isFTP() )
				sm_port = 21;
		}
		else
			sm_port = atoi( ( temp.substr( posA+1, posB - posA ) ).c_str() );
	}
	SM_Util::msg_out( "Port: " + SM_Util::itoa( sm_port ) );

	return true;
}

const bool SM_Net::set_address ( const std::string &new_url )
{
	if( ! isSupported () )
		return false;

	unsigned int posA = new_url.find( "//" ) + 2;
	unsigned int posB = new_url.find( "/", posA );

	if( posB == std::string::npos )
		return false;

	std::string temp = new_url.substr( posA, posB - posA );
	unsigned int posC = temp.find( ":" );

	if( posC == std::string::npos )
		sm_address = temp.substr( 0, posB - posA );
	else
		sm_address = temp.substr( 0, posC );

	SM_Util::msg_out( "Address: " + sm_address );

	return true;
}

const bool SM_Net::set_fileName ( const std::string &new_url )
{
	if( ! isSupported () )
		return false;

	unsigned int posA = new_url.find( "//" ) + 2;
	unsigned int posB = new_url.find( "/", posA );
	if( posB == std::string::npos )
		return false;

	sm_fileName = new_url.substr( posB );

	SM_Util::msg_out( "FileName: " + sm_fileName );

	return true;
}

const bool SM_Net::set_path ( const std::string &new_path )
{
	sm_path = new_path;

	return true;
}

const bool SM_Net::set_name ( const std::string &new_name )
{
	sm_download->name = new_name;

	return true;
}

const bool SM_Net::set_speed ( const int &new_speed )
{
	sm_download->speed = new_speed;

	return true;
}

const bool SM_Net::set_time ( const std::string &new_time )
{
	sm_download->time = new_time;

	return true;
}

const bool SM_Net::set_listPath ( const std::string &new_listPath )
{
	sm_download->listPath = new_listPath;
	return true;
}

const bool SM_Net::set_speedLimit ( const int &new_speedLimit )
{
	sm_speedLimit = new_speedLimit;

	if( isHTTP() && sm_httpSocket )
		sm_httpSocket->set_speedLimit( sm_speedLimit );
	else if( isFTP() && sm_ftpSocket )
		sm_ftpSocket->set_speedLimit( sm_speedLimit );

	return true;
}

const bool SM_Net::set_replace ( const bool &new_replace )
{
	replace = new_replace;
	return true;
}


	/* Get Functions */

const int & SM_Net::get_threadID ( void ) const
{
	return sm_threadID;
}

const int & SM_Net::get_id ( void ) const
{
	return sm_download->id;
}

const std::string & SM_Net::get_name ( void ) const
{
	return sm_download->name;
}

const int & SM_Net::get_totalSize ( void ) const
{
	return sm_download->size;
}

const int & SM_Net::get_comSize ( void ) const
{
	return sm_download->completed;
}

const int & SM_Net::get_percentage ( void ) const
{
	return sm_download->percentage;
}

const int & SM_Net::get_speed ( void ) const
{
	return sm_download->speed;
}

const std::string & SM_Net::get_time ( void ) const
{
	return sm_download->time;
}

const std::string & SM_Net::get_url ( void ) const
{
	return sm_download->url;
}

const std::string & SM_Net::get_path ( void ) const
{
	return sm_path;
}

const std::string & SM_Net::get_listPath ( void ) const
{
	return sm_download->listPath;
}

const int SM_Net::get_replace ( void ) const
{
	return replace;
}


	/* Check Functions */

const bool SM_Net::isHTTP ( void ) const
{
	return sm_protocol == 0;
}

const bool SM_Net::isFTP ( void ) const
{
	return sm_protocol == 1;
}

const bool SM_Net::isSupported ( void ) const
{
	return sm_protocol != 2;
}

const bool SM_Net::isCompleted ( void ) const
{
	return get_comSize() >= get_totalSize();
}

const bool SM_Net::isRunning ( void ) const
{
	return running;
}

const bool SM_Net::check_url ( const std::string &new_url ) const
{
	int token = 0;
	bool ok = false;

	for( unsigned int i = 0; i < new_url.length(); i++ )
	{
			/* (token)://.+ */
		if( token == 0 )
		{
			if( ( new_url[i] >= 'a' && new_url[i] <= 'z' )
			 || ( new_url[i] >= 'A' && new_url[i] <= 'Z' )
			 || ( new_url[i] >= '0' && new_url[i] <= '9' ))
			{
				ok = true;
			}
			else if( ok && new_url[i] == ':' )
			{
				token = 1;
				ok = false;
			}
			else
				return false;
		}
			/* :// */
		else if( token == 1 )
		{
			if( !ok && new_url[i] == '/' )
				ok = true;
			else if( ok && new_url[i] == '/' )
			{
				token = 2;
				ok = false;
			}
			else
				return false;
		}		
			/* .+://(token)[/:].+ */
		else if( token == 2 )
		{
			if( new_url[i] != ':' && new_url[i] != '/' )
				ok = true;
			else if( ok && new_url[i] == ':' )
			{
				token = 3;
				ok = false;
			}
			else if( ok && new_url[i] == '/' )
			{
				token = 4;
				ok = false;
			}
			else
				return false;
		}
			/* .+://.+:(token)/.+ */
		else if( token == 3 )
		{
			if( new_url[i] >= '0' && new_url[i] <= '9' )
				ok = true;
			else if( ok && new_url[i] == '/' )
			{
				token = 4;
				ok = false;
			}
			else
				return false;
		}
			/* .+://.+/.+ */
		else if( token == 4 )
			return true;
	}

	return false;
}

const bool SM_Net::check_http_head ( const std::string &head_text )
{	
	changeURL = false;

	unsigned int find_return = head_text.find( "200 OK" );
	if( find_return != std::string::npos )
		return true;

	find_return = head_text.find( "202 Accepted" );
	if( find_return != std::string::npos )
		return true;

	find_return = head_text.find( "300 Multiple Choices" );
	if( find_return != std::string::npos )
		changeURL = true;

	find_return = head_text.find( "301 Moved Permanently" );
	if( find_return != std::string::npos )
		changeURL = true;

	find_return = head_text.find("302 Found"  );
	if( find_return != std::string::npos )
		changeURL = true;

	find_return = head_text.find( "303 See Other" );
	if( find_return != std::string::npos )
		changeURL = true;

	find_return = head_text.find( "304 Not Modified" );
	if( find_return != std::string::npos )
		changeURL = true;

	if( changeURL )
	{
		int posA = head_text.find( "Location:" ) + 10;
		int posB = head_text.find( "Keep-A" ) - 2;
		std::string new_url = head_text.substr( posA, posB - posA );

		SM_Util::msg_out( "NewURL: " + new_url );

		create ( new_url );

		return true;
	}

	SM_Util::msg_err( "Error: could not found file at server." );
	show_error( "HTTP Error", "Could not found\nfile at server." , false );

	return false;
}


	/* error function */

const int SM_Net::show_error ( const std::string &title, const std::string &msg, const bool &show_cancel ) const
{
	SM_Msg *sm_msg = new SM_Msg( *sm_win, title, msg, show_cancel );
	int awser = sm_msg->run_msg();
	delete sm_msg;
	return awser;
}


	/* action function */

const bool SM_Net::define_info ( const bool &ask )
{
	if( isHTTP() )
	{
		if( ask )
		{
			do
			{
				sm_httpSocket = new SM_HTTP ();
				if( ! sm_httpSocket )
				{
					SM_Util::msg_err( "Error: could not allocate http socket." );
					show_error( "Memory Error", "Could not allocate\nhttp socket." , false );
					delete sm_httpSocket;
					sm_httpSocket = NULL;
					return false;
				}
				if( ! sm_httpSocket->configure( sm_address, sm_port ) )
				{
					SM_Util::msg_err( "Error: could not connect to server." );
					show_error( "Socket Error", "Could not connect\nto server." , false );
					delete sm_httpSocket;
					sm_httpSocket = NULL;
					return false;
				}
				const std::string head_text = sm_httpSocket->get_head( sm_fileName );
				if( head_text.compare( "" ) == 0 )
				{
					SM_Util::msg_err( "Error: could not get http file head." );
					show_error( "Socket Error", "Could not get\nhttp file head." , false );
					delete sm_httpSocket;
					sm_httpSocket = NULL;
					return false;
				}
				if( ! check_http_head( head_text ) )
				{
					delete sm_httpSocket;
					sm_httpSocket = NULL;
					return false;
				}

				set_totalSize ( head_text );

				replace = true;
				donwload = false;
					
				delete sm_httpSocket;
				sm_httpSocket = NULL;
			} while( changeURL );
		}
	}
	else if ( isFTP() )
	{
		if( ask )
		{
			sm_ftpSocket = new SM_FTP();

			if( ! sm_ftpSocket )
			{
				SM_Util::msg_err( "Error: could not allocate ftp socket." );
				delete sm_ftpSocket;
				sm_ftpSocket = NULL;
				return false;
			}
			if( ! sm_ftpSocket->configure( sm_address, sm_port ) )
			{
				SM_Util::msg_err( "Error: could not connect to server." );
				delete sm_ftpSocket;
				sm_ftpSocket = NULL;
				return false;
			}
			if( ! sm_ftpSocket->user_conf( sm_download->user, sm_download->pass ) )
			{
				SM_Util::msg_err( "Error: could not authenticate." );
				delete sm_ftpSocket;
				sm_ftpSocket = NULL;
				return false;
			}
			if( ! sm_ftpSocket->set_type( true ) )
			{
				SM_Util::msg_err( "Error: could not set connection type." );
				delete sm_ftpSocket;
				sm_ftpSocket = NULL;
				return false;
			}

			long size = sm_ftpSocket->get_size( sm_fileName );
			if( size == -1 )
			{
				SM_Util::msg_err( "Error: could not get file size." );
				delete sm_ftpSocket;
				sm_ftpSocket = NULL;
				return false;
			}
			set_totalSize( size );

			if( ! sm_ftpSocket->quit() )
			{
				SM_Util::msg_err( "Error: could not quit connection." );
				delete sm_ftpSocket;
				sm_ftpSocket = NULL;
				return false;
			}
			replace = true;
			donwload = false;
			delete sm_ftpSocket;
			sm_ftpSocket = NULL;
		}
	}
	else
	{
		SM_Util::msg_err( "Error: invalid network protocol." );
		return false;
	}
		/* file exists? */
	if( ( access( sm_path.c_str(), R_OK ) ) != -1 )
	{
		std::ifstream *inFile = new std::ifstream( sm_path.c_str(), std::ios::binary );
		sm_size = SM_Util::fileLen( inFile );

		inFile->close();
		delete inFile;

		if( ! ask )
		{
			set_comSize( sm_size );
					return true;
		}
		if( ( access( sm_path.c_str(), W_OK ) ) == -1 )
		{
			SM_Util::msg_err( "Error: file already exist, without write access." );
			show_error( "File Error", "The file already exist.\nCould not access." , false );
			donwload = true;
			return false;
		}
			/* resume file? */
		if( ! donwload && sm_size < get_totalSize() )
		{
			int answer = show_error( "File Alert", "The file already exist.\nWould you like to resume?" , true );

			if( answer )
				SM_Util::msg_out( "resume file: true" );
			else
				SM_Util::msg_out( "resume file: false" );

			if( answer == 0 )
			{
				set_comSize( 0 );
				replace = true;
			}
			else
			{
				set_comSize(  sm_size );
				replace = false;
			}
		}

			/* replace file? */
		else if( ! donwload )
		{
			int answer = show_error( "File Alert", "The file already exist.\nWould you like to replace?" , true );

			if( answer )
				SM_Util::msg_out( "replace file: true" );
			else
				SM_Util::msg_out( "replace file: false" );
		
			if( answer == 0 )
			{
				donwload = true;
				return false;
			}
			else
				replace = true;
			set_comSize( 0 );
		}
	}
	else
	{
		set_comSize( 0 );
		replace = true;
	}
	return true;
}

const bool SM_Net::download_file ( void )
{
	if( ! define_info( sm_download->option == OPTION_NOT_ASKED ) )
		return false;
	if( sm_download->option == OPTION_NOT_ASKED )
	{
		sm_download->size      = get_totalSize();
		sm_download->completed = get_comSize();
		sm_download->option    = get_replace();
	}
	else
	{
		set_totalSize ( sm_download->size );
		set_replace   ( sm_download->option );
	}

	if( isHTTP() )
	{
		sm_httpSocket = new SM_HTTP ();
		if( ! sm_httpSocket )
		{
			SM_Util::msg_err( "Error: could not allocate http socket." );
			delete sm_httpSocket;
			sm_httpSocket = NULL;
			return false;
		}
		if( ! sm_httpSocket->configure( sm_address, sm_port ) )
		{
			SM_Util::msg_err( "Error: could not connect to server." );
			delete sm_httpSocket;
			sm_httpSocket = NULL;
			return false;
		}
		if( ! donwload )
		{
			running = true;

			sm_httpSocket->set_speedLimit( sm_speedLimit );

			bool get_return;
			if( replace && get_comSize() == 0 )
				get_return = sm_httpSocket->get_file( sm_download, sm_fileName, sm_path, get_totalSize(), replace );
			else
				get_return = sm_httpSocket->get_file( sm_download, sm_fileName, sm_path, get_totalSize() - sm_size, replace );

			running = false;
			delete sm_httpSocket;
			sm_httpSocket = NULL;

			if( get_return )
			{
				if( isCompleted() )
					sm_signal_completed( this );
				else
					sm_signal_paused( this );
				return true;
			}
			else
			{
				SM_Util::msg_err( "Error: could not get file." );
				return false;
			}
		}
		delete sm_httpSocket;
		sm_httpSocket = NULL;
		return true;
	}
	else if ( isFTP() )
	{
		sm_ftpSocket = new SM_FTP();

		if( ! sm_ftpSocket )
		{
			SM_Util::msg_err( "Error: could not allocate ftp socket." );
			delete sm_ftpSocket;
			sm_ftpSocket = NULL;
			return false;
		}
		if( ! sm_ftpSocket->configure( sm_address, sm_port ) )
		{
			SM_Util::msg_err( "Error: could not connect to server." );
			delete sm_ftpSocket;
			sm_ftpSocket = NULL;
			return false;
		}
		if( ! sm_ftpSocket->user_conf( sm_download->user, sm_download->pass ) )
		{
			SM_Util::msg_err( "Error: could not authenticate." );
			delete sm_ftpSocket;
			sm_ftpSocket = NULL;
			return false;
		}
		if( ! sm_ftpSocket->set_type( true ) )
		{
			SM_Util::msg_err( "Error: could not set connection type." );
			delete sm_ftpSocket;
			sm_ftpSocket = NULL;
			return false;
		}
		if( ! donwload )
		{
			running = true;

			sm_ftpSocket->set_speedLimit( sm_speedLimit );

			bool get_return;
			if( replace && get_comSize() == 0 )
				get_return = sm_ftpSocket->get_file( sm_download, sm_fileName, sm_path, 0, replace );
			else
				get_return = sm_ftpSocket->get_file( sm_download, sm_fileName, sm_path, sm_size, replace );

			running = false;

			if( get_return )
			{
				delete sm_ftpSocket;
				sm_ftpSocket = NULL;

				if( isCompleted() )
					sm_signal_completed( this );
				else
					sm_signal_paused( this );
				return true;
			}
			else
			{
				SM_Util::msg_err( "Error: could not make download." );

				if( ! sm_ftpSocket->quit() )
					SM_Util::msg_err( "Error: could not quit connection." );

				delete sm_ftpSocket;
				sm_ftpSocket = NULL;

				return false;
			}
		}
		delete sm_ftpSocket;
		sm_ftpSocket = NULL;
		return true;
	}
	else
	{
		SM_Util::msg_err( "Error: network protocol not supported." );
		return false;
	}	
}

const bool SM_Net::start ( void )
{
	sm_thread = Glib::Thread::create( sigc::mem_fun( *this, &SM_Net::thread_function ), false );
	if ( ! sm_thread )
	{
		SM_Util::msg_err( "Error: could not create new thread." );
		show_error( "Fatal Error", "Could not create\nnew thread." , false );
		::exit(EXIT_FAILURE);
	}
	return true;
}

const bool SM_Net::stop ( void )
{
	if( isHTTP() )
	{
		if( sm_httpSocket )
			return sm_httpSocket->set_stop();
		else
			return false;	
	}
	else if ( isFTP() )
	{
		if( sm_ftpSocket )
			return sm_ftpSocket->set_stop();
		else
			return false;
	}
	return false;
}


	/* thread function */

void SM_Net::thread_function ( void )
{
	usleep( 300 );

	if( ! download_file() )
		sm_signal_failed( this );
}


	/* signal function */

sigc::signal< void, SM_Net * > & SM_Net::signal_paused ( void )
{
	return sm_signal_paused;
}

sigc::signal< void, SM_Net * > & SM_Net::signal_completed ( void )
{
	return sm_signal_completed;
}

sigc::signal< void, SM_Net * > & SM_Net::signal_failed ( void )
{
	return sm_signal_failed;
}

/* EOF */

