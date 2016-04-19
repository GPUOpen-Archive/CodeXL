//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atPrintableTestData.cpp
///
//==================================================================================

/// Local:
#include <inc/atPrintableTestData.h>

atPrintableTestData::atPrintableTestData(atTestData* pData) : std::tr1::shared_ptr<atTestData>(pData)
{
}

const char* atPrintableTestData::GetTestName() const
{
    const atTestData* pData = get();

    if (pData)
    {
        return pData->_testName.asASCIICharArray();
    }
    else
    {
        return "Error: Invalid test data";
    }
};

::std::ostream& operator<<(::std::ostream& os, const atPrintableTestData& printableTestData)
{
    return os << printableTestData.GetTestName();
}

