//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apAPIConnectionType.cpp
///
//==================================================================================

//------------------------------ apAPIConnectionType.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTAPIClasses/Include/apContextID.h>

// ---------------------------------------------------------------------------
// Name:        apAPIConnectionTypeToString
// Description: Translates apAPIConnectionType to a printable string.
// Arguments:   connectionType - The input connection type.
//              connectionTypeAsString - The output string.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/12/2009
// ---------------------------------------------------------------------------
bool apAPIConnectionTypeToString(apAPIConnectionType connectionType, gtString& connectionTypeAsString)
{
    bool retVal = true;
    connectionTypeAsString.makeEmpty();

    switch (connectionType)
    {
        case AP_INCOMING_EVENTS_API_CONNECTION:
            connectionTypeAsString = L"Incoming Events";
            break;

        case AP_SPIES_UTILITIES_API_CONNECTION:
            connectionTypeAsString = L"CodeXL Servers Manager";
            break;

        case AP_OPENGL_API_CONNECTION:
            connectionTypeAsString = L"CodeXL OpenGL Server";
            break;

        case AP_OPENCL_API_CONNECTION:
            connectionTypeAsString = L"CodeXL OpenCL Server";
            break;

        case AP_HSA_API_CONNECTION:
            connectionTypeAsString = L"CodeXL HSA Server";
            break;

        default:
            GT_ASSERT_EX(false, L"Error: unknown API connection type");
            connectionTypeAsString = L"Unknown API connection";
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apContextIDToAPIConnectionType
// Description: Return the needed API connection type for the context
// Return Val:  apAPIConnectionType
// Author:  AMD Developer Tools Team
// Date:        22/3/2010
// ---------------------------------------------------------------------------
apAPIConnectionType apContextIDToAPIConnectionType(apContextType contextType)
{
    apAPIConnectionType retVal = AP_SPIES_UTILITIES_API_CONNECTION;

    switch (contextType)
    {
        case AP_NULL_CONTEXT:
            retVal = AP_SPIES_UTILITIES_API_CONNECTION;
            break;

        case AP_OPENCL_CONTEXT:
            retVal = AP_OPENCL_API_CONNECTION;
            break;

        case AP_OPENGL_CONTEXT:
            retVal = AP_OPENGL_API_CONNECTION;
            break;

        default:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}
