//------------------------------ apBufferDataFormat.h ------------------------------

#ifndef __APBUFFERDATAFORMAT_H
#define __APBUFFERDATAFORMAT_H

// Infra:
#include <AMDTOSWrappers/Include/osOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

// Describes a buffer data format:
typedef enum
{
    AP_DEPTH_COMPONENT,
    AP_STENCIL_INDEX,
    AP_RGBA,
    AP_RGB
} apBufferDataFormat;


AP_API int apAmountOfDataComponentsPerPixel(apBufferDataFormat bufferDataFormat);
AP_API GLenum apBufferDataFormatToGLEnum(apBufferDataFormat bufferDataFormat);


#endif //__APBUFFERDATAFORMAT_H

