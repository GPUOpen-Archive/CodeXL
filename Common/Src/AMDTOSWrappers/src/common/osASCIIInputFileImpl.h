//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osASCIIInputFileImpl.h
///
//=====================================================================

//------------------------------ osASCIIInputFileImpl.h ------------------------------

#ifndef __OSASCIIINPUTFILEIMPL
#define __OSASCIIINPUTFILEIMPL

// C++:
#include <fstream>

// Local:
#include <common/osFileImpl.h>


// ----------------------------------------------------------------------------------
// Class Name:           osASCIIInputFileImpl : public osFileImpl
// General Description: Input file implementation.
// Author:      AMD Developer Tools Team
// Creation Date:        15/5/2004
// ----------------------------------------------------------------------------------
class osASCIIInputFileImpl : public osFileImpl
{
public:
    osASCIIInputFileImpl();
    virtual ~osASCIIInputFileImpl();

    bool open(const osFilePath& path, osChannel::osChannelType fileType);

    // Overrides osFileImpl:
    virtual void close();
    virtual void flush();
    virtual bool isOK() const;
    virtual bool write(const gtByte*, gtSize_t);
    virtual bool read(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead);
    virtual bool readLine(gtString&);
    virtual bool readLine(gtASCIIString& line);
    virtual bool readIntoString(gtString&);
    virtual bool isOpened() const;
    virtual bool seekCurrentPosition(osStream::osStreamPosition seekStartPosition, gtSize_t offset);
    virtual bool currentPosition(osStream::osStreamPosition positionReference, gtSize_t& offset) const;

private:
    // The output file stream:
    std::ifstream _inputFileStream;
};


#endif  // __OSASCIIINPUTFILEIMPL
