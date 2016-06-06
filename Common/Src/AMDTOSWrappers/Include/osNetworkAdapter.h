//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osNetworkAdapter.h
///
//=====================================================================

//------------------------------ osNetworkAdapter.h ------------------------------

#ifndef __OSNETWORKADAPTER_H
#define __OSNETWORKADAPTER_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           osNetworkAdapter
// General Description:
//   Represents a single network adapter.
//
// Author:      AMD Developer Tools Team
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------
struct osNetworkAdapter
{
    // The adapter name:
    gtString _name;

    // The adapter MAC address = network hardware ID:
    // (Empty if not valid)
    gtString _MACAddress;

    // The adapter TCP / IP address
    gtString _TCPIPAddress;
};


OS_API bool osGetCurrentMachineNetworkAdapters(gtPtrVector<osNetworkAdapter*>& networkAdapters);
OS_API bool osGetCurrentMachineIPAddresses(gtVector<gtString>& ipAddresses, bool includeLocalLoopback);


#endif //__OSNETWORKADAPTER_H

