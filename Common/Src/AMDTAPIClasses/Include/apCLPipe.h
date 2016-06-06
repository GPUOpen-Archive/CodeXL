//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLPipe.h
///
//==================================================================================

//------------------------------ apCLPipe.h ------------------------------

#ifndef __APCLPIPE_H
#define __APCLPIPE_H

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>

// Local:
#include <AMDTAPIClasses/Include/apCLMemObject.h>
#include <AMDTAPIClasses/Include/apOpenCLParameters.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLPipe : public apAllocatedObject
// General Description:
//   Represents an OpenCL pipe.
//
// Author:  AMD Developer Tools Team
// Creation Date:        18/11/2009
// ----------------------------------------------------------------------------------
class AP_API apCLPipe : public apCLMemObject
{
public:
    apCLPipe(gtInt32 pipeName = -1, gtUInt32 pipePacketSize = 0, gtUInt32 pipeMaxPackets = 0);
    virtual ~apCLPipe();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Pipe name:
    gtInt32 pipeName() const {return m_pipeName;};

    // Packet size:
    gtUInt32 pipePacketSize() const {return m_pipePacketSize;};

    // Max packets:
    gtUInt32 pipeMaxPackets() const {return m_pipeMaxPackets;};

private:
    // Pipe name:
    gtInt32 m_pipeName;

    // Pipe packet size:
    gtUInt32 m_pipePacketSize;

    // Pipe max packet count:
    gtUInt32 m_pipeMaxPackets;
};

#endif //__APCLPIPE_H

