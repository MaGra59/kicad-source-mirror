/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2019 KiCad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file eeschema/onleftclick.cpp
 */

#include <fctsys.h>
#include <kiway.h>
#include <eeschema_id.h>
#include <sch_draw_panel.h>
#include <confirm.h>
#include <sch_edit_frame.h>
#include <sim/sim_plot_frame.h>
#include <menus_helpers.h>

#include <sch_bus_entry.h>
#include <sch_text.h>
#include <sch_marker.h>
#include <sch_junction.h>
#include <sch_line.h>
#include <sch_no_connect.h>
#include <sch_component.h>
#include <sch_sheet.h>
#include <sch_sheet_path.h>
#include <sch_bitmap.h>

#include <netlist_object.h>
#include <class_library.h>      // for class SCHLIB_FILTER to filter power parts

#include <sch_view.h>

// TODO(hzeller): These pairs of elmenets should be represented by an object, but don't want
// to refactor too much right now to not get in the way with other code changes.
static SCH_BASE_FRAME::HISTORY_LIST s_CmpNameList;
static SCH_BASE_FRAME::HISTORY_LIST s_PowerNameList;


void SCH_EDIT_FRAME::OnLeftClick( wxDC* aDC, const wxPoint& aPosition )
{
    SCH_ITEM*   item = GetScreen()->GetCurItem();
    wxPoint     gridPosition = GetGridPosition( aPosition );
    // item_flags != 0 means a current item in edit, or new ...
    int item_flags = item ? (item->GetFlags() & ~HIGHLIGHTED) : 0;

    if( ( GetToolId() == ID_NO_TOOL_SELECTED ) || item_flags )
    {
        m_canvas->SetAutoPanRequest( false );
        SetRepeatItem( NULL );

        if( item_flags )
        {
            switch( item->Type() )
            {
            case SCH_LABEL_T:
            case SCH_GLOBAL_LABEL_T:
            case SCH_HIERARCHICAL_LABEL_T:
            case SCH_TEXT_T:
            case SCH_SHEET_PIN_T:
            case SCH_SHEET_T:
            case SCH_BUS_WIRE_ENTRY_T:
            case SCH_BUS_BUS_ENTRY_T:
            case SCH_JUNCTION_T:
            case SCH_COMPONENT_T:
            case SCH_FIELD_T:
            case SCH_BITMAP_T:
            case SCH_NO_CONNECT_T:
                AddItemToScreen( item );
                GetCanvas()->GetView()->ClearPreview();
                GetCanvas()->GetView()->ClearHiddenFlags();
                return;

            case SCH_LINE_T:    // May already be drawing segment.
                break;

            default:
                wxFAIL_MSG( wxT( "SCH_EDIT_FRAME::OnLeftClick error.  Item type <" ) +
                            item->GetClass() + wxT( "> is already being edited." ) );
                item->ClearFlags();
                break;
            }
        }
        else
        {
            item = LocateAndShowItem( aPosition );
        }
    }

    if( !item ) // If clicked on a empty area, clear any highligthed symbol
        GetCanvas()->GetView()->HighlightItem( nullptr, nullptr );

    switch( GetToolId() )
    {
    case ID_NO_TOOL_SELECTED:
        break;

    case ID_ZOOM_SELECTION:
        break;

    case ID_HIGHLIGHT_BUTT:
        // Moved to modern toolset
        break;

    case ID_NOCONN_BUTT:
        if( item_flags == 0 )
        {
            if( GetScreen()->GetItem( gridPosition, 0, SCH_NO_CONNECT_T ) == NULL )
            {
                SCH_NO_CONNECT*  no_connect = AddNoConnect( gridPosition );
                SetRepeatItem( no_connect );
                GetScreen()->SetCurItem( no_connect );
                m_canvas->SetAutoPanRequest( true );
            }
        }
        else
        {
            AddItemToScreen( item );
        }
        break;

    case ID_JUNCTION_BUTT:
        if( item_flags == 0 )
        {
            if( GetScreen()->GetItem( gridPosition, 0, SCH_JUNCTION_T ) == NULL )
            {
                SCH_JUNCTION* junction = AddJunction( gridPosition );
                SetRepeatItem( junction );
                GetScreen()->SetCurItem( junction );
                m_canvas->SetAutoPanRequest( true );
            }
        }
        else
        {
            AddItemToScreen( item );
        }
        break;

    case ID_WIRETOBUS_ENTRY_BUTT:
        if( item_flags == 0 )
        {
            CreateBusWireEntry();
            m_canvas->SetAutoPanRequest( true );
        }
        else
        {
            AddItemToScreen( item );
        }
        break;

    case ID_BUSTOBUS_ENTRY_BUTT:
        if( item_flags == 0 )
        {
            CreateBusBusEntry();
            m_canvas->SetAutoPanRequest( true );
        }
        else
        {
            AddItemToScreen( item );
        }
        break;

    case ID_SCHEMATIC_DELETE_ITEM_BUTT:
        DeleteItemAtCrossHair();
        break;

    case ID_WIRE_BUTT:
        BeginSegment( LAYER_WIRE );
        m_canvas->SetAutoPanRequest( true );
        break;

    case ID_BUS_BUTT:
        BeginSegment( LAYER_BUS );
        m_canvas->SetAutoPanRequest( true );
        break;

    case ID_LINE_COMMENT_BUTT:
        BeginSegment( LAYER_NOTES );
        m_canvas->SetAutoPanRequest( true );
        break;

    case ID_TEXT_COMMENT_BUTT:
        if( item_flags == 0 )
        {
            GetScreen()->SetCurItem( CreateNewText( LAYER_NOTES ) );
            m_canvas->SetAutoPanRequest( true );
        }
        else
        {
            AddItemToScreen( item );
        }
        break;

    case ID_ADD_IMAGE_BUTT:
        if( item_flags == 0 )
        {
            GetScreen()->SetCurItem( CreateNewImage( aDC ) );
            m_canvas->SetAutoPanRequest( true );
        }
        else
        {
            AddItemToScreen( item );
        }
        break;

    case ID_LABEL_BUTT:
        if( item_flags == 0 )
        {
            GetScreen()->SetCurItem( CreateNewText( LAYER_LOCLABEL ) );
            m_canvas->SetAutoPanRequest( true );
        }
        else
        {
            AddItemToScreen( item );
        }
        break;

    case ID_GLABEL_BUTT:
    case ID_HIERLABEL_BUTT:
        if( item_flags == 0 )
        {
            if( GetToolId() == ID_GLABEL_BUTT )
                GetScreen()->SetCurItem( CreateNewText( LAYER_GLOBLABEL ) );

            if( GetToolId() == ID_HIERLABEL_BUTT )
                GetScreen()->SetCurItem( CreateNewText( LAYER_HIERLABEL ) );

            m_canvas->SetAutoPanRequest( true );
        }
        else
        {
            AddItemToScreen( item );
        }
        break;

    case ID_SHEET_SYMBOL_BUTT:
        if( item_flags == 0 )
        {
            item = CreateSheet( aDC );

            if( item != NULL )
            {
                GetScreen()->SetCurItem( item );
                m_canvas->SetAutoPanRequest( true );
            }
        }
        else
        {
            AddItemToScreen( item );
        }
        break;

    case ID_IMPORT_HLABEL_BUTT:
    case ID_SHEET_PIN_BUTT:
        if( item_flags == 0 )
            item = LocateAndShowItem( aPosition, SCH_COLLECTOR::SheetsAndSheetLabels );

        if( item == NULL )
            break;

        if( (item->Type() == SCH_SHEET_T) && (item_flags == 0) )
        {
            if( GetToolId() == ID_IMPORT_HLABEL_BUTT )
                GetScreen()->SetCurItem( ImportSheetPin( (SCH_SHEET*) item ) );
            else
                GetScreen()->SetCurItem( CreateSheetPin( (SCH_SHEET*) item ) );
        }
        else if( (item->Type() == SCH_SHEET_PIN_T) && (item->GetFlags() != 0) )
        {
            AddItemToScreen( item );
        }
        break;

    case ID_SCH_PLACE_COMPONENT:
        // Moved to modern toolset
        break;

    case ID_PLACE_POWER_BUTT:
        // Moved to modern toolset
        break;

#ifdef KICAD_SPICE
    case ID_SIM_PROBE:
        {
            constexpr KICAD_T wiresAndComponents[] = { SCH_LINE_T,
                                                       SCH_COMPONENT_T,
                                                       SCH_SHEET_PIN_T,
                                                       EOT };
            item = LocateAndShowItem( aPosition, wiresAndComponents );

            if( !item )
                break;

            std::unique_ptr<NETLIST_OBJECT_LIST> netlist( BuildNetListBase() );

            for( NETLIST_OBJECT* obj : *netlist )
            {
                if( obj->m_Comp == item )
                {
                    SIM_PLOT_FRAME* simFrame = (SIM_PLOT_FRAME*) Kiway().Player( FRAME_SIMULATOR, false );

                    if( simFrame )
                        simFrame->AddVoltagePlot( obj->GetNetName() );

                    break;
                }
            }
        }
        break;

    case ID_SIM_TUNE:
        {
            constexpr KICAD_T fieldsAndComponents[] = { SCH_COMPONENT_T, SCH_FIELD_T, EOT };
            item = LocateAndShowItem( aPosition, fieldsAndComponents );

            if( !item )
                return;

            if( item->Type() != SCH_COMPONENT_T )
            {
                item = static_cast<SCH_ITEM*>( item->GetParent() );

                if( item->Type() != SCH_COMPONENT_T )
                    return;
            }

            SIM_PLOT_FRAME* simFrame = (SIM_PLOT_FRAME*) Kiway().Player( FRAME_SIMULATOR, false );

            if( simFrame )
                simFrame->AddTuner( static_cast<SCH_COMPONENT*>( item ) );
        }
        break;
#endif /* KICAD_SPICE */

    default:
        SetNoToolSelected();
        wxFAIL_MSG( wxT( "SCH_EDIT_FRAME::OnLeftClick invalid tool ID <" ) +
                    wxString::Format( wxT( "%d> selected." ), GetToolId() ) );
    }
}


/**
 * Function OnLeftDClick
 * called on a double click event from the drawpanel mouse handler
 *  if an editable item is found (text, component)
 *      Call the suitable dialog editor.
 *  Id a create command is in progress:
 *      validate and finish the command
 */
void SCH_EDIT_FRAME::OnLeftDClick( wxDC* aDC, const wxPoint& aPosition )

{
    EDA_ITEM* item = GetScreen()->GetCurItem();

    switch( GetToolId() )
    {
    case ID_NO_TOOL_SELECTED:
        if( ( item == NULL ) || ( item->GetFlags() == 0 ) )
        {
            item = LocateAndShowItem( aPosition, SCH_COLLECTOR::DoubleClickItems );
        }

        if( ( item == NULL ) || ( item->GetFlags() != 0 ) )
            break;

        switch( item->Type() )
        {
        case SCH_SHEET_T:
            g_CurrentSheet->push_back( (SCH_SHEET*) item );
            DisplayCurrentSheet();
            break;

        case SCH_COMPONENT_T:
            EditComponent( (SCH_COMPONENT*) item );
            GetCanvas()->MoveCursorToCrossHair();

            if( item->GetFlags() == 0 )
                GetScreen()->SetCurItem( NULL );

            GetCanvas()->Refresh();
            break;

        case SCH_TEXT_T:
        case SCH_LABEL_T:
        case SCH_GLOBAL_LABEL_T:
        case SCH_HIERARCHICAL_LABEL_T:
            EditSchematicText( (SCH_TEXT*) item );
            break;

        case SCH_BITMAP_T:

            // The bitmap is cached in Opengl: clear the cache, because
            // the cache data is perhaps invalid
            if( EditImage( (SCH_BITMAP*) item ) )
                GetCanvas()->GetView()->RecacheAllItems();

            break;

        case SCH_FIELD_T:
            EditComponentFieldText( (SCH_FIELD*) item );
            GetCanvas()->MoveCursorToCrossHair();
            break;

        case SCH_MARKER_T:
            ( (SCH_MARKER*) item )->DisplayMarkerInfo( this );
            break;

        default:
            break;
        }

        break;

    case ID_BUS_BUTT:
    case ID_WIRE_BUTT:
    case ID_LINE_COMMENT_BUTT:
        if( item && item->IsNew() )
            EndSegment();

        break;
    }
}
