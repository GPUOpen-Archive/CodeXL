//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This class is responsible for controling how a texture should be viualized on the HUD.
//==============================================================================

#ifndef HUD_TEXTURE_CONTOL_H
#define HUD_TEXTURE_CONTOL_H

#include "parser.h"
#include "xml.h"
#include "misc.h"
#include "HUDElement.h"

/// Cube map face enumerations
typedef enum _CUBE_MAP_FACES
{
    CMF_ALL_FACES = -1,
    CMF_X_POS = 0,
    CMF_X_NEG,
    CMF_Y_POS,
    CMF_Y_NEG,
    CMF_Z_POS,
    CMF_Z_NEG,
} CUBE_MAP_FACES;

//=============================================================================
/// This class is responsible for controling how a texture should be viualized on the HUD.
///
/// There are two sets of boolean values for RGBA channels. One set controls if checkboxes
/// should appear in the UI. So, an RGB texture
/// would only show RGB checkboxes. The other set allows the user to switch a
/// specific channel on or off in the display.
//=============================================================================
class HUDTextureControl : public HUDElement
{

public:

    //====================================================================================
    /// \brief Constructor
    //====================================================================================
    HUDTextureControl()
        :  m_bShowAlphaControl(false),
           m_bShowRedControl(true),
           m_bShowGreenControl(true),
           m_bShowBlueControl(true),
           m_bApplyGamma(false),
           m_bRedChannelValue(true),
           m_bGreenChannelValue(true),
           m_bBlueChannelValue(true),
           m_bAlphaChannelValue(false),
           m_bDepthValue(false),
           m_bNeedManualResolve(false),
           m_NumSamples(0),
           m_fTextureScaleValue(1.0f),
           m_fTextureBiasValue(0.0f),
           m_bAutoMIPMapLevel(true),
           m_fMIPMapLevel(0),
           m_CubeMapFace(CMF_ALL_FACES),   // show all faces by default
           m_ArrayElement(0),   // default to first element
           m_MSAASampleNumber(0)    // select sample 0 by default as resolve has a bug atm
    {
        AddCommand(CONTENT_HTML, "red",   "Red",   "red",   NO_DISPLAY, INCLUDE, m_bRedChannelValue);
        AddCommand(CONTENT_HTML, "green", "Green", "green", NO_DISPLAY, INCLUDE, m_bGreenChannelValue);
        AddCommand(CONTENT_HTML, "blue",  "Blue",  "blue",  NO_DISPLAY, INCLUDE, m_bBlueChannelValue);
        AddCommand(CONTENT_HTML, "alpha", "Alpha", "alpha", NO_DISPLAY, INCLUDE, m_bAlphaChannelValue);
        AddCommand(CONTENT_HTML, "scale", "Scale", "scale", NO_DISPLAY, INCLUDE, m_fTextureScaleValue);
        AddCommand(CONTENT_HTML, "bias",  "Bias",  "bias",  NO_DISPLAY, INCLUDE, m_fTextureBiasValue);
        AddCommand(CONTENT_HTML, "format", "Format", "format", NO_DISPLAY, INCLUDE, m_strFormat);
        AddCommand(CONTENT_HTML, "mipmaplevel", "MIPMapLevel", "MIPMapLevel", NO_DISPLAY, INCLUDE, m_fMIPMapLevel);
        AddCommand(CONTENT_HTML, "automipmaplevel",  "AutoMIPMapLevel",  "AutoMIPMapLevel",  NO_DISPLAY, INCLUDE, m_bAutoMIPMapLevel);
        AddCommand(CONTENT_HTML, "cubemapface", "CubeMapFace", "CubeMapFace", NO_DISPLAY, INCLUDE, m_CubeMapFace);
        AddCommand(CONTENT_HTML, "arrayelement",  "ArrayElement",  "ArrayElement",  NO_DISPLAY, INCLUDE, m_ArrayElement);
        AddCommand(CONTENT_HTML, "samplenumber",  "SampleNumber",  "SampleNumber",  NO_DISPLAY, INCLUDE, m_MSAASampleNumber);

    }

    //====================================================================================
    /// \brief Destructor
    //====================================================================================
    virtual ~HUDTextureControl()
    {
    }

    //====================================================================================
    /// \brief Set if the red control should be displayed in the UI.
    //====================================================================================
    void ShowRedControl(bool bShow)
    {
        m_bShowRedControl = bShow;
    }

    //====================================================================================
    /// \brief Returns true if the red tool should be displayed in the UI.
    //====================================================================================
    bool ShowRedControl()
    {
        return m_bShowRedControl;
    }

    //====================================================================================
    /// \brief Sets if the green control should be displayed in the UI.
    //====================================================================================
    void ShowGreenControl(bool bShow)
    {
        m_bShowGreenControl = bShow;
    }

    //====================================================================================
    /// \brief Returns true if the green tool should be displayed in the UI.
    //====================================================================================
    bool ShowGreenControl()
    {
        return m_bShowGreenControl;
    }

    //====================================================================================
    /// \brief Sets if the blue control should be displayed in the UI.
    //====================================================================================
    void ShowBlueControl(bool bShow)
    {
        m_bShowBlueControl = bShow;
    }

    //====================================================================================
    /// \brief Returns true if the blue tool should be displayed in the UI.
    //====================================================================================
    bool ShowBlueControl()
    {
        return m_bShowBlueControl;
    }

    //====================================================================================
    /// \brief Set if the alpha controls should be displayed in the UI.
    //====================================================================================
    void ShowAlphaControl(bool bShow)
    {
        m_bShowAlphaControl = bShow;
    }

    //====================================================================================
    /// \brief Returns true if the alpha tools should be displayed in the UI.
    //====================================================================================
    bool ShowAlphaControl()
    {
        return m_bShowAlphaControl;
    }

    //====================================================================================
    /// \brief Set the texture scale value.
    //====================================================================================
    void SetTextureScaleValue(float fScale)
    {
        m_fTextureScaleValue = fScale;
    }

    //====================================================================================
    /// \brief Set the texture bias value.
    //====================================================================================
    void SetTextureBiasValue(float fBias)
    {
        m_fTextureBiasValue = fBias;
    }

    //====================================================================================
    /// \brief Get the current bias value.
    //====================================================================================
    float GetTextureBiasValue()
    {
        return m_fTextureBiasValue.GetValue();
    }

    //====================================================================================
    /// \brief Get the current scale value.
    //====================================================================================
    float GetTextureScaleValue()
    {
        return m_fTextureScaleValue.GetValue();
    }

    //====================================================================================
    /// \brief Set the color channel values.
    /// \param red The red value.
    /// \param green The green value.
    /// \param blue The blue value.
    /// \param alpha The alpha value.
    //====================================================================================
    void SetChannelValues(bool red, bool green, bool blue, bool alpha)
    {
        SetRedChannelValue(red);
        SetGreenChannelValue(green) ;
        SetBlueChannelValue(blue) ;
        SetAlphaChannelValue(alpha);
    }

    //====================================================================================
    /// \brief Set the red control should be displayed in the UI.
    //====================================================================================
    void SetRedChannelValue(bool bSet)
    {
        m_bRedChannelValue = bSet;
    }

    //====================================================================================
    /// \brief Set if the green control should be displayed in the UI.
    //====================================================================================
    void SetGreenChannelValue(bool bSet)
    {
        m_bGreenChannelValue = bSet;
    }

    //====================================================================================
    /// \brief Set if the blue control should be displayed in the UI.
    //====================================================================================
    void SetBlueChannelValue(bool bSet)
    {
        m_bBlueChannelValue = bSet;
    }

    //====================================================================================
    /// \brief Set if the alpha control should be displayed in the UI.
    //====================================================================================
    void SetAlphaChannelValue(bool bSet)
    {
        m_bAlphaChannelValue = bSet;
    }

    //====================================================================================
    /// \brief Gets the red channel state (on/off).
    //====================================================================================
    bool GetRedChannelValue()
    {
        return m_bRedChannelValue;
    }

    //====================================================================================
    /// \brief Gets the green channel state (on/off).
    //====================================================================================
    bool GetGreenChannelValue()
    {
        return m_bGreenChannelValue;
    }

    //====================================================================================
    /// \brief Gets the blue channel state (on/off).
    //====================================================================================
    bool GetBlueChannelValue()
    {
        return m_bBlueChannelValue;
    }

    //====================================================================================
    /// \brief Gets the alpha channel state (on/off).
    //====================================================================================
    bool GetAlphaChannelValue()
    {
        return m_bAlphaChannelValue;
    }

    //====================================================================================
    /// \brief The depth value is used to indicate to the shader that the texture is a depth
    /// buffer. The shader can then take the red value and duplicate it in the G and B channels.
    /// \param bDepthValue The depth value to set
    //====================================================================================
    void SetDepthValue(bool bDepthValue)
    {
        m_bDepthValue = bDepthValue ;
    }

    //====================================================================================
    /// \brief Returns if the texture is a depth texture or not.
    //====================================================================================
    bool GetDepthValue()
    {
        return m_bDepthValue;
    }

    //====================================================================================
    /// \brief Sets the texture fromat string.
    //====================================================================================
    void SetTextureFormatString(gtASCIIString strFormat)
    {
        m_strFormat = strFormat.asCharArray();
    }

    //====================================================================================
    /// \brief Gets the alpha channel state (on/off).
    //====================================================================================
    float GetMIPMapLevel()
    {
        return m_fMIPMapLevel;
    }

    //====================================================================================
    /// \brief Gets the alpha channel state (on/off).
    //====================================================================================
    bool GetAutoMIPMapLevel()
    {
        return m_bAutoMIPMapLevel;
    }

    //====================================================================================
    /// \brief Sets the options for mipmap levels
    /// \param bAutoLevel set to true to automatically pick a mipmap level based on the
    ///        requested texture size
    /// \param uDesiredLevel the desired mipmap level to use (used only if bAutoLevel is false)
    //====================================================================================
    void SetMipMapLevel(bool bAutoLevel, unsigned int uDesiredLevel)
    {
        m_bAutoMIPMapLevel = bAutoLevel;
        m_fMIPMapLevel = (float)uDesiredLevel;
    }

    //====================================================================================
    /// \brief Gets the face of the cubemap to display
    //====================================================================================
    int GetCubeMapFace()
    {
        return m_CubeMapFace;
    }

    //====================================================================================
    /// \brief Gets the texture array element. if it returns -1 menas it want us to
    /// display the whole array.
    //====================================================================================
    int GetArrayElement()
    {
        return m_ArrayElement;
    }

    //====================================================================================
    /// \brief Sets the texture array element.
    /// \param nArrayElement the index of the element to display; -1 menas display the whole array.
    //====================================================================================
    void SetArrayElement(int nArrayElement)
    {
        m_ArrayElement = nArrayElement;
    }

    //====================================================================================
    /// \brief Gets a mask to be applied to the final color generated by the shader
    //====================================================================================
    void GetControlSettings(float* pElement)
    {
        pElement[0] = (m_bShowRedControl) ? 1.0f : 0.0f;
        pElement[1] = (m_bShowGreenControl) ? 1.0f : 0.0f;
        pElement[2] = (m_bShowBlueControl) ? 1.0f : 0.0f;
        pElement[3] = (m_bShowAlphaControl) ? 1.0f : 0.0f;
    }

    //====================================================================================
    /// \brief Gets the sample number to be used by the shader
    /// if the value is -1 then means we have to resolve
    //====================================================================================
    int GetSelectedSample()
    {
        return m_MSAASampleNumber;
    }

    //====================================================================================
    /// \brief Sets the sample number to be used by the shader
    /// \param nSampleNumber if the value is -1 then means we have to resolve
    //====================================================================================
    void SetSelectedSample(int nSampleNumber)
    {
        m_MSAASampleNumber = nSampleNumber;
    }

    //====================================================================================
    /// \brief Gets the number of samples to be used by the shader
    //====================================================================================
    int GetNumSamples()
    {
        return m_NumSamples;
    }

    //====================================================================================
    /// \brief Sets the numble of samples to be used by the shader
    //====================================================================================
    void SetNumSamples(int nNumSamples)
    {
        m_NumSamples = nNumSamples;
    }


    //====================================================================================
    /// \brief if pixel shader manual resolve is needed
    //====================================================================================
    bool NeedManualResolve()
    {
        return m_bNeedManualResolve;
    }

    //====================================================================================
    /// \brief Sets the boolean for manual resolve
    //====================================================================================
    void SetManualResolve(bool bNeedManualResolve)
    {
        m_bNeedManualResolve = bNeedManualResolve;
    }


    //--------------------------------------------------------------------------
    /// Sets which controls should be displayed in the UI.
    ///
    /// \param bShowRedControl true to show the red control; false to hide it
    /// \param bShowGreenControl true to show the green control; false to hide it
    /// \param bShowBlueControl true to show the blue control; false to hide it
    /// \param bShowAlphaControl true to show the alpha control; false to hide it
    /// \param strTextureFormat String representing the format of this texture
    /// \param bDepthValue true to indicate this texture has depth values; false for color values
    //--------------------------------------------------------------------------
    void  SetTextureControls(bool bShowRedControl,
                             bool bShowGreenControl,
                             bool bShowBlueControl,
                             bool bShowAlphaControl,
                             gtASCIIString strTextureFormat,
                             bool bDepthValue)
    {
        // Show UI controls
        ShowRedControl(bShowRedControl) ;
        ShowGreenControl(bShowGreenControl) ;
        ShowBlueControl(bShowBlueControl) ;
        ShowAlphaControl(bShowAlphaControl) ;
        SetDepthValue(bDepthValue) ;
        SetTextureFormatString(strTextureFormat);
    }

    //--------------------------------------------------------------------------
    /// Helper function to set the values with a single function call.
    ///
    /// \param fScale Used to scale the texture values.
    /// \param fBias Used to bias the texture values.
    /// \param bRed Controls if the red channel is shown in the texture image.
    /// \param bGreen Controls if the green channel is shown in the texture image.
    /// \param bBlue Controls if the blue channel is shown in the texture image.
    /// \param bAlpha Controls if the alpha channel is shown in the texture image.
    //--------------------------------------------------------------------------
    void SetShaderValues(float fScale, float fBias, bool bRed, bool bGreen, bool bBlue, bool bAlpha)
    {
        m_fTextureScaleValue = fScale;
        m_fTextureBiasValue = fBias;
        m_bRedChannelValue = bRed;
        m_bGreenChannelValue = bGreen;
        m_bBlueChannelValue = bBlue;
        m_bAlphaChannelValue = bAlpha;
    }

    //--------------------------------------------------------------------------
    /// Used to apply gamma correction when we render the texture (used only wehn we send an image back to the client).
    ///
    /// \param bFlag Turn gamma on or off.
    //--------------------------------------------------------------------------
    void ApplyGamma(bool bFlag)
    {
        m_bApplyGamma = bFlag;
    }

    //--------------------------------------------------------------------------
    /// Gets the current apply gamma bool
    ///
    /// \return True/False (On/Off)
    //--------------------------------------------------------------------------
    bool ApplyGamma()
    {
        return m_bApplyGamma;
    }

private:

    // These are used to manage the display of a control in the UI.
    // E.g. For R32 textures we only display the red control
    bool m_bShowAlphaControl;       ///< Indicates if the alpha control should be enabled
    bool m_bShowRedControl;             ///< Indicates if the red control should be enabled
    bool m_bShowGreenControl;           ///< Indicates if the green control should be enabled
    bool m_bShowBlueControl;            ///< Indicates if the blue control should be enabled
    bool m_bApplyGamma;              ///< Indicates if gamma should be applied to the texture

    // Control the display of the color components on the HUD.
    BoolCommandResponse m_bRedChannelValue;      ///< Allows the user to turn on and off the red channel
    BoolCommandResponse m_bGreenChannelValue;    ///< Allows the user to turn on and off the green channel
    BoolCommandResponse m_bBlueChannelValue;     ///< Allows the user to turn on and off the blue channel
    BoolCommandResponse m_bAlphaChannelValue;    ///< Allows the user to turn on and off the alpha channel

    /// Bool to let the shader know if the texture is a depth texture.
    /// The shader will handle the color differently.
    bool m_bDepthValue;

    ///Bool to use pixel shader for manual resolve
    bool m_bNeedManualResolve;

    /// Int the number of samples
    int m_NumSamples;

    /// Scale to control floating point values for display on HUD.
    /// The scale controls the range of values that can be viewed.
    FloatCommandResponse m_fTextureScaleValue;

    /// Bias to control floating point values for display on HUD.
    /// The bias controls where the range of visible values begins.
    FloatCommandResponse m_fTextureBiasValue;

    /// Indicates if the server should decide which mipmap level to use, or if it should use the setting from the client.
    BoolCommandResponse m_bAutoMIPMapLevel;

    /// MipMap Level settings for client and HUD
    FloatCommandResponse m_fMIPMapLevel;

    /// Cubemap face to display
    IntCommandResponse m_CubeMapFace;

    /// Textrue array element to display
    IntCommandResponse m_ArrayElement;

    /// Stores the stringf version of the texture format.
    TextCommandResponse m_strFormat;

    /// Sample Number to Display
    IntCommandResponse m_MSAASampleNumber;

};

#endif // HUD_TEXTURE_CONTOL_H
