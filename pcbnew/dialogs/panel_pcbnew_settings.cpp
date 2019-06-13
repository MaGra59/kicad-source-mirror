/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
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

#include <fctsys.h>
#include <pcbnew.h>
#include <pcb_edit_frame.h>
#include <board_design_settings.h>
#include <pcbnew_id.h>
#include <class_board.h>
#include <panel_pcbnew_settings.h>
#include <widgets/paged_dialog.h>
#include <pcb_view.h>
#include <pcb_painter.h>

PANEL_PCBNEW_SETTINGS::PANEL_PCBNEW_SETTINGS( PCB_EDIT_FRAME* aFrame, PAGED_DIALOG* aParent ) :
        PANEL_PCBNEW_SETTINGS_BASE( aParent->GetTreebook() ),
        m_Frame( aFrame )
{}


bool PANEL_PCBNEW_SETTINGS::TransferDataToWindow()
{
    const PCB_DISPLAY_OPTIONS* displ_opts = (PCB_DISPLAY_OPTIONS*) m_Frame->GetDisplayOptions();

    /* Set display options */
    m_PolarDisplay->SetSelection( m_Frame->GetShowPolarCoords() ? 1 : 0 );
    m_UnitsSelection->SetSelection( m_Frame->GetUserUnits() == INCHES ? 0 : 1 );
    m_OptDisplayCurvedRatsnestLines->SetValue( displ_opts->m_DisplayRatsnestLinesCurved );
    m_showGlobalRatsnest->SetValue( displ_opts->m_ShowGlobalRatsnest );
    m_showSelectedRatsnest->SetValue( displ_opts->m_ShowModuleRatsnest );
    m_OptDisplayCurvedRatsnestLines->SetValue( displ_opts->m_DisplayRatsnestLinesCurved );

    wxString rotationAngle;
    rotationAngle = AngleToStringDegrees( (double)m_Frame->GetRotationAngle() );
    m_RotationAngle->SetValue( rotationAngle );

    m_Segments_45_Only_Ctrl->SetValue( PCB_GENERAL_SETTINGS::g_Use45DegreeGraphicSegments );
    m_magneticPadChoice->SetSelection( PCB_GENERAL_SETTINGS::g_MagneticPads );
    m_magneticTrackChoice->SetSelection( PCB_GENERAL_SETTINGS::g_MagneticTracks );
    m_magneticGraphicsChoice->SetSelection( !PCB_GENERAL_SETTINGS::g_MagneticGraphics );
    m_UseEditKeyForWidth->SetValue( PCB_GENERAL_SETTINGS::g_EditHotkeyChangesTrackWidth );
    m_dragSelects->SetValue( PCB_GENERAL_SETTINGS::g_DragSelects );

    m_Show_Page_Limits->SetValue( m_Frame->ShowPageLimits() );

    return true;
}


bool PANEL_PCBNEW_SETTINGS::TransferDataFromWindow()
{
    m_Frame->SetShowPolarCoords( m_PolarDisplay->GetSelection() != 0 );
    m_Frame->SetUserUnits( m_UnitsSelection->GetSelection() == 0 ? INCHES : MILLIMETRES );

    m_Frame->SetRotationAngle( wxRound( 10.0 * wxAtof( m_RotationAngle->GetValue() ) ) );

    /* Updating the combobox to display the active layer. */

    PCB_GENERAL_SETTINGS::g_Use45DegreeGraphicSegments = m_Segments_45_Only_Ctrl->GetValue();
    PCB_GENERAL_SETTINGS::g_MagneticPads = (MAGNETIC_OPTIONS) m_magneticPadChoice->GetSelection();
    PCB_GENERAL_SETTINGS::g_MagneticTracks = (MAGNETIC_OPTIONS) m_magneticTrackChoice->GetSelection();
    PCB_GENERAL_SETTINGS::g_MagneticGraphics = !m_magneticGraphicsChoice->GetSelection();
    PCB_GENERAL_SETTINGS::g_EditHotkeyChangesTrackWidth = m_UseEditKeyForWidth->GetValue();
    PCB_GENERAL_SETTINGS::g_DragSelects = m_dragSelects->GetValue();

    m_Frame->SetShowPageLimits( m_Show_Page_Limits->GetValue() );

    // Apply changes to the GAL
    PCB_DISPLAY_OPTIONS*        displ_opts = (PCB_DISPLAY_OPTIONS*) m_Frame->GetDisplayOptions();
    KIGFX::VIEW*                view = m_Frame->GetGalCanvas()->GetView();
    KIGFX::PCB_PAINTER*         painter = static_cast<KIGFX::PCB_PAINTER*>( view->GetPainter() );
    KIGFX::PCB_RENDER_SETTINGS* settings = painter->GetSettings();

    displ_opts->m_DisplayRatsnestLinesCurved = m_OptDisplayCurvedRatsnestLines->GetValue();
    displ_opts->m_ShowGlobalRatsnest = m_showGlobalRatsnest->GetValue();
    displ_opts->m_ShowModuleRatsnest = m_showSelectedRatsnest->GetValue();

    settings->LoadDisplayOptions( displ_opts, m_Frame->ShowPageLimits() );
    view->RecacheAllItems();
    view->MarkTargetDirty( KIGFX::TARGET_NONCACHED );

    m_Frame->GetGalCanvas()->Refresh();

    return true;
}


