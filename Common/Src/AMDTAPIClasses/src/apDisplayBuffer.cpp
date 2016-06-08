//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDisplayBuffer.cpp
///
//==================================================================================

//------------------------------ apDisplayBuffer.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTAPIClasses/Include/apDisplayBuffer.h>


// ---------------------------------------------------------------------------
// Name:        apColorIndexBufferTypeToGLEnum
// Description: Translates a color index apDisplayBuffer to its equivalent GLenum.
// Arguments:   apDisplayBuffer - The input buffer type.
// Return Val:  GLenum  - Will get the output GLenum or GL_NONE on failure.
// Author:  AMD Developer Tools Team
// Date:        28/8/2007
// ---------------------------------------------------------------------------
GLenum apColorIndexBufferTypeToGLEnum(apDisplayBuffer bufferType)
{
    GLenum retVal = GL_NONE;

    switch (bufferType)
    {

        // Front Buffer
        case AP_FRONT_BUFFER:
        {
            retVal = GL_FRONT;
        }
        break;

        // Back Buffer
        case AP_BACK_BUFFER:
        {
            retVal = GL_BACK;
        }
        break;

        // Auxiliary Buffer [0]
        case AP_AUX0_BUFFER:
        {
            retVal = GL_AUX0;
        }
        break;

        // Auxiliary Buffer [1]
        case AP_AUX1_BUFFER:
        {
            retVal = GL_AUX1;
        }
        break;

        // Auxiliary Buffer [2]
        case AP_AUX2_BUFFER:
        {
            retVal = GL_AUX2;
        }
        break;

        // Auxiliary Buffer [3]
        case AP_AUX3_BUFFER:
        {
            retVal = GL_AUX3;
        }
        break;

        case AP_COLOR_ATTACHMENT0_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT0_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT1_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT1_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT2_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT2_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT3_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT3_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT4_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT4_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT5_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT5_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT6_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT6_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT7_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT7_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT8_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT8_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT9_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT9_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT10_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT10_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT11_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT11_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT12_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT12_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT13_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT13_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT14_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT14_EXT;
        }
        break;

        case AP_COLOR_ATTACHMENT15_EXT:
        {
            retVal = GL_COLOR_ATTACHMENT15_EXT;
        }
        break;

        case AP_DEPTH_ATTACHMENT_EXT:
        {
            retVal = GL_DEPTH_ATTACHMENT_EXT;
        }
        break;

        case AP_STENCIL_ATTACHMENT_EXT:
        {
            retVal = GL_STENCIL_ATTACHMENT_EXT;
        }
        break;

        // Buffer is not a color index type buffer, or buffer type is unknown
        default:
        {
            retVal = GL_NONE;
        }
        break;
    }

    return retVal;
}
apDisplayBuffer apGLEnumToColorIndexBufferType(GLuint openGLBufferType)
{
    apDisplayBuffer retVal = AP_DISPLAY_BUFFER_UNKNOWN;

    switch (openGLBufferType)
    {
        // Front Buffer
        case GL_FRONT:
        {
            retVal = AP_FRONT_BUFFER;
        }
        break;

        // Back Buffer
        case GL_BACK:
        {
            retVal = AP_BACK_BUFFER;
        }
        break;

        // Auxiliary Buffer [0]
        case GL_AUX0:
        {
            retVal = AP_AUX0_BUFFER;
        }
        break;

        // Auxiliary Buffer [1]
        case GL_AUX1:
        {
            retVal = AP_AUX1_BUFFER;
        }
        break;

        // Auxiliary Buffer [2]
        case GL_AUX2:
        {
            retVal = AP_AUX2_BUFFER;
        }
        break;

        // Auxiliary Buffer [3]
        case GL_AUX3:
        {
            retVal = AP_AUX3_BUFFER;
        }
        break;

        case GL_COLOR_ATTACHMENT0_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT0_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT1_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT1_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT2_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT2_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT3_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT3_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT4_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT4_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT5_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT5_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT6_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT6_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT7_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT7_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT8_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT8_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT9_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT9_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT10_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT10_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT11_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT11_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT12_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT12_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT13_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT13_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT14_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT14_EXT;
        }
        break;

        case GL_COLOR_ATTACHMENT15_EXT:
        {
            retVal = AP_COLOR_ATTACHMENT15_EXT;
        }
        break;

        case GL_DEPTH_ATTACHMENT_EXT:
        {
            retVal = AP_DEPTH_ATTACHMENT_EXT;
        }
        break;

        case GL_STENCIL_ATTACHMENT_EXT:
        {
            retVal = AP_STENCIL_ATTACHMENT_EXT;
        }
        break;

        // Buffer is not a color index type buffer, or buffer type is unknown
        default:
        {
            retVal = AP_DISPLAY_BUFFER_UNKNOWN;
        }
        break;
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        apGetBufferName
// Description: Returns buffer name for a given buffer type
// Arguments:   bufferType - The buffer type to generate the name for
//              bufferName - Output buffer name
// Return Val:  bool - Success / Failure
// Author:  AMD Developer Tools Team
// Date:        18/8/2007
// --------------------------------------------------------------------------
bool apGetBufferName(apDisplayBuffer bufferType, gtString& bufferName)
{
    bool retVal = true;

    switch (bufferType)
    {
        // Depth Buffer:
        case AP_DEPTH_BUFFER:
        {
            bufferName = L"Depth Buffer";
        }
        break;

        // Stencil buffer:
        case AP_STENCIL_BUFFER:
        {
            bufferName = L"Stencil Buffer";
        }
        break;

        // Front buffer:
        case AP_FRONT_BUFFER:
        {
            bufferName = L"Front Buffer";
        }
        break;

        // Back buffer:
        case AP_BACK_BUFFER:
        {
            bufferName = L"Back Buffer";
        }
        break;

        // Auxiliary buffer [0]:
        case AP_AUX0_BUFFER:
        {
            bufferName = L"Auxiliary Buffer(0)";
        }
        break;

        // Auxiliary buffer [1]:
        case AP_AUX1_BUFFER:
        {
            bufferName = L"Auxiliary Buffer(1)";
        }
        break;

        // Auxiliary buffer [2]:
        case AP_AUX2_BUFFER:
        {
            bufferName = L"Auxiliary Buffer(2)";
        }
        break;

        // Auxiliary buffer [3]:
        case AP_AUX3_BUFFER:
        {
            bufferName = L"Auxiliary Buffer(3)";
        }
        break;

        // Color Attachment Buffer[0]:
        case AP_COLOR_ATTACHMENT0_EXT:
        {
            bufferName = L"Color Attachment Buffer(0)";
        }
        break;

        // Color Attachment Buffer[1]:
        case AP_COLOR_ATTACHMENT1_EXT:
        {
            bufferName = L"Color Attachment Buffer(1)";
        }
        break;

        // Color Attachment Buffer[2]:
        case AP_COLOR_ATTACHMENT2_EXT:
        {
            bufferName = L"Color Attachment Buffer(2)";
        }
        break;

        // Color Attachment Buffer[3]:
        case AP_COLOR_ATTACHMENT3_EXT:
        {
            bufferName = L"Color Attachment Buffer(3)";
        }
        break;

        // Color Attachment Buffer[4]:
        case AP_COLOR_ATTACHMENT4_EXT:
        {
            bufferName = L"Color Attachment Buffer(4)";
        }
        break;

        // Color Attachment Buffer[5]:
        case AP_COLOR_ATTACHMENT5_EXT:
        {
            bufferName = L"Color Attachment Buffer(5)";
        }
        break;

        // Color Attachment Buffer[6]:
        case AP_COLOR_ATTACHMENT6_EXT:
        {
            bufferName = L"Color Attachment Buffer(6)";
        }
        break;

        // Color Attachment Buffer[7]:
        case AP_COLOR_ATTACHMENT7_EXT:
        {
            bufferName = L"Color Attachment Buffer(7)";
        }
        break;

        // Color Attachment Buffer[8]:
        case AP_COLOR_ATTACHMENT8_EXT:
        {
            bufferName = L"Color Attachment Buffer(8)";
        }
        break;

        // Color Attachment Buffer[9]:
        case AP_COLOR_ATTACHMENT9_EXT:
        {
            bufferName = L"Color Attachment Buffer(9)";
        }
        break;

        // Color Attachment Buffer[10]:
        case AP_COLOR_ATTACHMENT10_EXT:
        {
            bufferName = L"Color Attachment Buffer(10)";
        }
        break;

        // Color Attachment Buffer[11]:
        case AP_COLOR_ATTACHMENT11_EXT:
        {
            bufferName = L"Color Attachment Buffer(11)";
        }
        break;

        // Color Attachment Buffer[12]:
        case AP_COLOR_ATTACHMENT12_EXT:
        {
            bufferName = L"Color Attachment Buffer(12)";
        }
        break;

        // Color Attachment Buffer[13]:
        case AP_COLOR_ATTACHMENT13_EXT:
        {
            bufferName = L"Color Attachment Buffer(13)";
        }
        break;

        // Color Attachment Buffer[14]:
        case AP_COLOR_ATTACHMENT14_EXT:
        {
            bufferName = L"Color Attachment Buffer(14)";
        }
        break;

        // Color Attachment Buffer[15]:
        case AP_COLOR_ATTACHMENT15_EXT:
        {
            bufferName = L"Color Attachment Buffer(15)";
        }
        break;

        // Depth Attachment Buffer:
        case AP_DEPTH_ATTACHMENT_EXT:
        {
            bufferName = L"Depth Attachment Buffer";
        }
        break;

        // Stencil Attachment Buffer:
        case AP_STENCIL_ATTACHMENT_EXT:
        {
            bufferName = L"Stencil Attachment Buffer";
        }
        break;

        // Unsupported buffer type
        default:
        {
            // Name convert failed
            retVal = false;
            bufferName = L"";
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGetBufferName
// Description: Returns buffer name for a given buffer type
// Arguments:   bufferType - The buffer type to generate the name for
//              bufferName - Output buffer name
// Return Val:  bool - Success / Failure
// Author:  AMD Developer Tools Team
// Date:        18/8/2007
// --------------------------------------------------------------------------
bool apGetBufferShortName(apDisplayBuffer bufferType, gtString& bufferName)
{
    bool retVal = true;

    switch (bufferType)
    {
        // Depth Buffer:
        case AP_DEPTH_BUFFER:
        {
            bufferName = L"Depth";
        }
        break;

        // Stencil buffer:
        case AP_STENCIL_BUFFER:
        {
            bufferName = L"Stencil";
        }
        break;

        // Front buffer:
        case AP_FRONT_BUFFER:
        {
            bufferName = L"Front";
        }
        break;

        // Back buffer:
        case AP_BACK_BUFFER:
        {
            bufferName = L"Back";
        }
        break;

        // Auxiliary buffer [0]:
        case AP_AUX0_BUFFER:
        {
            bufferName = L"Aux(0)";
        }
        break;

        // Auxiliary buffer [1]:
        case AP_AUX1_BUFFER:
        {
            bufferName = L"Aux(1)";
        }
        break;

        // Auxiliary buffer [2]:
        case AP_AUX2_BUFFER:
        {
            bufferName = L"Aux(2)";
        }
        break;

        // Auxiliary buffer [3]:
        case AP_AUX3_BUFFER:
        {
            bufferName = L"Aux(3)";
        }
        break;

        // Color Attachment Buffer[0]:
        case AP_COLOR_ATTACHMENT0_EXT:
        {
            bufferName = L"Color(0)";
        }
        break;

        // Color Attachment Buffer[1]:
        case AP_COLOR_ATTACHMENT1_EXT:
        {
            bufferName = L"Color(1)";
        }
        break;

        // Color Attachment Buffer[2]:
        case AP_COLOR_ATTACHMENT2_EXT:
        {
            bufferName = L"Color(2)";
        }
        break;

        // Color Attachment Buffer[3]:
        case AP_COLOR_ATTACHMENT3_EXT:
        {
            bufferName = L"Color(3)";
        }
        break;

        // Color Attachment Buffer[4]:
        case AP_COLOR_ATTACHMENT4_EXT:
        {
            bufferName = L"Color(4)";
        }
        break;

        // Color Attachment Buffer[5]:
        case AP_COLOR_ATTACHMENT5_EXT:
        {
            bufferName = L"Color(5)";
        }
        break;

        // Color Attachment Buffer[6]:
        case AP_COLOR_ATTACHMENT6_EXT:
        {
            bufferName = L"Color(6)";
        }
        break;

        // Color Attachment Buffer[7]:
        case AP_COLOR_ATTACHMENT7_EXT:
        {
            bufferName = L"Color(7)";
        }
        break;

        // Color Attachment Buffer[8]:
        case AP_COLOR_ATTACHMENT8_EXT:
        {
            bufferName = L"Color(8)";
        }
        break;

        // Color Attachment Buffer[9]:
        case AP_COLOR_ATTACHMENT9_EXT:
        {
            bufferName = L"Color(9)";
        }
        break;

        // Color Attachment Buffer[10]:
        case AP_COLOR_ATTACHMENT10_EXT:
        {
            bufferName = L"Color(10)";
        }
        break;

        // Color Attachment Buffer[11]:
        case AP_COLOR_ATTACHMENT11_EXT:
        {
            bufferName = L"Color(11)";
        }
        break;

        // Color Attachment Buffer[12]:
        case AP_COLOR_ATTACHMENT12_EXT:
        {
            bufferName = L"Color(12)";
        }
        break;

        // Color Attachment Buffer[13]:
        case AP_COLOR_ATTACHMENT13_EXT:
        {
            bufferName = L"Color(13)";
        }
        break;

        // Color Attachment Buffer[14]:
        case AP_COLOR_ATTACHMENT14_EXT:
        {
            bufferName = L"Color(14)";
        }
        break;

        // Color Attachment Buffer[15]:
        case AP_COLOR_ATTACHMENT15_EXT:
        {
            bufferName = L"Color(15)";
        }
        break;

        // Depth Attachment Buffer:
        case AP_DEPTH_ATTACHMENT_EXT:
        {
            bufferName = L"Depth";
        }
        break;

        // Stencil Attachment Buffer:
        case AP_STENCIL_ATTACHMENT_EXT:
        {
            bufferName = L"Stencil";
        }
        break;

        // Unsupported buffer type
        default:
        {
            // Name convert failed
            retVal = false;
            bufferName = L"";
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsBuffersMonitor::apGetBufferNameCode
// Description: Generates a name code for a buffer type, for the output file name
// Arguments:   bufferType - The buffer type
//              bufferNameCode - The output buffer name code
// Return Val:  bool - Name code was generated successfully (true), otherwise (false)
// Author:  AMD Developer Tools Team
// Date:        26/8/2007
// ---------------------------------------------------------------------------
bool apGetBufferNameCode(apDisplayBuffer bufferType, gtString& bufferNameCode)
{
    bool retVal = true;

    switch (bufferType)
    {
        // Depth Buffer:
        case AP_DEPTH_BUFFER:
            bufferNameCode = L"DepthBuffer";
            break;

        // Stencil buffer:
        case AP_STENCIL_BUFFER:
            bufferNameCode = L"StencilBuffer";
            break;

        // Front buffer:
        case AP_FRONT_BUFFER:
            bufferNameCode = L"FrontBuffer";
            break;

        // Back buffer:
        case AP_BACK_BUFFER:
            bufferNameCode = L"BackBuffer";
            break;

        // Auxiliary buffer [0]:
        case AP_AUX0_BUFFER:
            bufferNameCode = L"Aux0Buffer";
            break;

        // Auxiliary buffer [1]:
        case AP_AUX1_BUFFER:
            bufferNameCode = L"Aux1Buffer";
            break;

        // Auxiliary buffer [2]:
        case AP_AUX2_BUFFER:
            bufferNameCode = L"Aux2Buffer";
            break;

        // Auxiliary buffer [3]:
        case AP_AUX3_BUFFER:
            bufferNameCode = L"Aux3Buffer";
            break;

        // Color Attachment [0]:
        case AP_COLOR_ATTACHMENT0_EXT:
            bufferNameCode = L"ColorAttachment0";
            break;

        // Color Attachment [1]:
        case AP_COLOR_ATTACHMENT1_EXT:
            bufferNameCode = L"ColorAttachment1";
            break;

        // Color Attachment [2]:
        case AP_COLOR_ATTACHMENT2_EXT:
            bufferNameCode = L"ColorAttachment2";
            break;

        // Color Attachment [3]:
        case AP_COLOR_ATTACHMENT3_EXT:
            bufferNameCode = L"ColorAttachment3";
            break;

        // Color Attachment [4]:
        case AP_COLOR_ATTACHMENT4_EXT:
            bufferNameCode = L"ColorAttachment4";
            break;

        // Color Attachment [5]:
        case AP_COLOR_ATTACHMENT5_EXT:
            bufferNameCode = L"ColorAttachment5";
            break;

        // Color Attachment [6]:
        case AP_COLOR_ATTACHMENT6_EXT:
            bufferNameCode = L"ColorAttachment6";
            break;

        // Color Attachment [7]:
        case AP_COLOR_ATTACHMENT7_EXT:
            bufferNameCode = L"ColorAttachment7";
            break;

        // Color Attachment [8]:
        case AP_COLOR_ATTACHMENT8_EXT:
            bufferNameCode = L"ColorAttachment8";
            break;

        // Color Attachment [9]:
        case AP_COLOR_ATTACHMENT9_EXT:
            bufferNameCode = L"ColorAttachment9";
            break;

        // Color Attachment [10]:
        case AP_COLOR_ATTACHMENT10_EXT:
            bufferNameCode = L"ColorAttachment10";
            break;

        // Color Attachment [11]:
        case AP_COLOR_ATTACHMENT11_EXT:
            bufferNameCode = L"ColorAttachment11";
            break;

        // Color Attachment [12]:
        case AP_COLOR_ATTACHMENT12_EXT:
            bufferNameCode = L"ColorAttachment12";
            break;

        // Color Attachment [13]:
        case AP_COLOR_ATTACHMENT13_EXT:
            bufferNameCode = L"ColorAttachment13";
            break;

        // Color Attachment [14]:
        case AP_COLOR_ATTACHMENT14_EXT:
            bufferNameCode = L"ColorAttachment14";
            break;

        // Color Attachment [15]:
        case AP_COLOR_ATTACHMENT15_EXT:
            bufferNameCode = L"ColorAttachment15";
            break;

        // Depth Attachment:
        case AP_DEPTH_ATTACHMENT_EXT:
            bufferNameCode = L"DepthAttachment";
            break;

        // Depth Attachment:
        case AP_STENCIL_ATTACHMENT_EXT:
            bufferNameCode = L"StencilAttachment";
            break;

        // Unsupported buffer type
        default:
        {
            // Empty the buffer name code string
            bufferNameCode.makeEmpty();

            retVal = false;
            GT_ASSERT_EX(false, L"Buffer type is not supported yet!");
        }
        break;
    }

    return retVal;
}
