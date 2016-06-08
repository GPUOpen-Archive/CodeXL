//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file CaptureTypes.h
/// \brief Defines the types of captures supported by a graphics server.
//==============================================================================

#ifndef CAPTURE_TYPES_H
#define CAPTURE_TYPES_H

/// Capture type enumeration
enum CaptureType
{
    CaptureType_APITrace = 1,
    CaptureType_GPUTrace = 2,
    CaptureType_LinkedTrace = 3,
    CaptureType_FullFrameCapture = 4
};

#endif //CAPTURE_TYPES_H

