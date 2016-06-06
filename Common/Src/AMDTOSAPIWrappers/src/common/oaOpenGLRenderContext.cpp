//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaOpenGLRenderContext.cpp
///
//=====================================================================

//------------------------------ oaOpenGLRenderContext.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osModule.h>

// Local:
#include <common/oaOpenGLFunctionPointers.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLRenderContext.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaStringConstants.h>
#include <AMDTOSAPIWrappers/Include/oaHiddenWindow.h>

// Static members initializations:
oaHiddenWindow* oaOpenGLRenderContext::_pDefaultRenderContextWindow = nullptr;

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::isValid
// Description: Returns true iff this render context is valid.
// Author:      AMD Developer Tools Team
// Date:        2/1/2006
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::isValid() const
{
    bool retVal = (_hOpenGLRC != NULL);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::isExtensionSupported
// Description:
// Arguments: extensionName - the name of the extension (eg GL_EXT_geometry_shader4)
// Return Val: bool - true if supported, false otherwise.
// Author:      AMD Developer Tools Team
// Date:        14/5/2008
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::isExtensionSupported(const gtString& extensionName)
{
    bool retVal = false;
    gtString extensionsList;
    bool extensionListAcquired = extensionsString(extensionsList);

    if (extensionListAcquired)
    {
        // The extension strings are space separated:
        gtStringTokenizer strTokenizer(extensionsList, L" ");
        gtString currentExtension;

        // Iterate over the extension strings, and push them into the output list:
        bool goOn = true;

        while (goOn && (strTokenizer.getNextToken(currentExtension)))
        {
            if (currentExtension == extensionName)
            {
                retVal = true;
                goOn = false;
            }
        }
    }

    if (!retVal)
    {
        // This could be a glX of wgl extension:
        extensionListAcquired = platformSpecificExtensionsString(extensionsList);

        if (extensionListAcquired)
        {
            // The extension strings are space separated:
            gtStringTokenizer strTokenizer(extensionsList, L" ");
            gtString currentExtension;

            // Iterate over the extension strings, and push them into the output list:
            bool goOn = true;

            while (goOn && (strTokenizer.getNextToken(currentExtension)))
            {
                if (currentExtension == extensionName)
                {
                    retVal = true;
                    goOn = false;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::getOpenGLString
// Description: A wrapper for the "glGetString" function.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/12/2008
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::getOpenGLString(/*GLenum*/ unsigned int name, gtString& stringValue)
{
    bool retVal = false;

    // If we don't have a pointer to glGetString:
    if (pOAglGetString == NULL)
    {
        // Get the system's OpenGL function pointers:
        bool rcOGLFuncPtrs = oaLoadOpenGLFunctionPointers();
        GT_ASSERT(rcOGLFuncPtrs);
    }

    // Make sure we got the pointer:
    GT_IF_WITH_ASSERT(pOAglGetString != NULL)
    {
        const char* tempStringValue = (const char*)pOAglGetString((GLenum)name);

        // If we got a result:
        if (tempStringValue != NULL)
        {
            // If the result is not empty:
            if (*tempStringValue != (wchar_t)0)
            {
                stringValue.fromASCIIString(tempStringValue);
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::extensionsString
// Description: Gets the supported extensions string by using glGetString(GL_EXTENSIONS)
// Arguments: extensions - the list goes here
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        19/5/2008
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::extensionsString(gtString& extensions)
{
    bool retVal = false;

    if (_extensionsString.isEmpty())
    {
        // Make the "default" render context the current context of this thread:
        // Note: The "default" render context should remain the current context of the application
        //       thread from now on!
        bool rc = makeCurrent();
        GT_IF_WITH_ASSERT(rc)
        {
            // Query OpenGL extensions list:
            // ----------------------------
            retVal = getOpenGLString(GL_EXTENSIONS, _extensionsString);
            rc = doneCurrent();
            GT_ASSERT(rc);
        }
    }
    else
    {
        retVal = true;
    }

    if (retVal)
    {
        extensions = _extensionsString;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::extractVendorType
// Description: Extract the vendor from the vendor string
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/10/2009
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::extractVendorType()
{
    bool retVal = false;

    _vendorType = OA_VENDOR_UNKNOWN;

    // Make this render context the current context of this thread:
    // Note: The "default" render context should remain the current context of the application
    //       thread from now on!
    bool rc = makeCurrent();
    GT_IF_WITH_ASSERT(rc)
    {
        // Get the renderer string from OpenGL:
        rc = getOpenGLString(GL_VENDOR, _vendorString);
        GT_IF_WITH_ASSERT(rc)
        {
            retVal = true;

            // Find one of the renderer types within the vendor string:
            if (_vendorString.startsWith(OA_STR_VENDOR_ATI))
            {
                _vendorType = OA_VENDOR_ATI;
            }
            else if (_vendorString.startsWith(OA_STR_VENDOR_NVIDIA))
            {
                _vendorType = OA_VENDOR_NVIDIA;
            }
            else if (_vendorString.startsWith(OA_STR_VENDOR_INTEL))
            {
                _vendorType = OA_VENDOR_INTEL;
            }
            else if (_vendorString.startsWith(OA_STR_VENDOR_S3))
            {
                _vendorType = OA_VENDOR_S3;
            }
            else if (_vendorString.startsWith(OA_STR_VENDOR_MICROSOFT))
            {
                _vendorType = OA_VENDOR_MICROSOFT;
            }
            else if (_vendorString.startsWith(OA_STR_VENDOR_MESA))
            {
                _vendorType = OA_VENDOR_MESA;
            }
            else
            {
                retVal = false;
            }
        }

        rc = doneCurrent();
        GT_ASSERT(rc);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaOpenGLRenderContext::InitHiddenWindow
// Description:
//   Create the hidden window that is used for creating the context
//   This function is public to allow the main thread to create the window before any other thread attempts to access
//   the render context. This is because on Windows only the thread that created a window is allowed to destroy it.
//
// Return Val: true if creation succeeded, false otherwise
//
// Author:      AMD Developer Tools Team
// Date:        Oct-22, 2015
// ---------------------------------------------------------------------------
bool oaOpenGLRenderContext::InitHiddenWindow()
{
    bool retVal = true;

    if (nullptr == _pDefaultRenderContextWindow)
    {
        // Create a window class:
        _pDefaultRenderContextWindow = new oaHiddenWindow;

        // Create a native hidden window (2X2 pixels):
        retVal = _pDefaultRenderContextWindow->create(L"oaOpenGLRenderContext::getDefaultRenderContext hidden window", 0, 0, 2, 2);
        GT_ASSERT(retVal)
    }

    return retVal;
}