//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDNSQueryThread.h
///
//=====================================================================

//------------------------------ osDNSQueryThread.h ------------------------------

#ifndef __OSDNSQUERYTHREAD_H
#define __OSDNSQUERYTHREAD_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>

// Platform specific includes and return type definitions:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #include "Winsock2.h"
    #define osDNSQueryResultType LPHOSTENT
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    #include <netdb.h>
    #define osDNSQueryResultType struct hostent*
#else
    #error Unknown build target!
#endif

// The timeout we use (in miliseconds)
#define OS_DNS_QUERY_THREAD_TIMEOUT 3000

// ----------------------------------------------------------------------------------
// Class Name:           osDNSQueryThread : public osThread

// General Description: A thread used as a wrapper for gethostbyname, as the function
//                      itself has no timeout value
// Author:      AMD Developer Tools Team
// Creation Date:        11/2/2009
// ----------------------------------------------------------------------------------
class osDNSQueryThread : public osThread
{
public:
    osDNSQueryThread();
    ~osDNSQueryThread();

    // Overrides osThread:
    virtual int entryPoint();
    virtual void beforeTermination();

public:
    // The input host name:
    gtString _hostName;

    // Will get the output host address:
    int _hostAddressLength;
    char* _hostAddress;

    // Contains true until the DNS query is over:
    bool _isDNSQueryPending;
};

#endif //__OSDNSQUERYTHREAD_H

