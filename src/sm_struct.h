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

#ifndef SM_INCLUDES_H
#define SM_INCLUDES_H

#include <string>


typedef struct _SM_CONFIG_DATA {
	int         tasksLimit;
	int         speedLimit;
	bool        startNow;
	std::string defaultPath;
	std::string defaultPass;
} SM_CONFIG_DATA;

typedef struct _SM_TASK_DATA {
	int         t;
	int         option;
	int         id;
	int         size;
	int         completed;
	int         percentage;
	int         speed;
	std::string name;
	std::string time;
	std::string url;
	std::string user;
	std::string pass;
	std::string dir;
	std::string listPath;
} SM_TASK_DATA;

typedef struct _SM_COMPLETED_DATA {
	std::string name;
	std::string size;
	std::string url;
	std::string listPath;
} SM_COMPLETED_DATA;

typedef struct _SM_FAILED_DATA {
	int         percentage;
	std::string name;
	std::string size;
	std::string completed;
	std::string url;
	std::string listPath;
} SM_FAILED_DATA;


#endif /* SM_INCLUDES_H */

/* EOF */
