//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A base class that contains the commands and callbacks necessary for
///         applying visualization controls to a texture which can be displayed
///         on the HUD or returned to the client and for obtaining a description
///         of the texture.
//==============================================================================

#ifndef HUD_TEXTURE_VISUALIZATION_H
#define HUD_TEXTURE_VISUALIZATION_H

#include "CommandProcessor.h"
#include "HUDTextureControl.h"
#include "CommonTypes.h"
#include "defines.h"

//=============================================================================
/// Class HUDTextureVisualization
///
/// A base class that contains the commands and callbacks necessary for
/// applying visualization controls to a texture which can be displayed
/// on the HUD or returned to the client and for obtaining a description
/// of the texture.
//=============================================================================
class HUDTextureVisualization : public HUDTextureControl
{
public:

    //--------------------------------------------------------------------------
    /// \brief Constructor
    //--------------------------------------------------------------------------
    HUDTextureVisualization();

    //--------------------------------------------------------------------------
    /// \brief Destructor
    //--------------------------------------------------------------------------
    virtual ~HUDTextureVisualization();

    //--------------------------------------------------------------------------
    /// \brief Enables adding API specific texture data (like format) to the
    ///  command processors settings.
    //--------------------------------------------------------------------------
    virtual string GetDerivedSettings();

    //--------------------------------------------------------------------------
    /// \brief Sets the pipeline stage of this object
    /// \param eType The pipeline stage.
    //--------------------------------------------------------------------------
    void SetStage(PIPELINE_STAGE eType)
    {
        m_eStage = eType;
    }

    //--------------------------------------------------------------------------
    /// \brief sends the current visualization settings in response to the vis settings command
    //--------------------------------------------------------------------------
    void SendVisualizationSettings()
    {
        // this calls into the base CommandProcessor so that the settings from
        // the entire inheritence chain will be added to the XML
        m_visSettings.Send(GetEditableCommandValues().c_str());
    }

    //=========================================================================================================
    /// Sets which controls are to be displayed based on the input format.
    /// \param eFormat the format to set the visualization channels based on
    //=========================================================================================================
    void UpdateTextureControls(int eFormat, gtASCIIString strFormatName);

    //=========================================================================================================
    /// Gets which channels are active for the input DXGI format.
    /// \param eFormat the format to set the visualization channels based on
    /// \param red Sets if the red control should be active.
    /// \param green Sets if the green control should be active.
    /// \param blue Sets if the blue control should be active.
    /// \param alpha Sets if the alpha control should be active.
    //=========================================================================================================
    virtual void GetDefaultEnabledChannels(int eFormat, bool& red, bool& green, bool& blue, bool& alpha) = 0;

    //=========================================================================================================
    /// Gets the default setting (display or not) for the input DXGI format.
    /// \param eFormat the format to set the visualization channels based on
    /// \param red Sets if the red channel should be displayed.
    /// \param green Sets if the green channel should be displayed.
    /// \param blue Sets if the blue channel should be displayed.
    /// \param alpha Sets if the alpha channel should be displayed.
    //=========================================================================================================
    virtual void GetDefaultDisplayChannels(int eFormat, bool& red, bool& green, bool& blue, bool& alpha) = 0;

protected:

    // These values must be populated by the derived class in order for them to be accurate in the settings.xml response
    unsigned int m_uNumSamples;            ///< The number of samples in the texture: 0 is non-ms, 1 is must resolve, 2+ is number of samples
    unsigned int m_uNumMipLevels;          ///< The number of mipmap levels in the texture
    unsigned int m_uMostDetailedMip;       ///< The index of the most detailed mipmap level
    unsigned int m_uMostDetailedMipWidth;  ///< The width of the most detailed mipmap level
    unsigned int m_uMostDetailedMipHeight; ///< The height of the most detailed mipmap level
    unsigned int m_uArraySize;             ///< The number of elements in the array: 0 is non-array, 1+ is array size
    unsigned int m_uArrayStart;            ///< Index of the first element in the array
    gtASCIIString m_strImageFormatName;    ///< String version of the image format
    int m_ImageFormat;                     ///< The image format that this object represents (either DXGI or GL image type enums)

public:

    PictureCommandResponse m_clientSidePicture;   ///< Activated when a client-side picture is requested
    PictureCommandResponse m_DDS;             ///< Activated when a client-side DDS is requested
    PictureCommandResponse m_clientSideThumbnail; ///< Activated when a client-side thumbnail is requested
    CommandResponse m_description;                ///< Activated when the description of this texture is requested
    CommandResponse m_visSettings;                ///< Activated when the visualization settings are requested

    unsigned int m_uIndex;                 ///< The index of this texture if it is in a HUDTextureVisualizationArray

    PIPELINE_STAGE m_eStage;               ///< The shader pipeline stage this visualization belongs to (used to share code across different stages).
};


#endif // HUD_TEXTURE_VISUALIZATION_H
