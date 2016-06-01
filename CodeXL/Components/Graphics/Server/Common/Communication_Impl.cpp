//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Communication classes and definitions for sending responses back to the client.
//==============================================================================

#include <AMDTOSWrappers/Include/osSystemError.h>
#include <AMDTOSWrappers/Include/osProcess.h>

#include "ICommunication.h"
#include "ICommunication_Impl.h"
#include "HTTPRequest.h"
#include "SharedMemoryManager.h"
#include "Logger.h"
#include "timer.h"
#include <unordered_map>
#include "parser.h"
#include "defines.h"
#include "misc.h"
#include "mymutex.h"
#include "NamedSemaphore.h"

/// Define a function pointer type
typedef bool (*ProcessRequest_type)(CommunicationID);

/// Declare a function pointer for storing the function to process a request
ProcessRequest_type g_processRequest = NULL;

//-----------------------------------------------------------------------------
/// Stores a response to a request.
///
/// This class is only used by the Communication interface for storing data
/// that is needed for generating the response to a particular request
//-----------------------------------------------------------------------------
class Response
{
public:

    /// Constructor
    Response()
    {
        client_socket = 0;
        m_bSendNoCache = true;
        m_bNeedToSendHeader = true;
        m_bStreamingEnabled = false;
        m_dwMaxStreamsPerSecond = COMM_MAX_STREAM_RATE;
        m_dwLastSent = 0;
    }

    /// Destructor
    ~Response()
    {
        SAFE_DELETE(client_socket);
    }

    /// The underlying socket which was received on
    NetSocket* client_socket;

    /// Indicates whether or not to tell the client to cache the response
    bool m_bSendNoCache;

    //for streaming

    /// Indicates whether or not the header needs to be sent before the response
    bool m_bNeedToSendHeader;

    /// Indicates whether or not streaming is enabled for this response
    bool m_bStreamingEnabled;

    /// The number of times a response should be allowed to be sent
    unsigned long m_dwMaxStreamsPerSecond;

    /// The time that the last response was sent at
    unsigned long m_dwLastSent;
};


//-----------------------------------------------------------------------------
/// MimeAssociation struct
/// Relates a file extension (with leading '.') to a MIME type so that the
/// file can be sent over HTTP appropriately
//-----------------------------------------------------------------------------
struct MimeAssociation
{
    /// the extension of the file
    const char* file_ext;

    /// the associated mime type
    const char* mime;
};

//-----------------------------------------------------------------------------
/// mimetypes array
/// Stores the known list of file extensions and the MIME type that the file
/// should be sent with
//-----------------------------------------------------------------------------
MimeAssociation mimetypes[] =
{
    { ".txt", "text/plain" },
    { ".html", "text/html" },
    { ".htm", "text/html" },
    { ".gif", "image/gif" },
    { ".png", "image/png" },
    { ".xsl", "text/xsl" },
    { ".xml", "text/xml" },
    { ".ico", "image/ico" }
};

/// typedefs for RequestMap, this makes declaring iterators much easier
typedef std::unordered_map< CommunicationID, HTTPRequestHeader* > RequestMap;

/// typedefs for ResponseMap, this makes declaring iterators much easier
typedef std::unordered_map< CommunicationID, Response* > ResponseMap;

/// instances of a RequestMap that are used for storage
static RequestMap g_requestMap;

/// instances of a ResponseMap that are used for storage
static ResponseMap g_streamingResponseMap;

/// Mutex for use in communications
static mutex s_mutex;

/// timer for rate-limiting streaming responses
static Timer g_streamTimer;

/// static data for buffering a response
static char* g_pBufferedResponse;

/// static data for buffering a response
static unsigned int g_uBufferedResponseSize;

/// shared memory name;
static char g_strSharedMemoryName[ PS_MAX_PATH ];

//=============================================================================
//     "private" methods - only used in this file
//=============================================================================

//-----------------------------------------------------------------------------
/// Closes the connection that the specified Response uses
//-----------------------------------------------------------------------------
static void CloseConnection(Response& rResponse)
{
    rResponse.m_bNeedToSendHeader = true;
    rResponse.m_bStreamingEnabled = false;
    rResponse.client_socket->close();
    rResponse.client_socket = 0;
}

//-----------------------------------------------------------------------------
/// Generates an HTTP header according to the options stored in the Response
//-----------------------------------------------------------------------------
static void GenerateHeader(Response& rResponse, char* pOut, DWORD dwBufferSize)
{
    strcpy_s(pOut, dwBufferSize, "HTTP/1.0 200 OK\r\n");

    if (rResponse.m_bSendNoCache == true)
    {
        strncat_s(pOut, dwBufferSize, "pragma: no-store, no-cache\r\n"
                  "cache-control: no-cache, no-store, must-revalidate, max-age = 0\r\n"
                  "expires: 0\r\n",
                  COMM_BUFFER_SIZE);
    }

    if (rResponse.m_bStreamingEnabled == true)
    {
        strncat_s(pOut, dwBufferSize, "Content-Type: multipart/x-mixed-replace; boundary=BoundaryString\r\n\r\n", COMM_BUFFER_SIZE);
    }
}

//-----------------------------------------------------------------------------
/// Send
///
/// Sends the specified data as a specified mime type over the socket contained
/// in the Response
/// \param rResponse the response to send
/// \param mime response format
/// \param pData the data to send
/// \param dwSize the size of the data
/// \return true if the Response could be sent; false if there was an error
//-----------------------------------------------------------------------------
static bool Send(Response& rResponse, const char* mime, const char* pData, unsigned long dwSize)
{
    char sendbuffer[COMM_BUFFER_SIZE];
    sendbuffer[0] = 0;

    // send the http header and the file contents to the browser
    if (rResponse.m_bNeedToSendHeader == true)
    {
        GenerateHeader(rResponse, sendbuffer, COMM_BUFFER_SIZE);
        rResponse.m_bNeedToSendHeader = false;
    }

    if (rResponse.m_bStreamingEnabled == true)
    {
        strncat_s(sendbuffer, COMM_BUFFER_SIZE, "--BoundaryString\r\n", COMM_BUFFER_SIZE);
    }

    DWORD len = (DWORD)strlen(sendbuffer);
    sprintf_s(sendbuffer + len, COMM_BUFFER_SIZE - len, "Content-Type: %s\r\n"
              "Content-Length: %ld\r\n"
              "\r\n",
              mime,
              dwSize);

    // send header
    bool res = rResponse.client_socket->Send(sendbuffer, (DWORD)strlen(sendbuffer));

    if (res == true)
    {
        // if header could be sent
        // send data
        res = rResponse.client_socket->Send(pData, dwSize);
    }
    else
    {
        osSystemErrorCode systemLastError = osGetLastSystemError();

        // If no system error was recorded:
        if (systemLastError != 0)
        {
            gtString systemErrorString;
            osGetLastSystemErrorAsString(systemErrorString);

            Log(logERROR, "Failed to send %s response data due to error %d: %s\n", mime, systemLastError, systemErrorString.asASCIICharArray());
        }

        // there was an error sending
        // so close the connection and return false
        CloseConnection(rResponse);
        return false;
    }

    // if the response is not streaming,
    // the connection should be closed
    // otherwise it should be left open
    if (rResponse.m_bStreamingEnabled == false)
    {
        CloseConnection(rResponse);
    }

    return true;
}

//--------------------------------------------------------------
/// Performs linear search through mimetypes array looking for
/// matching file extension returning index of mime type
/// \param filename the filename to be sent
/// \return the index into the array of the mime type required.
///         default to index 0 (text/plain)
//--------------------------------------------------------------
static int FindMimeType(const char* filename)
{
    const char* pos = strrchr(filename, '.');

    if (pos)
    {
        int numofelements = sizeof(mimetypes) / sizeof(MimeAssociation);

        for (int x = 0; x < numofelements; ++x)
        {
            if (_stricmp(mimetypes[x].file_ext, pos) == 0)
            {
                return (x);
            }
        }
    }

    return (0);   // return default mimetype  'text/plain'
}

//-----------------------------------------------------------------------------
/// Generates and sends the HTML needed to specify that a particular numeric
/// error has occurred. These error codes are well-defined
/// See http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html for status codes
/// \param socket the socket to use to send the data
/// \param nErrorCode the error code to send
/// \return true if successful, false if error
//-----------------------------------------------------------------------------
static bool OutputHTTPError(NetSocket* socket, int nErrorCode)
{
    /// generate the error code html
    static char headerBuffer[COMM_BUFFER_SIZE];
    static char htmlBuffer[COMM_BUFFER_SIZE];

    sprintf_s(htmlBuffer, COMM_BUFFER_SIZE, "<html><body><h2>Error: %d</h2></body></html>", nErrorCode);
    sprintf_s(headerBuffer, COMM_BUFFER_SIZE, "HTTP/1.0 %d\r\nContent-Type: text/html\r\nContent-Length: %zd\r\n\r\n", nErrorCode, strlen(htmlBuffer));

    bool nRes1;
    bool nRes2;
    nRes1 = socket->Send(headerBuffer, (DWORD)strlen(headerBuffer));
    nRes2 = socket->Send(htmlBuffer, (DWORD)strlen(htmlBuffer));

    socket->close();

    if ((nRes1 == false) ||
        (nRes2 == false))
    {
        Log(logERROR, "Failed to send HTTPError %d over socket %lu because of error %lu\n", nErrorCode, socket, osGetLastSystemError());
        return false;
    }

    return true;
}


bool SendServerStatusMessageAsXML(GRAPHICS_SERVER_STATE eServerState, NetSocket* pClientSocket, gtASCIIString requestString)
{
    /// generate the error code html
    static char headerBuffer[COMM_BUFFER_SIZE] = "";
    static char xmlBuffer[COMM_BUFFER_SIZE] = "";

    gtASCIIString xmlSrc("<XML>");

    if (eServerState == GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING)
    {
        xmlSrc += "<GRAPHICS_SERVER_STATE>GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING</GRAPHICS_SERVER_STATE></XML>";
        sprintf_s(xmlBuffer, COMM_BUFFER_SIZE, xmlSrc.asCharArray());
    }
    else if (eServerState == GRAPHICS_SERVER_STATE_STALLED)
    {
        xmlSrc += "<GRAPHICS_SERVER_STATE>GRAPHICS_SERVER_STATE_STALLED</GRAPHICS_SERVER_STATE></XML>";
        sprintf_s(xmlBuffer, COMM_BUFFER_SIZE, xmlSrc.asCharArray());
    }

    // Size of the xmlBuffer is used in the header
    sprintf_s(headerBuffer, COMM_BUFFER_SIZE, "HTTP/1.0 200\r\nContent-Type: text/xml\r\nContent-Length: %zd\r\n\r\n", strlen(xmlBuffer));

    bool nRes1;
    bool nRes2;
    nRes1 = pClientSocket->Send(headerBuffer, (DWORD)strlen(headerBuffer));
    nRes2 = pClientSocket->Send(xmlBuffer, (DWORD)strlen(xmlBuffer));

    pClientSocket->close();

    if ((nRes1 == false) ||
        (nRes2 == false))
    {
        Log(logERROR, "Failed to send SendServerStatusMessageAsXML over socket %lu because of error %lu\n", socket, osGetLastSystemError());
        return false;
    }

    return true;
}

bool SendServerStatusMessageAsHTML(GRAPHICS_SERVER_STATE eServerState, NetSocket* pClientSocket)
{
    /// generate the error code html
    static char headerBuffer[COMM_BUFFER_SIZE] = "";
    static char htmlBuffer[COMM_BUFFER_SIZE] = "";

    if (eServerState == GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING)
    {
        sprintf_s(htmlBuffer, COMM_BUFFER_SIZE, "<html><body><GRAPHICS_SERVER_STATE>GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING</GRAPHICS_SERVER_STATE></body></html>");
    }
    else if (eServerState == GRAPHICS_SERVER_STATE_STALLED)
    {
        sprintf_s(htmlBuffer, COMM_BUFFER_SIZE, "<html><body><GRAPHICS_SERVER_STATE>GRAPHICS_SERVER_STATE_STALLED</GRAPHICS_SERVER_STATE></body></html>");
    }

    // Size of the htmlBuffer is used in the header
    sprintf_s(headerBuffer, COMM_BUFFER_SIZE, "HTTP/1.0 200\r\nContent-Type: text/html\r\nContent-Length: %zd\r\n\r\n", strlen(htmlBuffer));

    bool nRes1;
    bool nRes2;
    nRes1 = pClientSocket->Send(headerBuffer, (DWORD)strlen(headerBuffer));
    nRes2 = pClientSocket->Send(htmlBuffer, (DWORD)strlen(htmlBuffer));

    pClientSocket->close();

    if ((nRes1 == false) ||
        (nRes2 == false))
    {
        Log(logERROR, "Failed to send SendServerStatusMessageAsHTML over socket %lu because of error %lu\n", socket, osGetLastSystemError());
        return false;
    }

    return true;
}

bool SendServerStatusMessageAsTEXT(GRAPHICS_SERVER_STATE eServerState, NetSocket* pClientSocket)
{
    /// generate the error code html
    static char headerBuffer[COMM_BUFFER_SIZE];
    static char textBuffer[COMM_BUFFER_SIZE];

    if (eServerState == GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING)
    {
        gtASCIIString xmlSrc = "GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING";
        sprintf_s(textBuffer, COMM_BUFFER_SIZE, xmlSrc.asCharArray());
    }
    else if (eServerState == GRAPHICS_SERVER_STATE_STALLED)
    {
        gtASCIIString xmlSrc = "GRAPHICS_SERVER_STATE_STALLED";
        sprintf_s(textBuffer, COMM_BUFFER_SIZE, xmlSrc.asCharArray());
    }

    // Size of the textBuffer is used in the header
    sprintf_s(headerBuffer, COMM_BUFFER_SIZE, "HTTP/1.0 200\r\nContent-Type: text/xml\r\nContent-Length: %zd\r\n\r\n", strlen(textBuffer));

    bool nRes1;
    bool nRes2;
    nRes1 = pClientSocket->Send(headerBuffer, (DWORD)strlen(headerBuffer));
    nRes2 = pClientSocket->Send(textBuffer, (DWORD)strlen(textBuffer));

    pClientSocket->close();

    if ((nRes1 == false) ||
        (nRes2 == false))
    {
        Log(logERROR, "Failed to send SendServerStatusMessageAsTEXT over socket %lu because of error %lu\n", socket, osGetLastSystemError());
        return false;
    }

    return true;
}

bool SendServerStatusMessageAsIMG(GRAPHICS_SERVER_STATE eServerState, NetSocket* pClientSocket)
{
    /// generate the error code html
    static char headerBuffer[COMM_BUFFER_SIZE] = "";
    static char imgBuffer[COMM_BUFFER_SIZE] = "";

    if (eServerState == GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING)
    {
        sprintf_s(imgBuffer, COMM_BUFFER_SIZE, "%d", GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING);
    }
    else if (eServerState == GRAPHICS_SERVER_STATE_STALLED)
    {
        sprintf_s(imgBuffer, COMM_BUFFER_SIZE, "%d", GRAPHICS_SERVER_STATE_STALLED);
    }

    sprintf_s(headerBuffer, COMM_BUFFER_SIZE, "HTTP/1.0 200\r\nContent-Type: text/png\r\nContent-Length: %zd\r\n\r\n", strlen(imgBuffer));

    bool nRes1;
    bool nRes2;
    nRes1 = pClientSocket->Send(headerBuffer, (DWORD)strlen(headerBuffer));
    nRes2 = pClientSocket->Send(imgBuffer, 1);

    pClientSocket->close();

    if ((nRes1 == false) ||
        (nRes2 == false))
    {
        Log(logERROR, "Failed to send SendServerStatusMessageAsIMG over socket %lu because of error %lu\n", socket, osGetLastSystemError());
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
/// MakeResponse
/// Creates or acquires a response (and socket) needed to send data the specified
/// requestID
/// \param requestID id of the request to create a response for
/// \param ppResponse [out] pointer to a Response pointer; if MakeResponse returns true,
///        then it will point to the new or existing response for the requestID
/// \param pClientSocket the socket used to send the response.
/// \return true if the rResponse has been updated for sending the response; false otherwise
//-----------------------------------------------------------------------------
static bool MakeResponse(CommunicationID requestID, Response** ppResponse, NetSocket* pClientSocket)
{
    // protect the maps from being changed by other threads using the mutex
    ScopeLock lock(s_mutex);

    PsAssert(ppResponse != NULL);

    // first see if we already have this ID as a streaming response
    ResponseMap::iterator iterResponse = g_streamingResponseMap.find(requestID);

    if (iterResponse != g_streamingResponseMap.end())
    {
        *ppResponse = iterResponse->second;
        return true;
    }

    // otherwise we need to create a new response based on the original request
    // so get the request
    RequestMap::iterator iterRequest = g_requestMap.find(requestID);

    if (iterRequest == g_requestMap.end())
    {
        // the original request couldn't be found, so return failure
        return false;
    }

    // need to create a new response
    if (PsNew(*ppResponse) == false)
    {
        return false;
    }

    HTTPRequestHeader* pRequest = iterRequest->second;
    PsAssert(pRequest != NULL);

    if (pRequest->GetReceivedOverSocket() == true && pClientSocket != NULL)
    {
        (*ppResponse)->client_socket = pClientSocket;
    }
    else
    {
#if defined (_WIN32)
        (*ppResponse)->client_socket = NetSocket::CreateFromDuplicate(pRequest->GetProtoInfo());
#else
        // create a new socket and connect to the streamSocket on the server
        (*ppResponse)->client_socket = NetSocket::Create();

        if ((*ppResponse)->client_socket != NULL)
        {
            osPortAddress portAddress((unsigned short)pRequest->GetPort());
            (*ppResponse)->client_socket->Connect(portAddress);
        }

#endif
    }

    if ((*ppResponse)->client_socket == NULL)
    {
        int Err = NetSocket::LastError();
        Log(logERROR, "Could not create socket: NetSocket failed with error: %ld\n", Err);
        return false;
    }

    // see if this should be added as a streaming response
    gtASCIIString strUrl(pRequest->GetUrl());
    int32 iStream = strUrl.find(STR_STREAM_TOKEN);

    if (iStream >= 0)
    {
        const char* pBuf = strUrl.asCharArray();
        const char* pRate = &pBuf[ iStream + strlen(STR_STREAM_TOKEN)];
        unsigned int uRate = 0;

        // try to get the rate from the command;
        if (sscanf_s(pRate, "%u", &uRate) < 1)
        {
            // default to max rate
            uRate = COMM_MAX_STREAM_RATE;
        }

        // set the response as streaming with the specified rate
        (*ppResponse)->m_bStreamingEnabled = true;
        (*ppResponse)->m_dwMaxStreamsPerSecond = uRate;
        g_streamingResponseMap[ requestID ] = *ppResponse;
    }
    else
    {
        // streaming requests need to be kept around so that
        // additional responses can be directed to the right place,
        // HOWEVER, non-streaming requests only get a single response
        // and we just created the response for it, so it is safe
        // to remove the request from the requestMap. This will
        // help keep communication quick as the number of incoming
        // requests grows.
        RemoveRequest(requestID);
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Destroy a response after it has been send
/// \param rRequestID the ID of the request corresponding to the response
/// \param ppResponse the response to destroy
//-----------------------------------------------------------------------------
static void DestroyResponse(CommunicationID& rRequestID, Response** ppResponse)
{
    PsAssert(ppResponse != NULL);

    if (*ppResponse == NULL)
    {
        return;
    }

    // protect the maps from being changed by other threads using the mutex
    ScopeLock lock(s_mutex);

    // remove from streaming map
    if ((*ppResponse)->m_bStreamingEnabled)
    {
        ResponseMap::iterator iter = g_streamingResponseMap.find(rRequestID);

        if (iter != g_streamingResponseMap.end())
        {
            g_streamingResponseMap.erase(iter);
        }
    }

    SAFE_DELETE(*ppResponse);
    rRequestID = 0;
}


//-----------------------------------------------------------------------------
/// ShouldResponseBeSent
///
/// indicates whether a response should be sent to the specified request based
/// on whether or not the request is streaming and is rate limited. If the
/// request is rate limited, but a response can be sent, the "last sent time"
/// will be updated if bUpdateTime is true
///
/// \param requestID id of the request that may or may not be rate limited
/// \param bUpdateTime indicates whether or not the "last sent time" will be
///    updated if the response is rate limited, but allowed to be sent
///
/// \return true if a response should NOT be sent; false otherwise
//-----------------------------------------------------------------------------
bool ShouldResponseBeSent(CommunicationID requestID, bool bUpdateTime)
{
    // protect the maps from being changed by other threads using the mutex
    ScopeLock lock(s_mutex);

    ResponseMap::iterator iterResponse = g_streamingResponseMap.find(requestID);

    if (iterResponse == g_streamingResponseMap.end())
    {
        // don't limit the send because we don't even know it is streaming
        return false;
    }

    Response* pResponse = iterResponse->second;
    PsAssert(pResponse != NULL);

    // if this is a streaming request, only send if rate allows
    if (pResponse->m_bStreamingEnabled == true)
    {
        if (pResponse->m_dwMaxStreamsPerSecond == COMM_MAX_STREAM_RATE ||
            pResponse->m_dwMaxStreamsPerSecond == 0)
        {
            return false;
        }

        DWORD dwCurrTime = g_streamTimer.GetAbsolute();

        if (dwCurrTime - pResponse->m_dwLastSent >= 1000 / pResponse->m_dwMaxStreamsPerSecond)
        {
            if (bUpdateTime)
            {
                pResponse->m_dwLastSent = dwCurrTime;
            }

            return false;
        }
        else
        {
            return true;
        }
    }

    return false;
}

//=============================================================================
//
// EntryPoints for Receiving Data
//
// It is agreed / understood that all requests will be primarily sent as
// text-based strings, but that a request may or may not have additional binary
// data. For those client-side plugins that wish to send their internal
// commands as pure binary, the associated text command should be "binary"
//
//=============================================================================

//-----------------------------------------------------------------------------
/// InitCommunication
///
/// This function only needs to be called by wrapper plugins. It sets up the
/// inter-process communication between the wrapper (which is in the app's
/// process space) and the PerfServer.
/// \param strShortDescription Description
/// \param pProcessRequestCallback Process request callback function
/// \return True if success false if fail.
//-----------------------------------------------------------------------------
bool InitCommunication(const char* strShortDescription, ProcessRequest_type pProcessRequestCallback)
{
    unsigned long pid = osGetCurrentProcessId();
#ifdef _WIN32
    sprintf_s(g_strSharedMemoryName, PS_MAX_PATH, "%lu/%s", pid, strShortDescription);
#else
    // the '/' character can't be used as a filename in Linux, so just use the plugin name as the shared memory name
    // (ignore the process ID)
    sprintf_s(g_strSharedMemoryName, PS_MAX_PATH, "%lu %s", pid, strShortDescription);
#endif

    if (smCreate(g_strSharedMemoryName, 100, sizeof(HTTPRequestHeader)) == false)
    {
        Log(logERROR, "InitCommunication: Can't open or create SharedMemory for %s.\n", strShortDescription);
        return false;
    }

    if (smOpen("PLUGINS_TO_GPS") == false)
    {
        smClose(g_strSharedMemoryName);
        Log(logERROR, "InitCommunication: Can't open SharedMemory for PLUGINS_TO_GPS.\n");
        return false;
    }

    // store a local pointer to the ProcessRequest function
    g_processRequest = pProcessRequestCallback;

    if (g_processRequest == NULL)
    {
        smClose(g_strSharedMemoryName);
        Log(logERROR, "InitCommunication: ProcessRequest is NULL\n");
        return false;
    }

    // protect the maps from being changed by other threads using the mutex
    ScopeLock lock(s_mutex);

    g_requestMap.clear();
    g_pBufferedResponse = NULL;
    g_uBufferedResponseSize = 0;

    return true;
}

//-----------------------------------------------------------------------------
/// DeinitCommunication
///
/// This function only needs to be called by wrapper plugins. It closes the
/// inter-process communication between the wrapper and the PerfServer.
//-----------------------------------------------------------------------------
void DeinitCommunication()
{
    smClose(g_strSharedMemoryName);
    smClose("PLUGINS_TO_GPS");

    g_processRequest = NULL;
}

//-----------------------------------------------------------------------------
/// GetPendingRequests
///
/// This is a non-blocking function that will iterate through the pending
/// requests and call ProcessRequest (See IProcessRequests.h) for each request.
/// Frame-based plugins should call this function at the start of each frame.
/// Global plugins do not need to call this function.
//-----------------------------------------------------------------------------
void GetPendingRequests()
{
    if (smLockGet(g_strSharedMemoryName) == false)
    {
        return;
    }

    HTTPHeaderData requestHeader;
    DWORD dwSize = sizeof(HTTPHeaderData);

    int nCount = 0;

    while (smGet(g_strSharedMemoryName, NULL, 0) > 0)
    {
        smGet(g_strSharedMemoryName, (void*)&requestHeader, dwSize);
        HTTPRequestHeader* pRequest = new HTTPRequestHeader(requestHeader);

        // Check to see if POST data is present.
        if (pRequest->GetPostDataSize() > 0)
        {
            string strError;
            bool bRes = pRequest->ReadPostData(strError, g_strSharedMemoryName);

            if (bRes == false)
            {
                Log(logERROR, "Failed to read POST data during smGet().\n");
            }

            // Debug code - leave in for now.
            //printf ( "%s\n", pRequest->GetPostData() );
        }

        CommunicationID requestID = CreateRequest(pRequest, false);
        nCount++;

        if (g_processRequest(requestID) == false)
        {
            SendHTTPErrorResponse(requestID, 404, NULL);
            break;
        }
    }

    smUnlockGet(g_strSharedMemoryName);

    if (nCount > 0)
    {
        Log(logMESSAGE, "Server loading: %d\n", nCount);
    }
}

gtASCIIString PeekPendingRequests()
{
    if (smLockGet(g_strSharedMemoryName) == false)
    {
        return "";
    }

    // Check to see if there is a command
    if (smGet(g_strSharedMemoryName, NULL, 0) == 0)
    {
        // There are no commands
        return "";
    }

    HTTPHeaderData requestHeaderData;
    DWORD dwSize = sizeof(HTTPHeaderData);

    // Get the header
    smPeek(g_strSharedMemoryName, (void*)&requestHeaderData, dwSize);
    HTTPRequestHeader* pRequest = new HTTPRequestHeader(requestHeaderData);

    gtASCIIString str = pRequest->GetUrl();

    smUnlockGet(g_strSharedMemoryName);

    // delete the header data
    delete pRequest;

    return str;
}

void GetSinglePendingRequest()
{
    if (smLockGet(g_strSharedMemoryName) == false)
    {
        return;
    }

    HTTPHeaderData requestHeader;
    DWORD dwSize = sizeof(HTTPHeaderData);

    if (smGet(g_strSharedMemoryName, NULL, 0) > 0)
    {
        smGet(g_strSharedMemoryName, (void*)&requestHeader, dwSize);
        HTTPRequestHeader* pRequest = new HTTPRequestHeader(requestHeader);

        // Check to see if POST data is present.
        if (pRequest->GetPostDataSize() > 0)
        {
            string strError;
            bool bRes = pRequest->ReadPostData(strError, g_strSharedMemoryName);

            if (bRes == false)
            {
                Log(logERROR, "Failed to read POST data during smGet().\n");
            }

            // Debug code - leave in for now.
            //printf ( "%s\n", pRequest->GetPostData() );
        }

        CommunicationID requestID = CreateRequest(pRequest, false);

        if (g_processRequest(requestID) == false)
        {
            SendHTTPErrorResponse(requestID, 404, NULL);
        }
    }

    smUnlockGet(g_strSharedMemoryName);
}


//-----------------------------------------------------------------------------
/// GetRequestText
///
/// This provides a method of accessing the text of a request.
///
/// \param requestID An ID for a particular request; the plugin will get this
///        id as a parameter to the ProcessRequest( ) function
///
/// \return pointer to the characters of the requests
//-----------------------------------------------------------------------------
const char* GetRequestText(CommunicationID requestID)
{
    HTTPRequestHeader* pRequest = (HTTPRequestHeader*) GetRequestBinary(requestID);

    if (pRequest == NULL)
    {
        return NULL;
    }

    char* pUrl = pRequest->GetUrl();

    // skip any leading '/' that may preceed the text
    while (IsToken(&pUrl, "/"))
    {
    }

    return (pUrl);
}

//-----------------------------------------------------------------------------
/// GetRequestBinary
///
/// This provides a method of accessing any binary data of a request. Requests
/// that have binary data as the primary means of communication will have the
/// string "binary" returned by GetRequestText(...); other commands may or may
/// not have associated binary data, as determined by the client.
///
/// \param requestID An ID for a particular request; the plugin will get this
///  id as a parameter to the ProcessRequest( ) function
///
/// \return pointer to the binary data that is associated with the request;
///  NULL if no binary data exists
//-----------------------------------------------------------------------------
void* GetRequestBinary(CommunicationID requestID)
{
    // protect the maps from being changed by other threads using the mutex
    ScopeLock lock(s_mutex);

    HTTPRequestHeader* pRequest = NULL;

    RequestMap::iterator iterRequest = g_requestMap.find(requestID);

    if (iterRequest != g_requestMap.end())
    {
        pRequest = iterRequest->second;
    }

    return pRequest;
}

//-----------------------------------------------------------------------------
/// IsResponseRateLimited
///
/// indicates whether a response should be sent to the specified request based
/// on whether or not the request is streaming and is rate limited
///
/// \param requestID id of the request that may or may not be rate limited
///
/// \return true if a response should NOT be sent; false otherwise
//-----------------------------------------------------------------------------
bool IsResponseRateLimited(CommunicationID requestID)
{
    return ShouldResponseBeSent(requestID, false);
}

//=============================================================================
//
// EntryPoints for sending data
//
// It is agreed / understood that there will be at least one response for every
// request that is received. A request for which a response is not sent, will
// remain in the request queue and the server will continue to receive that
// request until it sends an response. Each request is thus identified with an
// ID which must be included when a response is sent.
//
//=============================================================================

//---------------------------------------------------------
/// SendResponse
///
/// Sends the response either over sockets or shared memory
///
/// \param requestID the requestID to send the response to
/// \param cpsMimeType the mimetype to send the response as
/// \param cpsResponse the response to send
/// \param uResponseSize the size of the response being sent
/// \param bStreaming indicates that the response it to a streaming request
///
/// \return true if the response is 'sent' correctly; false otherwise
//---------------------------------------------------------
bool SendResponse(CommunicationID requestID, const char* cpsMimeType, const char* cpsResponse, unsigned int uResponseSize, bool bStreaming)
{
    // find out if this is a streaming response
    if (bStreaming)
    {
        Log(logTRACE, "Sending response over socket\n");
        // this is a streaming response
        // use the socket for comms

        return SendMimeResponse(requestID, cpsMimeType, cpsResponse, uResponseSize, NULL);
    }

    // use Shared memory if this is not a streaming response
    if (smLockPut("PLUGINS_TO_GPS", sizeof(requestID) + ((unsigned long) strlen(cpsMimeType) * sizeof(const char)) + uResponseSize, 3) == false)
    {
        Log(logASSERT, "Not enough space in shared memory for response.\n");
        return false;
    }

    NamedSemaphore semaphore;
    bool opened = semaphore.Open("PLUGINS_TO_GPS_SEMAPHORE");

    if (opened)
    {
        if (semaphore.Signal() == false)
        {
            Log(logWARNING, "Failed to signal PLUGINS_TO_GPS_SEMAPHORE. Response may be lost. Error is %d, Previous count is 0\n", osGetLastSystemError());
        }

        semaphore.Close();
    }
    else
    {
        Log(logWARNING, "Failed to open PLUGINS_TO_GPS_SEMAPHORE. Response may be delayed.\n");
    }

    bool bResult = (smPut("PLUGINS_TO_GPS", &requestID, sizeof(requestID)) &&
                    smPut("PLUGINS_TO_GPS", (void*)cpsMimeType, (unsigned long) strlen(cpsMimeType) * sizeof(const char)) &&
                    smPut("PLUGINS_TO_GPS", (void*)cpsResponse, uResponseSize));

    smUnlockPut("PLUGINS_TO_GPS");

    if (bResult == false)
    {
        Log(logASSERT, "Failed to put part of the response into shared memory\n");
    }
    else
    {
        // remove the request
        RemoveRequest(requestID);
    }

    return bResult;
}

//-----------------------------------------------------------------------------
/// BufferResponse
///
/// Proves a means of incrementally adding raw data to a buffered response.
///
/// \param cpData pointer to the data that should be added to the buffer
/// \param uSizeInBytes number of bytes of data pointed to by cpData
///
/// \return true if the response could be added to the buffer; false otherwise
//-----------------------------------------------------------------------------
bool BufferResponse(const char* cpData, unsigned int uSizeInBytes)
{
    if (cpData == NULL ||
        uSizeInBytes == 0)
    {
        Log(logERROR, "Failed to buffer response because data is NULL\n");
        return false;
    }

    char* pNewBuffer = NULL;

    unsigned int nBufferSize = g_uBufferedResponseSize + uSizeInBytes;

    try
    {
        pNewBuffer = new char[ nBufferSize ];
    }
    catch (std::bad_alloc)
    {
        Log(logERROR, "BufferResponse: Out of memory\n");
        return false;
    }

    if (g_pBufferedResponse != NULL)
    {
        // copy existing data if there is any
        memcpy_s(pNewBuffer, nBufferSize, g_pBufferedResponse, g_uBufferedResponseSize);

        // cleanup previous buffered response
        delete g_pBufferedResponse;

        // move new Buffer to be the buffered response
        g_pBufferedResponse = pNewBuffer;
        pNewBuffer = g_pBufferedResponse + g_uBufferedResponseSize;
    }
    else
    {
        // there was no previous buffered response,
        // so make the global pointer be the same as the new buffer
        g_pBufferedResponse = pNewBuffer;
    }

    // copy parameter data into buffered memory
    memcpy_s(pNewBuffer, nBufferSize, cpData, uSizeInBytes);

    g_uBufferedResponseSize += uSizeInBytes;

    return true;
}

//-----------------------------------------------------------------------------
/// SendBufferResponse
///
/// Sends the buffered response that was built using BufferResponse(...).
///
/// \param requestID An ID for a particular request; the plugin will get this
///        id as a parameter to the ProcessRequest( ) function
/// \param pClientSocket the socket used to send the response.
///
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendBufferResponse(CommunicationID& requestID, NetSocket* pClientSocket)
{
    return SendBinaryResponse(requestID, g_pBufferedResponse, g_uBufferedResponseSize, pClientSocket);
}

//-----------------------------------------------------------------------------
/// SendBinaryResponse
///
/// Sends a single buffer of raw data as the response.
///
/// \param requestID An ID for a particular request; the plugin will get this
///        id as a parameter to the ProcessRequest( ) function
/// \param cpData pointer to the data that should be sent as a response
/// \param uSizeInBytes number of bytes of data pointed to by cpData
/// \param pClientSocket the socket used to send the response.
///
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendBinaryResponse(CommunicationID& requestID,
                        const char* cpData,
                        unsigned int uSizeInBytes,
                        NetSocket* pClientSocket)
{
    if (cpData == NULL)
    {
        Log(logERROR, "Failed to send binary response because data is NULL\n");
        return false;
    }

    // see if the response is streaming and rate limited
    if (ShouldResponseBeSent(requestID, true) == true)
    {
        // the message shouldn't be sent because of the rate limiting
        // return true though so the server thinks it was sent successfully
        return true;
    }

    Response* pResponse = NULL;

    if (MakeResponse(requestID, &pResponse, pClientSocket) == false)
    {
        Log(logERROR, "Failed to make a response for requestID %d\n", requestID);
        return false;
    }

    if (Send(*pResponse, "application/octet-stream", cpData, uSizeInBytes) == false)
    {
        Log(logERROR, "Failed to 'Send' response for requestID %d\n", requestID);

        if (pResponse->m_bStreamingEnabled == false)
        {
            DestroyResponse(requestID, &pResponse);
        }

        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
/// SendTextResponse
///
/// Sends a character string to be interpreted as text by the recipient.
///
/// \param requestID An ID for a particular request; the plugin will get this
///        id as a parameter to the ProcessRequest( ) function
/// \param cpData pointer to the text that should be sent as a response
/// \param pClientSocket the socket used to send the response.
///
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendTextResponse(CommunicationID& requestID, const char* cpData, NetSocket* pClientSocket)
{
    if (cpData == NULL)
    {
        Log(logERROR, "Failed to send text response because data is NULL\n");
        return false;
    }

    // see if the response is streaming and rate limited
    if (ShouldResponseBeSent(requestID, true) == true)
    {
        // the message shouldn't be sent because of the rate limiting
        // return true though so the server thinks it was sent successfully
        return true;
    }

    Response* pResponse = NULL;

    if (MakeResponse(requestID, &pResponse, pClientSocket) == false)
    {
        Log(logERROR, "Failed to make a response for requestID %d to send content: %s\n", requestID, cpData);
        return false;
    }

    if (Send(*pResponse, "text/plain", cpData, (unsigned long)strlen(cpData)) == false)
    {
        Log(logERROR, "Failed to 'Send' response for requestID %d with content: %s\n", requestID, cpData);
        DestroyResponse(requestID, &pResponse);
        return false;
    }

    if (pResponse->m_bStreamingEnabled == false)
    {
        DestroyResponse(requestID, &pResponse);
    }

    return true;
}

//-----------------------------------------------------------------------------
/// SendFormattedTextResponse
///
/// Send a formatted string to be interpreted as text by the recipient.
///
/// \param requestID An ID for a particular request; the plugin will get this
///        id as a parameter to the ProcessRequest( ) function
/// \param pClientSocket the socket used to send the response.
/// \param cpFormat pointer to a character format that should be send as a
///        response; it is expected that this format consumes the remaining
///        parameters to this function
///
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendFormattedTextResponse(CommunicationID& requestID, NetSocket* pClientSocket, const char* cpFormat, ...)
{
    if (cpFormat == NULL)
    {
        Log(logERROR, "Failed to send formatted response because data is NULL\n");
        return false;
    }

    // see if the response is streaming and rate limited
    if (ShouldResponseBeSent(requestID, true) == true)
    {
        // the message shouldn't be sent because of the rate limiting
        // return true though so the server thinks it was sent successfully
        return true;
    }

    Response* pResponse = NULL;

    if (MakeResponse(requestID, &pResponse, pClientSocket) == false)
    {
        Log(logERROR, "Failed to make a response for requestID %d\n", requestID);
        return false;
    }

    static char string[ 10240 ];
    va_list arg_ptr;

    va_start(arg_ptr, cpFormat);
    vsprintf_s(string, 10240, cpFormat, arg_ptr);
    va_end(arg_ptr);

    if (Send(*pResponse, "text/plain", string, (unsigned long)strlen(string)) == false)
    {
        Log(logERROR, "Failed to 'Send' response for requestID %d\n", requestID);
        DestroyResponse(requestID, &pResponse);
        return false;
    }

    if (pResponse->m_bStreamingEnabled == false)
    {
        DestroyResponse(requestID, &pResponse);
    }

    return true;
}

//-----------------------------------------------------------------------------
/// SendHTMLResponse
///
/// Sends a character string to be interpreted as HTML by the recipient.
///
/// \param requestID An ID for a particular request; the plugin will get this
///        id as a parameter to the ProcessRequest( ) function
/// \param cpData pointer to the HTML code that should be sent as a response
/// \param pClientSocket the socket used to send the response.
///
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendHTMLResponse(CommunicationID& requestID, const char* cpData, NetSocket* pClientSocket)
{
    if (cpData == NULL)
    {
        Log(logERROR, "Failed to send HTML response because data is NULL\n");
        return false;
    }

    // see if the response is streaming and rate limited
    if (ShouldResponseBeSent(requestID, true) == true)
    {
        // the message shouldn't be sent because of the rate limiting
        // return true though so the server thinks it was sent successfully
        return true;
    }

    Response* pResponse = NULL;

    if (MakeResponse(requestID, &pResponse, pClientSocket) == false)
    {
        Log(logERROR, "Failed to make a response for requestID %d to send content: %s\n", requestID, cpData);
        return false;
    }

    if (Send(*pResponse, "text/html", cpData, (unsigned long)strlen(cpData)) == false)
    {
        Log(logERROR, "Failed to 'Send' response for requestID %d with content: %s\n", requestID, cpData);
        DestroyResponse(requestID, &pResponse);
        return false;
    }

    if (pResponse->m_bStreamingEnabled == false)
    {
        DestroyResponse(requestID, &pResponse);
    }

    return true;
}

//-----------------------------------------------------------------------------
/// SendXMLResponse
///
/// Sends a character string to be interpreted as XML by the recipient.
///
/// \param requestID An ID for a particular request; the plugin will get this
///        id as a parameter to the ProcessRequest( ) function
/// \param cpData pointer to the XML that should be sent as a response
/// \param pClientSocket the socket used to send the response.
///
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendXMLResponse(CommunicationID& requestID, const char* cpData, NetSocket* pClientSocket)
{
    if (cpData == NULL)
    {
        Log(logERROR, "Failed to send XML response because data is NULL\n");
        return false;
    }

    // see if the response is streaming and rate limited
    if (ShouldResponseBeSent(requestID, true) == true)
    {
        // the message shouldn't be sent because of the rate limiting
        // return true though so the server thinks it was sent successfully
        return true;
    }

    Response* pResponse = NULL;

    if (MakeResponse(requestID, &pResponse, pClientSocket) == false)
    {
        Log(logERROR, "Failed to make a response for requestID %d to send content: %s\n", requestID, cpData);
        return false;
    }

    if (Send(*pResponse, "text/xml", cpData, (unsigned long)strlen(cpData)) == false)
    {
        Log(logERROR, "Failed to 'Send' response for requestID %d with content: %s\n", requestID, cpData);
        DestroyResponse(requestID, &pResponse);
        return false;
    }

    if (pResponse->m_bStreamingEnabled == false)
    {
        DestroyResponse(requestID, &pResponse);
    }

    return true;
}

//-----------------------------------------------------------------------------
/// SendFileResponse
///
/// Sends the binary contents of a file to be interpreted by the recipient as
/// the type of file indicated by the file's extension.
///
/// \param requestID An ID for a particular request; the plugin will get this
///        id as a parameter to the ProcessRequest( ) function
/// \param cpFile pointer to the filename that should be sent as a response
/// \param pClientSocket the socket used to send the response.
///
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendFileResponse(CommunicationID& requestID, const char* cpFile, NetSocket* pClientSocket)
{
    if (cpFile == NULL)
    {
        Log(logERROR, "Failed to send file response because filename is NULL\n");
        return false;
    }

    // see if the response is streaming and rate limited
    if (ShouldResponseBeSent(requestID, true) == true)
    {
        // the message shouldn't be sent because of the rate limiting
        // return true though so the server thinks it was sent successfully
        return true;
    }

    Response* pResponse = NULL;

    if (MakeResponse(requestID, &pResponse, pClientSocket) == false)
    {
        Log(logERROR, "Failed to make a response for requestID %d to send file: %s\n", requestID, cpFile);
        return false;
    }

    // collect file data and generate response
    FILE* in;
    long fileSize;
    char* fileBuffer;
    fopen_s(&in, cpFile, "rb");    // read binary

    if (!in)
    {
        // file error, not found?
        OutputHTTPError(pResponse->client_socket, 404);     // 404 - not found
        return false;
    }

    // determine file size
    fseek(in, 0, SEEK_END);
    fileSize = ftell(in);
    fseek(in, 0, SEEK_SET);

    // allocate Buffer_ and read in file contents
    fileBuffer = new char[fileSize];
    fread(fileBuffer, sizeof(char), fileSize, in);
    fclose(in);

    bool bRes = Send(*pResponse, mimetypes[ FindMimeType(cpFile)].mime, fileBuffer, fileSize);

    if (bRes == false)
    {
        Log(logERROR, "Failed to 'Send' response for requestID %d\n", requestID);
        DestroyResponse(requestID, &pResponse);
    }

    delete [] fileBuffer;

    if (pResponse->m_bStreamingEnabled == false)
    {
        DestroyResponse(requestID, &pResponse);
    }

    return bRes;
}

//-----------------------------------------------------------------------------
/// SendMimeResponse
///
/// Sends a single buffer of raw data as the response which should be
/// interpreted by the recipient as being of the indicated MIME type
///
/// \param requestID An ID for a particular request; the plugin will get this
///        id as a parameter to the ProcessRequest( ) function
/// \param cpMimeType A string identifying which MIME type to use for
///        interpreting the data
/// \param cpData pointer to the data that should be sent as a response
/// \param uSizeInBytes number of bytes of data pointed to by cpData
/// \param pClientSocket the socket used to send the response.
///
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendMimeResponse(CommunicationID& requestID, const char* cpMimeType, const char* cpData, unsigned int uSizeInBytes, NetSocket* pClientSocket)
{
    if (cpMimeType == NULL)
    {
        Log(logERROR, "Failed to send %s response because data is NULL\n", cpMimeType);
        return false;
    }

    // see if the response is streaming and rate limited
    if (ShouldResponseBeSent(requestID, true) == true)
    {
        // the message shouldn't be sent because of the rate limiting
        // return true though so the server thinks it was sent successfully
        return true;
    }

    Response* pResponse = NULL;

    if (MakeResponse(requestID, &pResponse, pClientSocket) == false)
    {
        return false;
    }

    // if Data is NULL then close streaming connection
    if (pResponse->m_bStreamingEnabled == true)
    {
        if (cpData == NULL)
        {
            const char* pstr = "--BoundaryString\r\n";
            pResponse->client_socket->Send(pstr, (unsigned int)strlen(pstr));
            CloseConnection(*pResponse);
            DestroyResponse(requestID, &pResponse);
            return true;
        }
    }

    if (Send(*pResponse, cpMimeType, cpData, uSizeInBytes) == false)
    {
        DestroyResponse(requestID, &pResponse);
        return false;
    }

    if (pResponse->m_bStreamingEnabled == false)
    {
        DestroyResponse(requestID, &pResponse);
    }

    return true;
}

//-----------------------------------------------------------------------------
/// SendHTTPErrorResponse
///
/// Sends an error page back
///
/// \param requestID An ID for a particular request; the plugin will get this
///        id as a parameter to the ProcessRequest( ) function
/// \param nErrorCode numeric identifier of the error to return
/// \param pClientSocket the socket used to send the response.
///
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendHTTPErrorResponse(CommunicationID& requestID, int nErrorCode, NetSocket* pClientSocket)
{
    // see if the response is streaming and rate limited
    if (ShouldResponseBeSent(requestID, true) == true)
    {
        // the message shouldn't be sent because of the rate limiting
        // return true though so the server thinks it was sent successfully
        return true;
    }

    Response* pResponse = NULL;

    if (MakeResponse(requestID, &pResponse, pClientSocket) == false)
    {
        return false;
    }

    if (OutputHTTPError(pResponse->client_socket, nErrorCode) == false)
    {
        DestroyResponse(requestID, &pResponse);
        return false;
    }

    if (pResponse->m_bStreamingEnabled == false)
    {
        DestroyResponse(requestID, &pResponse);
    }

    return true;
}

//-----------------------------------------------------------------------------
/// SendRedirectResponse
///
/// Redirects the browser to a different URL than the one requested
///
/// \param requestID An ID for a particular request; the plugin will get this
///        id as a parameter to the ProcessRequest( ) function
/// \param pNewURL The url that the browser is being redirected to
/// \param pClientSocket the socket used to send the response.
///
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendRedirectResponse(CommunicationID& requestID, const char* pNewURL, NetSocket* pClientSocket)
{
    if (pNewURL == NULL)
    {
        return false;
    }

    // see if the response is streaming and rate limited
    if (ShouldResponseBeSent(requestID, true) == true)
    {
        // the message shouldn't be sent because of the rate limiting
        // return true though so the server thinks it was sent successfully
        return true;
    }

    Response* pResponse = NULL;

    if (MakeResponse(requestID, &pResponse, pClientSocket) == false)
    {
        return false;
    }

    // generate the redirect html
    static char headerBuffer[COMM_BUFFER_SIZE];
    static char htmlBuffer[COMM_BUFFER_SIZE];

    sprintf_s(htmlBuffer, COMM_BUFFER_SIZE, "<html><body><a href=\"%s\">%s</a></body></html>", pNewURL, pNewURL);
    sprintf_s(headerBuffer, COMM_BUFFER_SIZE, "HTTP/1.0 301\r\nContent-Type: text/html\r\nContent-Length: %zd\r\nLocation: %s\r\n\r\n", strlen(htmlBuffer), pNewURL);

    bool nRes1;
    bool nRes2;
    nRes1 = pResponse->client_socket->Send(headerBuffer, (DWORD)strlen(headerBuffer));
    nRes2 = pResponse->client_socket->Send(htmlBuffer, (DWORD)strlen(htmlBuffer));

    pResponse->client_socket->close();

    if ((nRes1 == false) ||
        (nRes2 == false))
    {
        DestroyResponse(requestID, &pResponse);
        return false;
    }

    if (pResponse->m_bStreamingEnabled == false)
    {
        DestroyResponse(requestID, &pResponse);
    }

    return true;
}

//-----------------------------------------------------------------------------
/// CreateRequest
///
/// Provides a way for the Server, which receives over sockets, to create a
/// request that it can later respond to using the communication interface.
///
/// \param pRequest the request that was received over a socket
/// \param bReceivedOverSocket indicates that the original request was received
///        over a socket and that the Server will attempt to respond over the same
///        socket.
///
/// \return unsigned int requestID that should be used for responding to the
///    request.
//-----------------------------------------------------------------------------
#ifdef _WIN32
    #pragma warning( push )
    #pragma warning( disable : 4311)
    #pragma warning( disable : 4302)
#endif
CommunicationID CreateRequest(HTTPRequestHeader* pRequest, bool bReceivedOverSocket)
{
    // protect the maps from being changed by other threads using the mutex
    ScopeLock lock(s_mutex);

    CommunicationID requestID = pRequest->GetClientHandle();

    pRequest->SetReceivedOverSocket(bReceivedOverSocket);

    if (g_requestMap.find(requestID) != g_requestMap.end())
    {
        Log(logWARNING, "RequestID %u already exists from request: %s\n", requestID, pRequest->GetUrl());
        // Remove the pre-existing request (required to cleanup memory).
        RemoveRequest(requestID);
    }

    g_requestMap[requestID] = pRequest;

    return requestID;
}
#ifdef _WIN32
    #pragma warning( pop )
#endif
//-----------------------------------------------------------------------------
/// RemoveRequest
///
/// Provides a way for the Server to remove a request that it will not be
/// responding to. This is typical if the request is passed on to a plugin
/// in a different process which will be sending a response.
///
/// \param requestID a requestID returned by a call to CreateRequest
//-----------------------------------------------------------------------------
void RemoveRequest(CommunicationID requestID)
{
    // protect the maps from being changed by other threads using the mutex
    ScopeLock lock(s_mutex);

    RequestMap::iterator iter = g_requestMap.find(requestID);

    if (iter != g_requestMap.end())
    {
        HTTPRequestHeader* pRequest = iter->second;
        delete pRequest;
        g_requestMap.erase(iter);
    }
}
