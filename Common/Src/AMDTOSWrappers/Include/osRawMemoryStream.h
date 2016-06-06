//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osRawMemoryStream.h
///
//=====================================================================

//------------------------------ osRawMemoryStream.h ------------------------------

#ifndef __OSRAWMEMORYSTREAM
#define __OSRAWMEMORYSTREAM

// Forward decelerations:
class gtIAllocationFailureObserver;

// Local:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>


// ----------------------------------------------------------------------------------
// Class Name:           osRawMemoryStream : public osChannel
// General Description:
//   A stream containing raw memory (gtByte elements).
// Author:      AMD Developer Tools Team
// Creation Date:        4/5/2004
// ----------------------------------------------------------------------------------
class OS_API osRawMemoryStream : public osChannel
{
public:
    osRawMemoryStream(size_t initialBufferSize = 256, bool safeAccessEnabled = false);
    virtual ~osRawMemoryStream();

    // Overrides osChannel:
    virtual osChannelType channelType() const;

    // Self functions:
    void clear();

    size_t currentReadPosition() const { return _currentReadPosition; };
    void seekReadPosition(size_t newReadPosition) { _currentReadPosition = newReadPosition; };

    size_t currentWritePosition() const { return _currentWritePosition; };
    void seekWritePosition(size_t newWritePosition) { _currentWritePosition = newWritePosition; };

    gtByte* getRawBufferPointer() const { return _pRawMemoryBuffer; };

    void registerAllocationFailureObserver(gtIAllocationFailureObserver* pIAllocationFailureObserver);

protected:
    virtual bool writeImpl(const gtByte* pDataBuffer, gtSize_t dataSize);
    virtual bool readImpl(gtByte* pDataBuffer, gtSize_t dataSize);
    virtual bool readAvailableDataImpl(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead);

private:
    bool resizeBuffer(size_t newBufferSize);

private:
    // The raw memory vector:
    gtByte* _pRawMemoryBuffer;

    // The size of the raw memory vector:
    size_t _rawMemoryBufferSize;

    // The position which we write to:
    size_t _currentWritePosition;

    // The position which we read from:
    size_t _currentReadPosition;

    // An observer to which we notify when this class fails to allocate the
    // memory needed for its operation:
    gtIAllocationFailureObserver* _pIAllocationFailureObserver;

    // Allows this to be thread-safe:
    osCriticalSection _writeAccessCS;
    bool _safeAccessEnabled;
};


#endif  // __OSRAWMEMORYSTREAM
