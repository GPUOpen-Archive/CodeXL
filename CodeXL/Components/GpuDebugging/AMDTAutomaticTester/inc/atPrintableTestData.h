//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atPrintableTestData.h
///
//==================================================================================
#ifndef __ATPRINTABLETESTDATA_H
#define __ATPRINTABLETESTDATA_H

#include <memory>
#include <Inc/atTestData.h>

//typedef  TestDataSmartPtr;
/// -----------------------------------------------------------------------------------------------
/// \class Name: atPrintableTestData
/// \brief Description: A wrapper class that allows passing test data to parameterized test cases.
///                     Unfortunately Google Test allows passing only a single parameter to
///                     parameterized test cases. This class is used to pass the test data by value,
///                     while actually using a pointer internally to reference the atTestData instance
///                     that contains the actual data.
///                     The reason we do all this wrapping instead of just using a atTestData pointer
///                     is so we can define a << operator on atPrintableTestData to make the test report
///                     more readable.
/// -----------------------------------------------------------------------------------------------
class atPrintableTestData : public std::tr1::shared_ptr<atTestData>
{
public:
    atPrintableTestData(atTestData* pData);
    const atTestData* GetData() const {return get();}
    const char* GetTestName() const;
};

::std::ostream& operator<<(::std::ostream& os, const atPrintableTestData& printableTestData);

#endif //__ATPRINTABLETESTDATA_H

