//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDNSQueryThread.cpp
///
//=====================================================================

//------------------------------ osDNSQueryThread.cpp ------------------------------

// Standard C:
#include <string.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osDNSQueryThread.h>


// ---------------------------------------------------------------------------
// Name:        osDNSQueryThread::osDNSQueryThread
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        5/1/2010
// ---------------------------------------------------------------------------
osDNSQueryThread::osDNSQueryThread()
    : osThread(L"osDNSQueryThread"), _hostAddressLength(0), _hostAddress(NULL), _isDNSQueryPending(true)
{
}

// ---------------------------------------------------------------------------
// Name:        osDNSQueryThread::~osDNSQueryThread
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        5/1/2010
// ---------------------------------------------------------------------------
osDNSQueryThread::~osDNSQueryThread()
{
    delete[] _hostAddress;
    _hostAddress = NULL;
}


// ---------------------------------------------------------------------------
// Name:        osDNSQueryThread::entryPoint
// Description: Entry point function
// Author:      AMD Developer Tools Team
// Date:        19/2/2009
// ---------------------------------------------------------------------------
int osDNSQueryThread::entryPoint()
{
    int retVal = 0;

    GT_IF_WITH_ASSERT_EX(!_hostName.isEmpty(), L"Attempted to request a host address without specifying a host name")
    {
        osDNSQueryResultType pResult = gethostbyname(_hostName.asASCIICharArray());
        GT_IF_WITH_ASSERT(pResult != NULL)
        {
            // an IPv4 address is no longer than 4 bytes:
            _hostAddressLength = pResult->h_length;

            if (_hostAddressLength > 4)
            {
                _hostAddressLength = 4;
            }

            GT_IF_WITH_ASSERT((_hostAddressLength > 0) && (pResult->h_addr != NULL))
            {
                _hostAddress = new char[_hostAddressLength];
                GT_IF_WITH_ASSERT(_hostAddress != NULL)
                {
                    ::memcpy(_hostAddress, pResult->h_addr, _hostAddressLength);
                }
            }
        }
    }

    // Note that we are done
    _isDNSQueryPending = false;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osDNSQueryThread::beforeTermination
// Description: is called after the entrypoint function ends, tells us to wait for
//              the main thread before deleting ourselves.
// Author:      AMD Developer Tools Team
// Date:        19/2/2009
// ---------------------------------------------------------------------------
void osDNSQueryThread::beforeTermination()
{
}

