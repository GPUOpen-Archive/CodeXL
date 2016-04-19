//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suBufferReader.h
///
//==================================================================================

//------------------------------ suBufferReader.h ------------------------------

#ifndef __SUBUFFERREADER_H
#define __SUBUFFERREADER_H

// Forward decelerations:
class osFilePath;

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>
#include <AMDTAPIClasses/Include/apFileType.h>

// Local:
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>

// TO_DO: OpenCL buffers  change name to buffer and textures!
// ----------------------------------------------------------------------------------
// Class Name:           suBufferReader
// General Description: Reads buffers data. Base class for a specific API buffer reading
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ----------------------------------------------------------------------------------
class SU_API suBufferReader
{
public:
    // Constructor:
    suBufferReader();

    // Destructor:
    virtual ~suBufferReader();

    // Return a pointer to the buffer raw data:
    gtByte* getReadBufferData() { return _pReadBufferData; };;

protected:

    // Reads the buffer content into an internal raw data buffer:
    bool readBufferContent(int bufferWidth, int bufferHeight, int bufferDepth, int bufferOffset, oaTexelDataFormat bufferDataFormat, oaDataType componentDataType);

    // Calculate the amount of memory required for the buffer size
    unsigned long calculateBufferMemorySize();

    // Allocate enough memory to hold the buffer data according to it's size and data type and format:
    bool allocateBufferData();

    // Release buffer raw data from memory:
    void releaseBufferData();

protected:

    virtual bool readBufferContentFromAPI() = 0;

protected:

    // The buffer dimensions:
    int _bufferWidth;
    int _bufferHeight;
    int _bufferDepth;

    // Offset from the beginning of the buffer:
    int _bufferOffset;

    // The buffer data format:
    oaTexelDataFormat _bufferDataFormat;

    // A single single data component's data type:
    oaDataType _componentDataType;

    // The buffer read data:
    gtByte* _pReadBufferData;

};


#endif //__suBufferReader_H
