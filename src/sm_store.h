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

#ifndef SM_STORE_H
#define SM_STORE_H

#include <iostream>

template < class T >
struct SM_TASK_STORE {
	T *data;

	struct SM_TASK_STORE *prev;
	struct SM_TASK_STORE *next;
};


template < class T >
class SM_Store
{
  public:

	SM_Store ( void )
	{
		sm_size  = 0;
		sm_first = NULL;
		sm_last  = NULL;
	}

	virtual ~SM_Store ( void )
	{
		struct SM_TASK_STORE < T > *temp1 = sm_first;
		struct SM_TASK_STORE < T > *temp2;

		while ( temp1 )
		{
			temp2 = temp1;
			
			temp1 = temp1->next;

			delete temp2;
		}
	}

		/* pop and push functions */

	T * pop_front ( void )
	{
		if( ! sm_first )
			return NULL;

		T *data = sm_first->data;

		if( ! erase( 0 ) )
			return NULL;

		return data;
	}

	const bool push_back ( T *new_data )
	{
		struct SM_TASK_STORE < T > *temp = new struct SM_TASK_STORE < T >;

		if( ! temp )
			return false;

		temp->data = new_data;
		temp->prev = sm_last;
		temp->next = NULL;

		if( ! sm_first )
			sm_first = temp;

		if( sm_last )
			sm_last->next = temp;

		sm_last = temp;

		sm_size++;

		return true;
	}

		/* get functions */

	T  * get_front ( void )
	{
		if( sm_first )
			return sm_first->data;

		return NULL;
	}

	T  * get_data ( const int pos )
	{
		struct SM_TASK_STORE < T > *temp = find( pos );
		if( ! temp )
			return NULL;

		return temp->data;
	}

	struct SM_TASK_STORE < T > * get_first ( void )
	{
		return sm_first;
	}

	struct SM_TASK_STORE < T > * get_last ( void )
	{
		return sm_last;
	}

	const int get_size ( void )
	{
		return sm_size;
	}


		/* usual functions */

	const bool insert ( const int &pos, T *new_data )
	{
		struct SM_TASK_STORE < T > *current = find( pos );

		if( ! current )
			return push_back( new_data );

		struct SM_TASK_STORE < T > *temp = new struct SM_TASK_STORE < T >;

		if( ! temp )
			return false;

		temp->data = new_data;
		temp->prev = current->prev;
		temp->next = current;

		current->prev = temp;

		if( temp->prev )
			( temp->prev )->next = temp;
		else
			sm_first = temp;

		sm_size++;

		return true;
	}

	const bool erase ( const int &pos )
	{
		struct SM_TASK_STORE < T > *temp = find( pos );

		if( ! temp )
			return false;

		if( temp->prev )
			( temp->prev )->next = temp->next;
		else
			sm_first = temp->next;

		if( temp->next )
			( temp->next )->prev = temp->prev;
		else
			sm_last = temp->prev;

		delete temp;

		sm_size--;

		return true;
	}

	const bool clear ( void )
	{
		int size = sm_size;
		for( int i = 0; i < size; i++ )
		{
			if( ! erase( 0 ) )
				return false;
		}
		return true;
	}

	const bool swap ( const int &pos_a, const int &pos_b )
	{
		if( pos_a >= sm_size || pos_b >= sm_size )
			return false;

		struct SM_TASK_STORE < T > *ptr_a = find( pos_a );
		struct SM_TASK_STORE < T > *ptr_b = find( pos_b );

		if( ! ptr_a || ! ptr_b )
			return false;

		T *data_a = ptr_a->data;
		T *data_b = ptr_b->data;

		erase( pos_a );
		insert( pos_a, data_b );

		erase( pos_b );
		insert( pos_b, data_a );

		return true;
	}

	void print  ( void )
	{
		struct SM_TASK_STORE < T > *temp = get_first();

		std::cout << std::endl;
		while( temp )
		{
			std::cout << "Name: " << ( temp->data )->name << std::endl;
			temp = temp->next;
		}
	}

	void rprint ( void )
	{
		struct SM_TASK_STORE < T > *temp = get_last();

		std::cout << std::endl;
		while( temp )
		{
			std::cout << "Name: " << ( temp->data )->name << std::endl;
			temp = temp->prev;
		}
	}


  private:

		/* find function */

	struct SM_TASK_STORE < T > * find ( const int &pos )
	{
		struct SM_TASK_STORE < T > *temp = get_first();
		for( int i = 0; temp; i++, temp = temp->next )
		{
			if( i == pos )
				return temp;
		}
		return NULL;
	}


	int sm_size;

	struct SM_TASK_STORE < T > *sm_first;
	struct SM_TASK_STORE < T > *sm_last;
};


#endif /* SM_STORE_H */


/* EOF */

