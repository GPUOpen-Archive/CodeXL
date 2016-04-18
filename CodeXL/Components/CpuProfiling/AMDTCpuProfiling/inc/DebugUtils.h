//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DebugUtils.h
///
//==================================================================================

#ifndef __DEBUGUTILS_H
#define __DEBUGUTILS_H

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include "inc/StdAfx.h"

inline void debugPrintDataArray(const gtString& name, const DataArray& da)
{
    gtString msg = name;
    msg += L":(";

    for (int i = 0; i < da.size(); i++)
    {
        msg.appendFormattedString(L"%.4f, ", da[i]);
    }

    msg += L")\n";
    osOutputDebugString(msg);
}

inline void debugPrintDataVector(const gtString& name, const gtVector<float>& da)
{
    gtString msg = name;
    msg += L":(";

    for (int i = 0; i < (int)da.size(); i++)
    {
        msg.appendFormattedString(L"%.4f, ", da[i]);
    }

    msg += L")\n";
    osOutputDebugString(msg);
}
inline void printStringList(const gtString& name, const QStringList& strList)
{
    gtString msg = name;
    msg += L":(";

    foreach (QString s, strList)
    {
        msg.append(acQStringToGTString(s));
        msg.appendFormattedString(L",");
    }

    msg += L")\n";
    osOutputDebugString(msg);
}

#endif //__DEBUGUTILS_H

