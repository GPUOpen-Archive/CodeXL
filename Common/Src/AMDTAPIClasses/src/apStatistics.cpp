//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apStatistics.cpp
///
//==================================================================================

//------------------------------ apStatistics.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osStdLibIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apFunctionCallStatistics.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTAPIClasses/Include/apStatistics.h>


// ---------------------------------------------------------------------------
// Name:        apStatistics::apStatistics
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
apStatistics::apStatistics(): _amountOfFullFrames(0), _amountOfFunctionCallsInFullFrames(0)
{

}


// ---------------------------------------------------------------------------
// Name:        apStatistics::~apStatistics
// Description: Destructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        6/8/2008
// ---------------------------------------------------------------------------
apStatistics::~apStatistics()
{

}

// ---------------------------------------------------------------------------
// Name:        apStatistics::type
// Description: Returns my transferable object type.
// Return Val: osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
osTransferableObjectType apStatistics::type() const
{
    return OS_TOBJ_ID_FUNC_CALL_STATISTICS;
}

// ---------------------------------------------------------------------------
// Name:        apStatistics::writeSelfIntoChannel
// Description: Writes my content into a channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
bool apStatistics::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the amount of full frames:
    ipcChannel << _amountOfFullFrames;

    // Write the number of function called in full frames:
    ipcChannel << _amountOfFunctionCallsInFullFrames;

    // Write the amount of functions:
    gtInt64 amountOfFunctions = (gtInt64)_functionStatisticsCallsVec.size();

    ipcChannel << amountOfFunctions;

    // Write each of the functions:
    for (int i = 0; i < amountOfFunctions; i++)
    {
        apFunctionCallStatistics* pFunctionStatistics = _functionStatisticsCallsVec[i];
        GT_IF_WITH_ASSERT(pFunctionStatistics != NULL)
        {
            bool rc = pFunctionStatistics->writeSelfIntoChannel(ipcChannel);
            retVal = retVal && rc;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apStatistics::readSelfFromChannel
// Description: Reads my content from an IPC channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/1/2006
// ---------------------------------------------------------------------------
bool apStatistics::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the amount of full frames:
    ipcChannel >> _amountOfFullFrames;

    // Read the number of function called in full frames:
    ipcChannel >> _amountOfFunctionCallsInFullFrames;

    // Read amount of functions:
    gtInt64 amountOfFunctions = 0;
    ipcChannel >> amountOfFunctions;

    // Destroy the currently function statistics vector:
    _functionStatisticsCallsVec.deleteElementsAndClear();

    for (int i = 0; i < amountOfFunctions; i++)
    {
        // Allocate a new function statistics object:
        apFunctionCallStatistics* pNewStatistics = new apFunctionCallStatistics();


        // Read the new function statistics:
        retVal = retVal && pNewStatistics->readSelfFromChannel(ipcChannel);

        // Add the function statistics to the vector of statistics:
        _functionStatisticsCallsVec.push_back(pNewStatistics);
    }

    return retVal;
}




// ---------------------------------------------------------------------------
// Name:        apStatistics::clearFunctionCallsStatistics
// Description: Is called when the user asks to clear statistics
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        15/7/2008
// ---------------------------------------------------------------------------
void apStatistics::clearFunctionCallsStatistics()
{
    // Clear the function call statistics vector:
    _functionStatisticsCallsVec.deleteElementsAndClear();
}


// ---------------------------------------------------------------------------
// Name:        apStatistics::getFunctionCallStatistics
// Description: Returns a function call statistics by id
// Arguments: int functionCallIndex
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/7/2008
// ---------------------------------------------------------------------------
bool apStatistics::getFunctionCallStatistics(int functionCallIndex, const apFunctionCallStatistics*& pFunctionStatistics) const
{
    bool retVal = false;
    pFunctionStatistics = NULL;

    // Check the function id range (sanity check):
    if ((functionCallIndex < (int)_functionStatisticsCallsVec.size()) && (functionCallIndex >= 0))
    {
        pFunctionStatistics = _functionStatisticsCallsVec[functionCallIndex];
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apStatistics::amountOfFunctionCallsStatistics
// Description: Returns number of logged function calls in the statistics object
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        15/7/2008
// ---------------------------------------------------------------------------
int apStatistics::amountOfFunctionCallsStatistics() const
{
    int amountOfFunctions = (int)_functionStatisticsCallsVec.size();
    return amountOfFunctions;
}


// ---------------------------------------------------------------------------
// Name:        apStatistics::addFunctionCallStatistics
// Description: Add a function call statistics to the vercor of function statistics
// Arguments: apFunctionCallStatistics* pFunctionCallStatistics
// Author:  AMD Developer Tools Team
// Date:        17/7/2008
// ---------------------------------------------------------------------------
void apStatistics::addFunctionCallStatistics(apFunctionCallStatistics* pFunctionCallStatistics)
{
    GT_IF_WITH_ASSERT(pFunctionCallStatistics != NULL)
    {
        _functionStatisticsCallsVec.push_back(pFunctionCallStatistics);
    }
}

// ---------------------------------------------------------------------------
// Name:        apStatistics::amountOfFullFrames
// Description:
// Return Val: int
// Author:  AMD Developer Tools Team
// Date:        23/7/2008
// ---------------------------------------------------------------------------
gtUInt64 apStatistics::amountOfFullFrames() const
{
    return _amountOfFullFrames;
}

// ---------------------------------------------------------------------------
// Name:        apStatistics::_amountOfFunctionCallsInFullFrames
// Description: Returns the number of functions called in full frames
// Return Val: int
// Author:  AMD Developer Tools Team
// Date:        23/7/2008
// ---------------------------------------------------------------------------
gtUInt64 apStatistics::amountOfFunctionCallsInFullFrames() const
{
    return _amountOfFunctionCallsInFullFrames;
}
