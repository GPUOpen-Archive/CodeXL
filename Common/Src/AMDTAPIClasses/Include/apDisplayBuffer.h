//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDisplayBuffer.h
///
//==================================================================================

//------------------------------ apDisplayBuffer.h ------------------------------

#ifndef __APDISPLAYBUFFER
#define __APDISPLAYBUFFER

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

// Predeclarations:
class gtString;

// Describes a display buffer:
typedef enum
{
    AP_FRONT_BUFFER,            // Front Buffer
    AP_BACK_BUFFER,             // Back Buffer
    AP_AUX0_BUFFER,             // Auxiliary Buffer [0]
    AP_AUX1_BUFFER,             // Auxiliary Buffer [1]
    AP_AUX2_BUFFER,             // Auxiliary Buffer [2]
    AP_AUX3_BUFFER,             // Auxiliary Buffer [3]
    AP_DEPTH_BUFFER,            // Depth Buffer
    AP_STENCIL_BUFFER,          // Stencil Buffer
    AP_COLOR_ATTACHMENT0_EXT,   // FBO color attachment 0
    AP_COLOR_ATTACHMENT1_EXT,   // FBO color attachment 1
    AP_COLOR_ATTACHMENT2_EXT,   // FBO color attachment 2
    AP_COLOR_ATTACHMENT3_EXT,   // FBO color attachment 3
    AP_COLOR_ATTACHMENT4_EXT,   // FBO color attachment 4
    AP_COLOR_ATTACHMENT5_EXT,   // FBO color attachment 5
    AP_COLOR_ATTACHMENT6_EXT,   // FBO color attachment 6
    AP_COLOR_ATTACHMENT7_EXT,   // FBO color attachment 7
    AP_COLOR_ATTACHMENT8_EXT,   // FBO color attachment 8
    AP_COLOR_ATTACHMENT9_EXT,   // FBO color attachment 9
    AP_COLOR_ATTACHMENT10_EXT,  // FBO color attachment 10
    AP_COLOR_ATTACHMENT11_EXT,  // FBO color attachment 11
    AP_COLOR_ATTACHMENT12_EXT,  // FBO color attachment 12
    AP_COLOR_ATTACHMENT13_EXT,  // FBO color attachment 13
    AP_COLOR_ATTACHMENT14_EXT,  // FBO color attachment 14
    AP_COLOR_ATTACHMENT15_EXT,  // FBO color attachment 15
    AP_DEPTH_ATTACHMENT_EXT,    // FBO depth attachment
    AP_STENCIL_ATTACHMENT_EXT,  // FBO stencil attachment
    AP_DISPLAY_BUFFER_UNKNOWN
} apDisplayBuffer;

AP_API GLenum apColorIndexBufferTypeToGLEnum(apDisplayBuffer bufferType);
AP_API apDisplayBuffer apGLEnumToColorIndexBufferType(GLuint openGLBufferType);
AP_API bool apGetBufferName(apDisplayBuffer bufferType, gtString& bufferName);
AP_API bool apGetBufferShortName(apDisplayBuffer bufferType, gtString& bufferName);
AP_API bool apGetBufferNameCode(apDisplayBuffer bufferType, gtString& bufferNameCode);

#endif  // __APDISPLAYBUFFER
