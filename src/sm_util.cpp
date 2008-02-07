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

#include <iostream>
#include <fstream>

#include "sm_util.h"

	/* 64MB */
#define MAX_LOG_BYTES 67108864

	/* log path */
std::string sm_logPath;


	/* converter functions */

const std::string SM_Util::itoa( const int &n )
{
	char *ptr = new char[12];

	sprintf(ptr, "%d", n);

	return ptr;
}

const std::string SM_Util::dtoa( const double &n )
{
	int count = 0, num = n;

	do
	{
		num /= 10;
		
		count++;
	} while (num != 0);
	
	char *ptr = new char[count+5];
	
	sprintf(ptr, "%.1lf", n);
	
	return ptr;
}


	/* file functions */

const long SM_Util::fileLen( std::ifstream *inFile )
{
	long posicaoAtual = inFile->tellg(),
		 size = 0;

	inFile->seekg( 0, std::ios::end );
	size = inFile->tellg();

	inFile->seekg( posicaoAtual );

	return size;
}


	/* show functions */

const std::string SM_Util::show_size ( const int &x )
{
	if( x <= 0 )
		return "";

		/* Bytes */
	if( x < 1024 )
		return itoa( x ) + " B";

		/* KBytes */
	else if( x >= 1024 && x < 1048576 )
	{
		double n = ( double ) x / 1024;
		return dtoa( n ) + " KB";
	}

		/* MBytes */
	else if( x >= 1048576 && x < 1073741824 )
	{
		double n = ( double ) x / 1048576;
		return dtoa( n ) + " MB";
	}

		/* GBytes */
	else
	{
		double n = ( double ) x / 1073741824;
		return dtoa( n ) + " GB";
	}
}

const std::string SM_Util::show_speed ( const int &x )
{
	if( x <= 0 )
		return "";

	return itoa( x ) + " KB/s";
}

const std::string SM_Util::show_time ( const int &x )
{
	std::string time_ready;

		/* seconds */
	if( x < 60 )
	{
		if( ( x / 10 ) == 0 )
			time_ready = "00:00:0" + itoa( x );
		else
			time_ready = "00:00:" + itoa( x );
	}
		/* seconds and minutes */
	else if( x < 3600 )
	{
		int s = x % 60;
		int m = x / 60;
		
		if( ( s / 10 ) == 0 || ( m / 10 ) == 0 )
		{
			if( ( s / 10 ) == 0 && ( m / 10 ) == 0 )
				time_ready = "00:0" + itoa( m ) + ":0" + itoa( s );
			else if( ( s / 10 ) == 0 )
				time_ready = "00:" + itoa( m ) + ":0" + itoa( s );
			else
				time_ready = "00:0" + itoa( m ) + ":" + itoa( s );
		}
		else
			time_ready = "00:" + itoa( m ) + ":" + itoa( s );
	}
		/* seconds, minutes and hours */
	else
	{
		int s = x % 60;
		int m = x / 60;
		int h = m / 60;
		m %= 60;
		
		if( ( s / 10 ) == 0 || ( m / 10 ) == 0 || ( h / 10 ) == 0 )
		{
			if( ( s / 10 ) == 0 && ( m / 10 ) == 0 && ( h / 10 ) == 0 )
				time_ready = "0" + itoa( h ) + ":0" + itoa( m ) + ":0" + itoa( s );
			else if( ( s / 10 ) == 0 && ( m / 10 ) == 0 )
				time_ready = itoa( h ) + ":0" + itoa( m ) + ":0" + itoa( s );
			else if( ( s / 10 ) == 0 && ( h / 10 ) == 0 )
				time_ready = "0" + itoa( h ) + ":" + itoa( m ) + ":0" + itoa( s );
			else if( ( m / 10 ) == 0 && ( h / 10 ) == 0 )
				time_ready = "0" + itoa( h ) + ":0" + itoa( m ) + ":" + itoa( s );
			else
				time_ready = "0" + itoa( h ) + ":" + itoa( m ) + ":" + itoa( s );
		}
		else
			time_ready = itoa( h ) + ":" + itoa( m ) + ":" + itoa( s );
	}

	return time_ready;
}

	/* log functions */

void SM_Util::set_logPath ( const std::string &new_logPath )
{
	sm_logPath = new_logPath;
}

void SM_Util::msg_out ( const std::string &msg )
{
#ifdef DEBUG
	std::cout << msg << std::endl;
#endif /* DEBUG */

	write_log( msg );
}

void SM_Util::msg_err ( const std::string &msg )
{
	std::cerr << msg << std::endl;
	write_log( msg );
}

void SM_Util::check_log ( void )
{
	std::ifstream *inFile = new std::ifstream( sm_logPath.c_str(), std::ios::binary );

	long sm_size = SM_Util::fileLen( inFile );

	inFile->close();
	delete inFile;

		/* reset log */
	if( sm_size >= MAX_LOG_BYTES )
	{
		std::ofstream outFile( sm_logPath.c_str(), std::ios::trunc | std::ios::binary );
		outFile.close();
	}

	msg_out( "\n-- SIMPLEMAN --\n\n" );
}

void SM_Util::write_log ( const std::string &msg )
{
	std::ofstream outFile( sm_logPath.c_str(), std::ios::app | std::ios::binary );

	if( outFile )
		outFile << msg << std::endl;

	outFile.close();
}

/* EOF */

