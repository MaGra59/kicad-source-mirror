/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 Cirilo Bernardo <cirilo.bernardo@gmail.com>
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


#include <iostream>

#include "vrml2_base.h"
#include "vrml2_shape.h"


WRL2SHAPE::WRL2SHAPE() : WRL2NODE()
{
    appearance = NULL;
    geometry = NULL;
    m_Type = WRL2_SHAPE;
    return;
}


WRL2SHAPE::WRL2SHAPE( WRL2NODE* aParent ) : WRL2NODE()
{
    appearance = NULL;
    geometry = NULL;
    m_Type = WRL2_SHAPE;
    m_Parent = aParent;

    if( NULL != m_Parent )
        m_Parent->AddChildNode( this );

    return;
}


WRL2SHAPE::~WRL2SHAPE()
{
    #ifdef DEBUG
    std::cerr << " * [INFO] Destroying Shape with " << m_Children.size();
    std::cerr << " children, " << m_Refs.size() << " references and ";
    std::cerr << m_BackPointers.size() << " backpointers\n";
    #endif
    return;
}


bool WRL2SHAPE::isDangling( void )
{
    // this node is dangling unless it has a parent of type WRL2_TRANSFORM

    if( NULL == m_Parent || m_Parent->GetNodeType() != WRL2_TRANSFORM )
        return true;

    return false;
}


bool WRL2SHAPE::AddRefNode( WRL2NODE* aNode )
{
    if( NULL == aNode )
    {
        #ifdef DEBUG
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] NULL passed for aNode\n";
        #endif

        return false;
    }

    WRL2NODES type = aNode->GetNodeType();

    if( !checkNodeType( type ) )
    {
        #ifdef DEBUG
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [INFO] bad file format; unexpected child node '";
        std::cerr << aNode->GetNodeTypeName( type ) << "'\n";
        #endif

        return false;
    }

    if( WRL2_APPEARANCE == type )
    {
        if( NULL != appearance )
        {
            #ifdef DEBUG
            std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [INFO] bad file format; multiple appearance nodes\n";
            #endif
            return false;
        }

        appearance = aNode;
        return WRL2NODE::AddRefNode( aNode );
    }

    if( NULL != geometry )
    {
        #ifdef DEBUG
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [INFO] bad file format; multiple geometry nodes\n";
        #endif
        return false;
    }

    geometry = aNode;
    return WRL2NODE::AddRefNode( aNode );
}


bool WRL2SHAPE::AddChildNode( WRL2NODE* aNode )
{
    if( NULL == aNode )
    {
        #ifdef DEBUG
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] NULL passed for aNode\n";
        #endif

        return false;
    }

    WRL2NODES type = aNode->GetNodeType();

    if( !checkNodeType( type ) )
    {
        #ifdef DEBUG
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [INFO] bad file format; unexpected child node '";
        std::cerr << aNode->GetNodeTypeName( type ) << "'\n";
        #endif

        return false;
    }

    if( WRL2_APPEARANCE == type )
    {
        if( NULL != appearance )
        {
            #ifdef DEBUG
            std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [INFO] bad file format; multiple appearance nodes\n";
            #endif
            return false;
        }

        appearance = aNode;
        return WRL2NODE::AddChildNode( aNode );
    }

    if( NULL != geometry )
    {
        #ifdef DEBUG
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [INFO] bad file format; multiple geometry nodes\n";
        #endif
        return false;
    }

    geometry = aNode;
    return WRL2NODE::AddChildNode( aNode );
}


bool WRL2SHAPE::checkNodeType( WRL2NODES aType )
{
    switch( aType )
    {
    case WRL2_APPEARANCE:
    case WRL2_BOX:
    case WRL2_CONE:
    case WRL2_CYLINDER:
    case WRL2_ELEVATIONGRID:
    case WRL2_EXTRUSION:
    case WRL2_INDEXEDFACESET:
    case WRL2_INDEXEDLINESET:
    case WRL2_POINTSET:
    case WRL2_SPHERE:
    case WRL2_TEXT:
        break;

    default:
        return false;
        break;
    }

    return true;
}


bool WRL2SHAPE::Read( WRLPROC& proc, WRL2BASE* aTopNode )
{
    if( NULL == aTopNode )
    {
        #ifdef DEBUG
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] aTopNode is NULL\n";
        #endif
        return false;
    }

    size_t line, column;
    proc.GetFilePosData( line, column );

    char tok = proc.Peek();

    if( proc.eof() )
    {
        #ifdef DEBUG
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [INFO] bad file format; unexpected eof at line ";
        std::cerr << line << ", column " << column << "\n";
        #endif
        return false;
    }

    if( '{' != tok )
    {
        #ifdef DEBUG
        std::cerr << proc.GetError() << "\n";
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [INFO] bad file format; expecting '{' but got '" << tok;
        std::cerr  << "' at line " << line << ", column " << column << "\n";
        #endif

        return false;
    }

    proc.Pop();
    std::string glob;

    while( true )
    {
        if( proc.Peek() == '}' )
        {
            proc.Pop();
            break;
        }

        if( !proc.ReadName( glob ) )
        {
            #ifdef DEBUG
            std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << proc.GetError() <<  "\n";
            #endif

            return false;
        }

        // expecting one of:
        // appearance
        // geometry

        proc.GetFilePosData( line, column );

        if( !glob.compare( "appearance" ) )
        {
            if( !aTopNode->ReadNode( proc, this, NULL ) )
            {
                #ifdef DEBUG
                std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
                std::cerr << " * [INFO] could not read appearance information\n";
                #endif
                return false;
            }
        }
        else if( !glob.compare( "geometry" ) )
        {
            if( !aTopNode->ReadNode( proc, this, NULL ) )
            {
                #ifdef DEBUG
                std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
                std::cerr << " * [INFO] could not read geometry information\n";
                #endif
                return false;
            }
        }
        else
        {
            #ifdef DEBUG
            std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [INFO] bad Shape at line " << line << ", column ";
            std::cerr << column << "\n";
            std::cerr << " * [INFO] file: '" << proc.GetFileName() << "'\n";
            #endif

            return false;
        }
    }   // while( true ) -- reading contents of Shape{}

    return true;
}


SGNODE* WRL2SHAPE::TranslateToSG( SGNODE* aParent )
{
    // XXX - TO IMPLEMENT
    return NULL;
}
