//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osInputFileImpl.h
///
//=====================================================================

//------------------------------ osInputFileImpl.h ------------------------------

#ifndef __OSINPUTFILEIMPL
#define __OSINPUTFILEIMPL

// C++:
#include <iosfwd>
#include <fstream>

// Local:
#include <common/osFileImpl.h>


// The size of the chunk that we read from the file:
#define OS_FILE_CHUNK_READ_SIZE 1024


// ----------------------------------------------------------------------------------
// Class Name:           osInputFileImpl : public osFileImpl
// General Description: Input file implementation.
// Author:      AMD Developer Tools Team
// Creation Date:        15/5/2004
// ----------------------------------------------------------------------------------
class osInputFileImpl : public osFileImpl
{
public:
    osInputFileImpl();
    virtual ~osInputFileImpl();

    bool open(const osFilePath& path, osChannel::osChannelType fileType);

    // Overrides osFileImpl:
    virtual void close();
    virtual void flush();
    virtual bool isOK() const;
    virtual bool write(const gtByte*, gtSize_t);
    virtual bool read(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead);
    virtual bool readLine(gtString& line);
    virtual bool readLine(gtASCIIString&);
    virtual bool readIntoString(gtString& str);
    virtual bool isOpened() const;
    virtual bool seekCurrentPosition(osStream::osStreamPosition seekStartPosition, gtSize_t offset);
    virtual bool currentPosition(osStream::osStreamPosition positionReference, gtSize_t& offset) const;


private:
    // The output file stream:
    FILE* _pInputFileStream;
};


#endif  // __OSINPUTFILEIMPL
