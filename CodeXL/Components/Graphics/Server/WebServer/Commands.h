//==============================================================================
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Process web commands received from the client
//==============================================================================

#ifndef GPS_COMMANDS_INCLUDED
#define GPS_COMMANDS_INCLUDED

class HTTPRequestHeader;
class NetSocket;

////////////////////////////////////////////////////////////////////////////////////////////
/// Handle the GET messages
/// \param pRequestHeader The request header to handle
/// \param pClientSocket the socket used to send the response
/// \param renderLoopStalled A bool that lets you know if the render loop has stalled.
////////////////////////////////////////////////////////////////////////////////////////////
void ProcessGetMethod(HTTPRequestHeader* pRequestHeader, NetSocket* pClientSocket, bool renderLoopStalled);

void ProcessPostMethod(HTTPRequestHeader* pRequestHeader, NetSocket* pClientSocket, bool renderLoopStalled);

#endif // GPS_COMMANDS_INCLUDED

