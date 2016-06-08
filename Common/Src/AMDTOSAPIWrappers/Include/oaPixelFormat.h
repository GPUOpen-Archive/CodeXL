//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaPixelFormat.h
///
//=====================================================================

//------------------------------ oaPixelFormat.h ------------------------------

#ifndef __OAPIXELFORMAT
#define __OAPIXELFORMAT

// Local:
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           oaPixelFormat
// General Description:
//   Describes pixel attributes:
//   - Is type: RGBA or color index
//   - Amount of color bits
//   - Amount of depth buffer bits
//   - Is its format supported by the current graphic hadware.
//   - etc
//
//   Serve as a wrapper class for:
//   - Win32 - Pixel format (PIXELFORMATDESCRIPTOR struct)
//   - UNIX - Visual info.
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/12/2003
// ----------------------------------------------------------------------------------
class OA_API oaPixelFormat
{
public:

    // Describes the pixel type:
    enum PixelType
    {
        RGBA,
        COLOR_INDEX
    };

    // Represents the hardware support for this pixel format:
    enum HardwareSupport
    {
        FULL_HARDWARE_ACCELERATION,
        PARTIAL_HARDWARE_ACCELERATION,
        NO_HARDWARE_ACCELERATION
    };

    oaPixelFormat(oaDeviceContextHandle hDC, oaPixelFormatId pixelFormatIndex);
    virtual ~oaPixelFormat();

    bool initialize();
    bool initializeGLESPixelFormatWithChannelValues(int amountOfRBits, int amountOfGBits, int amountOfBBits, int amountOfABits, int amountOfDepBits, int amountOfStenBits);
    bool isInitialized() const { return _isInitialized; };

    // Pixel format attributes access methods:
    oaPixelFormatId nativeId() const { return _nativeId; };
    bool supportsOpenGL() const { return _supportsOpenGL; };
    bool supportsNativeRendering() const { return _supportsNativeRendering; };
    bool isDoubleBuffered() const { return _isDoubleBuffered; };
    bool isStereoscopic() const { return _isStereoscopic; };
    HardwareSupport hardwareSupport() const { return _hardwareSupport; };
    PixelType pixelType() const { return _pixelType; };
    int amountOfColorBits() const { return _amountOfColorBits; };
    int amountOfRedBits() const { return _amountOfRedBits; };
    int amountOfGreenBits() const { return _amountOfGreenBits; };
    int amountOfBlueBits() const { return _amountOfBlueBits; };
    int amountOfAlphaBits() const { return _amountOfAlphaBits; };
    int amountOfZBufferBits() const { return _amountOfZBufferBits; };
    int amountOfAccumulationBufferBits() const { return _amountOfAccumulationBufferBits; };
    int amountOfStencilBufferBits() const { return _amountOfStencilBufferBits; };

private:
    // Do not allow the use of my default constructor:
    oaPixelFormat();

private:
    // A handle to the native device context in which this pixel format resides:
    oaDeviceContextHandle _hDC;

    // The native id of the pixel format (within its device context):
    // (On Win32 - the Pixel Format index)
    oaPixelFormatId _nativeId;

    // Contains true iff the oaPixelFormat attributes were initialized:
    bool _isInitialized;

    // Does it support OpenGL:
    bool _supportsOpenGL;

    // Does it support native rendering:
    bool _supportsNativeRendering;

    // Is it a single buffered / double buffered
    bool _isDoubleBuffered;

    // Does it support stereo:
    bool _isStereoscopic;

    // This pixel format hardware acceleration mode:
    HardwareSupport _hardwareSupport;

    // The type of pixel (RGBA / color index):
    PixelType _pixelType;

    // The amount of color buffer bits per pixel:
    int _amountOfColorBits;

    // The amount of bits used for the red part of the pixel:
    int _amountOfRedBits;

    // The amount of bits used for the green part of the pixel:
    int _amountOfGreenBits;

    // The amount of bits used for the blue part of the pixel:
    int _amountOfBlueBits;

    // The amount of bits used for the alpha part of the pixel:
    int _amountOfAlphaBits;

    // The amount of "Z" buffer bits per pixel:
    int _amountOfZBufferBits;

    // The amount of accumulation buffer bits:
    int _amountOfAccumulationBufferBits;

    // The amount of stencil buffer bits:
    int _amountOfStencilBufferBits;
};


#endif  // __OAPIXELFORMAT
