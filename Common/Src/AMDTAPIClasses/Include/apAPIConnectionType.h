//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apAPIConnectionType.h
///
//==================================================================================

//------------------------------ apAPIConnectionType.h ------------------------------

#ifndef __APAPICONNECTIONTYPE_H
#define __APAPICONNECTIONTYPE_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apContextID.h>

// Enumeration that specifies API connection type:
enum apAPIConnectionType
{
    AP_INCOMING_EVENTS_API_CONNECTION,  // Incoming events API connection.
    AP_SPIES_UTILITIES_API_CONNECTION,  // API connection with the Spies utilities module.
    AP_OPENGL_API_CONNECTION,           // API connection with the OpenGL Server.
    AP_OPENCL_API_CONNECTION,           // API connection with the OpenCL Server.
    AP_HSA_API_CONNECTION,              // API connection with the HSA Server.
    AP_AMOUNT_OF_API_CONNECTION_TYPES
};

AP_API bool apAPIConnectionTypeToString(apAPIConnectionType connectionType, gtString& connectionTypeAsString);
AP_API apAPIConnectionType apContextIDToAPIConnectionType(apContextType contextType);


#endif //__APAPICONNECTIONTYPE_H

