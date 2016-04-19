//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsBufferSerializer.cpp
///
//==================================================================================

//------------------------------ gsBufferSerializer.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSAPIWrappers/Include/oaRawFileSeralizer.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsWrappersCommon.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsBufferSerializer.h>


// ---------------------------------------------------------------------------
// Name:        gsBufferSerializer::gsBufferSerializer
// Description: Constructor
// Author:      Eran Zinman
// Date:        7/8/2007
// ---------------------------------------------------------------------------
gsBufferSerializer::gsBufferSerializer()
{

}


// ---------------------------------------------------------------------------
// Name:        gsBufferSerializer::~gsBufferSerializer
// Description: Destructor
// Author:      Eran Zinman
// Date:        7/8/2007
// ---------------------------------------------------------------------------
gsBufferSerializer::~gsBufferSerializer()
{

}

// ---------------------------------------------------------------------------
// Name:        gsBufferSerializer::saveBufferToFile
// Description: Saves a buffer to a file
// Arguments:   bufferType - The buffer type to be saved
//              bufferFilePath - The file name to save the buffer into
//              isDoubleBuffer - is the application double buffered
// Return Val:  Success / Failure
// Author:      Eran Zinman
// Date:        7/8/2007
// ---------------------------------------------------------------------------
bool gsBufferSerializer::saveBufferToFile(apStaticBuffer& staticBuffer, const osFilePath& bufferFilePath, bool isDoubleBuffer)
{
    bool retVal = false;

    // Get buffer type
    apDisplayBuffer bufferType = staticBuffer.bufferType();

    // Get the size of the hDC which is currently linked with current graphic context
    int hdcWidth = 0;
    int hdcHeight = 0;
    bool rc1 = gsStaticBuffersMonitor::getCurrentThreadHDCSize(hdcWidth, hdcHeight);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Get buffer data format and data format type:
        oaTexelDataFormat bufferDataFormat = OA_TEXEL_FORMAT_UNKNOWN;
        oaDataType componentDataType;
        bool rc3 = gsStaticBuffersMonitor::getBufferDataFormatAndDataType(bufferType, bufferDataFormat, componentDataType);
        GT_IF_WITH_ASSERT(rc3)
        {
            // Update the static buffer object:
            staticBuffer.setBufferDataFormat(bufferDataFormat, componentDataType);
            staticBuffer.setBufferDimensions(hdcWidth, hdcHeight);

            // Read the buffer data to memory:
            gsBufferReader bufferReader;
            bool rc2 = bufferReader.readBufferContent(bufferType, hdcWidth, hdcHeight, bufferDataFormat, componentDataType, isDoubleBuffer);
            GT_IF_WITH_ASSERT(rc2)
            {
                // Get buffer data
                gtByte* pBufferData = bufferReader.getReadBufferData();
                GT_IF_WITH_ASSERT(pBufferData != NULL)
                {
                    // Set the raw data file parameters:
                    oaRawFileSeralizer rawFileSeralizer;
                    rawFileSeralizer.setRawData(pBufferData);
                    rawFileSeralizer.setRawDataDimensions(hdcWidth, hdcHeight);
                    rawFileSeralizer.setRawDataFormat(bufferDataFormat, componentDataType);
                    rawFileSeralizer.setAmountOfPages(1);

                    // Write data to raw file:
                    retVal = rawFileSeralizer.saveToFile(bufferFilePath);

                    // Point the buffer reader to NULL, so it won't release the buffer reader memory
                    rawFileSeralizer.setRawData(NULL);
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsBufferSerializer::saveBufferToFile
// Description:
// Arguments: apStaticBuffer& staticBuffer
//            const osFilePath& bufferFilePath
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/5/2008
// ---------------------------------------------------------------------------
bool gsBufferSerializer::saveBufferToFile(apGLRenderBuffer& renderBuffer, const osFilePath& bufferFilePath)
{
    bool retVal = false;

    // Get buffer data format and data format type:
    oaTexelDataFormat bufferDataFormat = OA_TEXEL_FORMAT_UNKNOWN;
    oaDataType componentDataType;

    // Get the render buffer data format (if the user had already set it):
    bufferDataFormat = renderBuffer.bufferFormat();
    componentDataType = renderBuffer.bufferDataType();

    bool rc1 = false;

    // If the buffer data type is not set, get it according to buffer display type:
    if ((bufferDataFormat != OA_TEXEL_FORMAT_UNKNOWN) && (componentDataType != OA_UNKNOWN_DATA_TYPE))
    {
        rc1 = true;
    }
    else
    {
        rc1 = gsStaticBuffersMonitor::getBufferDataFormatAndDataType(renderBuffer.getBufferType(), bufferDataFormat, componentDataType);
    }

    GT_IF_WITH_ASSERT(rc1)
    {
        // Update the static buffer object:
        renderBuffer.setBufferDataFormat(bufferDataFormat);
        renderBuffer.setBufferDataType(componentDataType);

        // Get the buffer dimensions:
        GLint bufferWidth = 0, bufferHeight = 0;
        renderBuffer.getBufferDimensions(bufferWidth, bufferHeight);

        // Read the buffer data to memory:
        gsBufferReader bufferReader;
        bool rc2 = bufferReader.readBufferContent(renderBuffer.getBufferType(), bufferWidth, bufferHeight, bufferDataFormat, componentDataType, false);
        GT_IF_WITH_ASSERT(rc2)
        {
            // Get buffer data
            gtByte* pBufferData = bufferReader.getReadBufferData();
            GT_IF_WITH_ASSERT(pBufferData != NULL)
            {
                // Set the raw data file parameters:
                oaRawFileSeralizer rawFileSeralizer;
                rawFileSeralizer.setRawData(pBufferData);
                rawFileSeralizer.setRawDataDimensions(bufferWidth, bufferHeight);
                rawFileSeralizer.setRawDataFormat(bufferDataFormat, componentDataType);
                rawFileSeralizer.setAmountOfPages(1);

                // Write data to raw file:
                retVal = rawFileSeralizer.saveToFile(bufferFilePath);

                // Point the buffer reader to NULL, so it won't release the buffer reader memory
                rawFileSeralizer.setRawData(NULL);
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsBufferSerializer::saveBufferToFile
// Description:
// Arguments: apGLVBO& vboDetails
//            GLenum target - the target which the buffer is bound to
//            int offset - the offset from which to read to VBO
//            const osFilePath& bufferFilePath
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/4/2009
// ---------------------------------------------------------------------------
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    bool gsBufferSerializer::saveBufferToFile(apGLVBO& vboDetails, GLenum target, GLintptr offset, const osFilePath& bufferFilePath, PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData)
#else
    bool gsBufferSerializer::saveBufferToFile(apGLVBO& vboDetails, GLenum target, GLintptr offset, const osFilePath& bufferFilePath)
#endif
{
    bool retVal = false;

    // Get the VBO size:
    GLsizeiptr vboSize = (GLsizei)vboDetails.size();

    // Read the buffer data to memory:
    gsBufferReader bufferReader;
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    bool rc2 = bufferReader.readVBOContent(target, offset, vboSize, glGetBufferSubData);
#else
    bool rc2 = bufferReader.readVBOContent(target, offset, vboSize);
#endif
    GT_IF_WITH_ASSERT(rc2)
    {
        // Get buffer data
        gtByte* pBufferData = bufferReader.getReadBufferData();
        GT_IF_WITH_ASSERT(pBufferData != NULL)
        {
            // Set the raw data file parameters:
            oaRawFileSeralizer rawFileSeralizer;
            rawFileSeralizer.setRawData(pBufferData);
            rawFileSeralizer.setRawDataDimensions((int)vboSize, 1);
            rawFileSeralizer.setRawDataFormat(OA_TEXEL_FORMAT_V3F, OA_BYTE);
            rawFileSeralizer.setAmountOfPages(1);

            // Write data to raw file:
            retVal = rawFileSeralizer.saveToFile(bufferFilePath);

            // Point the buffer reader to NULL, so it won't release the buffer reader memory
            rawFileSeralizer.setRawData(NULL);
            retVal = true;
        }
    }

    return retVal;
}








