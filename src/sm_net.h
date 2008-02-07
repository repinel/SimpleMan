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

#ifndef SM_NET_H
#define SM_NET_H

#include <glibmm/thread.h>

#include <string>

#include "sm_struct.h"


class SM_HTTP;
class SM_FTP;

class SM_Win;

class SM_Net
{
  public:

	SM_Net ( SM_Win *, SM_TASK_DATA * );
	virtual ~SM_Net ( void );

		/* initial functions */

	const bool create        ( const std::string & );

		/* set functions */

	const bool set_threadID   ( const int & );
	const bool set_id         ( const int & );
	const bool set_path       ( const std::string & );
	const bool set_name       ( const std::string & );
	const bool set_speed      ( const int & );
	const bool set_time       ( const std::string & );
	const bool set_comSize    ( const int & );
	const bool set_totalSize  ( const int & );
	const bool set_listPath   ( const std::string & );
	const bool set_enqueued   ( const bool & );

	const bool set_speedLimit ( const int & );

	const bool set_replace    ( const bool & );

		/* get functions */

	const int &         get_threadID   ( void ) const;
	const int &         get_id         ( void ) const;
	const std::string & get_name       ( void ) const;
	const int &         get_totalSize  ( void ) const;
	const int &         get_comSize    ( void ) const;
	const int &         get_percentage ( void ) const;
	const int &         get_speed      ( void ) const;
	const std::string & get_time       ( void ) const;
	const std::string & get_url        ( void ) const;
	const std::string & get_path       ( void ) const;
	const std::string & get_listPath   ( void ) const;

	const int           get_replace    ( void ) const;

		/* check functions */

	const bool isHTTP      ( void ) const;
	const bool isFTP       ( void ) const;
	const bool isSupported ( void ) const;
	const bool isCompleted ( void ) const;
	const bool isRunning   ( void ) const;

		/* action function */

	const bool define_info   ( const bool & );
	const bool start         ( void );
	const bool stop          ( void );

		/* signal function */

	sigc::signal< void, SM_Net * > & signal_paused    ( void );
	sigc::signal< void, SM_Net * > & signal_completed ( void );
	sigc::signal< void, SM_Net * > & signal_failed    ( void );

  private:

		/* set functions */
	
	const bool set_totalSize ( const std::string & );
	const bool set_url       ( const std::string & );
	const bool set_port      ( const std::string & );
	const bool set_address   ( const std::string & );
	const bool set_fileName  ( const std::string & );

		/* check function */

	const bool check_url       ( const std::string & ) const;
	const bool check_http_head ( const std::string & );

		/* error function */

	const int  show_error      ( const std::string &, const std::string &, const bool & ) const;

		/* action function */

	const bool download_file ( void );

		/* thread function */

	void   thread_function   ( void );


		/* socket */

	SM_HTTP *sm_httpSocket;
	SM_FTP  *sm_ftpSocket;

		/* protocol: 0 -> HTTP |  1 -> FTP | 2 -> invalid */

	int sm_protocol;

		/* file size in disc */

	long sm_size;

		/* thread id */

	int sm_threadID;

		/* speed limit */

	int sm_speedLimit;

		/* boolean options */

	bool replace;
	bool running;
	bool donwload;
	bool changeURL;

		/* socket information */

	unsigned int sm_port;

	std::string sm_address;
	std::string sm_fileName;

		/* local information */

	std::string sm_path;

		/* GUI information */

	SM_TASK_DATA *sm_download;

		/* window parent */

	SM_Win *sm_win;

		/* thread */

	Glib::Thread *sm_thread;

		/* signals */

	sigc::signal< void, SM_Net * > sm_signal_paused;
	sigc::signal< void, SM_Net * > sm_signal_completed;
	sigc::signal< void, SM_Net * > sm_signal_failed;
};

#endif /* SM_NET_H */


/* EOF */

