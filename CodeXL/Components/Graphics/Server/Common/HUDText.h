//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  a class that stores the position and size of an element
///         on the HUD and whether or not that element should be displayed
//==============================================================================

#ifndef HUDTEXT_H
#define HUDTEXT_H

#include "defines.h"
#include "HUDElement.h"
#include "CommandProcessor.h"

//=============================================================================
/// Class HUDText
/// Is a HUDElement which can display client-specified text on the HUD.
//=============================================================================
class HUDText : public HUDElement
{
public:
    HUDText()
    {
        AddCommand(CONTENT_XML, "Text", "Text", "text", DISPLAY, INCLUDE, m_strText);
    }

    virtual ~HUDText() {};

public:
    TextCommandResponse m_strText; ///< Command response object
};


#endif // HUDTEXT_H
