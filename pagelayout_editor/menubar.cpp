/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016-2019 KiCad Developers, see AUTHORS.txt for contributors.
 * Copyright (C) 2013 CERN
 * @author Jean-Pierre Charras, jp.charras at wanadoo.fr
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <kiface_i.h>
#include <menus_helpers.h>
#include <pgm_base.h>
#include <tool/conditional_menu.h>
#include <tool/tool_manager.h>
#include <tool/selection.h>
#include <tools/pl_actions.h>
#include <tools/pl_selection_tool.h>
#include "hotkeys.h"
#include "pl_editor_frame.h"
#include "pl_editor_id.h"


void PL_EDITOR_FRAME::ReCreateMenuBar()
{
    PL_SELECTION_TOOL* selTool = m_toolManager->GetTool<PL_SELECTION_TOOL>();
    // wxWidgets handles the Mac Application menu behind the scenes, but that means
    // we always have to start from scratch with a new wxMenuBar.
    wxMenuBar*  oldMenuBar = GetMenuBar();
    wxMenuBar*  menuBar = new wxMenuBar();

    auto modifiedDocumentCondition = [ this ] ( const SELECTION& sel ) {
        return GetScreen() && GetScreen()->IsModify();
    };

    wxString msg;
    static ACTION_MENU* openRecentMenu;  // Open Recent submenu,
                                         // static to remember this menu

    // Before deleting, remove the menus managed by m_fileHistory
    // (the file history will be updated when adding/removing files in history
    if( openRecentMenu )
        Kiface().GetFileHistory().RemoveMenu( openRecentMenu );

    // Recreate all menus:

    //
    // File Menu:
    //
    CONDITIONAL_MENU* fileMenu = new CONDITIONAL_MENU( false, selTool );

    openRecentMenu = new ACTION_MENU();
    openRecentMenu->SetTool( selTool );
    openRecentMenu->SetTitle( _( "Open Recent" ) );
    openRecentMenu->SetIcon( recent_xpm );

    Kiface().GetFileHistory().UseMenu( openRecentMenu );
    Kiface().GetFileHistory().AddFilesToMenu();

    fileMenu->AddItem( ACTIONS::doNew,         SELECTION_CONDITIONS::ShowAlways );
    fileMenu->AddItem( ACTIONS::open,          SELECTION_CONDITIONS::ShowAlways );
    fileMenu->AddMenu( openRecentMenu,         SELECTION_CONDITIONS::ShowAlways );

    fileMenu->AddSeparator();
    fileMenu->AddItem( ACTIONS::save,          modifiedDocumentCondition );
    fileMenu->AddItem( ACTIONS::saveAs,        SELECTION_CONDITIONS::ShowAlways );

    fileMenu->AddSeparator();
    fileMenu->AddItem( ACTIONS::pageSetup,     SELECTION_CONDITIONS::ShowAlways );
    fileMenu->AddItem( ACTIONS::print,         SELECTION_CONDITIONS::ShowAlways );

    fileMenu->AppendSeparator();
    fileMenu->AddItem( ACTIONS::quit,          SELECTION_CONDITIONS::ShowAlways );

    //
    // Edit Menu:
    //
    CONDITIONAL_MENU* editMenu = new CONDITIONAL_MENU( false, selTool );

    auto enableUndoCondition = [ this ] ( const SELECTION& sel ) {
        return GetScreen() && GetScreen()->GetUndoCommandCount() != 0;
    };
    auto enableRedoCondition = [ this ] ( const SELECTION& sel ) {
        return GetScreen() && GetScreen()->GetRedoCommandCount() != 0;
    };

    editMenu->AddItem( ACTIONS::undo,         enableUndoCondition );
    editMenu->AddItem( ACTIONS::redo,         enableRedoCondition );

    editMenu->AddSeparator();
    editMenu->AddItem( ACTIONS::doDelete,     SELECTION_CONDITIONS::MoreThan( 0 ) );

    //
    // View Menu:
    //
    CONDITIONAL_MENU* viewMenu = new CONDITIONAL_MENU( false, selTool );

    auto whiteBackgroundCondition = [ this ] ( const SELECTION& aSel ) {
        return GetDrawBgColor() == WHITE;
    };
    auto gridShownCondition = [ this ] ( const SELECTION& aSel ) {
        return IsGridVisible();
    };
    auto fullCrosshairCondition = [ this ] ( const SELECTION& aSel ) {
        return GetGalDisplayOptions().m_fullscreenCursor;
    };

    viewMenu->AddSeparator();
    viewMenu->AddItem( ACTIONS::zoomInCenter,                SELECTION_CONDITIONS::ShowAlways );
    viewMenu->AddItem( ACTIONS::zoomOutCenter,               SELECTION_CONDITIONS::ShowAlways );
    viewMenu->AddItem( ACTIONS::zoomFitScreen,               SELECTION_CONDITIONS::ShowAlways );
    viewMenu->AddItem( ACTIONS::zoomTool,                    SELECTION_CONDITIONS::ShowAlways );
    viewMenu->AddItem( ACTIONS::zoomRedraw,                  SELECTION_CONDITIONS::ShowAlways );

    viewMenu->AddSeparator();
    viewMenu->AddCheckItem( PL_ACTIONS::toggleBackground,    whiteBackgroundCondition );
    viewMenu->AddCheckItem( ACTIONS::toggleGrid,             gridShownCondition );
    viewMenu->AddCheckItem( ACTIONS::toggleCursorStyle,      fullCrosshairCondition );

    //
    // Place Menu:
    //
    CONDITIONAL_MENU* placeMenu = new CONDITIONAL_MENU( false, selTool );

    placeMenu->AddItem( PL_ACTIONS::drawLine,                SELECTION_CONDITIONS::ShowAlways );
    placeMenu->AddItem( PL_ACTIONS::drawRectangle,           SELECTION_CONDITIONS::ShowAlways );
    placeMenu->AddItem( PL_ACTIONS::placeText,               SELECTION_CONDITIONS::ShowAlways );
    placeMenu->AddItem( PL_ACTIONS::placeImage,              SELECTION_CONDITIONS::ShowAlways );

    placeMenu->AddSeparator();
    placeMenu->AddItem( PL_ACTIONS::appendImportedWorksheet, SELECTION_CONDITIONS::ShowAlways );

    //
    // Menu for preferences
    //
    wxMenu* preferencesMenu = new wxMenu;

    msg = AddHotkeyName( _( "&Preferences..." ), PlEditorHotkeysDescr, HK_PREFERENCES );
    AddMenuItem( preferencesMenu, wxID_PREFERENCES,  msg,
                 _( "Show preferences for all open tools" ),
                 KiBitmap( preference_xpm ) );

    // Language submenu
    Pgm().AddMenuLanguageList( preferencesMenu );

    // Append menus to the menubar
    menuBar->Append( fileMenu, _( "&File" ) );
    menuBar->Append( editMenu, _( "&Edit" ) );
    menuBar->Append( viewMenu, _( "&View" ) );
    menuBar->Append( placeMenu, _( "&Place" ) );
    menuBar->Append( preferencesMenu, _( "P&references" ) );
    AddStandardHelpMenu( menuBar );

    SetMenuBar( menuBar );
    delete oldMenuBar;
}
