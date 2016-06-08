//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPipeExecutor.cpp
///
//=====================================================================

//------------------------------ osPipeExecutor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osPipeExecutor.h>


// ---------------------------------------------------------------------------
// Name:        osPipeExecutor::osPipeExecutor
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        29/10/2008
// ---------------------------------------------------------------------------
osPipeExecutor::osPipeExecutor()
{

}

// ---------------------------------------------------------------------------
// Name:        osPipeExecutor::~osPipeExecutor
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        29/10/2008
// ---------------------------------------------------------------------------
osPipeExecutor::~osPipeExecutor()
{

}

// ---------------------------------------------------------------------------
// Name:        osPipeExecutor::executeCommand
// Description: Executes command and puts its output in the output parameter
//              note if the command ran but had problems the retval here will
//              be true, but the output might not be what was needed.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/10/2008
// ---------------------------------------------------------------------------
bool osPipeExecutor::executeCommand(const gtString& command, gtString& output)
{
    bool retVal = false;

    output.makeEmpty();

    FILE* pFile = ::popen(command.asASCIICharArray(), "r");

    gtASCIIString outputAscii;

    if (pFile != nullptr)
    {
        retVal = true;

        char buffer[100];

        // Add the results to the output string (note this works one line at a time):
        // We cannot recognize any pipe read errors since they "look" like line endings.
        // To make sure we got all the info we need the expected number of output lines,
        // which we don't know here:
        while (::fgets(buffer, sizeof(buffer), pFile) != nullptr)
        {
            outputAscii.append(buffer);
        }

        // Close the pipe
        ::pclose(pFile);


        // TO_DO: Unicode: check if this copy is necessary.
        // Copy the ASCII string to unicode:
        output.fromASCIIString(outputAscii.asCharArray());
    }

    return retVal;
}
