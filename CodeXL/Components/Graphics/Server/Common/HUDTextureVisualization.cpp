//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A base class that contains the commands and callbacks necessary for
///         applying visualization controls to a texture which can be displayed
///         on the HUD or returned to the client and for obtaining a description
///         of the texture.
//==============================================================================

#include "HUDTextureVisualization.h"

//=============================================================================
/// Class HUDTextureVisualization
/// Maintains data necessary to for rendering textures onto the HUD.
//=============================================================================

//--------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------
HUDTextureVisualization::HUDTextureVisualization() :
    m_uNumSamples(0),
    m_uNumMipLevels(1),
    m_uMostDetailedMip(0),
    m_uMostDetailedMipWidth(0),
    m_uMostDetailedMipHeight(0),
    m_uArraySize(0),
    m_uArrayStart(0),
    m_strImageFormatName(""),
    m_ImageFormat(0),
    m_uIndex(0),
    m_eStage(PIPELINE_STAGE_NONE)
{
    AddCommand(CONTENT_PNG, "picture",     "Picture",     "picture",     NO_DISPLAY, INCLUDE, m_clientSidePicture);
    AddCommand(CONTENT_PNG, "picture",     "Picture",     "thumbnail",   NO_DISPLAY, INCLUDE, m_clientSideThumbnail);
    AddCommand(CONTENT_XML, "Description", "Description", "Description.xml", NO_DISPLAY, INCLUDE, m_description);
    AddCommand(CONTENT_XML, "VisSettings", "VisSettings", "vissettings.xml", NO_DISPLAY, INCLUDE, m_visSettings);
    AddCommand(CONTENT_DDS, "rawdata",     "RawData",     "rawdata.dds",     NO_DISPLAY, INCLUDE, m_DDS);
}

//--------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------
HUDTextureVisualization::~HUDTextureVisualization()
{
}

//--------------------------------------------------------------------------
/// Updates the format string and show channel values.
/// \return XML indicating which channels of the texture should be editable
///   based on the texture format
//--------------------------------------------------------------------------
string HUDTextureVisualization::GetDerivedSettings()
{
    stringstream strOut;

    strOut << XML("ShowRed",   ShowRedControl()).asCharArray();
    strOut << XML("ShowGreen", ShowGreenControl()).asCharArray();
    strOut << XML("ShowBlue",  ShowBlueControl()).asCharArray();
    strOut << XML("ShowAlpha", ShowAlphaControl()).asCharArray();
    strOut << XMLBool("IsDepth", GetDepthValue()).asCharArray();
    strOut << XML("NumSamples", m_uNumSamples).asCharArray();
    strOut << XML("NumMipLevels", m_uNumMipLevels).asCharArray();
    strOut << XML("MostDetailedMip", m_uMostDetailedMip).asCharArray();
    strOut << XML("MostDetailedMipWidth", m_uMostDetailedMipWidth).asCharArray();
    strOut << XML("MostDetailedMipHeight", m_uMostDetailedMipHeight).asCharArray();
    strOut << XML("ArraySize", m_uArraySize).asCharArray();
    strOut << XML("ArrayStart", m_uArrayStart).asCharArray();

    return strOut.str();
}

//=========================================================================================================
///
/// Sets which controls are to be displayed based on the input format.
///
/// \param eFormat The texture format.
/// \param strFormatName Sets the name of the control
///
//=========================================================================================================
void HUDTextureVisualization::UpdateTextureControls(int eFormat, gtASCIIString strFormatName)
{
    // BOOLS to control which controls arte activated in the APV
    bool showRedControl = false;
    bool showGreenControl = false;
    bool showBlueControl = false;
    bool showAlphaControl = false;

    // Get the active channels (specific to each DXGI format)
    // These value manage if the control for thet channel is enabled.
    GetDefaultEnabledChannels(eFormat, showRedControl, showGreenControl, showBlueControl, showAlphaControl);

    // Set the controls based on the active channels.
    SetTextureControls(showRedControl, showGreenControl, showBlueControl, showAlphaControl, strFormatName, GetDepthValue());

    // Bools to control which of the active channels are set to be on/off
    bool displayRedChannel = false;
    bool displayshowGreenChannel = false;
    bool displayshowBlueChannel = false;
    bool displayshowAlphaChannel = false;

    GetDefaultDisplayChannels(eFormat, displayRedChannel, displayshowGreenChannel, displayshowBlueChannel, displayshowAlphaChannel);

    // This sets which of the active channels is currently displayed (can be set by the user).
    SetChannelValues(displayRedChannel, displayshowGreenChannel, displayshowBlueChannel, displayshowAlphaChannel);
}
