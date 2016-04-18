//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdAidOperators.cpp
///
//==================================================================================

//------------------------------ vsdAidOperators.cpp ------------------------------

// Local:
#include <src/vsdAidOperators.h>

bool vsdCompareGuids(const GUID& first, const GUID& second)
{
    bool retVal = false;

    if (first.Data1 > second.Data1)
    {
        retVal = false;
    }
    else if (first.Data1 < second.Data1)
    {
        retVal = true;
    }
    else if (first.Data2 > second.Data2)
    {
        retVal = false;
    }
    else if (first.Data2 < second.Data2)
    {
        retVal = true;
    }
    else if (first.Data3 > second.Data3)
    {
        retVal = false;
    }
    else if (first.Data3 < second.Data3)
    {
        retVal = true;
    }
    else if (first.Data4[0] > second.Data4[0])
    {
        retVal = false;
    }
    else if (first.Data4[0] < second.Data4[0])
    {
        retVal = true;
    }
    else if (first.Data4[1] > second.Data4[1])
    {
        retVal = false;
    }
    else if (first.Data4[1] < second.Data4[1])
    {
        retVal = true;
    }
    else if (first.Data4[2] > second.Data4[2])
    {
        retVal = false;
    }
    else if (first.Data4[2] < second.Data4[2])
    {
        retVal = true;
    }
    else if (first.Data4[3] > second.Data4[3])
    {
        retVal = false;
    }
    else if (first.Data4[3] < second.Data4[3])
    {
        retVal = true;
    }
    else if (first.Data4[4] > second.Data4[4])
    {
        retVal = false;
    }
    else if (first.Data4[4] < second.Data4[4])
    {
        retVal = true;
    }
    else if (first.Data4[5] > second.Data4[5])
    {
        retVal = false;
    }
    else if (first.Data4[5] < second.Data4[5])
    {
        retVal = true;
    }
    else if (first.Data4[6] > second.Data4[6])
    {
        retVal = false;
    }
    else if (first.Data4[6] < second.Data4[6])
    {
        retVal = true;
    }
    else if (first.Data4[7] > second.Data4[7])
    {
        retVal = false;
    }
    else if (first.Data4[7] < second.Data4[7])
    {
        retVal = true;
    }
    else
    {
        retVal = false;    // Equal
    }

    return retVal;
}
bool operator< (const AD_PROCESS_ID& first, const AD_PROCESS_ID& second)
{
    bool retVal = false;

    if (first.ProcessIdType == AD_PROCESS_ID_SYSTEM)
    {
        if (second.ProcessIdType == first.ProcessIdType)
        {
            retVal = first.ProcessId.dwProcessId < second.ProcessId.dwProcessId;
        }
        else
        {
            retVal = true;    // consider all GUIDs bigger than all system ids.
        }
    }
    else // first.ProcessIdType != AD_PROCESS_ID_SYSTEM
    {
        if (second.ProcessIdType == first.ProcessIdType)
        {
            retVal = vsdCompareGuids(first.ProcessId.guidProcessId, second.ProcessId.guidProcessId);
        }
        else
        {
            retVal = false;    // consider all GUIDs bigger than all system ids.
        }
    }

    return retVal;
}

