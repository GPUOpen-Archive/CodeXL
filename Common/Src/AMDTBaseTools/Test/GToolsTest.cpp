//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ GToolsTest.cpp ------------------------------

// C:
#include <iostream>

// TebekaTools:
#include <GRBaseTools/gtString.h>
#include <AMDTOSWrappers/Include/osTime.h>

int main(int argc, char* argv[])
{
    cout << "Starting TebekaTools.dll tests:\n";
    cout << "------------------------------:\n\n";

    osTime timeA;
    timeA.setFromCurrentTime();
    time_t foo = timeA.secondsFrom1970();


    // String initialization test:
    gtString strA = "This is the first string";
    gtString strB = "This is the second string";

    cout << strA.asCharArray() << endl;
    cout << strB.asCharArray() << endl;

    // String catenation test:
    gtString testCatenate;
    testCatenate.append(strA);
    testCatenate.append(strB);

    cout << "This is the catenated string: " << testCatenate.asCharArray() << endl;

    // Empty String test:
    gtString emptyStringA;
    gtString emptyStringB;
    gtString emptyCatenation;
    emptyCatenation.append(emptyStringA);
    emptyCatenation.append(emptyStringB);

    cout << "This is the empty catenated string: " << emptyCatenation.asCharArray() << endl;

    testCatenate.append(emptyCatenation);
    testCatenate.append("");


    // String operator= and copy constructor test:
    gtString strC = "bla";
    gtString strD = strC;
    gtString strE = strD;

    cout << "This is the operator = string: " << strE.asCharArray() << endl;

    cout.flush();

    return 0;
}

