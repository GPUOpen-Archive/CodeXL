//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFileImpl.h
///
//=====================================================================

//------------------------------ osFileImpl.h ------------------------------

#ifndef __OSFILEIMPL
#define __OSFILEIMPL

// C++:
#include <ios>

// Local:
#include <AMDTOSWrappers/Include/osFile.h>


// ----------------------------------------------------------------------------------
// Class Name:           osFileImpl
// General Description: File implementation base class.
// Author:      AMD Developer Tools Team
// Creation Date:        15/5/2004
// ----------------------------------------------------------------------------------
class osFileImpl
{
public:
    osFileImpl();
    virtual ~osFileImpl();

    // Must be implemented by sub-classes:
    virtual void close() = 0;
    virtual void flush() = 0;
    virtual bool isOK() const = 0;
    virtual bool write(const gtByte* pDataBuffer, gtSize_t dataSize) = 0;
    virtual bool read(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead) = 0;
    virtual bool readLine(gtString& line) = 0;
    virtual bool readLine(gtASCIIString& line) = 0;
    virtual bool readIntoString(gtString& str) = 0;
    virtual bool isOpened() const = 0;
    virtual bool seekCurrentPosition(osStream::osStreamPosition seekStartPosition, gtSize_t offset) = 0;
    virtual bool currentPosition(osStream::osStreamPosition positionReference, gtSize_t& offset) const = 0;

protected:
    std::ios_base::openmode fileOpenModeToIosOpenMode(osFile::osOpenMode openMode, osChannel::osChannelType fileType);
    std::ios_base::seekdir streamPositionToIosSeekDir(osFile::osStreamPosition streamPosition);
};


#endif  // __OSFILEIMPL
