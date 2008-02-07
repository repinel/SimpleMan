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

#ifndef SM_UTIL_H
#define SM_UTIL_H

#include <string>


class SM_Util
{
  public:

 	SM_Util ( void ) {};
	virtual ~SM_Util ( void ) {};

		/* converter functions */

	static const std::string itoa        ( const int & );
	static const std::string dtoa        ( const double & );

		/* file functions */

	static const long        fileLen     ( std::ifstream * );

		/* show functions */

	static const std::string show_size   ( const int & );
	static const std::string show_speed  ( const int & );
	static const std::string show_time   ( const int & );

		/* log functions */

	static void              set_logPath ( const std::string & );
	static void              msg_out     ( const std::string & );
	static void              msg_err     ( const std::string & );
	static void              check_log   ( void );

  private:

	static void              write_log   ( const std::string & );
};


#endif /* SM_UTIL_H */

/* EOF */

