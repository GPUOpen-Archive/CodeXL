//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osChannel.cpp
///
//=====================================================================

//------------------------------ osChannel.cpp ------------------------------
//C++ standard
#include <memory>
// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osCommunicationDebugManager.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTime.h>

// Default read and write time outs (applies when the sub class does not
// override defaultReadOperationTimeOut() and defaultWriteOperationTimeOut()):
#define OS_CHANNEL_DEFAULT_READ_TIMEOUT 15000
#define OS_CHANNEL_DEFAULT_WRITE_TIMEOUT 5000

// ---------------------------------------------------------------------------
// Name:        osChannel::osChannel
// Description: Constructor.
// Author:      AMD Developer Tools Team
// Date:        24/8/2005
// ---------------------------------------------------------------------------
osChannel::osChannel() : _isExcludedFromCommunicationDebug(false)
{
    // Initialize the read and write operations timeouts to be the default
    // timeouts for the specific sub-class:
    _readOperationTimeOut = defaultReadOperationTimeOut();
    _writeOperationTimeOut = defaultWriteOperationTimeOut();

    osProcessId pid = osGetCurrentProcessId();
    osThreadId tid = osGetCurrentThreadId();

    _selfDetailsForDebug.appendFormattedString(L"PID %d, TID %d, %x, ", static_cast<unsigned int>(pid), static_cast<unsigned int>(tid), this);
}


// ---------------------------------------------------------------------------
// Name:        osChannel::~osChannel
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        31/1/2004
// ---------------------------------------------------------------------------
osChannel::~osChannel()
{
}


// ---------------------------------------------------------------------------
// Name:        osChannel::setReadOperationTimeOut
// Description: Sets this channel read operation time out.
// Arguments:   timeout - The new timeout for this channel, measured in milliseconds.
// Author:      AMD Developer Tools Team
// Date:        24/8/2005
// ---------------------------------------------------------------------------
void osChannel::setReadOperationTimeOut(long timeout)
{
    // If the user requests the default timeout for this channel type:
    if (timeout == OS_CHANNEL_DEFAULT_TIME_OUT)
    {
        _readOperationTimeOut = defaultWriteOperationTimeOut();
    }
    else
    {
        _readOperationTimeOut = timeout;
    }
}


// ---------------------------------------------------------------------------
// Name:        osChannel::setWriteOperationTimeOut
// Description: Sets this channel write operation time out.
// Arguments:   timeout - The new timeout for this channel, measured in milliseconds.
// Author:      AMD Developer Tools Team
// Date:        24/8/2005
// ---------------------------------------------------------------------------
void osChannel::setWriteOperationTimeOut(long timeout)
{
    // If the user requests the default timeout for this channel type:
    if (timeout == OS_CHANNEL_DEFAULT_TIME_OUT)
    {
        _writeOperationTimeOut = defaultWriteOperationTimeOut();
    }
    else
    {
        _writeOperationTimeOut = timeout;
    }
}


// ---------------------------------------------------------------------------
// Name:        osChannel::defaultReadOperationTimeOut
// Description: Returns the default read operation time out (measured in milliseconds).
//              Sub-classes may override this method.
// Author:      AMD Developer Tools Team
// Date:        24/8/2005
// ---------------------------------------------------------------------------
long osChannel::defaultReadOperationTimeOut() const
{
    return OS_CHANNEL_DEFAULT_READ_TIMEOUT;
}


// ---------------------------------------------------------------------------
// Name:        osChannel::defaultWriteOperationTimeOut
// Description: Returns the default write operation time out (measured in milliseconds).
//              Sub-classes may override this method.
// Author:      AMD Developer Tools Team
// Date:        24/8/2005
// ---------------------------------------------------------------------------
long osChannel::defaultWriteOperationTimeOut() const
{
    return OS_CHANNEL_DEFAULT_WRITE_TIMEOUT;
}

// ---------------------------------------------------------------------------
// Name:        osChannel::write
// Description: Wrap the writing of the buffer to the channel with debug calls that log it to a debug server
// Arguments:   gtString& str
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
bool osChannel::write(const gtByte* pDataBuffer, gtSize_t dataSize)
{
    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        beforeWrite(pDataBuffer, dataSize);
    }

    bool ret = writeImpl(pDataBuffer, dataSize);

    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        afterWrite(dataSize, ret);
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        osChannel::read
// Description: Wrap the reading of a buffer from the channel with debug calls that log it to a debug server
// Arguments:   pDataBuffer - Buffer to read the data into.
//              dataSize - The size of the data to be read.
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
bool osChannel::read(gtByte* pDataBuffer, gtSize_t dataSize)
{
    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        beforeRead(dataSize);
    }

    bool ret = readImpl(pDataBuffer, dataSize);

    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        afterRead(pDataBuffer, dataSize, ret);
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        osChannel::readAvailableData
// Description: Wrap the reading of a variable size buffer from the channel with debug calls that log it to a debug server
// Arguments:   pDataBuffer - Buffer to read the data into.
//              dataSize - The size of the data to be read.
//              amountOfDataRead - The amount of data read by this function call.
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
bool osChannel::readAvailableData(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead)
{
    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        beforeReadAvailableData(bufferSize);
    }

    bool ret = readAvailableDataImpl(pDataBuffer, bufferSize, amountOfDataRead);

    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        afterReadAvailableData(pDataBuffer, bufferSize, amountOfDataRead, ret);
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        osChannel::writeString
// Description: Wrap the writing of the string to the channel with debug calls that log it to a debug server
// Arguments:   gtString& str
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
bool osChannel::writeString(const gtString& str)
{
    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        beforeWriteString(str);
    }

    bool ret = writeStringImpl(str);

    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        afterWriteString(str, ret);
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        osChannel::writeString
// Description: Wrap the writing of the string to the channel with debug calls that log it to a debug server
// Arguments:   gtASCIIString& str
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
bool osChannel::writeString(const gtASCIIString& str)
{
    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        beforeWriteString(str);
    }

    bool ret = writeStringImpl(str);

    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        afterWriteString(str, ret);
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        osChannel::readString
// Description: Wrap the reading of a string from the channel with debug calls that log it to a debug server
// Arguments:   gtString& str
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
bool osChannel::readString(gtString& str)
{
    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        beforeReadString();
    }

    bool ret = readStringImpl(str);

    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        afterReadString(str, ret);
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        osChannel::readString
// Description: Wrap the reading of a string from the channel with debug calls that log it to a debug server
// Arguments:   gtASCIIString& str
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
bool osChannel::readString(gtASCIIString& str)
{
    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        beforeReadString();
    }

    bool ret = readStringImpl(str);

    if (!_isExcludedFromCommunicationDebug && theCommunicationDebugManager.isCommunicationDebugEnabled())
    {
        afterReadString(str, ret);
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        osChannel::writeStringImpl
// Description: Write the string to the channel
// Arguments:   gtString& str
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        7/9/2010
// ---------------------------------------------------------------------------
bool osChannel::writeStringImpl(const gtString& str)
{
    bool retVal = false;
    const char* converted_str = nullptr;
    gtInt32 stringLength = 0; 
    if (!str.isEmpty())
    {
        converted_str = str.asASCIICharArray();
        stringLength = (gtInt32)strlen(converted_str);
    }

    // Do not write the string length into text channels:
    if (channelType() == osChannel::OS_BINARY_CHANNEL)
    {
        // Write the string length:
        (*this) << stringLength;
    }

    // Write the string content:
    if (stringLength > 0)
    {
        // Write the multi byte char pointer:
        bool rc = write((gtByte*)converted_str, stringLength);
        GT_IF_WITH_ASSERT(rc)
        {
            retVal = true;
        }
    }
    else
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osChannel::writeStringImpl
// Description: Write the string to the channel - ASCII version
// Arguments:   gtASCIIString& str
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/9/2010
// ---------------------------------------------------------------------------
bool osChannel::writeStringImpl(const gtASCIIString& str)
{
    bool retVal = false;

    // Writing an ASCII string to a unicode channel is not allowed:
    GT_IF_WITH_ASSERT(channelType() != osChannel::OS_UNICODE_TEXT_CHANNEL)
    {
        // Get the string length:
        gtInt32 stringLength = str.length();

        // Do not write the string length into text channels:
        if (channelType() == osChannel::OS_BINARY_CHANNEL)
        {
            // Write the string length:
            (*this) << str.length();
        }

        // Write the string content:
        if (stringLength > 0)
        {
            const char* pString = str.asCharArray();
            bool rc = write((const gtByte*)pString, stringLength);
            GT_IF_WITH_ASSERT(rc)
            {
                retVal = true;
            }
        }
        else
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osChannel::readStringImpl
// Description: Reads the string from the channel
// Arguments:   gtString& str
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        7/9/2010
// ---------------------------------------------------------------------------
bool osChannel::readStringImpl(gtString& str)
{
    bool retVal = false;

    // We do not support reading from a text channel:
    if ((channelType() == osChannel::OS_ASCII_TEXT_CHANNEL) || (channelType() == osChannel::OS_UNICODE_TEXT_CHANNEL))
    {
        str = L"Error - operator>> is trying to read a string from a text channel !!";
        GT_ASSERT(false);
    }
    else
    {
        retVal = true;

        // Read the string length:
        gtInt32 stringLength = 0;
        (*this) >> stringLength;

        if (stringLength > 0)
        {
            // Read the string content:
            // Read the string as bytes pointer:
            // TO_DO: Unicode - use preprocessor definitions (http://www.firstobject.com/wchar_t-string-on-linux-osx-windows.htm)
            gtVector<gtByte> buf_ptr(stringLength + 1);
            gtByte* buf = &buf_ptr[0];
            bool rc = read(buf, stringLength);
            buf[stringLength] = (gtByte)0;

            if (rc)
            {
                str.fromASCIIString((const char*)buf, stringLength);
            }
            else
            {
                // An error occurred:
                GT_ASSERT(false);
                retVal = false;
            }
        }
        else
        {
            // This is an empty string:
            str.makeEmpty();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osChannel::readString
// Description: Reads the string from the channel - ASCII version
// Arguments:   gtASCIIString& str
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/9/2010
// ---------------------------------------------------------------------------
bool osChannel::readStringImpl(gtASCIIString& str)
{
    bool retVal = false;

    // We do not support reading from a text channel:
    if ((channelType() == osChannel::OS_ASCII_TEXT_CHANNEL) || (channelType() == osChannel::OS_UNICODE_TEXT_CHANNEL))
    {
        str = "Error - operator>> is trying to read a string from a text channel !!";
        GT_ASSERT(false);
    }
    else
    {
        retVal = true;

        // Read the string length:
        gtInt32 stringLength = 0;
        (*this) >> stringLength;

        if (stringLength > 0)
        {
            // Read the string content:
            int bufferSize = stringLength + 1;
            gtVector<char> stringData(bufferSize);
            char* pStringContent = &(stringData[0]);

            // Read the string as bytes pointer:
            bool rc = read((gtByte*)pStringContent, stringLength);

            // Put null at the end of the buffer:
            pStringContent[stringLength] = 0;

            if (rc)
            {
                str = ((const char*)pStringContent);
            }
            else
            {
                // An error occurred:
                GT_ASSERT(false);
                retVal = false;
            }
        }
        else
        {
            // This is an empty string:
            str.makeEmpty();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osChannel::beforeWrite
// Description: Send debug info to debug server before calling write
// Arguments:   const gtByte* pDataBuffer, gtSize_t dataSize
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
void osChannel::beforeWrite(const gtByte* pDataBuffer, gtSize_t dataSize)
{
    gtString debugMsg;
    gtString strTime;
    osTime::currentPreciseTimeAsString(strTime, osTime::DATE_TIME_DISPLAY);

    if (dataSize >= 8)
    {
        debugMsg.appendFormattedString(L"%ls, %ls, write, %d bytes, %x %x %x %x %x %x %x %x ...\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), dataSize,
                                       pDataBuffer[0], pDataBuffer[1], pDataBuffer[2], pDataBuffer[3], pDataBuffer[4], pDataBuffer[5], pDataBuffer[6], pDataBuffer[7]);
    }
    else if (dataSize >= 4)
    {
        debugMsg.appendFormattedString(L"%ls, %ls, write, %d bytes, %x %x %x %x ...\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), dataSize,
                                       pDataBuffer[0], pDataBuffer[1], pDataBuffer[2], pDataBuffer[3]);
    }
    else
    {
        debugMsg.appendFormattedString(L"%ls, %ls, write, %d bytes\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), dataSize);
    }

    theCommunicationDebugManager.push(debugMsg);
}

// ---------------------------------------------------------------------------
// Name:        osChannel::afterWrite
// Description: Send debug info to debug server after calling write
// Arguments:   const gtByte* pDataBuffer, gtSize_t dataSize
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
void osChannel::afterWrite(gtSize_t dataSize, bool opReturnValue)
{
    gtString debugMsg;
    gtString strTime;
    osTime::currentPreciseTimeAsString(strTime, osTime::DATE_TIME_DISPLAY);
    debugMsg.appendFormattedString(L"%ls, %ls, write completed, %d bytes, return value = %d\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), dataSize, (int)opReturnValue);
    theCommunicationDebugManager.push(debugMsg);
}

// ---------------------------------------------------------------------------
// Name:        osChannel::beforeWriteString
// Description: Send debug info to debug server before calling writeString
// Arguments:   const gtString& str
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
void osChannel::beforeWriteString(const gtString& str)
{
    gtString debugMsg;
    gtString strTime;
    osTime::currentPreciseTimeAsString(strTime, osTime::DATE_TIME_DISPLAY);
    debugMsg.appendFormattedString(L"%ls, %ls, writeString, string length %d, \"%ls\"\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), str.length(), str.asCharArray());
    theCommunicationDebugManager.push(debugMsg);
}

// ---------------------------------------------------------------------------
// Name:        osChannel::afterWriteString
// Description: Send debug info to debug server after calling write
// Arguments:   const gtString& str, bool opReturnValue
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
void osChannel::afterWriteString(const gtString& str, bool opReturnValue)
{
    gtString debugMsg;
    gtString strTime;
    osTime::currentPreciseTimeAsString(strTime, osTime::DATE_TIME_DISPLAY);
    debugMsg.appendFormattedString(L"%ls, %ls, writeString completed, %d chars, return value = %d\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), str.length(), (int)opReturnValue);
    theCommunicationDebugManager.push(debugMsg);
}

// ---------------------------------------------------------------------------
// Name:        osChannel::beforeWriteString
// Description: Send debug info to debug server before calling writeString
// Arguments:   const gtASCIIString& str
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
void osChannel::beforeWriteString(const gtASCIIString& str)
{
    gtString debugMsg;
    gtString strTime;
    osTime::currentPreciseTimeAsString(strTime, osTime::DATE_TIME_DISPLAY);
#if (GR_BUILD_TARGET == GR_WINDOWS_OS)
    // Use the capital 'S' type specifier supported by MSVC
    debugMsg.appendFormattedString(L"%ls, %ls, writeString, string length %d, \"%S\"\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), str.length(), str.asCharArray());
#else
    gtString strConvertedToWideChars;
    strConvertedToWideChars.fromASCIIString(str.asCharArray(), str.length());
    debugMsg.appendFormattedString(L"%ls, %ls, writeString, string length %d, \"%s\"\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), str.length(), strConvertedToWideChars.asCharArray());
#endif

    theCommunicationDebugManager.push(debugMsg);
}

// ---------------------------------------------------------------------------
// Name:        osChannel::afterWriteString
// Description: Send debug info to debug server after calling write
// Arguments:   const gtASCIIString& str, bool opReturnValue
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
void osChannel::afterWriteString(const gtASCIIString& str, bool opReturnValue)
{
    gtString debugMsg;
    gtString strTime;
    osTime::currentPreciseTimeAsString(strTime, osTime::DATE_TIME_DISPLAY);
    debugMsg.appendFormattedString(L"%ls, %ls, writeString completed, %d chars, return value = %d\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), str.length(), (int)opReturnValue);
    theCommunicationDebugManager.push(debugMsg);
}

// ---------------------------------------------------------------------------
// Name:        osChannel::beforeRead
// Description: Send debug info to debug server before calling read
// Arguments:   gtSize_t dataSize
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
void osChannel::beforeRead(gtSize_t dataSize)
{
    gtString debugMsg;
    gtString strTime;
    osTime::currentPreciseTimeAsString(strTime, osTime::DATE_TIME_DISPLAY);
    debugMsg.appendFormattedString(L"%ls, %ls, read, %d bytes\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), dataSize);

    theCommunicationDebugManager.push(debugMsg);
}

// ---------------------------------------------------------------------------
// Name:        osChannel::afterRead
// Description: Send debug info to debug server after calling read
// Arguments:   gtByte* pDataBuffer, gtSize_t dataSize, bool opReturnValue
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
void osChannel::afterRead(gtByte* pDataBuffer, gtSize_t dataSize, bool opReturnValue)
{
    gtString debugMsg;
    gtString strTime;
    osTime::currentPreciseTimeAsString(strTime, osTime::DATE_TIME_DISPLAY);

    if (dataSize >= 8)
    {
        debugMsg.appendFormattedString(L"%ls, %ls, read completed, %d bytes, return value = %d, %x %x %x %x %x %x %x %x ...\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), dataSize, (int)opReturnValue,
                                       pDataBuffer[0], pDataBuffer[1], pDataBuffer[2], pDataBuffer[3], pDataBuffer[4], pDataBuffer[5], pDataBuffer[6], pDataBuffer[7]);
    }
    else if (dataSize >= 4)
    {
        debugMsg.appendFormattedString(L"%ls, %ls, read completed, %d bytes, return value = %d, %x %x %x %x ...\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), dataSize, (int)opReturnValue,
                                       pDataBuffer[0], pDataBuffer[1], pDataBuffer[2], pDataBuffer[3]);
    }
    else
    {
        debugMsg.appendFormattedString(L"%ls, %ls, read completed, %d bytes, return value = %d\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), dataSize, (int)opReturnValue);
    }

    theCommunicationDebugManager.push(debugMsg);
}

// ---------------------------------------------------------------------------
// Name:        osChannel::beforeReadAvailableData
// Description: Send debug info to debug server before calling readAvailableData
// Arguments:   gtSize_t bufferSize
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/9/2010
// ---------------------------------------------------------------------------
void osChannel::beforeReadAvailableData(gtSize_t bufferSize)
{
    gtString debugMsg;
    gtString strTime;
    osTime::currentPreciseTimeAsString(strTime, osTime::DATE_TIME_DISPLAY);
    debugMsg.appendFormattedString(L"%ls, %ls, readAvailableData, buffer size %d bytes\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), bufferSize);

    theCommunicationDebugManager.push(debugMsg);

}

// ---------------------------------------------------------------------------
// Name:        osChannel::afterReadAvailableData
// Description: Send debug info to debug server after calling readAvailableData
// Arguments:   gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead, bool opReturnValue
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/9/2010
// ---------------------------------------------------------------------------
void osChannel::afterReadAvailableData(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead, bool opReturnValue)
{
    gtString debugMsg;
    gtString strTime;
    osTime::currentPreciseTimeAsString(strTime, osTime::DATE_TIME_DISPLAY);

    if (amountOfDataRead >= 8)
    {
        debugMsg.appendFormattedString(L"%ls, %ls, readAvailableData completed, buffer size %d bytes, %d bytes read, return value = %d, %x %x %x %x %x %x %x %x ...",
                                       strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), bufferSize, amountOfDataRead, (int)opReturnValue,
                                       pDataBuffer[0], pDataBuffer[1], pDataBuffer[2], pDataBuffer[3], pDataBuffer[4], pDataBuffer[5], pDataBuffer[6], pDataBuffer[7]);
    }
    else if (amountOfDataRead >= 4)
    {
        debugMsg.appendFormattedString(L"%ls, %ls, readAvailableData completed, buffer size %d bytes, %d bytes read, return value = %d, %x %x %x %x ...",
                                       strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), bufferSize, amountOfDataRead, (int)opReturnValue,
                                       pDataBuffer[0], pDataBuffer[1], pDataBuffer[2], pDataBuffer[3]);
    }
    else
    {
        debugMsg.appendFormattedString(L"%ls, %ls, readAvailableData completed, buffer size %d bytes, %d bytes read, return value = %d",
                                       strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), bufferSize, amountOfDataRead, (int)opReturnValue);
    }

    theCommunicationDebugManager.push(debugMsg);
}

// ---------------------------------------------------------------------------
// Name:        osChannel::beforeReadString
// Description: Send debug info to debug server before calling readString
// Arguments:   gtSize_t dataSize
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
void osChannel::beforeReadString()
{
    gtString debugMsg;
    gtString strTime;
    osTime::currentPreciseTimeAsString(strTime, osTime::DATE_TIME_DISPLAY);
    debugMsg.appendFormattedString(L"%ls, %ls, readString\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray());

    theCommunicationDebugManager.push(debugMsg);
}

// ---------------------------------------------------------------------------
// Name:        osChannel::afterReadString
// Description: Send debug info to debug server after calling readString
// Arguments:   const gtString& str, bool opReturnValue
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
void osChannel::afterReadString(const gtString& str, bool opReturnValue)
{
    gtString debugMsg;
    gtString strTime;
    osTime::currentPreciseTimeAsString(strTime, osTime::DATE_TIME_DISPLAY);
    debugMsg.appendFormattedString(L"%ls, %ls, readString completed, return value = %d, string length %d, \"%ls\"\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), (int)opReturnValue, str.length(), str.asCharArray());
    theCommunicationDebugManager.push(debugMsg);
}

// ---------------------------------------------------------------------------
// Name:        osChannel::afterReadString
// Description: Send debug info to debug server after calling readString
// Arguments:   const gtASCIIString& str, bool opReturnValue
// Return Val:  bool - Success / failure.
// Author:      Doron Ofek
// Date:        Dec-22, 2015
// ---------------------------------------------------------------------------
void osChannel::afterReadString(const gtASCIIString& str, bool opReturnValue)
{
    gtString debugMsg;
    gtString strTime;
    osTime::currentPreciseTimeAsString(strTime, osTime::DATE_TIME_DISPLAY);
#if (GR_BUILD_TARGET == GR_WINDOWS_OS)
    // Use the capital 'S' type specifier supported by MSVC
    debugMsg.appendFormattedString(L"%ls, %ls, readString completed, return value = %d, string length %d, \"%S\"\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), (int)opReturnValue, str.length(), str.asCharArray());
#else
    gtString strConvertedToWideChars;
    strConvertedToWideChars.fromASCIIString(str.asCharArray(), str.length());
    debugMsg.appendFormattedString(L"%ls, %lls, readString completed, return value = %d, string length %d, \"%s\"\n", strTime.asCharArray(), _selfDetailsForDebug.asCharArray(), (int)opReturnValue, str.length(), strConvertedToWideChars.asCharArray());
#endif

    theCommunicationDebugManager.push(debugMsg);
}
