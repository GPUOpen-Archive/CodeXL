//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apRenderPrimitivesStatistics.cpp
///
//==================================================================================

//------------------------------ apRenderPrimitivesStatistics.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osStdLibIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apRenderPrimitivesStatistics.h>


// ---------------------------------------------------------------------------
// Name:        apRenderPrimitivesStatistics::apRenderPrimitivesStatistics
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        14/5/2009
// ---------------------------------------------------------------------------
apRenderPrimitivesStatistics::apRenderPrimitivesStatistics()
{
    clearStatistics();
}

// ---------------------------------------------------------------------------
// Name:        apRenderPrimitivesStatistics::~apRenderPrimitivesStatistics
// Description: Destructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        14/5/2009
// ---------------------------------------------------------------------------
apRenderPrimitivesStatistics::~apRenderPrimitivesStatistics()
{

}

// ---------------------------------------------------------------------------
// Name:        apRenderPrimitivesStatistics::type
// Description: Returns my transferable object type.
// Return Val: osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        14/5/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apRenderPrimitivesStatistics::type() const
{
    return OS_TOBJ_ID_FUNC_CALL_STATISTICS;
}

// ---------------------------------------------------------------------------
// Name:        apRenderPrimitivesStatistics::writeSelfIntoChannel
// Description: Writes my content into a channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/5/2009
// ---------------------------------------------------------------------------
bool apRenderPrimitivesStatistics::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the size of the map:
    gtUInt64 amountOfMappings = (gtUInt64)_numOfVerticesToNumOfBatchesMap.size();
    ipcChannel << amountOfMappings;

    gtMap<int, gtUInt64>::const_iterator iter = _numOfVerticesToNumOfBatchesMap.begin();
    gtMap<int, gtUInt64>::const_iterator endIter = _numOfVerticesToNumOfBatchesMap.end();

    for (; iter != endIter; iter++)
    {
        // Write the current amount of vertices & amount of batches:
        gtInt32 amountOfVertices = (*iter).first;
        gtUInt64 amountOfBatches = (*iter).second;
        ipcChannel << amountOfVertices;
        ipcChannel << amountOfBatches;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apRenderPrimitivesStatistics::readSelfFromChannel
// Description: Reads my content from an IPC channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/5/2009
// ---------------------------------------------------------------------------
bool apRenderPrimitivesStatistics::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the amount of mappings:
    gtUInt64 amountOfMappings = 0;
    ipcChannel >> amountOfMappings;

    // Read the batches counters:
    for (gtUInt64 i = 0; i < amountOfMappings; i++)
    {
        // Read the values:
        gtInt32 amountOfVertices = 0;
        gtUInt64 amountOfBatches = 0;
        ipcChannel >> amountOfVertices;
        ipcChannel >> amountOfBatches;

        // Add to map:
        _numOfVerticesToNumOfBatchesMap[amountOfVertices] = amountOfBatches;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apRenderPrimitivesStatistics::clearStatistics
// Description: Is called when the user asks to clear statistics
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void apRenderPrimitivesStatistics::clearStatistics()
{
    // Clear the amounts of batches from the map:
    gtMap<int, gtUInt64>::iterator iter = _numOfVerticesToNumOfBatchesMap.begin();
    gtMap<int, gtUInt64>::iterator endIter = _numOfVerticesToNumOfBatchesMap.end();

    for (; iter != endIter; iter++)
    {
        // Write the current amount of vertices & amount of batches:
        int amountOfVertices = (*iter).first;
        _numOfVerticesToNumOfBatchesMap[amountOfVertices] = 0;
    }

}

// ---------------------------------------------------------------------------
// Name:        apRenderPrimitivesStatistics::addBatchStatistics
// Description: Add batch to the batch statistics
// Arguments: int numOfVertices
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void apRenderPrimitivesStatistics::addBatchStatistics(int numOfVertices)
{
    if (numOfVertices > 0)
    {
        // Check if this values exists:
        gtMap<int, gtUInt64>::iterator iter = _numOfVerticesToNumOfBatchesMap.find(numOfVertices);

        if (iter == _numOfVerticesToNumOfBatchesMap.end())
        {
            _numOfVerticesToNumOfBatchesMap[numOfVertices] = 1;
        }
        else
        {
            _numOfVerticesToNumOfBatchesMap[numOfVertices]++;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        apRenderPrimitivesStatistics::getBatchesAmount
// Description: Return amount of batches for amount of vertices
// Arguments: int amountOfVertices
// Return Val: gtUInt64
// Author:  AMD Developer Tools Team
// Date:        18/5/2009
// ---------------------------------------------------------------------------
gtUInt64 apRenderPrimitivesStatistics::getBatchesAmount(int amountOfVertices) const
{
    gtUInt64 retVal = 0;


    // Check if this values exists:
    gtMap<int, gtUInt64>::const_iterator iter = _numOfVerticesToNumOfBatchesMap.find(amountOfVertices);

    if (iter != _numOfVerticesToNumOfBatchesMap.end())
    {
        retVal = (*iter).second;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apRenderPrimitivesStatistics::getMaxVerticesAmount
// Description: Return the maximum amount of vertices
// Return Val: int
// Author:  AMD Developer Tools Team
// Date:        19/5/2009
// ---------------------------------------------------------------------------
int apRenderPrimitivesStatistics::getMaxVerticesAmount() const
{
    int maxVerticesItem = 0;

    // Loop through the map and find the maximum item:
    gtMap<int, gtUInt64>::const_iterator iter = _numOfVerticesToNumOfBatchesMap.begin();
    gtMap<int, gtUInt64>::const_iterator iterEnd = _numOfVerticesToNumOfBatchesMap.end();

    for (; iter != iterEnd; iter++)
    {
        int currentVerticesAmount = (*iter).first;

        if (currentVerticesAmount > maxVerticesItem)
        {
            maxVerticesItem = currentVerticesAmount;
        }
    }

    return maxVerticesItem;
}



// ---------------------------------------------------------------------------
// Name:        apRenderPrimitivesStatistics::getAmountOfItems
// Description: Return the amount of items
// Return Val: int
// Author:  AMD Developer Tools Team
// Date:        19/5/2009
// ---------------------------------------------------------------------------
int apRenderPrimitivesStatistics::getAmountOfItems() const
{
    int amountOfItems = 0;

    // Loop through the map and find the maximum item:
    gtMap<int, gtUInt64>::const_iterator iter = _numOfVerticesToNumOfBatchesMap.begin();
    gtMap<int, gtUInt64>::const_iterator iterEnd = _numOfVerticesToNumOfBatchesMap.end();

    for (; iter != iterEnd; iter++)
    {
        gtUInt64 currentBatchesAmount = (*iter).second;

        if (currentBatchesAmount > 0)
        {
            amountOfItems++;
        }
    }

    return amountOfItems;
}
