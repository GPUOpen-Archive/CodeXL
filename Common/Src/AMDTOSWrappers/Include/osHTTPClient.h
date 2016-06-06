//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osHTTPClient.h
///
//=====================================================================

//------------------------------ osHTTPClient.h ------------------------------

#ifndef __OSHTTPCLIENT_H
#define __OSHTTPCLIENT_H

#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osTCPSocketClient.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>

// ----------------------------------------------------------------------------------
// Class Name:           OS_API osHTTPClient
// General Description: Provides connection by HTTP
// Author:      AMD Developer Tools Team
// Creation Date:        14/4/2008
// ----------------------------------------------------------------------------------
class OS_API osHTTPClient
{
public:
    osHTTPClient();
    osHTTPClient(const osPortAddress& portAddress);
    ~osHTTPClient();

    void setServerAndPort(const osPortAddress& portAddress) {_httpServerAddress = portAddress;};
    const osPortAddress& getServerAndPort() {return _httpServerAddress;};
    bool connect();
    bool disconnect();
    void clearGETRequestArguments();
    void addGETRequestArgument(const gtASCIIString& argumentName,  const gtASCIIString& argumentValue);
    bool requestPage(const gtASCIIString& pageRelativeURL, gtASCIIString& outputPage, bool isUsingProxy, const gtASCIIString& serverAddress = "", bool isGraphicsServer = false);
    gtASCIIString getLastErrorCode() { return _errorCode; };

    // Handling POST requests:
    void setPostBuffer(const gtASCIIString& postBuffer) {_postBuffer = postBuffer;};
    const gtASCIIString& getPostBuffer() {return _postBuffer;};
    bool RequestPagePost(const gtASCIIString& pageRelativeURL, gtASCIIString& outputPage, bool isUsingProxy, const gtASCIIString& serverAddress = "");
    bool ReadTCPSocketIntoPage(gtASCIIString& outputPage, unsigned int uBufferSize = _READ_BUFFER_SIZE);

    bool ReadTCPSocketIntoBuffer(unsigned char*& pReturnData, unsigned long& dataBufferSize, unsigned int uBufferSize = _READ_BUFFER_SIZE);

    bool TrimHTTPResult(unsigned char*& pBuffer, unsigned long& dataBufferSize, bool packetLoss);

    /// Request for a page post with binary data
    bool RequestPageWithBinaryData(const gtASCIIString& pageRelativeURL, unsigned char*& pReturnData, unsigned long& dataBufferSize, bool isUsingProxy, const gtASCIIString& serverAddress = "", bool isGraphicsServer = false);

    osTCPSocketClient& GetTCPSocket() { return _tcpClient; }

private:
    void encodeGETRequestString(const gtASCIIString& requestURLString, gtASCIIString& encodedString);

private:
    // A TCP IP client, used to perform the HTTP request:
    osTCPSocketClient _tcpClient;

    // Contains the HTTP server address:
    osPortAddress _httpServerAddress;

    // POST request data buffer:
    gtASCIIString _postBuffer;

    // GET request arguments and a URL encoded string:
    gtASCIIString _GETRequestArgumentsString;

    // Contains the HTTP request result buffer:
    gtASCIIString _httpResultBuffer;

    // Contains the error code, returned from the HTTP server:
    gtASCIIString _errorCode;

    // default read buffer size for TCP read
    static const unsigned int _READ_BUFFER_SIZE = 1024;
    static const unsigned int _READ_BUFFER_SIZE_GRAPHICS_SERVER = 131072;
};

#endif //__OSHTTPCLIENT_H

