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

#include <gtkmm/stock.h>

#include "sm_new.h"

#define SIZE_X 500
#define SIZE_Y 268


SM_New::SM_New ( Gtk::Window &win, const SM_CONFIG_DATA &sm_config )
	: Gtk::Dialog ( "New Task", win, false, false ),
	  sm_defaultPass( sm_config.defaultPass ),
	  sm_boxFrameFile( false, 0 ),
	  sm_boxURL( false, 0 ),
	  sm_boxDir( false, 0 ),
	  sm_boxName( false, 0 ),
	  sm_boxRadio( false, 0 ),
	  sm_LabelURL(  "  URL:          ", false ),
	  sm_LabelDir(  "  Save To:    ", false ),
	  sm_LabelName( "  Name:       ", false ),
	  sm_LabelUser( "  User:         ", false ),
	  sm_LabelPass( "  Pass:         ", false ),
	  sm_Button_Folder ( "..." ),
	  sm_RadioButtonOn( "On" ),
	  sm_RadioButtonOff( "Off" ),
	  sm_group ( sm_RadioButtonOn.get_group() ),
	  sm_AlignmentRadio( Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, 0.0, 0.0 )
{
	set_size_request( SIZE_X, SIZE_Y );
	set_resizable ( false );
	
	sm_newBox = get_vbox();
	sm_newBox->set_homogeneous( false );
	sm_newBox->set_spacing( 0 );

		/* File Options */
	
	sm_FrameFile.set_label( "File Options" );
	sm_FrameFile.set_label_align( Gtk::ALIGN_CENTER, Gtk::ALIGN_TOP );
	sm_FrameFile.set_shadow_type( Gtk::SHADOW_ETCHED_OUT );	
	sm_newBox->pack_start( sm_FrameFile, Gtk::PACK_SHRINK, 3 );
	
	sm_EntryURL.set_width_chars( 50 );
	sm_EntryURL.set_activates_default( true );
	sm_EntryURL.signal_changed().connect( sigc::mem_fun( *this, &SM_New::on_changed_url ) );
	sm_boxURL.pack_start( sm_LabelURL, Gtk::PACK_SHRINK, 0 );
	sm_boxURL.pack_start( sm_EntryURL, Gtk::PACK_SHRINK, 0 );
	
	sm_EntryDir.set_text( sm_config.defaultPath );
	sm_EntryDir.set_width_chars( 47 );
	sm_EntryDir.set_activates_default( true );
	sm_Button_Folder.signal_clicked().connect( sigc::mem_fun( *this, &SM_New::on_button_folder_new ) );
	sm_boxDir.pack_start( sm_LabelDir, Gtk::PACK_SHRINK, 0 );
	sm_boxDir.pack_start( sm_EntryDir, Gtk::PACK_SHRINK, 0 );
	sm_boxDir.pack_start( sm_Button_Folder, Gtk::PACK_SHRINK, 0 );
	
	sm_EntryName.set_width_chars( 50 );
	sm_EntryName.set_activates_default( true );
	sm_boxName.pack_start( sm_LabelName, Gtk::PACK_SHRINK, 0 );
	sm_boxName.pack_start( sm_EntryName, Gtk::PACK_SHRINK, 0 );
	
	sm_FrameFile.add( sm_boxFrameFile );
	sm_boxFrameFile.pack_start( sm_boxURL, Gtk::PACK_SHRINK, 0 );
	sm_boxFrameFile.pack_start( sm_boxDir, Gtk::PACK_SHRINK, 0 );
	sm_boxFrameFile.pack_start( sm_boxName, Gtk::PACK_SHRINK, 0 );

		/* FTP Options */

	sm_FrameFTP.set_label( "FTP Options" );
	sm_FrameFTP.set_label_align( Gtk::ALIGN_CENTER, Gtk::ALIGN_TOP );
	sm_FrameFTP.set_shadow_type( Gtk::SHADOW_ETCHED_OUT );
	sm_newBox->pack_start( sm_FrameFTP, Gtk::PACK_SHRINK, 3 );

	sm_EntryUser.set_text( "anonymous" );
	sm_EntryUser.set_width_chars( 50 );
	sm_EntryUser.set_activates_default( true );
	sm_boxUser.pack_start( sm_LabelUser, Gtk::PACK_SHRINK, 0 );
	sm_boxUser.pack_start( sm_EntryUser, Gtk::PACK_SHRINK, 0 );

	sm_EntryPass.set_text( sm_defaultPass );
	sm_EntryPass.set_width_chars( 50 );
	sm_EntryPass.set_activates_default( true );
	sm_boxPass.pack_start( sm_LabelPass, Gtk::PACK_SHRINK, 0 );
	sm_boxPass.pack_start( sm_EntryPass, Gtk::PACK_SHRINK, 0 );

	sm_FrameFTP.add( sm_boxFrameFTP );
	sm_boxFrameFTP.pack_start( sm_boxUser, Gtk::PACK_SHRINK, 0 );
	sm_boxFrameFTP.pack_start( sm_boxPass, Gtk::PACK_SHRINK, 0 );

		/* Starting now? */
		
	sm_FrameStartOptions.set_label( "Starting now?" );
	sm_FrameStartOptions.set_label_align( Gtk::ALIGN_CENTER, Gtk::ALIGN_TOP );
	sm_FrameStartOptions.set_shadow_type( Gtk::SHADOW_ETCHED_OUT );	
	sm_newBox->pack_start( sm_FrameStartOptions, Gtk::PACK_SHRINK, 3 );

	sm_RadioButtonOff.set_group( sm_group );
	sm_AlignmentRadio.add( sm_boxRadio );
	sm_boxRadio.pack_start( sm_RadioButtonOn, Gtk::PACK_SHRINK, 0 );
	sm_boxRadio.pack_start( sm_RadioButtonOff, Gtk::PACK_SHRINK, 0 );
	sm_FrameStartOptions.add( sm_AlignmentRadio );

	set_start( sm_config.startNow );

		/* buttons */

	add_button( Gtk::Stock::CANCEL, 0 );
	add_button( Gtk::Stock::OK, 1 );

	set_default_response( 1 );	
	activate_default();

	show_all_children();	
}


	/* event functions */

void SM_New::on_changed_url ( void )
{
	std::string url = sm_EntryURL.get_text();

	unsigned int posA = url.find( "://" );

		/* 3 or 4 chars bebore :// ( ftp or http ) */
	if( posA != std::string::npos && ( posA == 3 || posA == 4 ) )
	{
		posA += 3;

			/* define file name */

		unsigned int posB = url.find_last_of( "/" );
		if ( posB != std::string::npos && posB > posA )
		{
			posB++;
			sm_EntryName.set_text( url.substr( posB, url.length() - posB ) );
		}

			/* define ftp options */

		posB = url.find( "@", posA );
		if( posB != std::string::npos && posB > posA )
		{
			unsigned int posC = url.find( "/", posA );
			if( posC != std::string::npos && posC > posB )
			{
				posC = url.find( ":", posA );
				if( posC != std::string::npos && posC < posB && posC > posA )
				{
					sm_EntryUser.set_text( url.substr( posA, posC - posA ) );
					posC++;
					sm_EntryPass.set_text( url.substr( posC, posB - posC ) );
				}
			}
		}
		else
		{
			sm_EntryUser.set_text( "anonymous" );
			sm_EntryPass.set_text( sm_defaultPass );			
		}
	}	
}

void SM_New::on_button_folder_new ( void )
{
	Gtk::FileChooserDialog sm_folderDialog( "Please choose a folder", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER );
	sm_folderDialog.set_transient_for( *this );

		/* add response buttons the the dialog */

	sm_folderDialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
	sm_folderDialog.add_button( "Select", Gtk::RESPONSE_OK );

	int result = sm_folderDialog.run();

		/* handle the response */

	if( result == Gtk::RESPONSE_OK )
		sm_EntryDir.set_text( sm_folderDialog.get_filename() );
}


	/* set functions */

void SM_New::set_url ( const std::string &url )
{
	sm_EntryURL.set_text( url );
	on_changed_url();
}

void SM_New::set_start ( const bool & startNow )
{
	if( startNow )
		sm_RadioButtonOn.set_active();
	else
		sm_RadioButtonOff.set_active();		
}


	/* get functions */

const std::string SM_New::get_url ( void ) const
{
	return sm_EntryURL.get_text();
}

const std::string SM_New::get_dir ( void ) const
{
	return sm_EntryDir.get_text();
}

const std::string SM_New::get_name ( void ) const
{
	return sm_EntryName.get_text();
}

const std::string SM_New::get_user ( void ) const
{
	return sm_EntryUser.get_text();
}

const std::string SM_New::get_pass ( void ) const
{
	return sm_EntryPass.get_text();
}

const bool SM_New::get_start ( void ) const
{
	return sm_RadioButtonOn.get_active();
}


/* EOF */

