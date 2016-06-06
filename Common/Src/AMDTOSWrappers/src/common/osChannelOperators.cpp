//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osChannelOperators.cpp
///
//=====================================================================

//------------------------------ osChannelOperators.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osTime.h>

// Local:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osRawMemoryStream.h>

// Data types sizes:
static unsigned long statBoolSize = sizeof(bool);
static unsigned long statCharSize = sizeof(char);
static unsigned long statShortSize = sizeof(short);
// static unsigned long statWChartSize = sizeof(wchar_t);
static unsigned long statIntSize = sizeof(int);
static unsigned long statLongSize = sizeof(long);
static unsigned long statLongLongSize = sizeof(long long);
static unsigned long statUnsignedLongLongSize = sizeof(unsigned long long);
static unsigned long statFloatSize = sizeof(float);
static unsigned long statDoubleSize = sizeof(double);
static unsigned long statTimeSize = sizeof(unsigned long long);
// static unsigned long statPtrSize = sizeof(void*);


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a signed char value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, signed char c)
{
    const gtByte* pDataBuffer = (const gtByte*)&c;
    bool rc = ipcChannel.write(pDataBuffer, statCharSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes an unsigned char value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, unsigned char uc)
{
    const gtByte* pDataBuffer = (const gtByte*)&uc;
    bool rc = ipcChannel.write(pDataBuffer, statCharSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a short value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, short s)
{
    const gtByte* pDataBuffer = (const gtByte*)&s;
    bool rc = ipcChannel.write(pDataBuffer, statShortSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes an unsigned short value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, unsigned short us)
{
    const gtByte* pDataBuffer = (const gtByte*)&us;
    bool rc = ipcChannel.write(pDataBuffer, statShortSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a bool value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, bool b)
{
    const gtByte* pDataBuffer = (const gtByte*)&b;
    bool rc = ipcChannel.write(pDataBuffer, statBoolSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes an int value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, int i)
{
    const gtByte* pDataBuffer = (const gtByte*)&i;
    bool rc = ipcChannel.write(pDataBuffer, statIntSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes an unsigned int value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, unsigned int ui)
{
    const gtByte* pDataBuffer = (const gtByte*)&ui;
    bool rc = ipcChannel.write(pDataBuffer, statIntSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a long value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, long l)
{
    const gtByte* pDataBuffer = (const gtByte*)&l;
    bool rc = ipcChannel.write(pDataBuffer, statLongSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a long long value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        26/12/2007
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, long long ll)
{
    const gtByte* pDataBuffer = (const gtByte*)&ll;
    bool rc = ipcChannel.write(pDataBuffer, statLongLongSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes an unsigned long long value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        29/04/2008
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, unsigned long long ll)
{
    const gtByte* pDataBuffer = (const gtByte*)&ll;
    bool rc = ipcChannel.write(pDataBuffer, statUnsignedLongLongSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes an unsigned long value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, unsigned long ul)
{
    const gtByte* pDataBuffer = (const gtByte*)&ul;
    bool rc = ipcChannel.write(pDataBuffer, statLongSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a float value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, float f)
{
    const gtByte* pDataBuffer = (const gtByte*)&f;
    bool rc = ipcChannel.write(pDataBuffer, statFloatSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a double value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, double d)
{
    const gtByte* pDataBuffer = (const gtByte*)&d;
    bool rc = ipcChannel.write(pDataBuffer, statDoubleSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a string (given as NULL terminated char array) into a channel.
// Author:      AMD Developer Tools Team
// Date:        1/12/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, const wchar_t* pString)
{
    gtString tmpString = pString;
    return operator<<(ipcChannel, tmpString);
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a gtString value into a channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, const gtString& str)
{
    // Write the string to the channel:
    bool rc = ipcChannel.writeString(str);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a gtString value into a channel - ASCII version
// Author:      AMD Developer Tools Team
// Date:        13/9/2010
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, const gtASCIIString& str)
{
    // Write the string to the channel:
    bool rc = ipcChannel.writeString(str);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Sends a time value into a channel
// Author:      AMD Developer Tools Team
// Date:        20/8/2006
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, const osTime& timeToBeSent)
{
    // Get the time as the elapsed seconds from 1/1/1970:
    time_t secondsFrom1970 = timeToBeSent.secondsFrom1970();
    unsigned long long secondsFrom1970AsULL = (unsigned long long)secondsFrom1970;

    // Write this number into the socket as unsigned long long:
    bool rc = ipcChannel.write((gtByte*)&secondsFrom1970AsULL, statTimeSize);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a raw memory stream content into a channel.
// Arguments: ipcChannel - The channel into which the raw memory stream content will be written.
//            rawMemoryStream - The raw memory stream.
// Return Val:  osChannel - The channel into which raw memory stream content was written.
// Author:      AMD Developer Tools Team
// Date:        24/1/2008
// ---------------------------------------------------------------------------
OS_API osChannel& operator<<(osChannel& ipcChannel, osRawMemoryStream& rawMemoryStream)
{
    osChannel& retVal = ipcChannel;

    // Calculate the amount of data to be written:
    size_t currentWritePosition = rawMemoryStream.currentWritePosition();
    size_t currentReadPosition = rawMemoryStream.currentReadPosition();
    size_t dataToBeWrittenSize = currentWritePosition - currentReadPosition;
    GT_IF_WITH_ASSERT(0 < dataToBeWrittenSize)
    {
        // Write my content into the input channel:
        gtByte* pRawMemoryBuffer = rawMemoryStream.getRawBufferPointer();
        gtByte* pCurrReadPosition = pRawMemoryBuffer + currentReadPosition;
        bool rc1 = ipcChannel.write(pCurrReadPosition, dataToBeWrittenSize);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Mark the the raw memory stream is now empty:
            rawMemoryStream.clear();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads a signed char value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, signed char& c)
{
    gtByte* pDataBuffer = (gtByte*)&c;
    bool rc = ipcChannel.read(pDataBuffer, statCharSize);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads an unsigned char value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, unsigned char& uc)
{
    gtByte* pDataBuffer = (gtByte*)&uc;

    bool rc = ipcChannel.read(pDataBuffer, statCharSize);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads a short value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, short& s)
{
    gtByte* pDataBuffer = (gtByte*)&s;
    bool rc = ipcChannel.read(pDataBuffer, statShortSize);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads an unsigned short value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, unsigned short& us)
{
    gtByte* pDataBuffer = (gtByte*)&us;
    bool rc = ipcChannel.read(pDataBuffer, statShortSize);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads a bool value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, bool& b)
{
    gtByte* pDataBuffer = (gtByte*)&b;
    bool rc = ipcChannel.read(pDataBuffer, statBoolSize);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads an int value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, int& i)
{
    gtByte* pDataBuffer = (gtByte*)&i;
    bool rc = ipcChannel.read(pDataBuffer, statIntSize);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads an unsigned int value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, unsigned int& ui)
{
    gtByte* pDataBuffer = (gtByte*)&ui;
    bool rc = ipcChannel.read(pDataBuffer, statIntSize);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads a long value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, long& l)
{
    gtByte* pDataBuffer = (gtByte*)&l;
    bool rc = ipcChannel.read(pDataBuffer, statLongSize);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads an unsigned long value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, unsigned long& ul)
{
    gtByte* pDataBuffer = (gtByte*)&ul;
    bool rc = ipcChannel.read(pDataBuffer, statLongSize);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads a long long value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        26/12/2007
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, long long& ll)
{
    gtByte* pDataBuffer = (gtByte*)&ll;
    bool rc = ipcChannel.read(pDataBuffer, statLongLongSize);
    GT_ASSERT(rc);

    return ipcChannel;
}

// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads an unsigned long long value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        29/04/2008
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, unsigned long long& ll)
{
    gtByte* pDataBuffer = (gtByte*)&ll;
    bool rc = ipcChannel.read(pDataBuffer, statUnsignedLongLongSize);
    GT_ASSERT(rc);

    return ipcChannel;
}

// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads a float value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, float& f)
{
    gtByte* pDataBuffer = (gtByte*)&f;
    bool rc = ipcChannel.read(pDataBuffer, statFloatSize);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads a double value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, double& d)
{
    gtByte* pDataBuffer = (gtByte*)&d;
    bool rc = ipcChannel.read(pDataBuffer, statDoubleSize);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads a string value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        17/3/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, gtString& str)
{
    // Read the string from channel:
    bool rc = ipcChannel.readString(str);
    GT_ASSERT(rc);

    return ipcChannel;
}

// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads a string value from an IPC channel - ASCII version
// Author:      AMD Developer Tools Team
// Date:        13/9/2010
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, gtASCIIString& str)
{
    // Read the string from channel:
    bool rc = ipcChannel.readString(str);
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads a time value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        20/8/2006
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, osTime& timeToBeRecieved)
{
    // Read the time as a unsigned long long value:
    unsigned long long recievedTimeAsULL = 0;

    bool rc1 = ipcChannel.read((gtByte*)&recievedTimeAsULL, statTimeSize);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Output the read time:
        time_t recievedTime = (time_t)recievedTimeAsULL;
        timeToBeRecieved.setTime(recievedTime);
    }

    return ipcChannel;
}



