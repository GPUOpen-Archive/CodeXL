//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Class to implement handling of HTTP requests and communication
///         between the client and server
//==============================================================================

#include <AMDTOSWrappers/Include/osSystemError.h>
#include "HTTPRequest.h"
#include "CommandTimingManager.h"

////////////////////////////////////////////////////////////////////////////////////////////
/// Initiaize the header data.
////////////////////////////////////////////////////////////////////////////////////////////
void HTTPRequestHeader::InitializeHeaderData()
{
    // Initialize the data in the struct
    memset(m_httpHeaderData.method, 0, sizeof(char) * SMALL_BUFFER_SIZE);
    memset(m_httpHeaderData.url, 0, sizeof(char) * COMM_MAX_URL_SIZE);
    memset(m_httpHeaderData.httpversion, 0, sizeof(char) * SMALL_BUFFER_SIZE);
    m_httpHeaderData.bReceivedOverSocket = false;
    memset(&m_httpHeaderData.client_ip, 0, sizeof(SockAddrIn));
    memset(&m_httpHeaderData.ProtoInfo, 0, sizeof(m_httpHeaderData.ProtoInfo));
    m_httpHeaderData.nPostDataSize = 0 ;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Set bReceivedOverSocket member.
/// \param bFlag The current state.
////////////////////////////////////////////////////////////////////////////////////////////
void HTTPRequestHeader::SetReceivedOverSocket(bool bFlag)
{
    m_httpHeaderData.bReceivedOverSocket = bFlag;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Get the bReceivedOverSocket member.
/// \return true or false.
////////////////////////////////////////////////////////////////////////////////////////////
bool HTTPRequestHeader::GetReceivedOverSocket()
{
    return m_httpHeaderData.bReceivedOverSocket;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Get the client socket handle.
/// \param handle the handle used to identify this data.
////////////////////////////////////////////////////////////////////////////////////////////
void HTTPRequestHeader::SetClientHandle(CommunicationID handle)
{
    m_httpHeaderData.handle = handle;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Set client_ip member.
/// \param clientIP The client IP
////////////////////////////////////////////////////////////////////////////////////////////
void HTTPRequestHeader::SetClientIP(SockAddrIn clientIP)
{
    m_httpHeaderData.client_ip = clientIP;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Get the client IP address.
/// \return The IP address.
////////////////////////////////////////////////////////////////////////////////////////////
SockAddrIn HTTPRequestHeader::GetClientIP()
{
    return m_httpHeaderData.client_ip;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Get the client socket handle.
/// \return a handle. Handles are 32-bit to allow compatibility between 32 and 64 bit
////////////////////////////////////////////////////////////////////////////////////////////
CommunicationID HTTPRequestHeader::GetClientHandle()
{
    return m_httpHeaderData.handle;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Get the port that this object is using.
/// \return the port beign used
////////////////////////////////////////////////////////////////////////////////////////////
DWORD HTTPRequestHeader::GetPort()
{
    return m_httpHeaderData.dwPort;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Set the URL member.
/// \param pURL The input URL string.
////////////////////////////////////////////////////////////////////////////////////////////
void HTTPRequestHeader::SetUrl(char* pURL)
{
    gtASCIIString strNewURL = pURL;
    // clear the old URL
    memset(m_httpHeaderData.url, 0, COMM_MAX_URL_SIZE);
    // copy in the new one
    memcpy_s(m_httpHeaderData.url, COMM_MAX_URL_SIZE, strNewURL.asCharArray(), strNewURL.length());
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Get the URL string
/// \return The URL string.
////////////////////////////////////////////////////////////////////////////////////////////
char* HTTPRequestHeader::GetUrl()
{
    return m_httpHeaderData.url;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Set the size of the posts data. This includes the name of the data, the '=' and the data itself in one string.
/// \param nSize The size of the POST data.
////////////////////////////////////////////////////////////////////////////////////////////
void HTTPRequestHeader::SetPostDataSize(unsigned int nSize)
{
    m_httpHeaderData.nPostDataSize = nSize;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Gets the size of the post data.
/// \return The size in bytes.
////////////////////////////////////////////////////////////////////////////////////////////
unsigned int HTTPRequestHeader::GetPostDataSize()
{
    return m_httpHeaderData.nPostDataSize;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Set the post data
/// \param pData The input data to copy.
////////////////////////////////////////////////////////////////////////////////////////////
void HTTPRequestHeader::SetPostData(char* pData)
{
    if (m_pPostData != NULL)
    {
        free(m_pPostData);
    }

    m_pPostData = pData;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Gets the post data.
/// \return A pointer to the post data.
////////////////////////////////////////////////////////////////////////////////////////////
char* HTTPRequestHeader::GetPostData()
{
    return m_pPostData;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Get the shader code within the POST data.
/// \return Pointer to the shader coide.
////////////////////////////////////////////////////////////////////////////////////////////
char* HTTPRequestHeader::GetShader()
{
    char* pDataPtr = strstr(m_pPostData, "postdata=");

    if (pDataPtr != NULL)
    {
        // Skip over the "postdata="
        return &pDataPtr[9];
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Gets the method string.
/// \return The method string.
////////////////////////////////////////////////////////////////////////////////////////////
char* HTTPRequestHeader::GetMethod()
{
    return m_httpHeaderData.method;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Gets a pointer to the header data.
/// \return A pointer to the header data.
////////////////////////////////////////////////////////////////////////////////////////////
HTTPHeaderData* HTTPRequestHeader::GetHeaderData()
{
    return &m_httpHeaderData;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Reads and parses the HTTP hedaer data from the SOCKET
/// \param strError Output error string.
/// \param pClientSocket the socket to read
/// \return True if success, false if fail.
////////////////////////////////////////////////////////////////////////////////////////////
HTTP_REQUEST_RESULT HTTPRequestHeader::ReadWebRequest(string& strError, NetSocket* pClientSocket)
{
    gtSize_t nRead = 0;

    // The header will fit within this buffer.
    char readBuffer[COMM_BUFFER_SIZE];
    // clear the memory.
    ZeroMemory(readBuffer, COMM_BUFFER_SIZE);
    // Read up to a buffer size worth of data. The function will detect the end of the header and will not read any more data.
    nRead = SocketReadHeader(pClientSocket, readBuffer, COMM_BUFFER_SIZE);

    // Check for error
    if (nRead == 0)
    {
        strError = "HTTPRequestHeader: SocketReadHeader read 0 bytes.";
        return HTTP_SOCKET_ERROR;
    }

    // Check for error
    if (nRead == (gtSize_t) - 1)
    {
        strError = "HTTPRequestHeader: SocketReadHeader read -1 bytes.";
        return HTTP_SOCKET_ERROR;
    }

    // Terminate the string
    readBuffer[nRead] = '\0';

    // Parse the data we just read in and populate our own internal data fields.
    bool bRes = ExtractHeaderData(&readBuffer[0]);

    // Return early if error
    if (bRes == false)
    {
        strError = "HTTPRequestHeader: ExtractHeaderData failed.";
        return HTTP_PARSE_ERROR;
    }

    // Check to see if POST data is present.
    if (GetPostDataSize() > 0)
    {
        // Read the POST data
        bRes = ReadPostData(strError, pClientSocket);

        if (bRes == false)
        {
            strError = "HTTPRequestHeader: ReadPostData failed.";
            return HTTP_PARSE_ERROR;
        }
    }

    return HTTP_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Preamble to reading the POST data section of a web request.
/// \param strError Output error string
/// \return length of data to read or 0 if error.
////////////////////////////////////////////////////////////////////////////////////////////
unsigned int HTTPRequestHeader::StartReadPostData(string& strError)
{
    // Clean up any existing data.
    if (m_pPostData != NULL)
    {
        free(m_pPostData);
    }

    // Get the length of the data to read.
    unsigned int nContentLength = GetPostDataSize();

    if (nContentLength == 0)
    {
        strError = "ReadPostData: Error: Content length is 0.\n";
        Log(logERROR, " Error: Content length is 0.");
        return nContentLength;
    }

    //// Create space fopr the post data.
    m_pPostData = (char*)malloc(sizeof(char) * nContentLength + 1);

    if (m_pPostData == NULL)
    {
        strError = "ReadPostData: Malloc failed in POST data.";
        Log(logERROR, "Malloc failed in POST data.");
        return 0;
    }

    return nContentLength;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Read the POST data section of a web request from a socket
/// Both input streams read pointers must be set at the beginning of the POST data.
/// \param strError Output error string
/// \param pClientSocket the socket to read
/// \return True if success, False if fail.
////////////////////////////////////////////////////////////////////////////////////////////
bool HTTPRequestHeader::ReadPostData(string& strError, NetSocket* pClientSocket)
{
    unsigned int nContentLength = StartReadPostData(strError);

    if (nContentLength == 0)
    {
        return false;
    }

    gtSize_t nRead = 0;

    // Read data from a socket
    nRead = SocketRead(pClientSocket, m_pPostData, nContentLength);

    // Terminate the buffer.
    m_pPostData[nRead] = '\0';

    //// Keep in for Debugging
    //StreamLog::Ref() << "READ IN SHADER:" << pPostDataBuffer << "\n";
    //printf ( "Read shader from web request: \n%s\n", m_pPostData);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Read the POST data section of a web request from shared memory
/// Both input streams read pointers must be set at the beginning of the POST data.
/// \param strError Output error string
/// \param pSharedMemoryName name of the shared memory to read
/// \return True if success, False if fail.
////////////////////////////////////////////////////////////////////////////////////////////
bool HTTPRequestHeader::ReadPostData(string& strError, const char* pSharedMemoryName)
{
    unsigned int nContentLength = StartReadPostData(strError);

    if (nContentLength == 0)
    {
        return false;
    }

    gtSize_t nRead = 0;

    // Read data from shared memory
    nRead = smGet(pSharedMemoryName, (void*)m_pPostData, nContentLength);

    // Terminate the buffer.
    m_pPostData[nRead] = '\0';

    //// Keep in for Debugging
    //StreamLog::Ref() << "READ IN SHADER:" << pPostDataBuffer << "\n";
    //printf ( "Read shader from web request: \n%s\n", m_pPostData);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////
/// Read data from the input socket
/// \param client_socket The socket connection.
/// \param receiveBuffer The buffer to read the data into.
/// \param bytesToRead The size of the output buffer.
/// \return The number of bytes read.
///////////////////////////////////////////////////////////////////////////////////////
gtSize_t HTTPRequestHeader::SocketRead(NetSocket* client_socket, char* receiveBuffer, gtSize_t bytesToRead)
{
    bool success = true;
    gtSize_t size;
    gtSize_t readSoFar = 0;

    while (readSoFar < bytesToRead && success == true)
    {
        success = client_socket->Receive(receiveBuffer + readSoFar, bytesToRead - readSoFar, size);

        if (success == true)
        {
            readSoFar += size;
        }
    }

    return readSoFar;
}

///////////////////////////////////////////////////////////////////////////////////////
/// Read just the web header
/// \param client_socket The socket connection.
/// \param pOutBuffer The buffer to read the data into.
/// \param nBufferSize The size of the output buffer.
/// \return The number of bytes read.
///////////////////////////////////////////////////////////////////////////////////////
gtSize_t HTTPRequestHeader::SocketReadHeader(NetSocket* client_socket, char* pOutBuffer, gtSize_t nBufferSize)
{
    gtSize_t  size = 0;
    gtSize_t  totalsize = 0;
    bool success = false;

    do
    {
        success = client_socket->Receive(pOutBuffer + totalsize, 1, size);

        if (size > 0 && success == true)
        {
            totalsize += size;

            // are we done reading the http header?
            if (strstr(pOutBuffer, "\r\n\r\n"))
            {
                break;
            }

            // have we overrun the buffer?
            if (totalsize > nBufferSize - 1)
            {
                success = false;
            }
        }
        else
        {
            totalsize = size;   // remember error state for return
        }
    }
    while (size > 0 && success == true);

    return (totalsize);
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Extract the data fields from the header buffer and populate the member data fields.
/// \param pReceiveBuffer The input data
/// \return True if suucess, False if fail.
////////////////////////////////////////////////////////////////////////////////////////////
bool HTTPRequestHeader::ExtractHeaderData(char* pReceiveBuffer)
{
    char* pos = NULL;
    char* context = NULL;

    ////////////////////////////////////////////////////////////////////////
    // tokenize the buffer for method
    pos = strtok_s(pReceiveBuffer, " ", &context);

    if (pos == NULL)
    {
        Log(logERROR, "Failed to tokenize HTTPHeader for method\n");
        return false;
    }

    // get the method (GET or POST)
    size_t methodLen = strlen(pos);

    if (methodLen > SMALL_BUFFER_SIZE)
    {
        Log(logERROR, "HTTP method is larger than buffer: %u > %u\n", methodLen, SMALL_BUFFER_SIZE);
        return false;
    }

    strncpy_s(m_httpHeaderData.method, SMALL_BUFFER_SIZE, pos, SMALL_BUFFER_SIZE);

    ////////////////////////////////////////////////////////////////////////
    //tokenize the buffer for url
    pos = strtok_s(NULL, " ", &context);

    if (pos == NULL)
    {
        Log(logERROR, "Failed to tokenize HTTPHeader for url\n");
        return false;
    }

    // get the url
    size_t urlLen = strlen(pos);

    if (urlLen > COMM_MAX_URL_SIZE)
    {
        Log(logERROR, "HTTP url is larger than buffer: %u > %u\n", urlLen, COMM_MAX_URL_SIZE);
        return false;
    }

    strncpy_s(m_httpHeaderData.url, COMM_MAX_URL_SIZE, pos, COMM_MAX_URL_SIZE);

    ////////////////////////////////////////////////////////////////////////
    // tokenize the buffer for version
    pos = strtok_s(NULL, "\r", &context);

    if (pos == NULL)
    {
        Log(logERROR, "Failed to tokenize HTTPHeader for version\n");
        return false;
    }

    // get the version
    size_t verLen = strlen(pos);

    if (verLen > SMALL_BUFFER_SIZE)
    {
        Log(logERROR, "HTTP version string is larger than buffer: %u > %u\n", verLen, SMALL_BUFFER_SIZE);
        return false;
    }

    strncpy_s(m_httpHeaderData.httpversion, SMALL_BUFFER_SIZE, pos, SMALL_BUFFER_SIZE);

    ////////////////////////////////////////////////////////////////////////
    // tokenize the buffer for content
    pos = strtok_s(NULL, "\r", &context);

    if (pos == NULL)
    {
        Log(logERROR, "Failed to tokenize HTTPHeader for Content-Type\n");
        return false;
    }

    // Currently we do nothing with content type.

    ////////////////////////////////////////////////////////////////////////
    // tokenize the buffer for Host
    pos = strtok_s(NULL, "\r", &context);

    if (pos == NULL)
    {
        Log(logERROR, "Failed to tokenize HTTPHeader for Host\n");
        return false;
    }

    // Currently we do nothing with Host.

    ////////////////////////////////////////////////////////////////////////
    // Check to see if POST method is being used to send anay data.
    if (strcmp(m_httpHeaderData.method, "POST") == 0)
    {
        // If so, record how much data is being sent.
        m_httpHeaderData.nPostDataSize = GetContentLength(context);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////
/// Get the content length of the POST data
/// \param pBuffer THe buffer to search in.
/// \return The content length of the POST data
////////////////////////////////////////////////////////////////////////
int HTTPRequestHeader::GetContentLength(char* pBuffer)
{
    char* p = strstr(pBuffer, "Content-Length:");

    int nLength = 0 ;

    if (p != NULL)
    {
        // Advance the pointer beyond the "Content-Length: " part of the s
        // tring so that it points to the content length number.
        p += 16;
        sscanf_s(p, "%d", &nLength);
    }

    return nLength ;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Checks to see if the process ID referenced in the request is still runnning
/// \return True if running, false if not.
////////////////////////////////////////////////////////////////////////////////////////////
bool HTTPRequestHeader::CheckProcessStillRunning()
{
    char* ptr = this->GetUrl();
    char* sCmd = &ptr[1];
    bool isAlive = true;

    gtASCIIString str(sCmd);

    int end = str.find('/', 0);
    int processID = 0;

    if (end > 0)
    {
        str.truncate(0, end - 1);
        processID = atoi(str.asCharArray());

        Log(logMESSAGE, "HTTPRequestHeader::CheckProcessStillRunning: processID = %d\n", processID);

        if (processID > 0)
        {
            // check to see if process is still running
            osIsProcessAlive((DWORD)processID, isAlive);
        }
    }

    return isAlive;
}
