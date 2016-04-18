//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atTestsDataVector.h
///
//==================================================================================
#ifndef __ATTESTSDATAVECTOR_H
#define __ATTESTSDATAVECTOR_H

#include <AMDTBaseTools/Include/gtPtrVector.h>

// forward deceleration
#include <inc/atPrintableTestData.h>


/// -----------------------------------------------------------------------------------------------
/// \class Name: atTestsDataVector : public gtPtrVector<atTestData*>
/// \brief Description: Reads the XML file and stores the data for the test cases.
///                     Since GoogleTest does all of the test cases instantiation before the main
///                     function is called, we are forced to perform the XML reading in the constructor
///                     of a globally declared variable. Ughhhh.
/// \tparam atTestData*:
/// -----------------------------------------------------------------------------------------------
class atTestsDataVector : public gtVector<atPrintableTestData>
{
public:
    atTestsDataVector();
    atTestsDataVector(const gtString& xmlPath);

    void addTestsFromXMLFile(const gtString& xmlPath);
    bool IsXmlFileLoadedOK() const {return m_bIsXmlFileLoadedOK;};

private:
    bool m_bIsXmlFileLoadedOK;
};

#endif //__ATGPUDEBUGGINGTEST_H

