//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Responsible for maintaining a string which is added
/// throughout a frame and will be sent in response to a CommandObject.
//==============================================================================

#ifndef GPS_HTTPLOGGER_H
#define GPS_HTTPLOGGER_H

#include <AMDTBaseTools/Include/gtASCIIString.h>
#include "CommandProcessor.h"

/// Responsible for maintaining a string which is added throughout a frame and will be sent in response to a CommandObject.
class HTTPLogger : public gtASCIIString, public ProfilerCommandResponse
{
public:

    /// send the gtASCIIString as a response
    void SendText()
    {
        CommandResponse::Send(asCharArray());
    }
};

#endif // GPS_HTTPLOGGER_H
