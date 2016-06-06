//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCGIInputDataReader.cpp
///
//=====================================================================

//------------------------------ osCGIInputDataReader.cpp ------------------------------

// Standard C:
#include <stdlib.h>
#include <string.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osCGIInputDataReader.h>

// An environment variable holding the cgi-bin request method:
// Options are: GET, POST
#define OS_CGI_REQUEST_METHOD_ENV_VARIABLE_NAME L"REQUEST_METHOD"
#define OS_CGI_GET_REQUEST_METHOD_ENV_VARIABLE_VALUE L"GET"
#define OS_CGI_POST_REQUEST_METHOD_ENV_VARIABLE_VALUE L"POST"

// The name of an environment variable that holds the GET request query string:
#define OS_CGI_GET_REQUEST_QUERY_STRING_ENV_VARIABLE_NAME L"QUERY_STRING"

// The name of an environment variable that holds the POST request content type:
#define OS_CGI_POST_REQUEST_CONTENT_TYPE_ENV_VARIABLE_NAME L"CONTENT_TYPE"

// The value of a POST request content type that reflects data that was sent from an internet form:
#define OS_CGI_POST_REQUEST_FORM_URL_DECODED_CONTENT_TYPE_ENV_VARIABLE_VALUE L"application/x-www-form-urlencoded"

// The name of an environment variable that holds the POST request query string length:
#define OS_CGI_POST_REQUEST_QUERY_STRING_LENGTH_ENV_VARIABLE_NAME L"CONTENT_LENGTH"

// The name of an environment variable that holds the CGI-bin client's IP address:
#define OS_CGI_CLIENT_IP_ADDRESS_ENV_VARIABLE_NAME L"REMOTE_ADDR"

// The name of an environment variable that holds the CGI-bin server's name:
#define OS_CGI_SERVER_NAME_ENV_VARIABLE_NAME L"SERVER_NAME"

// The name of an environment variable that holds the CGI-bin server's port:
#define OS_CGI_SERVER_PORT_ENV_VARIABLE_NAME L"SERVER_PORT"

// The name of an environment variable that holds the CGI-bin server's name:
#define OS_CGI_REMOTE_HOST_ENV_VARIABLE_NAME L"REMOTE_HOST"

// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::osCGIInputDataReader
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        11/5/2008
// ---------------------------------------------------------------------------
osCGIInputDataReader::osCGIInputDataReader()
{
}


// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader~osCGIInputDataReader
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        11/5/2008
// ---------------------------------------------------------------------------
osCGIInputDataReader::~osCGIInputDataReader()
{
    // Release allocated memory:
    _dataItems.deleteElementsAndClear();
}


// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::readGetInputData
// Description:
//   Reads the GET method input data into this class.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/5/2008
// Implementation notes:
//   Technical details about obtaining GET and POST queries data from a C/C++
//   CGI-bin can be found at:
//   - http://hoohoo.ncsa.uiuc.edu/cgi/forms.html
//   - http://www.guyrutenberg.com/2007/09/07/introduction-to-c-cgi-processing-forms/
//   - http://library.thinkquest.org/16728/content/cgi/cplusplus.html
// ---------------------------------------------------------------------------
bool osCGIInputDataReader::readGetInputData()
{
    bool retVal = false;

    // Get the used request method:
    gtString requestMethod;
    bool rc1 = osGetCurrentProcessEnvVariableValue(OS_CGI_REQUEST_METHOD_ENV_VARIABLE_NAME, requestMethod);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Verify that used request method is "GET":
        if (requestMethod == OS_CGI_GET_REQUEST_METHOD_ENV_VARIABLE_VALUE)
        {
            // Get the GET request query string:
            gtString getRequestQueryString;
            bool rc2 = osGetCurrentProcessEnvVariableValue(OS_CGI_GET_REQUEST_QUERY_STRING_ENV_VARIABLE_NAME, getRequestQueryString);
            GT_IF_WITH_ASSERT(rc2)
            {
                // Read the request string items into the _dataItems vector:
                bool rc3 = readRequestString(getRequestQueryString);
                GT_IF_WITH_ASSERT(rc3)
                {
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::readPostInputData
// Description:
//   Reads the POST method input data into this class.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/5/2008
// Implementation notes:
//   See Implementation notes at readGetInputData function documentation.
// ---------------------------------------------------------------------------
bool osCGIInputDataReader::readPostInputData()
{
    bool retVal = false;

    // Get the used request method:
    gtString requestMethod;
    bool rc0 = osGetCurrentProcessEnvVariableValue(OS_CGI_REQUEST_METHOD_ENV_VARIABLE_NAME, requestMethod);
    GT_IF_WITH_ASSERT(rc0)
    {
        // Verify that used request method is "POST":
        if (requestMethod == OS_CGI_POST_REQUEST_METHOD_ENV_VARIABLE_VALUE)
        {
            // Get the request content type:
            gtString postRequestContentType;
            bool rc1 = osGetCurrentProcessEnvVariableValue(OS_CGI_POST_REQUEST_CONTENT_TYPE_ENV_VARIABLE_NAME, postRequestContentType);
            GT_IF_WITH_ASSERT(rc1)
            {
                // Verify that we got a content that was sent from an internet form:
                GT_IF_WITH_ASSERT(postRequestContentType == OS_CGI_POST_REQUEST_FORM_URL_DECODED_CONTENT_TYPE_ENV_VARIABLE_VALUE)
                {
                    // Get the POST request query string length:
                    gtString postRequestQueryStringLengthAsStr;
                    bool rc2 = osGetCurrentProcessEnvVariableValue(OS_CGI_POST_REQUEST_QUERY_STRING_LENGTH_ENV_VARIABLE_NAME, postRequestQueryStringLengthAsStr);
                    GT_IF_WITH_ASSERT(rc2)
                    {
                        // Translate the length from string into an integer:
                        int postRequestQueryStringLength = 0;
                        bool rc3 = postRequestQueryStringLengthAsStr.toIntNumber(postRequestQueryStringLength);
                        GT_IF_WITH_ASSERT(rc3)
                        {
                            // Allocate a buffer that will contain the input post request data:
                            char* pPostDataBuffer = new char[postRequestQueryStringLength + 1];


                            // Read the post request data into the buffer:
                            size_t rc4 = fread(pPostDataBuffer, sizeof(char), postRequestQueryStringLength, stdin);
                            GT_IF_WITH_ASSERT(rc4 == (size_t)postRequestQueryStringLength)
                            {
                                // Translate the read data into a string:
                                pPostDataBuffer[postRequestQueryStringLength] = 0;
                                gtString postRequestQueryString;
                                postRequestQueryString.fromASCIIString(pPostDataBuffer);

                                // Read the request string items into the _dataItems vector:
                                bool rc5 = readRequestString(postRequestQueryString);
                                GT_IF_WITH_ASSERT(rc5)
                                {
                                    retVal = true;
                                }
                            }

                            // Clean up:
                            delete[] pPostDataBuffer;
                            pPostDataBuffer = NULL;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::amountOfDataItems
// Description:
//   Returns the amount of CGI-bin input data items.
//   This function should be called after a call to readGetInputData or readPostInputData.
// Author:      AMD Developer Tools Team
// Date:        11/5/2008
// ---------------------------------------------------------------------------
int osCGIInputDataReader::amountOfDataItems() const
{
    int retVal = (int)_dataItems.size();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::getDataItem
// Description: Returns a CGI-bin input data item.
// Arguments: itemIndex - The CGI-bin input item index, in the range: [0, amountOfDataItems - 1].
//            itemData - Will get the CGI-bin input item data.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/5/2008
// ---------------------------------------------------------------------------
bool osCGIInputDataReader::getDataItem(int itemIndex, osCGIInputDataItem& itemData) const
{
    bool retVal = false;

    // Sanity check:
    int dataItemsAmount = amountOfDataItems();

    if ((0 <= itemIndex) && (itemIndex < dataItemsAmount))
    {
        osCGIInputDataItem* pItemData = _dataItems[itemIndex];
        GT_IF_WITH_ASSERT(pItemData != NULL)
        {
            itemData = *pItemData;
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::getDataItemValue
// Description: Inputs a CGI-bin data item name and outputs its value (if exists).
// Arguments: dataItemName - The data item name.
//            dataItemValue - Will get the data item value.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/5/2008
// ---------------------------------------------------------------------------
bool osCGIInputDataReader::getDataItemValue(const gtString& dataItemName, gtString& dataItemValue) const
{
    bool retVal = false;
    // Iterate the CGI-bin input items:
    gtPtrVector<osCGIInputDataItem*>::const_iterator iter = _dataItems.begin();
    gtPtrVector<osCGIInputDataItem*>::const_iterator endIter = _dataItems.end();

    while (iter != endIter)
    {
        // Get the current data item:
        const osCGIInputDataItem* pCurrDataItem = *iter;
        GT_IF_WITH_ASSERT(pCurrDataItem != NULL)
        {
            // Get the current input item name:
            const gtString& currItemName = pCurrDataItem->_name;

            // If this is the name we are looking for:
            if (currItemName == dataItemName)
            {
                // Output the data item value:
                dataItemValue = pCurrDataItem->_value;
                retVal = true;
                break;
            }
        }
        ++iter;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::getClientIPAddress
// Description: Retrieves the CGI-bin client's IP address.
// Arguments: clientIPAddress - Will get the client's IP address.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        14/5/2008
// ---------------------------------------------------------------------------
bool osCGIInputDataReader::getClientIPAddress(gtString& clientIPAddress) const
{
    bool retVal = false;

    // Get the used request method:
    bool rc1 = osGetCurrentProcessEnvVariableValue(OS_CGI_CLIENT_IP_ADDRESS_ENV_VARIABLE_NAME, clientIPAddress);

    if (rc1)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::getServerName
// Description: Retrieves the CGI-bin server name.
// Arguments: serverName - Will get the CGI-bin server's name (ex: www.gremedy.com).
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        14/5/2008
// ---------------------------------------------------------------------------
bool osCGIInputDataReader::getServerName(gtString& serverName) const
{
    bool retVal = false;

    // Get the used request method:
    bool rc1 = osGetCurrentProcessEnvVariableValue(OS_CGI_SERVER_NAME_ENV_VARIABLE_NAME, serverName);

    if (rc1)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::getServerPort
// Description: Retrieves the CGI-bin port number.
// Arguments: serverName - Will get the CGI-bin server's name (ex: 80).
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        25/5/2011
// ---------------------------------------------------------------------------
bool osCGIInputDataReader::getServerPort(unsigned int& serverPort) const
{
    bool retVal = false;

    gtString serverPortStr;
    // Get the used request method:
    bool rc1 = osGetCurrentProcessEnvVariableValue(OS_CGI_SERVER_PORT_ENV_VARIABLE_NAME, serverPortStr);

    if (rc1)
    {
        serverPortStr.toUnsignedIntNumber(serverPort);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::getServerPort
// Description: Retrieves the CGI-bin port number.
// Arguments: serverName - Will get the CGI-bin server's name
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/7/2011
// ---------------------------------------------------------------------------
bool osCGIInputDataReader::getRemoteHost(gtString& serverName) const
{
    bool retVal = false;

    // Get the used request method:
    bool rc1 = osGetCurrentProcessEnvVariableValue(OS_CGI_REMOTE_HOST_ENV_VARIABLE_NAME, serverName);

    if (rc1)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::readRequestString
// Description:
//   Parses the CGI request string and adds its contained data items into the
//   _dataItems vector.
//
// Arguments: requestString - The input request string, sent from the form into
//                            the CGI-bin.
//
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        12/5/2008
//
// Implementation notes:
//   The request string contains name=value pairs separated by the & character.
//   Each name=value pair is URL encoded, I.E: spaces are changed into plusses
//   and some characters are encoded into hexadecimal.
// ---------------------------------------------------------------------------
bool osCGIInputDataReader::readRequestString(const gtString& requestString)
{
    bool retVal = true;

    // Log the request string to the debug log file (if debug log severity is DEBUG):
    debugLogRequestString(requestString);

    // Break the request string into name=value pairs:
    gtString tokenizer1Token = L"&";
    gtStringTokenizer strTokenizer1(requestString, tokenizer1Token);
    gtString currentPair;

    while (strTokenizer1.getNextToken(currentPair))
    {
        // Get the current data item name:
        gtString tokenizer2Token = L"=";
        gtStringTokenizer strTokenizer2(currentPair, tokenizer2Token);
        gtString currentPairName;
        bool rc1 = strTokenizer2.getNextToken(currentPairName);

        if (!rc1)
        {
            // An error occurred:
            GT_ASSERT(rc1);
            retVal = false;
            break;
        }
        else
        {
            // Get the current data item value:
            gtString currentPairValue;
            bool rc2 = strTokenizer2.getNextToken(currentPairValue);

            if (!rc2)
            {
                // An error occurred:
                GT_ASSERT(rc2);
                retVal = false;
                break;
            }
            else
            {
                // Decode the data item value:
                gtString currentPairValueDecoded;
                bool rc3 = decodeFormRequestString(currentPairValue, currentPairValueDecoded);

                if (!rc3)
                {
                    // An error occurred:
                    GT_ASSERT(rc3);
                    retVal = false;
                    break;
                }
                else
                {
                    // Add the current data item to the _dataItems vector:
                    osCGIInputDataItem* pCurrentDataItem = new osCGIInputDataItem;


                    pCurrentDataItem->_name = currentPairName;
                    pCurrentDataItem->_value = currentPairValueDecoded;
                    _dataItems.push_back(pCurrentDataItem);
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::decodeFormRequestString
// Description: Decodes a string encoded by a web form back to the original
//              form data, filled by the user.
// Arguments: encodedString - The form encoded string.
//            decodedString - Will get the decoded string.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/5/2008
// Implementation notes:
//   Decodes a request string sent from an internet form. The request string values are
//   encoded, when sent by a form, in the following way:
//   - Spaces are changed into plusses
//   - Some characters are encoded into hexadecimal (%Hex).
//   This function inverts the above decoding.
// ---------------------------------------------------------------------------
bool osCGIInputDataReader::decodeFormRequestString(const gtString& encodedString, gtString& decodedString)
{
    bool retVal = false;

    // Sanity check:
    int encodedStringLength = encodedString.length();

    if (0 < encodedStringLength)
    {
        // Allocate space for the decoded string:
        wchar_t* pDecodedString = new wchar_t[encodedStringLength + 1]();


        // Initialize the decoded string to be the encoded string:
        wcscpy(pDecodedString, encodedString.asCharArray());

        // Replace spaces are changed into plusses:
        for (int i = 0; i < encodedStringLength; i++)
        {
            if (pDecodedString[i] == '+')
            {
                pDecodedString[i] = ' ';
            }
        }

        // Replace %Hex with the original char value:
        int sourceIndex = 0;
        int targetIndex = 0;

        while (pDecodedString[sourceIndex] != 0)
        {
            pDecodedString[targetIndex] = pDecodedString[sourceIndex];

            if (pDecodedString[sourceIndex] == '%')
            {
                pDecodedString[targetIndex] = hexToChar(&pDecodedString[sourceIndex + 1]);
                sourceIndex += 2;
            }

            ++targetIndex;
            ++sourceIndex;
        }

        pDecodedString[targetIndex] = 0;

        // Output the decoded string:
        decodedString = pDecodedString;

        // Clean up:
        delete[] pDecodedString;
        pDecodedString = NULL;

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::hexToChar
// Description: Decodes form encoded hexadecimal value.
// Arguments: pHexValue - The input encoded hexadecimal value.
// Return Val: char - Will get the decoded value.
// Author:      AMD Developer Tools Team
// Date:        13/5/2008
// ---------------------------------------------------------------------------
wchar_t osCGIInputDataReader::hexToChar(wchar_t* pHexValue) const
{
    wchar_t retVal = 0;

    if (pHexValue[0] >= 'A')
    {
        retVal = ((pHexValue[0] & 0xdf) - 'A') + 10;
    }
    else
    {
        retVal = (pHexValue[0] - '0');
    }

    retVal *= 16;

    if (pHexValue[1] >= 'A')
    {
        retVal += ((pHexValue[1] & 0xdf) - 'A') + 10;
    }
    else
    {
        retVal += (pHexValue[1] - '0');
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCGIInputDataReader::debugLogRequestString
// Description:
//  If debug log severity is DEBUG, logs the CGI request string to the debug log file
// Arguments: requestString - The CGI input request string.
// Author:      AMD Developer Tools Team
// Date:        25/1/2010
// ---------------------------------------------------------------------------
void osCGIInputDataReader::debugLogRequestString(const gtString& requestString)
{
    // If debug log severity is DEBUG:
    osDebugLogSeverity debugLogSeverity = osDebugLog::instance().loggedSeverity();

    if (OS_DEBUG_LOG_DEBUG <= debugLogSeverity)
    {
        gtString dbgMsg = L"CGI bin request string is: ";
        dbgMsg += requestString;
        OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}

