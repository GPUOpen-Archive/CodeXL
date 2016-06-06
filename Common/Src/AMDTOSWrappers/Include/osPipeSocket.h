//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPipeSocket.h
///
//=====================================================================

//------------------------------ osPipeSocket.h ------------------------------

#ifndef __OSPIPESOCKET_H
#define __OSPIPESOCKET_H

// Forward decelerations:
class osFilePath;

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osSocket.h>


// ----------------------------------------------------------------------------------
// Class Name:           osPipeSocket : public osSocket
// General Description:
//   A socket that is implemented by the use of the native OS pipes.
//   This enables efficient communication between two processes running on the same
//   machine.
//
// Author:      AMD Developer Tools Team
// Creation Date:        21/12/2006
// ----------------------------------------------------------------------------------
class OS_API osPipeSocket : public osSocket
{
public:
    osPipeSocket(const gtString& pipeName, const gtString& socketName);
    virtual ~osPipeSocket();

    // Overrides osChannel:
    virtual osChannelType channelType() const;
    virtual long defaultReadOperationTimeOut() const;
    virtual long defaultWriteOperationTimeOut() const;

    // Overrides osSocket:
    virtual bool isOpen() const;
    virtual bool close();

    void getServerFilePath(gtString& filePath);
    void getClientFilePath(gtString& filePath);

protected:
    void getPipeFIFOFilesPaths(gtString& clientToServer, gtString& serverToClient);
    bool readDataFromPipe(gtByte* pDataBuffer, gtSize_t bufferSize, bool readOnlyAvailableData, gtSize_t& readDataSize);

    // Overrides osSocket:
    virtual bool writeImpl(const gtByte* pDataBuffer, gtSize_t dataSize);
    virtual bool readImpl(gtByte* pDataBuffer, gtSize_t dataSize);
    virtual bool readAvailableDataImpl(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead);

protected:
    // The pipe name:
    gtString _pipeName;

    // Contains true iff this pipe is open:
    bool _isOpen;

    // Contains true iff this pipe is currently waiting for a connection:
    bool _isDuringPipeConnectionWait;

    // We use 2 half duplex pipes to implement a full duplex pipe:
    osPipeHandle _incomingPipe;
    osPipeHandle _outgoingPipe;

    // Contains 0 iff the pipe didn't encountered an error condition:
    int _errorsCount;
};


#endif //__OSPIPESOCKET_H

