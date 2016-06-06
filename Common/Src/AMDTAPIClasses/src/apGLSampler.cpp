//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLSampler.cpp
///
//==================================================================================

// -----------------------------   apGLSampler.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apGLSampler.h>

// GL Enums to string:
#include <AMDTAPIClasses/Include/apGLenumParameter.h>

// C++:
#include <algorithm>

// ---------------------------------------------------------------------------
// Name:        apGLSampler::apGLSampler
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
apGLSampler::apGLSampler(GLuint name)
    : apAllocatedObject(),
      m_boundTextures(),
      m_name(name),
      m_textureBorderColorRed(0.0f),
      m_textureBorderColorGreen(0.0f),
      m_textureBorderColorBlue(0.0f),
      m_textureBorderColorAlpha(0.0f),
      m_textureCompareFunc(GL_LEQUAL),
      m_textureCompareMode(GL_NONE),
      m_textureLodBias(0.0f),
      m_textureMaxLod(1000.0f),
      m_textureMinLod(-1000.0f),
      m_textureMagFilter(GL_LINEAR),

      // See the note about the default values of the following data members in the .h file:
      m_textureMinFilter(GL_NEAREST_MIPMAP_LINEAR),
      m_textureWrapS(GL_REPEAT),
      m_textureWrapT(GL_REPEAT),
      m_textureWrapR(GL_REPEAT)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLSampler::apGLSampler
// Description: Copy constructor
// Arguments: other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
apGLSampler::apGLSampler(const apGLSampler& other)
{
    apGLSampler::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apGLSampler::~apGLSampler
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
apGLSampler::~apGLSampler()
{
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::operator=
// Description: Assignment operator
// Arguments:   other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
apGLSampler& apGLSampler::operator=(const apGLSampler& other)
{
    if (this != &other)
    {
        // Copy the vector.
        m_boundTextures = other.m_boundTextures;

        // Copy all other data members.
        m_name                       = other.m_name;
        m_textureBorderColorRed      = other.m_textureBorderColorRed;
        m_textureBorderColorGreen    = other.m_textureBorderColorGreen;
        m_textureBorderColorBlue     = other.m_textureBorderColorBlue;
        m_textureBorderColorAlpha    = other.m_textureBorderColorAlpha;
        m_textureCompareFunc         = other.m_textureCompareFunc;
        m_textureCompareMode         = other.m_textureCompareMode;
        m_textureLodBias             = other.m_textureLodBias;
        m_textureMaxLod              = other.m_textureMaxLod;
        m_textureMinLod              = other.m_textureMinLod;
        m_textureMagFilter           = other.m_textureMagFilter;
        m_textureMinFilter           = other.m_textureMinFilter;
        m_textureWrapS               = other.m_textureWrapS;
        m_textureWrapT               = other.m_textureWrapT;
        m_textureWrapR               = other.m_textureWrapR;

        // Take care of the base class' data.
        setAllocatedObjectId(other.getAllocatedObjectId(), true);
    }

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
osTransferableObjectType apGLSampler::type() const
{
    return OS_TOBJ_ID_GL_SAMPLER;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
bool apGLSampler::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // First pass vector size.
    size_t boundTexturesCount = m_boundTextures.size();
    ipcChannel << (gtUInt32)boundTexturesCount;

    // Now, if the vector is not empty, pass the its elements.
    for (size_t i = 0; i < boundTexturesCount; ++i)
    {
        ipcChannel << (gtUInt32)m_boundTextures[i];
    }

    // Now, continue and pass all other data members of the class (primitives).
    ipcChannel << (gtUInt32)m_name;
    ipcChannel << (gtFloat32)m_textureBorderColorRed;
    ipcChannel << (gtFloat32)m_textureBorderColorGreen;
    ipcChannel << (gtFloat32)m_textureBorderColorBlue;
    ipcChannel << (gtFloat32)m_textureBorderColorAlpha;
    ipcChannel << (gtUInt32)m_textureCompareFunc;
    ipcChannel << (gtUInt32)m_textureCompareMode;
    ipcChannel << (gtFloat32)m_textureLodBias;
    ipcChannel << (gtFloat32)m_textureMaxLod;
    ipcChannel << (gtFloat32)m_textureMinLod;
    ipcChannel << (gtUInt32)m_textureMagFilter;
    ipcChannel << (gtUInt32)m_textureMinFilter;
    ipcChannel << (gtUInt32)m_textureWrapS;
    ipcChannel << (gtUInt32)m_textureWrapT;
    ipcChannel << (gtUInt32)m_textureWrapR;

    // Take care of the base class' data.
    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
bool apGLSampler::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // First read the vector's size.
    gtUInt32 boundTexturesCountAsInt32 = 0;
    ipcChannel >> boundTexturesCountAsInt32;

    // Now, read the vector's elements (if any).
    gtUInt32 tmpSamplerNameAsInt32 = 0;

    for (size_t i = 0; i < boundTexturesCountAsInt32; ++i)
    {
        tmpSamplerNameAsInt32 = 0;
        ipcChannel >> tmpSamplerNameAsInt32;
        m_boundTextures.push_back((GLuint)(tmpSamplerNameAsInt32));
    }

    // Prepare the buffers.
    gtUInt32 nameAsUInt32                        = 0;
    gtFloat32 textureBorderColorRedAsFloat32     = 0.0f;
    gtFloat32 textureBorderColorGreenAsFloat32   = 0.0f;
    gtFloat32 textureBorderColorBlueAsFloat32    = 0.0f;
    gtFloat32 textureBorderColorAlphaAsFloat32   = 0.0f;
    gtUInt32 textureCompareFuncAsUInt32          = 0;
    gtUInt32 textureCompareModeAsUInt32          = 0;
    gtFloat32 textureLodBiasAsFloat32            = 0.0f;
    gtFloat32 textureMaxLodAsFloat32             = 0.0f;
    gtFloat32 textureMinLodAsFloat32             = 0.0f;
    gtUInt32 textureMagFilterAsUInt32            = 0;
    gtUInt32 textureMinFilterAsUInt32            = 0;
    gtUInt32 textureWrapSAsUInt32                = 0;
    gtUInt32 textureWrapTAsUInt32                = 0;
    gtUInt32 textureWrapRAsUInt32                = 0;

    // Read the values.
    ipcChannel >> nameAsUInt32;
    ipcChannel >> textureBorderColorRedAsFloat32;
    ipcChannel >> textureBorderColorGreenAsFloat32;
    ipcChannel >> textureBorderColorBlueAsFloat32;
    ipcChannel >> textureBorderColorAlphaAsFloat32;
    ipcChannel >> textureCompareFuncAsUInt32;
    ipcChannel >> textureCompareModeAsUInt32;
    ipcChannel >> textureLodBiasAsFloat32;
    ipcChannel >> textureMaxLodAsFloat32;
    ipcChannel >> textureMinLodAsFloat32;
    ipcChannel >> textureMagFilterAsUInt32;
    ipcChannel >> textureMinFilterAsUInt32;
    ipcChannel >> textureWrapSAsUInt32;
    ipcChannel >> textureWrapTAsUInt32;
    ipcChannel >> textureWrapRAsUInt32;

    // Assign the data members.
    m_name                       = (GLenum)nameAsUInt32;
    m_textureBorderColorRed      = (GLfloat)textureBorderColorRedAsFloat32;
    m_textureBorderColorGreen    = (GLfloat)textureBorderColorGreenAsFloat32;
    m_textureBorderColorBlue     = (GLfloat)textureBorderColorBlueAsFloat32;
    m_textureBorderColorAlpha    = (GLfloat)textureBorderColorAlphaAsFloat32;
    m_textureCompareFunc         = (GLenum)textureCompareFuncAsUInt32;
    m_textureCompareMode         = (GLenum)textureCompareModeAsUInt32;
    m_textureLodBias             = (GLfloat)textureLodBiasAsFloat32;
    m_textureMaxLod              = (GLfloat)textureMaxLodAsFloat32;
    m_textureMinLod              = (GLfloat)textureMinLodAsFloat32;
    m_textureMagFilter           = (GLenum)textureMagFilterAsUInt32;
    m_textureMinFilter           = (GLenum)textureMinFilterAsUInt32;
    m_textureWrapS               = (GLenum)textureWrapSAsUInt32;
    m_textureWrapT               = (GLenum)textureWrapTAsUInt32;
    m_textureWrapR               = (GLenum)textureWrapRAsUInt32;

    // Take care of the base class' data.
    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::BindToTextureUnit
// Description: Binds this sampler to the given texture unit
// Arguments:   GLuint textureUnit - the name of the texture unit to which this
//                                   sampler would be bound
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// ---------------------------------------------------------------------------
void apGLSampler::bindToTextureUnit(GLuint textureUnit)
{
    // Add the item if it hasn't already been added.
    if (std::find(m_boundTextures.begin(), m_boundTextures.end(), textureUnit) == m_boundTextures.end())
    {
        m_boundTextures.push_back(textureUnit);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::unbindToTextureUnit
// Description: Unbinds this sampler from the given texture unit
// Arguments:   GLuint textureUnit - the name of the texture unit from which this
//                                   sampler is to be bound
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// ---------------------------------------------------------------------------
bool apGLSampler::unbindToTextureUnit(GLuint textureUnit)
{
    bool ret = false;

    // Search for the relevant item.
    int index = -1;
    int textureCount = (int)m_boundTextures.size();

    for (int i = 0; index == -1 && i < textureCount; ++i)
    {
        if (textureUnit == m_boundTextures[i] && textureUnit != 0)
        {
            // We have found it, we can stop.
            index = i;
        }
    }

    if (index > -1)
    {
        // Remove the item.
        m_boundTextures.removeItem(index);
        ret = true;
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::getSamplerColor
// Description: Extracts the sampler RGBA color
// Arguments:   GLfloat& r - an output parameter to hold the Red RGBA part
//              GLfloat& g - an output parameter to hold the Green RGBA part
//              GLfloat& b - an output parameter to hold the Blue RGBA part
//              GLfloat& a - an output parameter to hold the Alpha RGBA part
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// ---------------------------------------------------------------------------
void apGLSampler::getSamplerRgbaColor(GLfloat& r, GLfloat& g, GLfloat& b, GLfloat& a) const
{
    r = m_textureBorderColorRed;
    g = m_textureBorderColorGreen;
    b = m_textureBorderColorBlue;
    a = m_textureBorderColorAlpha;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::getSamplerColor
// Description: Setter function for the RGBA color of this sampler
// Arguments:   GLfloat r - the Red RGBA part
//              GLfloat g - the Green RGBA part
//              GLfloat b - the Blue RGBA part
//              GLfloat a - the Alpha RGBA part
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// ---------------------------------------------------------------------------
void apGLSampler::setSamplerColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    m_textureBorderColorRed      = r;
    m_textureBorderColorGreen    = g;
    m_textureBorderColorBlue     = b;
    m_textureBorderColorAlpha    = a;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::setSamplerComparisonFunction
// Description: Setter function for this sampler's comparison function
// Arguments:   GLenum comparisonFunction - the input comparison function
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// ---------------------------------------------------------------------------
void apGLSampler::setSamplerComparisonFunction(GLenum comparisonFunction)
{
    m_textureCompareFunc = comparisonFunction;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::setSamplerComparisonMode
// Description: Setter function for this sampler's comparison mode
// Arguments:   GLenum comparisonMode - the input comparison mode
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// ---------------------------------------------------------------------------
void apGLSampler::setSamplerComparisonMode(GLenum comparisonMode)
{
    m_textureCompareMode = comparisonMode;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::setSamplerLodBias
// Description: Setter function for this sampler's level of detail bias
// Arguments:   GLfloat lodBias - the input LOD bias
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// ---------------------------------------------------------------------------
void apGLSampler::setSamplerLodBias(GLfloat lodBias)
{
    m_textureLodBias = lodBias;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::setSamplerMaxLod
// Description: Setter function for this sampler's max level of detail
// Arguments:   GLfloat maxLod - the input max LOD bias
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// ---------------------------------------------------------------------------
void apGLSampler::setSamplerMaxLod(GLfloat maxLod)
{
    m_textureMaxLod = maxLod;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::setSamplerMinLod
// Description: Setter function for this sampler's min level of detail
// Arguments:   GLfloat maxLod - the input min LOD bias
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// ---------------------------------------------------------------------------
void apGLSampler::setSamplerMinLod(GLfloat minLod)
{
    m_textureMinLod = minLod;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::setSamplerMagnificationFunction
// Description: Setter function for this sampler's magnification function
// Arguments:   GLenum magFunction - the input magnification function
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// ---------------------------------------------------------------------------
void apGLSampler::setSamplerMagnificationFunction(GLenum magFunction)
{
    m_textureMagFilter = magFunction;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::setSamplerMinificationFunction
// Description: Setter function for this sampler's minification function
// Arguments:   GLenum minFunction - the input minification function
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// ---------------------------------------------------------------------------
void apGLSampler::setSamplerMinificationFunction(GLenum minFunction)
{
    m_textureMinFilter = minFunction;
}

// ---------------------------------------------------------------------------
// Name:        apGLSampler::setSwrapMode
// Description: Setter function for this sampler's texcoord s wrap mode
// Arguments:   GLenum sWrapMode - the input texcoord s wrap mode
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// ---------------------------------------------------------------------------
void apGLSampler::setSwrapMode(GLenum sWrapMode)
{
    m_textureWrapS = sWrapMode;
}

// -------------------------------------------------------------------------------------------------
// Name:        apGLSampler::setTwrapMode
// Description: Setter function for this sampler's texcoord s wrap mode
// Arguments:   GLenum tWrapMode - the input texcoord t wrap mode (2D, 3D, cube map textures only)
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// -------------------------------------------------------------------------------------------------
void apGLSampler::setTwrapMode(GLenum tWrapMode)
{
    m_textureWrapT = tWrapMode;
}

// ---------------------------------------------------------------------------------
// Name:        apGLSampler::setRwrapMode
// Description: Setter function for this sampler's texcoord s wrap mode
// Arguments:   GLenum rWrapMode - the input texcoord r wrap mode (3D textures only)
// Author:  AMD Developer Tools Team
// Date:        25/6/2014
// ---------------------------------------------------------------------------------
void apGLSampler::setRwrapMode(GLenum rWrapMode)
{
    m_textureWrapR = rWrapMode;
}

// ---------------------------------------------------------------------------------
// Name:        apGLSampler::getBoundTextures
// Description: Creates a copy of the container that holds the names of all texture
//              units to which this sampler is bound
// Arguments:   gtVector<GLuint>& buffer - an output parameter to hold the result
// Author:  AMD Developer Tools Team
// Date:        29/6/2014
// ---------------------------------------------------------------------------------
void apGLSampler::getBoundTextures(gtVector<GLuint>& buffer) const
{
    buffer = m_boundTextures;
}

// ---------------------------------------------------------------------------------
// Name:        apGLSampler::getSamplerComparisonModeAsString
// Description: Fills the buffer with a string representation of the Texture Comparison
//              Mode of this sampler.
// Arguments:   gtString& buffer - an output parameter to hold the result
// Author:  AMD Developer Tools Team
// Date:        29/6/2014
// ---------------------------------------------------------------------------------
bool apGLSampler::getSamplerComparisonModeAsString(gtString& buffer) const
{
    bool ret = false;

    if (m_textureCompareMode == GL_NONE)
    {
        buffer = L"GL_NONE";
        ret = true;
    }
    else
    {
        ret = apGLenumValueToString(m_textureCompareMode, buffer);
    }

    return ret;
}

