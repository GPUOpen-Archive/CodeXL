//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osHTTPClient.cpp
///
//=====================================================================

//------------------------------ osHTTPClient.cpp ------------------------------

//std
#include <memory>

// This should probably move to osOSDefinitions.h:88
#define NOMINMAX 1

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osHTTPClient.h>


// ---------------------------------------------------------------------------
// Name:        osHTTPClient::osHTTPClient
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        16/4/2008
// ---------------------------------------------------------------------------
osHTTPClient::osHTTPClient()
{

}

// ---------------------------------------------------------------------------
// Name:        osHTTPClient::osHTTPClient
// Description: Constructor, setting portAddress as the server address.
// Author:      AMD Developer Tools Team
// Date:        16/4/2008
// ---------------------------------------------------------------------------
osHTTPClient::osHTTPClient(const osPortAddress& portAddress):
    _httpServerAddress(portAddress)
{

}


// ---------------------------------------------------------------------------
// Name:        osHTTPClient::~osHTTPClient
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        16/4/2008
// ---------------------------------------------------------------------------
osHTTPClient::~osHTTPClient()
{
    if (_tcpClient.isOpen())
    {
        disconnect();
    }
}

// ---------------------------------------------------------------------------
// Name:        osHTTPClient::connect
// Description: Connects to the server specified in _httpServerAddress.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        16/4/2008
// ---------------------------------------------------------------------------
bool osHTTPClient::connect()
{
    bool retVal = false;
    bool disconnectionSuccessful = true;

    if (_tcpClient.isOpen())
    {
        // we want to d/c and reconnect since the user may have changed the server since last connecting.
        disconnectionSuccessful = disconnect();
    }

    if (!disconnectionSuccessful)
    {
        _errorCode = OS_STR_cannotEndTCPSession;
    }
    else
    {
        bool tcpSocketOpen = _tcpClient.open();

        if (!tcpSocketOpen)
        {
            _errorCode = OS_STR_cannotOpenTCPSocket;
        }
        else
        {
            bool tcpConnected = _tcpClient.connect(_httpServerAddress);

            if (tcpConnected)
            {
                retVal = true;
            }
            else
            {
                _errorCode = OS_STR_cannotConnectToTCPServer1;
                _errorCode.append(_httpServerAddress.hostName().asASCIICharArray());
                _errorCode.appendFormattedString(OS_STR_cannotConnectToTCPServer2, _httpServerAddress.portNumber());
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osHTTPClient::disconnect
// Description: Disconnects current session.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        16/4/2008
// ---------------------------------------------------------------------------
bool osHTTPClient::disconnect()
{
    bool retVal = false;

    if (!_tcpClient.isOpen())
    {
        retVal = true;
    }
    else
    {
        retVal = _tcpClient.close();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osHTTPClient::clearGETRequestArguments
// Description: Clears the GET request arguments data buffer.
// Author:      AMD Developer Tools Team
// Date:        27/5/2008
// ---------------------------------------------------------------------------
void osHTTPClient::clearGETRequestArguments()
{
    _GETRequestArgumentsString.makeEmpty();
}


// ---------------------------------------------------------------------------
// Name:        osHTTPClient::addGETRequestArgument
// Description: Adds a GET request argument and its value to the GET request arguments
//              data buffer.
// Arguments: argumentName - The POST argument to be added.
//            argumentValue - The POST argument value.
// Author:      AMD Developer Tools Team
// Date:        27/5/2008
// ---------------------------------------------------------------------------
void osHTTPClient::addGETRequestArgument(const gtASCIIString& argumentName, const gtASCIIString& argumentValue)
{
    // If we already have get arguments - add a "&" separator:
    bool isGETRequestArgumentsStringEmpty = (_GETRequestArgumentsString.length() == 0);

    if (!isGETRequestArgumentsStringEmpty)
    {
        _GETRequestArgumentsString += "&";
    }

    // Add the argument name:
    _GETRequestArgumentsString += argumentName;

    // Add the name value separator:
    _GETRequestArgumentsString += "=";

    // Add an encoded argument value:
    gtASCIIString encodedValue;
    encodeGETRequestString(argumentValue, encodedValue);
    _GETRequestArgumentsString += encodedValue;
}


// ---------------------------------------------------------------------------
// Name:        osHTTPClient::requestPage
// Description: Requests a page using the GET method. Note that the post buffer
//              is not used here
// Arguments: pageRelativeURL - the relative URL of the page. Can include input
//              variables as /pagename.ext?var1=value1&var2=value2...
//            outputPage - the page content will go here
//            isUsingProxy - is the webserver we want to get the information from different
//              than the one we are connecting through
//            serverAddress - if isUsingProxy is true, this is the DNS (www.foo.com) or
//              IP (191.14.8.86) of the server we will request the pages from.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        16/4/2008
// ---------------------------------------------------------------------------
bool osHTTPClient::requestPage(const gtASCIIString& pageRelativeURL, gtASCIIString& outputPage, bool isUsingProxy, const gtASCIIString& serverAddress, bool isGraphicsServer)
{
    bool retVal = false;
    outputPage = "";
    bool isConnected = true;
    _errorCode.makeEmpty();

    if (!_tcpClient.isOpen()) // If we lost the connection, try to reconnect:
    {
        isConnected = connect();
    }

    if (isConnected)
    {
        // Build the GET request string:
        gtASCIIString requestString = "GET ";

        if (isUsingProxy)
        {
            if (!serverAddress.isEmpty())
            {
                // Add the server address to the request:
                requestString.append("http://");
                requestString.append(serverAddress);
            }
        }

        requestString.append(pageRelativeURL);

        // If the user set GET arguments - add them to the GET request string:
        if (0 < _GETRequestArgumentsString.length())
        {
            requestString += "?";
            requestString += _GETRequestArgumentsString;
        }

        requestString.append(" HTTP/1.0\r\n");
        gtASCIIString header1 = "Host: ";

        if ((!isUsingProxy) || serverAddress.isEmpty())
        {
            header1.append(_httpServerAddress.hostName().asASCIICharArray());
        }
        else
        {
            header1.append(serverAddress);
        }

        if (isGraphicsServer)
        {
            // clear the buffer from previous request
            _httpResultBuffer.makeEmpty();
            header1.append("Accept: text/xml;q=0.1");
        }

        header1.append("\r\n");
        gtASCIIString header2 = "User-Agent: amdAgent\r\n\r\n";


        requestString.append(header1);
        requestString.append(header2);

        int stringLength = requestString.length();
        bool rc10 = _tcpClient.write((gtByte*)requestString.asCharArray(), stringLength);

        if (rc10)
        {
            if (!isGraphicsServer)
            {
                retVal = ReadTCPSocketIntoPage(outputPage);
            }
            else
            {
                retVal = ReadTCPSocketIntoPage(outputPage, _READ_BUFFER_SIZE_GRAPHICS_SERVER);
            }
        }
        else
        {
            _errorCode = OS_STR_requestError;
        }

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osHTTPClient::requestPagePost
// Description: Requests a page using the POST method. Make sure you set the
//              post buffer before doing this.
// Arguments: pageRelativeURL - the relative URL to be called.
//            outputPage - the page content will go here
//            isUsingProxy - is the webserver we want to get the information from different
//              than the one we are connecting through
//            serverAddress - if isUsingProxy is true, this is the DNS (www.foo.com) or
//              IP (191.14.8.86) of the server we will request the pages from.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        16/4/2008
// ---------------------------------------------------------------------------
bool osHTTPClient::RequestPagePost(const gtASCIIString& pageRelativeURL, gtASCIIString& outputPage, bool isUsingProxy, const gtASCIIString& serverAddress)
{
    bool retVal = false;
    outputPage = "";
    bool isConnected = true;

    if (!_tcpClient.isOpen()) // If we lost the connection, try to reconnect:
    {
        isConnected = connect();
    }

    if (isConnected)
    {
        // Build the POST request string:
        unsigned long postBufferLength = _postBuffer.length();
        gtASCIIString requestString = "POST ";

        if (isUsingProxy)
        {
            if (!serverAddress.isEmpty())
            {
                // Add the server address to the request:
                requestString.append("http://");
                requestString.append(serverAddress);
            }
        }       requestString.append(pageRelativeURL);

        requestString.append(" HTTP/1.0\r\n");
        gtASCIIString header1 = "Host: ";

        if ((!isUsingProxy) || serverAddress.isEmpty())
        {
            header1.append(_httpServerAddress.hostName().asASCIICharArray());
        }
        else
        {
            header1.append(serverAddress);
        }

        header1.append("\r\n");
        gtASCIIString header2 = "User-Agent: amdAgent\r\n";
        gtASCIIString header3 = "Content-Type: application/x-www-form-urlencoded\r\n";
        gtASCIIString header4 = "Content-Length: ";
        header4.appendFormattedString("%lu\r\n", postBufferLength);

        int stringLength = requestString.length();
        bool rc10 = _tcpClient.write((gtByte*)requestString.asCharArray(), stringLength);
        stringLength = header1.length();
        bool rc11 = _tcpClient.write((gtByte*)header1.asCharArray(), stringLength);
        stringLength = header2.length();
        bool rc12 = _tcpClient.write((gtByte*)header2.asCharArray(), stringLength);
        stringLength = header3.length();
        bool rc13 = _tcpClient.write((gtByte*)header3.asCharArray(), stringLength);
        stringLength = header4.length();
        bool rc14 = _tcpClient.write((gtByte*)header4.asCharArray(), stringLength);
        bool rc15 = _tcpClient.write((gtByte*)"\r\n", 2);
        bool rc16 = _tcpClient.write((gtByte*)_postBuffer.asCharArray(), postBufferLength);

        if (rc10 && rc11 && rc12 && rc13 && rc14 && rc15 && rc16)
        {
            retVal = ReadTCPSocketIntoPage(outputPage);
        }
        else
        {
            _errorCode = OS_STR_requestError;
        }

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osHTTPClient::readTCPSocketIntoPage
// Description: Reads the page result into an output string. If the page is an
//              error page (eg HTTP response 404) it is logged into _errorCode
// Arguments: outputPage - the page content goes here.
//            uBufferSize - specifies buffer size
//
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        16/4/2008
// ---------------------------------------------------------------------------
bool osHTTPClient::ReadTCPSocketIntoPage(gtASCIIString& outputPage, unsigned int uBufferSize)
{
    bool retVal = false;



    gtSize_t amountOfDataRead = 0;
    bool moreToRead = true;
    bool lastBufferEmpty = false;
    bool packetLoss = false;
    gtSize_t totalDataRead = 0;

    if (0 == uBufferSize)
    {
        // default
        uBufferSize = _READ_BUFFER_SIZE;
    }

    {
        std::unique_ptr<gtByte[]> buff(new gtByte[uBufferSize + 1]());

        while (moreToRead)
        {
            memset(buff.get(), '\0', uBufferSize + 1);

            // Try to read from the TCP socket
            bool rc2 = _tcpClient.readAvailableData(buff.get(), uBufferSize, amountOfDataRead);

            if (!rc2)
            {
                if (!lastBufferEmpty)
                {
                    // We tried to read, but there is no data available:
                    lastBufferEmpty = true;
                }
                else
                {
                    // We already waited for data, but didn't get it after the sleep period:
                    moreToRead = false;
                    packetLoss = true;
                }
            }
            else
            {
                // We did manage to read from the socket:
                lastBufferEmpty = false;

                if (amountOfDataRead == 0)
                {
                    // The socket was closed
                    moreToRead = false;
                }
                else
                {
                    // We got data:
                    buff[amountOfDataRead] = 0;
                    totalDataRead += amountOfDataRead;
                    gtASCIIString interimTextBuffer = buff.get();
                    _httpResultBuffer.append(interimTextBuffer);
                }
            }
        }
    }

    if (_httpResultBuffer.find("HTTP/") == -1)
    {
        _errorCode = OS_STR_serverRespondError;
    }
    else
    {
        int firstSpaceLocation = _httpResultBuffer.find(' ');
        char responseCode = _httpResultBuffer.asCharArray()[firstSpaceLocation + 1];

        if (packetLoss)
        {
            responseCode = 0;
        }

        switch (responseCode)
        {
            case '1': // Success / Information
            case '2': // Success
            case '3': // Redirection
            {
                int findNLNL = _httpResultBuffer.find("\n\n", firstSpaceLocation);

                if (findNLNL <= firstSpaceLocation) { findNLNL = (int)totalDataRead; }

                int findCRCR = _httpResultBuffer.find("\r\r", firstSpaceLocation);

                if (findCRCR <= firstSpaceLocation) { findCRCR = (int)totalDataRead; }

                int findCNCN = _httpResultBuffer.find("\r\n\r\n", firstSpaceLocation);

                if (findCNCN <= firstSpaceLocation) { findCNCN = (int)totalDataRead; }

                int findNCNC = _httpResultBuffer.find("\n\r\n\r", firstSpaceLocation);

                if (findNCNC <= firstSpaceLocation) { findNCNC = (int)totalDataRead; }

                int headersEndLocation = std::min(std::min(findNLNL + 2, findCRCR + 2), std::min(findCNCN + 4, findNCNC + 4));

                _httpResultBuffer.getSubString(headersEndLocation, -1, outputPage);
                retVal = true;
            }
            break;

            case '4': // Client error
            case '5': // Server error
            {
                _httpResultBuffer.getSubString(firstSpaceLocation + 1, firstSpaceLocation + 3, _errorCode);
            }
            break;

            case '\0':
            {
                int findNLNL = _httpResultBuffer.find("\n\n", firstSpaceLocation);

                if (findNLNL <= firstSpaceLocation) { findNLNL = (int)totalDataRead; }

                int findCRCR = _httpResultBuffer.find("\r\r", firstSpaceLocation);

                if (findCRCR <= firstSpaceLocation) { findCRCR = (int)totalDataRead; }

                int findCNCN = _httpResultBuffer.find("\r\n\r\n", firstSpaceLocation);

                if (findCNCN <= firstSpaceLocation) { findCNCN = (int)totalDataRead; }

                int findNCNC = _httpResultBuffer.find("\n\r\n\r", firstSpaceLocation);

                if (findNCNC <= firstSpaceLocation) { findNCNC = (int)totalDataRead; }

                int headersEndLocation = std::min(std::min(findNLNL + 2, findCRCR + 2), std::min(findCNCN + 4, findNCNC + 4));

                _httpResultBuffer.getSubString(headersEndLocation, -1, outputPage);
                _errorCode = OS_STR_packetLoss;
            }
            break;

            default:
            {
                _errorCode = OS_STR_unknownError;
            }
            break;
        }
    }

    return retVal;
}


bool osHTTPClient::ReadTCPSocketIntoBuffer(unsigned char*& pReturnData, unsigned long& dataBufferSize, unsigned int uBufferSize)
{
    bool retVal = false;

    gtSize_t amountOfDataRead = 0;
    bool moreToRead = true;
    bool lastBufferEmpty = false;
    bool packetLoss = false;

    if (0 == uBufferSize)
    {
        // default
        uBufferSize = _READ_BUFFER_SIZE;
    }

    dataBufferSize = 0;

    // Try to read from the TCP socket
    gtByte* pTempBuffer = new gtByte[uBufferSize + 1];

    while (moreToRead)
    {
        bool rc2 = _tcpClient.readAvailableData(pTempBuffer, uBufferSize + 1, amountOfDataRead);

        if (!rc2)
        {
            if (!lastBufferEmpty)
            {
                // We tried to read, but there is no data available:
                lastBufferEmpty = true;
            }
            else
            {
                // We already waited for data, but didn't get it after the sleep period:
                moreToRead = false;
                packetLoss = true;
            }
        }
        else
        {
            // We did manage to read from the socket:
            lastBufferEmpty = false;

            if (amountOfDataRead == 0)
            {
                // The socket was closed
                moreToRead = false;
            }
            else
            {
                // We got data:
                size_t memorySize = ((dataBufferSize + amountOfDataRead) * sizeof(unsigned char));
                pReturnData = (unsigned char*)(realloc(pReturnData , memorySize));
                memcpy(pReturnData + dataBufferSize, pTempBuffer, amountOfDataRead);

                dataBufferSize += (unsigned long)amountOfDataRead;
            }
        }
    }

    // Release the temporary buffer memory
    delete[] pTempBuffer;

    if (pReturnData != nullptr)
    {
        // Trim the header from the binary data
        retVal = TrimHTTPResult(pReturnData, dataBufferSize, packetLoss);
    }


    return retVal;
}


int FindHeaderEndLocation(unsigned char* pBuffer, unsigned long dataBufferSize, unsigned int startFrom)
{
    int retVal = startFrom;

    // Go over the buffer and look for the first occurance of "\n\n", "\r\r", "\r\n\r\n" or "\n\r\n\r"
    for (unsigned int i = startFrom; i < dataBufferSize; i++)
    {
        int findNCNC = strncmp((const char*)(pBuffer + i), "\n\r\n\r", 4);

        if (findNCNC == 0)
        {
            retVal = i + 4;
            break;
        }

        int findCNCN = strncmp((const char*)(pBuffer + i), "\r\n\r\n", 4);

        if (findCNCN == 0)
        {
            retVal = i + 4;
            break;
        }

        int findCRCR = strncmp((const char*)(pBuffer + i), "\r\r", 2);

        if (findCRCR == 0)
        {
            retVal = i + 2;
            break;
        }

        int findNLNL = strncmp((const char*)(pBuffer + i), "\n\n", 2);

        if (findNLNL == 0)
        {
            retVal = i + 2;
            break;
        }
    }

    return retVal;
}
int FindChar(unsigned char* pBuffer, unsigned long dataBufferSize, unsigned char ch)
{
    int retVal = 0;

    for (unsigned int i = 0; i < dataBufferSize; i++)
    {
        if (pBuffer[i] == ch)
        {
            retVal = i;
            break;
        }
    }

    return retVal;
}

bool osHTTPClient::TrimHTTPResult(unsigned char*& pBuffer, unsigned long& dataBufferSize, bool packetLoss)
{
    int headersEndLocation = 0;
    bool retVal = false;

    if (strcmp((const char*)pBuffer, "HTTP/") < 0)
    {
        _errorCode = OS_STR_serverRespondError;
    }
    else
    {
        // Find the response code (the character after the first space)
        char responseCode = 0;
        int firstSpaceLocation = FindChar(pBuffer, dataBufferSize, ' ');

        if (firstSpaceLocation >= 0)
        {
            responseCode = pBuffer[firstSpaceLocation + 1];
        }

        if (packetLoss)
        {
            responseCode = 0;
        }

        switch (responseCode)
        {
            case '1': // Success / Information
            case '2': // Success
            case '3': // Redirection
            case '\0':
            {
                headersEndLocation = FindHeaderEndLocation(pBuffer, dataBufferSize, firstSpaceLocation);

                if (responseCode == '\0')
                {
                    _errorCode = OS_STR_packetLoss;
                }

                retVal = true;
            }
            break;

            case '4': // Client error
            case '5': // Server error
            {
                _httpResultBuffer.getSubString(firstSpaceLocation + 1, firstSpaceLocation + 3, _errorCode);
            }
            break;

            default:
            {
                _errorCode = OS_STR_unknownError;
            }
            break;
        }
    }

    if (_errorCode.isEmpty())
    {
        retVal = true;

        // Remove the header section from the output buffer
        dataBufferSize -= headersEndLocation;

        // Allocate a new buffer with only the binary data
        unsigned char* pResultBuffer = new unsigned char[dataBufferSize];
        memcpy(pResultBuffer, pBuffer + headersEndLocation, dataBufferSize);

        // Release the buffer memory
        free(pBuffer);
        pBuffer = pResultBuffer;
    }

    return retVal;
}

bool osHTTPClient::RequestPageWithBinaryData(const gtASCIIString& pageRelativeURL, unsigned char*& pReturnData, unsigned long& dataBufferSize, bool isUsingProxy, const gtASCIIString& serverAddress, bool isGraphicsServer)
{
    bool retVal = false;
    pReturnData = nullptr;
    dataBufferSize = 0;
    bool isConnected = true;
    _errorCode.makeEmpty();

    if (!_tcpClient.isOpen()) // If we lost the connection, try to reconnect:
    {
        isConnected = connect();
    }

    if (isConnected)
    {
        // Build the GET request string:
        gtASCIIString requestString = "GET ";

        if (isUsingProxy)
        {
            if (!serverAddress.isEmpty())
            {
                // Add the server address to the request:
                requestString.append("http://");
                requestString.append(serverAddress);
            }
        }

        requestString.append(pageRelativeURL);

        // If the user set GET arguments - add them to the GET request string:
        if (0 < _GETRequestArgumentsString.length())
        {
            requestString += "?";
            requestString += _GETRequestArgumentsString;
        }

        requestString.append(" HTTP/1.0\r\n");
        gtASCIIString header1 = "Host: ";

        if ((!isUsingProxy) || serverAddress.isEmpty())
        {
            header1.append(_httpServerAddress.hostName().asASCIICharArray());
        }
        else
        {
            header1.append(serverAddress);
        }

        if (isGraphicsServer)
        {
            // clear the buffer from previous request
            _httpResultBuffer.makeEmpty();
            header1.append("Accept: text/xml;q=0.1");
        }

        header1.append("\r\n");
        gtASCIIString header2 = "User-Agent: amdAgent\r\n";

        int stringLength = requestString.length();
        bool rc10 = _tcpClient.write((gtByte*)requestString.asCharArray(), stringLength);
        stringLength = header1.length();
        bool rc11 = _tcpClient.write((gtByte*)header1.asCharArray(), stringLength);
        stringLength = header2.length();
        bool rc12 = _tcpClient.write((gtByte*)header2.asCharArray(), stringLength);
        bool rc13 = _tcpClient.write((gtByte*)"\r\n", 2);

        if (rc10 && rc11 && rc12 && rc13)
        {
            if (!isGraphicsServer)
            {
                retVal = ReadTCPSocketIntoBuffer(pReturnData, dataBufferSize);
            }
            else
            {
                retVal = ReadTCPSocketIntoBuffer(pReturnData, dataBufferSize, _READ_BUFFER_SIZE_GRAPHICS_SERVER);
            }
        }
        else
        {
            _errorCode = OS_STR_requestError;
        }

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osHTTPClient::encodeGETRequestString
// Description: Encodes a given GET request string to an HTTP application/x-www-form-urlencoded
//              string.
// Arguments: requestURLString - The input GET request string.
//            encodedString - The output encoded string.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        27/5/2008
// Implementation notes:
//   To get an HTTP application/x-www-form-urlencoded from a given string, we need to
//   replace some ASCII characters with their hexadecimal representation (%Hex) according
//   to the below table:
//      SPACE    %20
//      "   %22
//      #   %23
//      $   %24
//      %   %25
//      &   %26
//      +   %2B
//      ,   %2C
//      /   %2F
//      :   %3A
//      ;   %3B
//      <   %3C
//      =   %3D
//      >   %3E
//      ?   %3F
//      @   %40
//      [   %5B
//      \   %5C
//      ]   %5D
//      ^   %5E
//      `   %60
//      {   %7B
//      |   %7C
//      }   %7D
//      ~   %7E
//
// For more details see:
// - http://www.aptana.com/docs/index.php/URL_Escape_Codes
// - http://www.blooberry.com/indexdot/html/topics/urlencoding.htm
// ---------------------------------------------------------------------------
void osHTTPClient::encodeGETRequestString(const gtASCIIString& requestURLString, gtASCIIString& encodedString)
{
    encodedString.makeEmpty();

    // An aid buffer, holding hexadecimal representation of chars:
    char hexNumbersBuff[4];
    hexNumbersBuff[0] = '%';
    hexNumbersBuff[3] = 0;

    // Iterate the input request string characters:
    int requestStringLength = requestURLString.length();

    for (int i = 0; i < requestStringLength; i++)
    {
        // If the current character needs to be decoded:
        char currentChar = requestURLString[i];

        if ((currentChar == ' ') || (currentChar == '#') || (currentChar == '$') || (currentChar == '%') || (currentChar == '&') ||
            (currentChar == '+') || (currentChar == ',') || (currentChar == '/') || (currentChar == ':') || (currentChar == ';') ||
            (currentChar == '<') || (currentChar == '=') || (currentChar == '>') || (currentChar == '?') || (currentChar == '@') ||
            (currentChar == '[') || (currentChar == '\\') || (currentChar == ']') || (currentChar == '^') || (currentChar == '`') ||
            (currentChar == '{') || (currentChar == '|') || (currentChar == '}') || (currentChar == '~'))
        {
            // Calculate an hexadecimal representation that will replace the current character:
            // TO_DO: Unicode code review
            unsigned short charAsUnsignedShort = currentChar;
            gtUByte firstDigit = (gtUByte)(charAsUnsignedShort / 16);
            gtUByte secondDigit = (gtUByte)(charAsUnsignedShort % 16);

            char firstDigitAsChar = 0;

            if (firstDigit < 10)
            {
                firstDigitAsChar = firstDigit + '0';
            }
            else
            {
                firstDigitAsChar = firstDigit - 10 + 'A';
            }

            char secondDigitAsChar = 0;

            if (secondDigit < 10)
            {
                secondDigitAsChar = secondDigit + '0';
            }
            else
            {
                secondDigitAsChar = secondDigit - 10 + 'A';
            }

            hexNumbersBuff[1] = firstDigitAsChar;
            hexNumbersBuff[2] = secondDigitAsChar;

            // Add the hexadecimal representation instead of the character to the encoded string:
            encodedString += hexNumbersBuff;
        }
        else
        {
            // Add the character to the encoded string:
            encodedString += currentChar;
        }
    }
}

