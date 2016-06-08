//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaPixelFormat.cpp
///
//=====================================================================

//------------------------------ oaPixelFormat.cpp ------------------------------

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaPixelFormat.h>


// ---------------------------------------------------------------------------
// Name:        oaCalcPixelFormatHardwareSupport
// Description: Calculates the hardware acceleration mode of an input pixel format
// Author:      AMD Developer Tools Team
// Date:        6/12/2003
// ---------------------------------------------------------------------------
oaPixelFormat::HardwareSupport oaCalcPixelFormatHardwareSupport(const PIXELFORMATDESCRIPTOR& pixelFormatDescriptior)
{
    oaPixelFormat::HardwareSupport retVal = oaPixelFormat::NO_HARDWARE_ACCELERATION;

    // Get the pixel buffer format flags:
    unsigned long bufferFlags = pixelFormatDescriptior.dwFlags;

    // ------------------------------------------------------------------
    // Notice: The below code is very sensitive - edit it with care !!!!!
    // ------------------------------------------------------------------

    // Is this pixel format rendered by a software generic renderer ?
    // In these days (December 2003), it is usually one of the below:
    // a. The software renderer that comes with the Windows OS by Microsoft.
    // b. The software renderer for Windows by SGI (Not supported anymore by SGI).
    bool isSoftwareDriver = ((bufferFlags & PFD_GENERIC_FORMAT) && !(bufferFlags & PFD_GENERIC_ACCELERATED));

    // Is the generic implementation supplemented by hardware acceleration ?
    // True usually means that an MCD (Mini Client Driver) is at work.
    // (An MCD is a small driver that only exposes the rasterization interface of the
    //  underlying hardware).
    bool isMiniClientDriver = ((bufferFlags & PFD_GENERIC_FORMAT) && (bufferFlags & PFD_GENERIC_ACCELERATED));

    // Is OpenGL fully implemented by the graphic harware ?
    // True usually means that an ICD (Installable Client Driver) is at work.
    // (An ICD is a full OpenGL implementation by a hardware vendor driver)
    bool isInstallableClientDriver = (!(bufferFlags & PFD_GENERIC_FORMAT) && !(bufferFlags & PFD_GENERIC_ACCELERATED));


    // Map the above options the HardwareSupport enumeration:
    if (isInstallableClientDriver)
    {
        retVal = oaPixelFormat::FULL_HARDWARE_ACCELERATION;
    }
    else if (isMiniClientDriver)
    {
        retVal = oaPixelFormat::PARTIAL_HARDWARE_ACCELERATION;
    }
    else if (isSoftwareDriver)
    {
        retVal = oaPixelFormat::NO_HARDWARE_ACCELERATION;
    }
    else
    {
        // We encountered a bufferFlags bit combination that we do not expect:
        GT_ASSERT(0);
        retVal = oaPixelFormat::NO_HARDWARE_ACCELERATION;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaCalcPixelType
// Description: Calculate the pixel type in an input pixel format.
// Author:      AMD Developer Tools Team
// Date:        6/12/2003
// ---------------------------------------------------------------------------
oaPixelFormat::PixelType oaCalcPixelType(const PIXELFORMATDESCRIPTOR& pixelFormatDescriptior)
{
    oaPixelFormat::PixelType retVal = oaPixelFormat::RGBA;

    if (pixelFormatDescriptior.iPixelType == PFD_TYPE_COLORINDEX)
    {
        retVal = oaPixelFormat::COLOR_INDEX;
    }
    else if (pixelFormatDescriptior.iPixelType == PFD_TYPE_RGBA)
    {
        retVal = oaPixelFormat::RGBA;
    }
    else
    {
        // An unknown pixel type:
        GT_ASSERT(0);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaPixelFormat::oaPixelFormat
// Description: Constructor - Buld this representation out of a given
//                            Win32 pixel format.
// Arguments:   hDC - A handle to the device context in which this pixel format resides.
//              pixelFormatIndex - The index of the pixel format in this context.
// Author:      AMD Developer Tools Team
// Date:        2/6/2003
// Implementation Notes:
//
// ---------------------------------------------------------------------------
oaPixelFormat::oaPixelFormat(oaDeviceContextHandle hDC, oaPixelFormatId pixelFormatIndex)
    : _hDC(hDC), _nativeId(pixelFormatIndex), _isInitialized(false),
      _supportsOpenGL(false), _supportsNativeRendering(false), _isDoubleBuffered(false),
      _isStereoscopic(false), _hardwareSupport(NO_HARDWARE_ACCELERATION), _pixelType(RGBA),
      _amountOfColorBits(0), _amountOfRedBits(0), _amountOfGreenBits(0), _amountOfBlueBits(0),
      _amountOfAlphaBits(0), _amountOfZBufferBits(0), _amountOfAccumulationBufferBits(0),
      _amountOfStencilBufferBits(0)
{
}


// ---------------------------------------------------------------------------
// Name:        oaPixelFormat::~oaPixelFormat
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        2/1/2006
// ---------------------------------------------------------------------------
oaPixelFormat::~oaPixelFormat()
{
}

// ---------------------------------------------------------------------------
// Name:        oaPixelFormat::initialize
// Description: Initializes oaPixelFormat attributes from the corresponding
//              Win32 PIXELFORMATDESCRIPTOR.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        6/12/2003
// ---------------------------------------------------------------------------
bool oaPixelFormat::initialize()
{
    // Get the Win32 PixelFormat description:
    PIXELFORMATDESCRIPTOR pixelFormatDescriptior;
    int rc = ::DescribePixelFormat(_hDC, _nativeId, sizeof(PIXELFORMATDESCRIPTOR),
                                   &pixelFormatDescriptior);
    GT_IF_WITH_ASSERT(rc != 0)
    {
        // Fill the pixel format attributes:
        _supportsOpenGL = ((pixelFormatDescriptior.dwFlags & PFD_SUPPORT_OPENGL) != 0);
        _supportsNativeRendering = ((pixelFormatDescriptior.dwFlags & PFD_SUPPORT_GDI) != 0);
        _isDoubleBuffered = ((pixelFormatDescriptior.dwFlags & PFD_DOUBLEBUFFER) != 0);
        _isStereoscopic = ((pixelFormatDescriptior.dwFlags & PFD_STEREO) != 0);
        _hardwareSupport = oaCalcPixelFormatHardwareSupport(pixelFormatDescriptior);
        _pixelType = oaCalcPixelType(pixelFormatDescriptior);
        _amountOfColorBits = pixelFormatDescriptior.cColorBits;
        _amountOfRedBits = pixelFormatDescriptior.cRedBits;
        _amountOfGreenBits = pixelFormatDescriptior.cGreenBits;
        _amountOfBlueBits = pixelFormatDescriptior.cBlueBits;
        _amountOfAlphaBits = pixelFormatDescriptior.cAlphaBits;
        _amountOfZBufferBits = pixelFormatDescriptior.cDepthBits;
        _amountOfAccumulationBufferBits = pixelFormatDescriptior.cAccumBits;
        _amountOfStencilBufferBits = pixelFormatDescriptior.cStencilBits;

        _isInitialized = true;
    }

    return _isInitialized;
}

// ---------------------------------------------------------------------------
// Name:        oaPixelFormat::initializeGLESPixelFormatWithChannelValues
// Description: Initializes the Pixel format objects using default values and values
//              obtained from OpenGL ES. Should only be used by EAGL (iPhone).
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        23/5/2010
// ---------------------------------------------------------------------------
bool oaPixelFormat::initializeGLESPixelFormatWithChannelValues(int amountOfRBits, int amountOfGBits, int amountOfBBits, int amountOfABits, int amountOfDepBits, int amountOfStenBits)
{
    GT_UNREFERENCED_PARAMETER(amountOfRBits);
    GT_UNREFERENCED_PARAMETER(amountOfGBits);
    GT_UNREFERENCED_PARAMETER(amountOfBBits);
    GT_UNREFERENCED_PARAMETER(amountOfABits);
    GT_UNREFERENCED_PARAMETER(amountOfDepBits);
    GT_UNREFERENCED_PARAMETER(amountOfStenBits);
    // We should not get here on non-EAGL implementations:
    GT_ASSERT(false);

    return _isInitialized;
}
