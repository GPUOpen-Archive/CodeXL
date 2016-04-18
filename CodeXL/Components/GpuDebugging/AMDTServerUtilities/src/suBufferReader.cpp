//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suBufferReader.cpp
///
//==================================================================================

//------------------------------ suBufferReader.cpp ------------------------------

// Standard C:
#include <stdlib.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTServerUtilities/Include/suBufferReader.h>

// ---------------------------------------------------------------------------
// Name:        suBufferReader::suBufferReader
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
suBufferReader::suBufferReader():
    _bufferWidth(0), _bufferHeight(0), _bufferDepth(0), _bufferOffset(0),
    _bufferDataFormat(OA_TEXEL_FORMAT_UNKNOWN), _componentDataType(OA_BYTE),
    _pReadBufferData(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        suBufferReader::~suBufferReader
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
suBufferReader::~suBufferReader()
{
    // Release allocated buffer data (if exists):
    releaseBufferData();
}


// ---------------------------------------------------------------------------
// Name:        suBufferReader::releaseBufferData
// Description: Releases a read buffer data (if exists)
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
void suBufferReader::releaseBufferData()
{
    // If memory is allocated
    if (_pReadBufferData)
    {
        // Release memory
        free(_pReadBufferData);

        // Set pointer to null
        _pReadBufferData = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        suBufferReader::readBufferContent
// Description: Reads an API Buffer content
// Arguments: int bufferWidth
//            int bufferHeight
//            oaTexelDataFormat bufferDataFormat
//            oaDataType componentDataType
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool suBufferReader::readBufferContent(int bufferWidth, int bufferHeight, int bufferDepth, int bufferOffset, oaTexelDataFormat bufferDataFormat, oaDataType componentDataType)
{
    bool retVal = false;

    // Store buffer parameters:
    _bufferDataFormat = bufferDataFormat;
    _bufferWidth = bufferWidth;
    _bufferHeight = bufferHeight;
    _bufferDepth = bufferDepth;
    _bufferOffset = bufferOffset;
    _componentDataType = componentDataType;

    // Release previous operation allocated buffer data (if exists):
    releaseBufferData();

    // Allocate space for the buffer data:
    bool rc1 = allocateBufferData();

    GT_IF_WITH_ASSERT(rc1)
    {
        // Read the buffer content:
        bool rc2 = readBufferContentFromAPI();
        GT_IF_WITH_ASSERT(rc2)
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suBufferReader::calculateBufferMemorySize
// Description: Calculate the memory space that will receive the buffer's data.
// Return Val:  amount of memory size for the buffer
// Author:      Eran Zinman
// Date:        14/8/2007
// ---------------------------------------------------------------------------
unsigned long suBufferReader::calculateBufferMemorySize()
{
    unsigned long bufferSize = 0;

    // Calculate raw data pixel size
    int rawDataPixelSize = oaCalculatePixelUnitByteSize(_bufferDataFormat, _componentDataType);
    GT_IF_WITH_ASSERT(rawDataPixelSize != -1)
    {
        // Calculate the required buffer size (in bytes):
        bufferSize = _bufferWidth * _bufferHeight * _bufferDepth * rawDataPixelSize;
    }

    return bufferSize;
}

// ---------------------------------------------------------------------------
// Name:        suBufferReader::allocateBufferData
// Description: Allocates the memory space that will receive the buffer's data.
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/7/2006
// ---------------------------------------------------------------------------
bool suBufferReader::allocateBufferData()
{
    bool retVal = false;

    // Calculate the amount of memory required to hold the buffer
    unsigned long bufferSize = calculateBufferMemorySize();
    GT_IF_WITH_ASSERT(bufferSize > 0)
    {
        // Allocate the buffer:
        _pReadBufferData = (gtByte*)malloc(bufferSize);
        GT_IF_WITH_ASSERT(_pReadBufferData != NULL)
        {
            retVal = true;
        }
    }

    return retVal;
}
