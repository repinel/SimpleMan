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

#include "sm_model_r.h"


SM_Model_R::SM_Model_R ( void )
{
	add( sm_col_id );
	add( sm_col_name );
	add( sm_col_status );
	add( sm_col_size );
	add( sm_col_completed );
	add( sm_col_percentage );
	add( sm_col_speed );
	add( sm_col_time );
	add( sm_col_url );
}


/* EOF */
