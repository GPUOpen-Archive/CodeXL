// AMDTAnalyzerBackendTester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// Standard C++:
#include <iostream>
#include <string>

#include <gtest/gtest.h>

// ---------------------------------------------------------------------------
// Name:        main
// Description: Main function for the automatic tester executable
// Author:      Amit Ben-Moshe
// Date:        Mar-4, 2015
// ---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    // Run the test cases
    int rc = RUN_ALL_TESTS();

    // Wait for the user to accept the output.
    int __z;
    std::cin >> __z;

    return rc;
}



