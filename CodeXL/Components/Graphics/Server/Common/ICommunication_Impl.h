//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  defines some additional entrypoints into the communication interface
///         that are needed by the Server for opening a socket
//==============================================================================

#ifndef GPS_COMMUNICATION_IMPL_INTERFACE
#define GPS_COMMUNICATION_IMPL_INTERFACE

class HTTPRequestHeader;

//-----------------------------------------------------------------------------
/// CreateRequest
///
/// Provides a way for the Server, which receives over sockets, to create a
/// request that it can later respond to using the communication interface.
///
/// \param pRequest the request that was received over a socket
/// \param bReceivedOverSocket indicates that the original request was received
///   over a socket and that the Server will attempt to respond over the same
///   socket.
///
/// \return unsigned int requestID that should be used for responding to the
///    request.
//-----------------------------------------------------------------------------
CommunicationID CreateRequest(HTTPRequestHeader* pRequest, bool bReceivedOverSocket);

//-----------------------------------------------------------------------------
/// RemoveRequest
///
/// Provides a way for the Server to remove a request that it will not be
/// responding to. This is typical if the request is passed on to a plugin
/// in a different process which will be sending a response.
///
/// \param requestID a requestID returned by a call to CreateRequest
//-----------------------------------------------------------------------------
void RemoveRequest(CommunicationID requestID);

#endif //GPS_COMMUNICATION_IMPL_INTERFACE