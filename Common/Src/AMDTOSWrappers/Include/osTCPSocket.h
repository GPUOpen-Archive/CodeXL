//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osTCPSocket.h ------------------------------

#ifndef __OSTCPSOCKET
#define __OSTCPSOCKET

// Forward declarations:
class osPortAddress;

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osSocket.h>


// ----------------------------------------------------------------------------------
// Class Name:           osTCPSocket
// General Description:
//   Represents a TCP / IP socket.
//   A TCP / IP socket enables reliable communication between two local / remote processes.
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/12/2003
// ----------------------------------------------------------------------------------
class OS_API osTCPSocket : public osSocket
{
public:
    osTCPSocket(const gtString& socketName);
    virtual ~osTCPSocket();

    // Self functions:
    bool getPeerHostAddress(osPortAddress& peerHostAddress) const;
    bool getCurrentAddress(osPortAddress& currentAddress) const;
    bool isConnectionAlive(bool& isAlive);

    // Turn on TCP KeepAlive setting for this socket.
    bool setKeepAlive(unsigned long timeoutMs, unsigned long intervalMs);

    // Accessors:
    void setBlockOnDNS(bool blockOnDNS) {_blockOnDNS = blockOnDNS;};

    // Utilities.
    static bool getIpAddresses(gtVector<gtString>& o_addresses);

    // Overrides osChannel:
    virtual osChannelType channelType() const;

    // Overrides osSocket:
    // ======================
    virtual bool open();
    // Opens the socket end point and specifies the sizes for the send and receive buffers
    bool open(int sizeOfReceiveBuffer, int sizeOfSendBuffer);
    virtual bool close();
    virtual bool isOpen() const;

    // Check if the error code indicates that the socket disconnected
    // and set the isOpen flag accordingly.
    void verifyConnectionAfterError(osSystemErrorCode lastSystemError);
    void verifyConnectionAfterError();

protected:
    static osSocketDescriptor NO_OS_SOCKET_DESCRIPTOR;

    osTCPSocket(unsigned int osSocketDescriptor, const gtString& socketName);

    void setOSDescriptor(osSocketDescriptor socketDescriptor);
    osSocketDescriptor OSSocketDescriptor() const { return _socketDescriptor; };

    bool readDataFromSocket(gtByte* pDataBuffer, gtSize_t bufferSize, bool readOnlyAvailableData, gtSize_t& readDataSize);

    virtual bool writeImpl(const gtByte* pDataBuffer, gtSize_t dataSize);
    virtual bool readImpl(gtByte* pDataBuffer, gtSize_t dataSize);
    virtual bool readAvailableDataImpl(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead);


    // Linux and Mac only:
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
protected:
    // Contains 0 iff the pipe didn't encountered an error condition:
    int _errorsCount;
#endif

    // Should this socket block connection attempts on DNS queries, or delegate them to a different thread?
    bool _blockOnDNS;

private:
    // The OS socket descriptor:
    osSocketDescriptor _socketDescriptor;

    // Contains true iff the socket is open:
    bool _isOpen;
};


#endif  // __OSTCPSOCKET
