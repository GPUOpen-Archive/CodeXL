//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osRawMemoryBuffer.h ------------------------------

#ifndef __OSRAWMEMORYBUFFER_H
#define __OSRAWMEMORYBUFFER_H

// Forward declarations:
class osFilePath;

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>


// ----------------------------------------------------------------------------------
// Class Name:          OS_API osRawMemoryBuffer : public osTransferableObject
// General Description: An opaque class holding an arbitrary amount of data, making
//                      it readable and writable from / to osChannels and files.
// Author:      AMD Developer Tools Team
// Creation Date:       5/11/2009
// ----------------------------------------------------------------------------------
class OS_API osRawMemoryBuffer : public osTransferableObject
{
public:
    osRawMemoryBuffer();
    osRawMemoryBuffer(const osRawMemoryBuffer& other);
    ~osRawMemoryBuffer();
    osRawMemoryBuffer& operator=(const osRawMemoryBuffer& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
    osTransferableObject* clone() const;

    // Read to / from file:
    bool fromFile(const osFilePath& filePath);
    bool toFile(const osFilePath& filePath, bool createIfNeeded = true) const;
    bool appendToFile(const osFilePath& filePath, bool createIfNeeded = true) const;

private:
    void clearBuffer();

private:
    // The data in the buffer:
    gtByte* _pData;

    // The data size:
    gtSize_t _dataSize;
};

#endif //__OSRAWMEMORYBUFFER_H

