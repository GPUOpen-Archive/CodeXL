//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osHTTPRequest.cpp ------------------------------

Yaki - 5 / 7 / 2007 - This class is not in use anymore. Use wxHTTP instead(See usae example at osBugReporter).

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Windows Platform SDK:
#include "include/specstrings.h"
#include "include/Winhttp.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osHTTPRequest.h>


// ---------------------------------------------------------------------------
// Name:        osHTTPRequest::osHTTPRequest
// Description: Constructor
// Arguments: webServerURL - The Web server URL
//            requestRelativeURL - The relative name of a file / service in this server.
//            httpRequestType - The HTTP request type.
// Author:      AMD Developer Tools Team
// Date:        24/11/2005
// ---------------------------------------------------------------------------
    osHTTPRequest::osHTTPRequest(const gtString& webServerURL,  const gtString& requestRelativeURL,
                                 osHTTPRequestType httpRequestType)
        : _webServerURL(webServerURL), _requestRelativeURL(requestRelativeURL), _httpRequestType(httpRequestType)
{
}


// ---------------------------------------------------------------------------
// Name:        osHTTPRequest::~osHTTPRequest
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        24/11/2005
// ---------------------------------------------------------------------------
osHTTPRequest::~osHTTPRequest()
{
}


// ---------------------------------------------------------------------------
// Name:        osHTTPRequest::reportBug
// Description: Performs the HTTP request.
// Arguments: resultContent - Will get the HTTP request result content (the web
//                            page sent to us by the mail server)
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/11/2005
// ---------------------------------------------------------------------------
bool osHTTPRequest::sendRequest(gtString& resultContent)
{
    bool retVal = false;
    resultContent.makeEmpty();

    // Create an HTTP session (use Windows defined default proxy):
    HINTERNET hSession = WinHttpOpen(L"osHTTPRequest", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    GT_IF_WITH_ASSERT(hSession != NULL)
    {
        // Convert the web server URL string to Unicode:
        wchar_t webServerURLUnicode[512];
        gtANSIStringToUnicodeString(_webServerURL.asCharArray(), webServerURLUnicode, 512);

        // Specify the HTTP server to which we will connect later.
        HINTERNET hConnect = WinHttpConnect(hSession, webServerURLUnicode,  INTERNET_DEFAULT_HTTP_PORT, 0);
        GT_IF_WITH_ASSERT(hConnect != NULL)
        {
            // Convert the file relative path to Unicode:
            wchar_t getArgumentsString[16384];
            gtANSIStringToUnicodeString(_requestRelativeURL.asCharArray(), getArgumentsString, 16384);

            // Create an HTTP request:
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", getArgumentsString , NULL, WINHTTP_NO_REFERER,
                                                    WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH);
            GT_IF_WITH_ASSERT(hRequest != NULL)
            {
                // Send the request.
                BOOL rc = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
                DWORD lastErr = GetLastError();
                GT_IF_WITH_ASSERT(rc != FALSE)
                {
                    // Receive the respond from the server:
                    rc = WinHttpReceiveResponse(hRequest, NULL);

                    if (rc)
                    {
                        // Keep checking for data until there is nothing left.
                        DWORD dwSize = 0;

                        do
                        {
                            // Check for available data.
                            rc = WinHttpQueryDataAvailable(hRequest, &dwSize);
                            GT_IF_WITH_ASSERT(rc == TRUE)
                            {
                                // Allocate space for the received data:
                                char* pszOutBuffer = new char[dwSize + 1];
                                GT_IF_WITH_ASSERT(pszOutBuffer != NULL)
                                {
                                    // Read the data.
                                    ZeroMemory(pszOutBuffer, dwSize + 1);

                                    // Read the currently available data:
                                    DWORD dwDownloaded = 0;
                                    BOOL rc1 = WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded);
                                    GT_IF_WITH_ASSERT(rc1 == TRUE)
                                    {
                                        // Add the current read data to the output result content:
                                        resultContent += pszOutBuffer;

                                        retVal = true;
                                    }

                                    // Delete the allocated buffer:
                                    delete[] pszOutBuffer;
                                }
                            }
                        }
                        while (dwSize > 0);
                    }
                }

                // Clean up:
                WinHttpCloseHandle(hRequest);
            }

            // Clean up:
            WinHttpCloseHandle(hConnect);
        }

        // Clean up:
        WinHttpCloseHandle(hSession);
    }

    return retVal;
}
