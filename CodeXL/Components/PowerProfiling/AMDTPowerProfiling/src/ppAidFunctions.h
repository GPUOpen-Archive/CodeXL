//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppAidFunctions.h
///
//==================================================================================

//------------------------------ ppAidFunctions.h ------------------------------

#ifndef __PPAIDFUNCTION
#define __PPAIDFUNCTION

#include <qstring.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>

#include <AMDTApplicationComponents/Include/acIcons.h>

class ppAidFunction
{
public:
    ppAidFunction() {};
    ~ppAidFunction() {};

    /// @param[in] categoryID  counter category index
    /// @returns   category name
    static QString CounterCategoryToStr(AMDTPwrCategory categoryID);

    /// @param[in] categoryID  counter category index
    /// @returns   category ID
    static acIconId GetCategoryIconId(AMDTPwrCategory categoryID);
};

#endif //__PPAIDFUNCTION
