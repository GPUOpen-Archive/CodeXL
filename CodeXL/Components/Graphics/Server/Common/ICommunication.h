//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Interface for the communication between components of GPUPerfStudio.
///         This was authored with the server-side communication as a primary focus and
///         may need to be extended once we have a complete understanding of client-side
///         requirements.
///
///         A plugin which calls the methods ListenForRequests(..) or
///         GetPendingRequests(..) should implement those methods declared in
///         IProcessRequests.h.
//==============================================================================

#ifndef GPS_COMMUNICATION_INTERFACE
#define GPS_COMMUNICATION_INTERFACE

#define STR_STREAM_TOKEN "?Stream=" ///< Stream token to search for in client commands

#include <AMDTBaseTools/Include/gtASCIIString.h>
#include "../Common/GraphicsServerState.h"

/// CommunicationID needs to be 32-bit on windows for compatibility between
/// 32 bit webserver and 64 bit plugins & vice versa.
typedef unsigned int  CommunicationID;

class NetSocket;

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
bool InitCommunication(const char* strShortDescription, bool (*pProcessRequestCallback)(CommunicationID));

//-----------------------------------------------------------------------------
/// DeinitCommunication
///
/// This function only needs to be called by wrapper plugins. It closes the
/// inter-process communication between the wrapper and the PerfServer.
//-----------------------------------------------------------------------------
void DeinitCommunication();

//-----------------------------------------------------------------------------
/// GetPendingRequests
///
/// This is a non-blocking function that will iterate through the pending
/// requests and call ProcessRequest (See IProcessRequests.h) for each request.
/// Frame-based plugins should call this function at the start of each frame.
/// Global plugins do not need to call this function.
//-----------------------------------------------------------------------------
void GetPendingRequests();

//-----------------------------------------------------------------------------
/// Checks to see if there is at least one command in the buffer.
/// \return the URL string of the command
//-----------------------------------------------------------------------------
gtASCIIString PeekPendingRequests();

/// Loads a single command from the shared memory system.
/// Used in the frame debugger command processing loop in FrameDebugger::OnDrawCall
void GetSinglePendingRequest();

//-----------------------------------------------------------------------------
/// GetRequestText
///
/// This provides a method of accessing the text of a request.
///
/// \param requestID An ID for a particular request; the plugin will get this
///  id as a parameter to the ProcessRequest( ) function
///
/// \return pointer to the characters of the requests
//-----------------------------------------------------------------------------
const char* GetRequestText(CommunicationID requestID);

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
void* GetRequestBinary(CommunicationID requestID);

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
bool IsResponseRateLimited(CommunicationID requestID);

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
bool SendResponse(CommunicationID requestID, const char* cpsMimeType, const char* cpsResponse, unsigned int uResponseSize, bool bStreaming);

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
bool BufferResponse(const char* cpData, unsigned int uSizeInBytes);

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
bool SendBufferResponse(CommunicationID& requestID, NetSocket* pClientSocket);

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
                        NetSocket* pClientSocket);

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
bool SendTextResponse(CommunicationID& requestID, const char* cpData, NetSocket* pClientSocket);

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
bool SendFormattedTextResponse(CommunicationID& requestID,
                               NetSocket* pClientSocket,
                               const char* cpFormat,
                               ...);

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
bool SendHTMLResponse(CommunicationID& requestID, const char* cpData, NetSocket* pClientSocket);

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
bool SendXMLResponse(CommunicationID& requestID, const char* cpData, NetSocket* pClientSocket);

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
bool SendFileResponse(CommunicationID& requestID, const char* cpFile, NetSocket* pClientSocket);

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
bool SendMimeResponse(CommunicationID& requestID,
                      const char* cpMimeType,
                      const char* cpData,
                      unsigned int uSizeInBytes,
                      NetSocket* pClientSocket);

//-----------------------------------------------------------------------------
/// SendHTTPErrorResponse - Sends an error page back
/// \param requestID An ID for a particular request; the plugin will get this
///        id as a parameter to the ProcessRequest( ) function
/// \param nErrorCode numeric identifier of the error to return
/// \param pClientSocket The socket used to send the response.
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendHTTPErrorResponse(CommunicationID& requestID, int nErrorCode, NetSocket* pClientSocket);

//-----------------------------------------------------------------------------
/// SendServerStatusMessageAsHTML - Sends a server status state value back to the client
/// \param eServerState The stsus value to send back to the client
/// \param pClientSocket The socket used to send the response.
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendServerStatusMessageAsHTML(GRAPHICS_SERVER_STATE eServerState, NetSocket* pClientSocket);

//-----------------------------------------------------------------------------
/// SendServerStatusMessageAsXML - Sends a server status state value back to the client
/// \param eServerState The stsus value to send back to the client
/// \param pClientSocket The socket used to send the response.
/// \param requestString The URL string
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendServerStatusMessageAsXML(GRAPHICS_SERVER_STATE eServerState, NetSocket* pClientSocket, gtASCIIString requestString);

//-----------------------------------------------------------------------------
/// SendServerStatusMessageAsTEXT - Sends a server status state value back to the client
/// \param eServerState The stsus value to send back to the client
/// \param pClientSocket The socket used to send the response.
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendServerStatusMessageAsTEXT(GRAPHICS_SERVER_STATE eServerState, NetSocket* pClientSocket);

//-----------------------------------------------------------------------------
/// SendServerStatusMessageAsPNG - Sends a server status state value back to the client
/// \param eServerState The stsus value to send back to the client
/// \param pClientSocket The socket used to send the response.
/// \return true if the response could be sent; false otherwise
//-----------------------------------------------------------------------------
bool SendServerStatusMessageAsIMG(GRAPHICS_SERVER_STATE eServerState, NetSocket* pClientSocket);


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
bool SendRedirectResponse(CommunicationID& requestID, const char* pNewURL, NetSocket* pClientSocket);

#endif
