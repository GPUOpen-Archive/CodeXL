//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osNULLSocket.cpp
///
//=====================================================================

//------------------------------ osNULLSocket.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osNULLSocket.h>


// ---------------------------------------------------------------------------
// Name:        osNULLSocket::channelType
// Description: Returns a dummy channel type.
// Author:      AMD Developer Tools Team
// Date:        25/8/2005
// ---------------------------------------------------------------------------
osChannel::osChannelType osNULLSocket::channelType() const
{
    return osChannel::OS_BINARY_CHANNEL;
}


// ---------------------------------------------------------------------------
// Name:        osNULLSocket::write
// Description: Fails and generates an assertion failure.
// Author:      AMD Developer Tools Team
// Date:        25/8/2005
// ---------------------------------------------------------------------------
bool osNULLSocket::writeImpl(const gtByte* pDataBuffer, gtSize_t dataSize)
{
    (void)(pDataBuffer); // unused
    (void)(dataSize); // unused
    GT_ASSERT(false);
    return false;
}


// ---------------------------------------------------------------------------
// Name:        osNULLSocket::read
// Description: Fails and generates an assertion failure.
// Author:      AMD Developer Tools Team
// Date:        25/8/2005
// ---------------------------------------------------------------------------
bool osNULLSocket::readImpl(gtByte* pDataBuffer, gtSize_t dataSize)
{
    (void)(pDataBuffer); // unused
    (void)(dataSize); // unused
    GT_ASSERT(false);
    return false;
}


// ---------------------------------------------------------------------------
// Name:        osNULLSocket::readAvailableData
// Description: Fails and generates an assertion failure.
// Author:      AMD Developer Tools Team
// Date:        10/2/2008
// ---------------------------------------------------------------------------
bool osNULLSocket::readAvailableDataImpl(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead)
{
    (void)(pDataBuffer); // unused
    (void)(bufferSize); // unused
    (void)(amountOfDataRead); // unused
    GT_ASSERT(false);
    return false;
}


// ---------------------------------------------------------------------------
// Name:        osNULLSocket::open
// Description: Fails and generates an assertion failure.
// Author:      AMD Developer Tools Team
// Date:        25/8/2005
// ---------------------------------------------------------------------------
bool osNULLSocket::open()
{
    GT_ASSERT(false);
    return false;
}


// ---------------------------------------------------------------------------
// Name:        osNULLSocket::close
// Description: Fails and generates an assertion failure.
// Author:      AMD Developer Tools Team
// Date:        25/8/2005
// ---------------------------------------------------------------------------
bool osNULLSocket::close()
{
    GT_ASSERT(false);
    return false;
}


// ---------------------------------------------------------------------------
// Name:        osNULLSocket::isOpen
// Description: Fails and generates an assertion failure.
// Author:      AMD Developer Tools Team
// Date:        25/8/2005
// ---------------------------------------------------------------------------
bool osNULLSocket::isOpen() const
{
    GT_ASSERT(false);
    return false;
}


