//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csBufferSerializer.cpp
///
//==================================================================================

//------------------------------ csBufferSerializer.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSAPIWrappers/Include/oaRawFileSeralizer.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <src/csBufferSerializer.h>
#include <src/csMemoryObjectReader.h>


// ---------------------------------------------------------------------------
// Name:        csBufferSerializer::csBufferSerializer
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
csBufferSerializer::csBufferSerializer()
{

}


// ---------------------------------------------------------------------------
// Name:        csBufferSerializer::~csBufferSerializer
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
csBufferSerializer::~csBufferSerializer()
{

}

// ---------------------------------------------------------------------------
// Name:        csBufferSerializer::
// Description: Saves a buffer to a file
// Arguments: apCLBuffer& bufferDetails
//            const osFilePath& bufferFilePath
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool csBufferSerializer::saveBufferToFile(apCLBuffer& bufferDetails, const osFilePath& bufferFilePath, oaCLCommandQueueHandle commandQueueHandle)
{
    bool retVal = false;

    // Get the buffer size:
    gtSize_t bufferSize = (GLsizei)bufferDetails.bufferSize();

    // Read the buffer data to memory:
    csMemoryObjectReader bufferReader;
    oaTexelDataFormat dataFormat = OA_TEXEL_FORMAT_V1F;
    oaDataType componentDataType = OA_BYTE;
    oaCLMemHandle bufferHandle = bufferDetails.memObjectHandle();
    bool rc = bufferReader.readMemoryObjectContent(commandQueueHandle, bufferHandle, (int)bufferSize, 1, 1, 0, OS_TOBJ_ID_CL_BUFFER, dataFormat, componentDataType);
    GT_IF_WITH_ASSERT(rc)
    {
        // Get buffer data
        gtByte* pBufferData = bufferReader.getReadBufferData();
        GT_IF_WITH_ASSERT(pBufferData != NULL)
        {
            // Set the raw data file parameters:
            oaRawFileSeralizer rawFileSeralizer;
            rawFileSeralizer.setRawData(pBufferData);
            rawFileSeralizer.setRawDataDimensions((int)bufferSize, 1);
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


// ---------------------------------------------------------------------------
// Name:        csBufferSerializer::saveSubBufferToFile
// Description: Saves a sub-buffer to a file
// Arguments:   apCLSubBuffer& subBufferDetails
//              const osFilePath& subBufferFilePath
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/1/2011
// ---------------------------------------------------------------------------
bool csBufferSerializer::saveSubBufferToFile(apCLSubBuffer& subBufferDetails, const apCLBuffer& ownerBuffer, const osFilePath& subBufferFilePath, oaCLCommandQueueHandle commandQueueHandle)
{
    bool retVal = false;

    // Get the buffer size:
    gtSize_t bufferSize = (GLsizei)subBufferDetails.bufferRegion().size;
    int bufferOffset = (int)subBufferDetails.bufferRegion().origin;


    // Read the buffer data to memory:
    csMemoryObjectReader bufferReader;
    oaTexelDataFormat dataFormat = OA_TEXEL_FORMAT_V1F;
    oaDataType componentDataType = OA_BYTE;
    oaCLMemHandle bufferHandle = ownerBuffer.memObjectHandle();
    bool rc = bufferReader.readMemoryObjectContent(commandQueueHandle, bufferHandle, (int)bufferSize, 1, 1, bufferOffset, OS_TOBJ_ID_CL_BUFFER, dataFormat, componentDataType);
    GT_IF_WITH_ASSERT(rc)
    {
        // Get buffer data
        gtByte* pBufferData = bufferReader.getReadBufferData();
        GT_IF_WITH_ASSERT(pBufferData != NULL)
        {
            // Set the raw data file parameters:
            oaRawFileSeralizer rawFileSeralizer;
            rawFileSeralizer.setRawData(pBufferData);
            rawFileSeralizer.setRawDataDimensions((int)bufferSize, 1);
            rawFileSeralizer.setRawDataFormat(OA_TEXEL_FORMAT_V3F, OA_BYTE);
            rawFileSeralizer.setAmountOfPages(1);

            // Write data to raw file:
            retVal = rawFileSeralizer.saveToFile(subBufferFilePath);

            // Point the buffer reader to NULL, so it won't release the buffer reader memory
            rawFileSeralizer.setRawData(NULL);
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csBufferSerializer::
// Description: Saves a buffer to a file
// Arguments: apCLImage& textureDetails
//            const osFilePath& textureFilePath
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool csBufferSerializer::saveTextureToFile(apCLImage& textureDetails, const osFilePath& textureFilePath, oaCLCommandQueueHandle commandQueueHandle)
{
    bool retVal = false;

    // Get the texture dimensions:
    gtSize_t textureWidth = 0, textureHeight = 0, textureDepth = 0;
    textureDetails.getDimensions(textureWidth, textureHeight, textureDepth);

    // Read the buffer data to memory:
    csMemoryObjectReader textureReader;

    // Get the texture OpenCL data format and type:
    cl_uint clDataFormat = textureDetails.dataFormat();
    cl_uint clDataType = textureDetails.dataType();

    // Get the texture details:
    oaTexelDataFormat dataFormat;
    oaDataType componentDataType;

    // Get the formats as oaTexelDataFormat and oaDataType:
    bool rc1 = oaCLImageFormatToTexelFormat(clDataFormat, dataFormat);
    bool rc2 = oaCLImageDataTypeToOSDataType(clDataType, componentDataType);

    GT_IF_WITH_ASSERT(rc1 && rc2)
    {
        oaCLMemHandle textureHandle = textureDetails.memObjectHandle();
        bool rc = textureReader.readMemoryObjectContent(commandQueueHandle, textureHandle, (int)textureWidth, (int)textureHeight, (int)textureDepth, 0, OS_TOBJ_ID_CL_IMAGE, dataFormat, componentDataType);

        GT_IF_WITH_ASSERT(rc)
        {
            // Get buffer data
            gtByte* pBufferData = textureReader.getReadBufferData();
            GT_IF_WITH_ASSERT(pBufferData != NULL)
            {
                // Calculate amount of pages which is depth + 1
                gtSize_t amountOfPages = 1;

                if (textureDepth > 1)
                {
                    amountOfPages = textureDepth;
                }

                // Set the raw data file parameters:
                oaRawFileSeralizer rawFileSeralizer;
                rawFileSeralizer.setRawData(pBufferData);
                rawFileSeralizer.setRawDataDimensions((int)textureWidth, (int)textureHeight);
                rawFileSeralizer.setRawDataFormat(dataFormat, componentDataType);
                rawFileSeralizer.setAmountOfPages((int)amountOfPages);

                // Write data to raw file:
                retVal = rawFileSeralizer.saveToFile(textureFilePath);

                // Point the buffer reader to NULL, so it won't release the buffer reader memory
                rawFileSeralizer.setRawData(NULL);
                retVal = true;
            }
        }

    }

    return retVal;
}










