//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  class that stores the position and size of an element
///         on the HUD and whether or not that element should be displayed
//==============================================================================

#ifndef HUDELEMENT_H
#define HUDELEMENT_H

#include "frect.h"
#include "CommandProcessor.h"
#include "xml.h"
#include "misc.h"

//=============================================================================
/// Class HUDElement
/// Maintains the pixel locations of the left, top corner of the element along
/// with the width and height of the quad for rendering elements onto the HUD.
//=============================================================================
class HUDElement : public CommandProcessor
{
public:

    /// Default constructor
    HUDElement()
    {
        m_bVisibleOnHUD = false;
        m_nTop     = 0;
        m_nLeft    = 0;
        m_nWidth   = 100;
        m_nHeight  = 100;

        AddCommand(CONTENT_XML, "show",   "Show",   "show",   NO_DISPLAY, INCLUDE, m_bVisibleOnHUD);
        AddCommand(CONTENT_XML, "top",    "Top",    "top",    NO_DISPLAY, INCLUDE, m_nTop);
        AddCommand(CONTENT_XML, "left",   "Left",   "left",   NO_DISPLAY, INCLUDE, m_nLeft);
        AddCommand(CONTENT_XML, "width",  "Width",  "width",  NO_DISPLAY, INCLUDE, m_nWidth);
        AddCommand(CONTENT_XML, "height", "Height", "height", NO_DISPLAY, INCLUDE, m_nHeight);
    };

    /// destructor
    virtual ~HUDElement() {};

public:

    /// indicates if this hud element should be displayed
    /// \return true if the element should be displayed on the HUD; false if not
    virtual bool IsVisibleOnHUD()
    {
        return m_bVisibleOnHUD;
    }

    /// Accessor to an FRECT that indicates where this element should be displayed
    /// \return the position and dimensions of the element
    virtual FRECT GetFRect()
    {
        return FRECT((float) m_nLeft, (float)m_nTop, (float)m_nLeft + (float)m_nWidth, (float)m_nTop + (float)m_nHeight);
    }

    /// Gets the layout data from this HUD element
    /// \param pcszPath the path to access this element in a command
    /// \param pcszName the name to assign to this layout
    /// \param nIndex the index of this layout ( use -1 if layout is not part of an array ).
    /// \returns A string that contains the layout data
    std::string GetLayout(const char* pcszPath, const char* pcszName, int nIndex)
    {
        std::stringstream strXML;

        strXML << "<layout" << XMLAttribStream("path", pcszPath) << XMLAttribStream("name", pcszName) << XMLAttribStream("index", nIndex) << ">";

        if (m_bVisibleOnHUD == true)
        {
            strXML << XMLStream("show", "true");
        }
        else
        {
            strXML << XMLStream("show", "false");
        }

        strXML << XMLStream("top", m_nTop.GetValue());
        strXML << XMLStream("left", m_nLeft.GetValue());
        strXML << XMLStream("width", m_nWidth.GetValue());
        strXML << XMLStream("height", m_nHeight.GetValue());

        strXML << "</layout>";

        return strXML.str();
    }

    /// Sets the layout data for the HUD element.
    /// \param bShow Bool to control if the element is dsiplayed or not.
    /// \param nTop Position of the top of the element.
    /// \param nLeft Position of the left hand side of element.
    /// \param nWidth Width of the element.
    /// \param nHeight Heigth of the element.
    void Set(bool bShow, int nTop, int nLeft, int nWidth, int nHeight)
    {
        m_bVisibleOnHUD = bShow;
        m_nTop     = nTop;
        m_nLeft    = nLeft;
        m_nWidth   = nWidth;
        m_nHeight  = nHeight;
    }

protected:

    /// indicates wether or not the HUDElement is visible
    BoolCommandResponse m_bVisibleOnHUD;

    /// top position of the element
    IntCommandResponse  m_nTop;

    /// left position of the element
    IntCommandResponse  m_nLeft;

    /// width of the element
    IntCommandResponse  m_nWidth;

    /// height of the element
    IntCommandResponse  m_nHeight;

private:
    // No additional settings
    virtual string GetDerivedSettings() { return ""; }
};

#endif // HUDELEMENT_H
