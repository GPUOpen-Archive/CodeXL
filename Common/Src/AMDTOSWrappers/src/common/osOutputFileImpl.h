//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osOutputFileImpl.h
///
//=====================================================================

//------------------------------ osOutputFileImpl.h ------------------------------

#ifndef __OSOUTPUTFILEIMPL
#define __OSOUTPUTFILEIMPL

// C++:
#include <fstream>

// Local:
#include <common/osFileImpl.h>


// ----------------------------------------------------------------------------------
// Class Name:           osOutputFileImpl : public osFileImpl
// General Description: Output file implementation.
// Author:      AMD Developer Tools Team
// Creation Date:        15/5/2004
// ----------------------------------------------------------------------------------
class osOutputFileImpl : public osFileImpl
{
public:
    osOutputFileImpl();
    virtual ~osOutputFileImpl();

    bool open(const osFilePath& path, osChannel::osChannelType fileType = osChannel::OS_UNICODE_TEXT_CHANNEL,
              osFile::osOpenMode openMode = osFile::OS_OPEN_TO_WRITE);

    // Overrides osFileImpl:
    virtual void close();
    virtual void flush();
    virtual bool isOK() const;
    virtual bool write(const gtByte* pDataBuffer, gtSize_t dataSize);
    virtual bool read(gtByte*, gtSize_t, gtSize_t&);
    virtual bool readLine(gtString&);
    virtual bool readLine(gtASCIIString&);
    virtual bool readIntoString(gtString&);
    virtual bool isOpened() const;
    virtual bool seekCurrentPosition(osStream::osStreamPosition seekStartPosition, gtSize_t offset);
    virtual bool currentPosition(osStream::osStreamPosition positionReference, gtSize_t& offset) const;

private:
    // The output file stream:
    std::ofstream _outputFileStream;
};

#endif  // __OSOUTPUTFILEIMPL
