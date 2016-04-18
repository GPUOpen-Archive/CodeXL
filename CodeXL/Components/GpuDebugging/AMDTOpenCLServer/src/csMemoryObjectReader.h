//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csMemoryObjectReader.h
///
//==================================================================================

//------------------------------ csBufferReader.h ------------------------------

#ifndef __CSBUFFERREADER_H
#define __CSBUFFERREADER_H

// Forward decelerations:
class osFilePath;

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>
#include <AMDTAPIClasses/Include/apFileType.h>

// Spy utilities:
#include <AMDTServerUtilities/Include/suBufferReader.h>

// TO_DO: OpenCL buffers  change file name
// ----------------------------------------------------------------------------------
// Class Name:           csBufferReader : public suBufferReader
// General Description: Reads OpenCL buffers data
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ----------------------------------------------------------------------------------
class csMemoryObjectReader : public suBufferReader
{
public:
    // Constructor:
    csMemoryObjectReader();

    // Destructor:
    virtual ~csMemoryObjectReader();

    // Reads the memory object content into an internal raw data buffer:
    bool readMemoryObjectContent(oaCLCommandQueueHandle commandQueueHandle, oaCLMemHandle bufferMemoryHandle, int bufferWidth, int bufferHeight, int bufferDepth, int bufferOffset,
                                 osTransferableObjectType objectType, oaTexelDataFormat bufferDataFormat, oaDataType componentDataType);

protected:
    virtual bool readBufferContentFromAPI();

private:

    // Buffer memory handle:
    oaCLMemHandle _bufferMemoryHandle;

    // Command queue handle:
    oaCLCommandQueueHandle _commandQueueHandle;

    // Memory object type:
    osTransferableObjectType _memoryObjectType;

};


#endif //__csBufferReader_H
