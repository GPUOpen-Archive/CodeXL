//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apAPIVersion.cpp
///
//==================================================================================

//------------------------------ apAPIVersion.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIVersion.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>



// ---------------------------------------------------------------------------
// Name:        apOpenGLVersionToString
// Description: Translate an OpenGL version enumeration to a string
// Arguments: apAPIVersion version
//            gtString& versionAsStr
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/2/2009
// ---------------------------------------------------------------------------
bool apAPIVersionToString(apAPIVersion version, gtString& versionAsStr)
{
    bool retVal = true;

    switch (version)
    {
        case AP_GL_VERSION_1_0:
            versionAsStr = L"OpenGL 1.0";
            break;

        case AP_GL_VERSION_1_1:
            versionAsStr = L"OpenGL 1.1";
            break;

        case AP_GL_VERSION_1_2:
            versionAsStr = L"OpenGL 1.2";
            break;

        case AP_GL_VERSION_1_3:
            versionAsStr = L"OpenGL 1.3";
            break;

        case AP_GL_VERSION_1_4:
            versionAsStr = L"OpenGL 1.4";
            break;

        case AP_GL_VERSION_1_5:
            versionAsStr = L"OpenGL 1.5";
            break;

        case AP_GL_VERSION_2_0:
            versionAsStr = L"OpenGL 2.0";
            break;

        case AP_GL_VERSION_2_1:
            versionAsStr = L"OpenGL 2.1";
            break;

        case AP_GL_VERSION_3_0:
            versionAsStr = L"OpenGL 3.0";
            break;

        case AP_GL_VERSION_3_1:
            versionAsStr = L"OpenGL 3.1";
            break;

        case AP_GL_VERSION_3_2:
            versionAsStr = L"OpenGL 3.2";
            break;

        case AP_GL_VERSION_4_0:
            versionAsStr = L"OpenGL 4.0";
            break;

        case AP_GL_VERSION_4_1:
            versionAsStr = L"OpenGL 4.1";
            break;

        case AP_GL_VERSION_4_2:
            versionAsStr = L"OpenGL 4.2";
            break;

        case AP_GL_VERSION_4_3:
            versionAsStr = L"OpenGL 4.3";
            break;

        case AP_CL_VERSION_1_0:
            versionAsStr = L"OpenCL 1.0";
            break;

        case AP_CL_VERSION_1_1:
            versionAsStr = L"OpenCL 1.1";
            break;

        case AP_CL_VERSION_1_2:
            versionAsStr = L"OpenCL 1.2";
            break;

        case AP_CL_VERSION_2_0:
            versionAsStr = L"OpenCL 2.0";
            break;

        case AP_GL_VERSION_NONE:
        case AP_CL_VERSION_NONE:
            versionAsStr = L"None";
            break;

        default:
            GT_ASSERT_EX(false, L"Unsupported API version");
            retVal = false;
            break;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        apGLSLVersionToString
// Description: Translate the apOpenGLSLVersion enum to a string
// Arguments: apGLSLVersion version
//            gtString& versionAsStr
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        24/3/2009
// ---------------------------------------------------------------------------
bool apGLSLVersionToString(apGLSLVersion version, gtString& versionAsStr)
{
    bool retVal = true;

    switch (version)
    {
        case AP_GLSL_VERSION_1_0:
            versionAsStr = L"1.0";
            break;

        case AP_GLSL_VERSION_1_1:
            versionAsStr = L"1.1";
            break;

        case AP_GLSL_VERSION_1_2:
            versionAsStr = L"1.2";
            break;

        case AP_GLSL_VERSION_1_3:
            versionAsStr = L"1.3";
            break;

        case AP_GLSL_VERSION_1_4:
            versionAsStr = L"1.4";
            break;

        case AP_GLSL_VERSION_NONE:
            versionAsStr = L"None";
            break;

        default:
            GT_ASSERT_EX(false, L"Unsupported GLSL version");
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apStringToGLSLVersion
// Description: Extract the GLSL
// Arguments: const gtString& versionAsStr
//            apGLSLVersion& version
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        24/3/2009
// ---------------------------------------------------------------------------
bool apStringToGLSLVersion(const gtString& versionAsStr, apGLSLVersion& version)
{
    bool retVal = false;

    version = AP_GLSL_VERSION_NONE;

    // Convert the string to a number:
    retVal = versionAsStr.isIntegerNumber();

    if (retVal)
    {
        // Extract the number from the string:
        int versionAsNumber = 0;
        retVal = versionAsStr.toIntNumber(versionAsNumber);

        if (retVal)
        {
            // Translate the number to a GLSL version:
            if (versionAsNumber <= 100)
            {
                version = AP_GLSL_VERSION_1_0;
            }
            else if ((versionAsNumber > 100) && (versionAsNumber <= 110))
            {
                version = AP_GLSL_VERSION_1_1;
            }
            else if ((versionAsNumber > 110) && (versionAsNumber <= 120))
            {
                version = AP_GLSL_VERSION_1_2;
            }
            else if ((versionAsNumber > 120) && (versionAsNumber <= 130))
            {
                version = AP_GLSL_VERSION_1_3;
            }
            else if ((versionAsNumber > 130) && (versionAsNumber <= 140))
            {
                version = AP_GLSL_VERSION_1_4;
            }
            else
            {
                version = AP_GLSL_VERSION_NONE;
                retVal = false;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apOpenGLVersionFromInts
// Description: Returns the enum value appropriate to the given numbers
// Return Val: apAPIVersion
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
apAPIVersion apOpenGLVersionFromInts(int oglMajorVersion, int oglMinorVersion)
{
    apAPIVersion retVal = AP_GL_VERSION_NONE;

    if (oglMajorVersion == 1)
    {
        if (oglMinorVersion == 0)
        {
            retVal = AP_GL_VERSION_1_0;
        }
        else if (oglMinorVersion == 1)
        {
            retVal = AP_GL_VERSION_1_1;
        }
        else if (oglMinorVersion == 2)
        {
            retVal = AP_GL_VERSION_1_2;
        }
        else if (oglMinorVersion == 3)
        {
            retVal = AP_GL_VERSION_1_3;
        }
        else if (oglMinorVersion == 4)
        {
            retVal = AP_GL_VERSION_1_4;
        }
        else if (oglMinorVersion == 5)
        {
            retVal = AP_GL_VERSION_1_5;
        }
        else
        {
            // Unexpected 1.x value, return 1.0 to be "safe":
            retVal = AP_GL_VERSION_1_0;
            GT_ASSERT(false);
        }
    }
    else if (oglMajorVersion == 2)
    {
        if (oglMinorVersion == 0)
        {
            retVal = AP_GL_VERSION_2_0;
        }
        else if (oglMinorVersion == 1)
        {
            retVal = AP_GL_VERSION_2_1;
        }
        else
        {
            // Unexpected 2.x value, return 2.0 to be "safe":
            retVal = AP_GL_VERSION_2_0;
            GT_ASSERT(false);
        }
    }
    else if (oglMajorVersion == 3)
    {
        if (oglMinorVersion == 0)
        {
            retVal = AP_GL_VERSION_3_0;
        }
        else if (oglMinorVersion == 1)
        {
            retVal = AP_GL_VERSION_3_1;
        }
        else if (oglMinorVersion == 2)
        {
            retVal = AP_GL_VERSION_3_2;
        }
        else if (oglMinorVersion == 3)
        {
            retVal = AP_GL_VERSION_3_3;
        }
        else
        {
            // Unexpected 3.x value, return 3.0 to be "safe":
            retVal = AP_GL_VERSION_3_0;
            GT_ASSERT(false);
        }
    }
    else if (oglMajorVersion == 4)
    {
        if (oglMinorVersion == 0)
        {
            retVal = AP_GL_VERSION_4_0;
        }
        else if (oglMinorVersion == 1)
        {
            retVal = AP_GL_VERSION_4_1;
        }
        else if (oglMinorVersion == 2)
        {
            retVal = AP_GL_VERSION_4_2;
        }
        else if (oglMinorVersion == 3)
        {
            retVal = AP_GL_VERSION_4_3;
        }
        else if (oglMinorVersion > 4)
        {
            // Future OpenGL versions, treat as the highest version we support
            // but output a message:
            retVal = AP_GL_VERSION_4_3;
            gtString logMsg;
            logMsg.appendFormattedString(AP_STR_unexpectedOGLVersionTreatedAsLower, oglMajorVersion, oglMinorVersion);
            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }
        else
        {
            // Unexpected 4.x value, return 4.0 to be "safe":
            retVal = AP_GL_VERSION_4_0;
            GT_ASSERT(false);
        }
    }
    else if (oglMajorVersion > 4)
    {
        // Future OpenGL versions, treat as the highest version we support
        // but output a message:
        retVal = AP_GL_VERSION_4_3;
        gtString logMsg;
        logMsg.appendFormattedString(AP_STR_unexpectedOGLVersionTreatedAsLower, oglMajorVersion, oglMinorVersion);
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
    else
    {
        // Unexpected value (0.x or negative value):
        GT_ASSERT(false);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apOpenGLVersionToInts
// Description: Returns the major and minor version numbers from an OpenGL version enum
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
void apOpenGLVersionToInts(apAPIVersion oglVersion, int& oglMajorVersion, int& oglMinorVersion)
{
    switch (oglVersion)
    {
        case AP_GL_VERSION_1_0:
        {
            oglMajorVersion = 1;
            oglMinorVersion = 0;
        }
        break;

        case AP_GL_VERSION_1_1:
        {
            oglMajorVersion = 1;
            oglMinorVersion = 1;
        }
        break;

        case AP_GL_VERSION_1_2:
        {
            oglMajorVersion = 1;
            oglMinorVersion = 2;
        }
        break;

        case AP_GL_VERSION_1_3:
        {
            oglMajorVersion = 1;
            oglMinorVersion = 3;
        }
        break;

        case AP_GL_VERSION_1_4:
        {
            oglMajorVersion = 1;
            oglMinorVersion = 4;
        }
        break;

        case AP_GL_VERSION_1_5:
        {
            oglMajorVersion = 1;
            oglMinorVersion = 5;
        }
        break;

        case AP_GL_VERSION_2_0:
        {
            oglMajorVersion = 2;
            oglMinorVersion = 0;
        }
        break;

        case AP_GL_VERSION_2_1:
        {
            oglMajorVersion = 2;
            oglMinorVersion = 1;
        }
        break;

        case AP_GL_VERSION_3_0:
        {
            oglMajorVersion = 3;
            oglMinorVersion = 0;
        }
        break;

        case AP_GL_VERSION_3_1:
        {
            oglMajorVersion = 3;
            oglMinorVersion = 1;
        }
        break;

        case AP_GL_VERSION_3_2:
        {
            oglMajorVersion = 3;
            oglMinorVersion = 2;
        }
        break;

        case AP_GL_VERSION_4_0:
        {
            oglMajorVersion = 4;
            oglMinorVersion = 0;
        }
        break;

        case AP_GL_VERSION_4_1:
        {
            oglMajorVersion = 4;
            oglMinorVersion = 1;
        }
        break;

        case AP_GL_VERSION_4_2:
        {
            oglMajorVersion = 4;
            oglMinorVersion = 2;
        }
        break;

        case AP_GL_VERSION_4_3:
        {
            oglMajorVersion = 4;
            oglMinorVersion = 3;
        }
        break;

        case AP_GL_VERSION_NONE:
        {
            oglMajorVersion = 0;
            oglMinorVersion = 0;
        }
        break;

        default:
        {
            // A version was added to the enum but not updated here:
            GT_ASSERT(false);
        }
        break;
    }
}


