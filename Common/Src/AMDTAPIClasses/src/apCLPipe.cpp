//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLPipe.cpp
///
//==================================================================================

//------------------------------ apCLPipe.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apCLPipe.h>


// ---------------------------------------------------------------------------
// Name:        apCLPipe::apCLPipe
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        1/10/2014
// ---------------------------------------------------------------------------
apCLPipe::apCLPipe(gtInt32 pipeName, gtUInt32 pipePacketSize, gtUInt32 pipeMaxPackets)
    : m_pipeName(pipeName), m_pipePacketSize(pipePacketSize), m_pipeMaxPackets(pipeMaxPackets)
{

}


// ---------------------------------------------------------------------------
// Name:        apCLPipe::~apCLPipe
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        1/10/2014
// ---------------------------------------------------------------------------
apCLPipe::~apCLPipe()
{
}

// ---------------------------------------------------------------------------
// Name:        apCLPipe::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        1/10/2014
// ---------------------------------------------------------------------------
osTransferableObjectType apCLPipe::type() const
{
    return OS_TOBJ_ID_CL_PIPE;
}


// ---------------------------------------------------------------------------
// Name:        apCLPipe::writeSelfIntoChannel
// Description: Writes this object into an IPC Channel.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/10/2014
// ---------------------------------------------------------------------------
bool apCLPipe::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    // Write the pipe name:
    ipcChannel << m_pipeName;

    // Write the pipe packet size:
    ipcChannel << m_pipePacketSize;

    // Write the pipe max packets count:
    ipcChannel << m_pipeMaxPackets;

    // Write the mem object Info:
    retVal = apCLMemObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLPipe::readSelfFromChannel
// Description: Reads this object from an IPC Channel.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/10/2014
// ---------------------------------------------------------------------------
bool apCLPipe::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the pipe name:
    ipcChannel >> m_pipeName;

    // Read the pipe packet size:
    ipcChannel >> m_pipePacketSize;

    // Read the pipe max packets count:
    ipcChannel >> m_pipeMaxPackets;

    // Read the allocated object Info:
    retVal = apCLMemObject::readSelfFromChannel(ipcChannel);

    return retVal;
}

