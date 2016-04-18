//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  a class that stores the position and size of an element
///         on the HUD and whether or not that element should be displayed
//==============================================================================

#ifndef HUDIMAGE_H
#define HUDIMAGE_H

#include "HUDElement.h"
#include "CommandProcessor.h"

//=============================================================================
/// Class HUDImage
/// Has a HUDElement and allows the contents of that element to be requested
/// back to the client.
//=============================================================================
class HUDImage : public CommandProcessor
{
public:

    /// Default Constructor
    HUDImage()
    {
        AddCommand(CONTENT_PNG, "picture",    "Image",    "picture", DISPLAY,    INCLUDE, m_clientSidePicture);
        AddProcessor("hudelement", "HUD", "hud", "", NO_DISPLAY, m_hudElement);
    }

    /// Destructor
    virtual ~HUDImage() {}

public:

    /// Indicates if this image should be displayed on the HUD
    /// \return true if the image should be displayed on the HUD; false if not
    bool IsVisibleOnHUD()
    {
        return m_hudElement.IsVisibleOnHUD();
    }

    /// Accessor to an FRECT that indicates where this image should be displayed
    /// \return the position and dimensions of the image
    FRECT GetFRect()
    {
        return m_hudElement.GetFRect();
    }

    /// Gets the layout data from this image
    /// \param pcszPath the path to access this element in a command
    /// \param pcszName the name to assign to this layout
    /// \param nIndex the index of this layout ( use -1 if layout is not part of an array ).
    /// \returns A string that contains the layout data
    std::string GetLayout(const char* pcszPath, const char* pcszName, int nIndex)
    {
        return m_hudElement.GetLayout(pcszPath, pcszName, nIndex);
    }

    /// Accessor to this image's underlying element
    /// \return the HUDElement for this image
    HUDElement GetHUDElement()
    {
        return m_hudElement;
    }

private:
    // No additional settings
    virtual string GetDerivedSettings() { return ""; }

public:

    /// Command which allows the client to request a picture of the HUDElement be returned
    PictureCommandResponse m_clientSidePicture;

    /// position of this image on the HUD
    HUDElement m_hudElement;
};

#endif // HUDIMAGE_H
