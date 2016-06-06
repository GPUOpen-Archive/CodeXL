//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apAIDFunctions.cpp
///
//==================================================================================

//------------------------------ ap2DPoint.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apAIDFunctions.h>

// Infra:
#include <AMDTBaseTools/Include/gtStringTokenizer.h>

// ---------------------------------------------------------------------------
// Name:        apIsTextureUniformType
// Description: Does the uniform type represent a texture uniform type?
// Arguments: GLenum uniformType
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool apIsTextureUniformType(GLenum uniformType)
{
    bool retVal = false;

    switch (uniformType)
    {
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_2D_RECT_ARB:
        case GL_SAMPLER_2D_RECT_SHADOW_ARB:
        case GL_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
        {
            retVal = true;
            break;
        }

        default:
        {
            retVal = false;
            break;
        }

    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apHandleXMLEscaping
// Description: Handles escaping of angled brackets for use in XML files
// Arguments: string - the string to escape / unescape
//            toEscape - true = changes <, > to ^((, ^)) and (, ) to ^(, ^)
//              false = changes back ( ^(( to < and so on)
// Author:  AMD Developer Tools Team
// Date:        1/6/2008
// ---------------------------------------------------------------------------
void apHandleXMLEscaping(gtString& string, bool toEscape)
{
    if (toEscape)
    {
        string.replace(L"(", L"^(");
        string.replace(L")", L"^)");
        string.replace(L"<", L"^((");
        string.replace(L">", L"^))");
    }
    else
    {
        string.replace(L"^((", L"<");
        string.replace(L"^))", L">");
        string.replace(L"^(", L"(");
        string.replace(L"^)", L")");
    }
}


// ---------------------------------------------------------------------------
// Name:        apLookForFileInAdditionalDirectories
// Description: Look for a file in additional directories supplied in a string separated with ';'
// Return Val:  bool AP_API - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/7/2012
// ---------------------------------------------------------------------------
bool AP_API apLookForFileInAdditionalDirectories(const osFilePath& targetFile, const gtString& additionalDir, osFilePath& foundPath)
{
    bool retVal = false;

    // The Additional Directories are ';' separated:
    gtStringTokenizer strTokenizer(additionalDir, L";");
    gtString currentAdditionalDirectory;

    // Iterate over the Additional Directories strings to find the source file:
    while (strTokenizer.getNextToken(currentAdditionalDirectory))
    {
        foundPath = targetFile;
        foundPath.setFileDirectory(currentAdditionalDirectory);

        if (foundPath.exists() && foundPath.isRegularFile())
        {
            retVal = true;
            break;
        }
    }

    return retVal;
}
