//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osToAndFromString.cpp
///
//=====================================================================

//------------------------------ osToAndFromString.cpp ------------------------------

// Local:
#include <AMDTOSWrappers/Include/osToAndFromString.h>


// ---------------------------------------------------------------------------
// Name:        osProcessIdToString
// Description: Translates a process id into a string.
// Arguments: processId - The input process id.
//            outString - The output string.
// Author:      AMD Developer Tools Team
// Date:        12/10/2006
// ---------------------------------------------------------------------------
bool osProcessIdToString(osProcessId processId, gtString& outString)
{
    bool retVal = false;

    // Write the process id to a temporary buffer:
    wchar_t buff[250];
    int rc = swprintf_s(buff, 250, L"%u", processId);

    if (rc != -1)
    {
        outString = buff;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osProcessIdToString
// Description: Translates a process id into a string - ASCII version
// Arguments:   processId - The input process id.
//              outString - The output string.
// Author:      AMD Developer Tools Team
// Date:        13/9/2010
// ---------------------------------------------------------------------------
bool osProcessIdToString(osProcessId processId, gtASCIIString& outString)
{
    bool retVal = false;

    // Write the process id to a temporary buffer:
    char buff[250];
    int rc = sprintf_s(buff, 250, "%u", processId);

    if (rc != -1)
    {
        outString = buff;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osProcessIdFromString
// Description: Reads a process id from a string that was written
//              using osProcessIdToString.
// Arguments: string - The input string.
//            processId - The output process id.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        12/10/2006
// ---------------------------------------------------------------------------
bool osProcessIdFromString(const gtString& string, osProcessId& processId)
{
    bool retVal = false;

    // Verify that the string is not empty:
    if (!string.isEmpty())
    {
        // Read the process id:
        int rc = swscanf_s(string.asCharArray(), L"%u", &processId);

        if (rc == 1)
        {
            retVal = true;
        }
    }

    return retVal;
}