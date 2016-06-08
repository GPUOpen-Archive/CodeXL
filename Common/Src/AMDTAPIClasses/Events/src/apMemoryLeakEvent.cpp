//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMemoryLeakEvent.cpp
///
//==================================================================================

//------------------------------ apMemoryLeakEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apMemoryLeakEvent.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::apMemoryLeakEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
apMemoryLeakEvent::apMemoryLeakEvent()
    : _leakType(AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK), _leakingObjectsContainerIndex(0), _numberOfLeakingTextures(-1), m_numberOfLeakingGLSamplers(-1),
      _numberOfLeakingRBOs(-1), _numberOfLeakingFBOs(-1), _numberOfLeakingVBOs(-1), _numberOfLeakingShadingPrograms(-1),
      _numberOfLeakingShaders(-1), m_numberOfLeakingPipelines(-1), _numberOfLeakingDisplayLists(-1),
      _numberOfLeakingCommandQueues(-1), m_numberOfLeakingEvents(-1), _numberOfLeakingCLBuffers(-1), _numberOfLeakingImages(-1),
      _numberOfLeakingCLSamplers(-1), _numberOfLeakingComputationPrograms(-1), _numberOfLeakingKernels(-1), _memoryLeakSize(0)
{

}


// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apMemoryLeakEvent::type() const
{
    return OS_TOBJ_ID_MEMORY_LEAK_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the memory leak attributes into the channel:
    ipcChannel << (gtInt32)_leakType;

    // Write the Leaking GL Contexts:
    gtInt32 amountOfLeakingGLContexts = (gtInt32)m_leakingGLContexts.size();
    ipcChannel << amountOfLeakingGLContexts;

    for (int i = 0; i < amountOfLeakingGLContexts; i++)
    {
        ipcChannel << (gtUInt32)m_leakingGLContexts[i];
    }

    // Write the Leaking PBuffers:
    gtInt32 amountOfLeakingPBuffers = (gtInt32)_leakingPBuffers.size();
    ipcChannel << amountOfLeakingPBuffers;

    for (int i = 0; i < amountOfLeakingPBuffers; i++)
    {
        ipcChannel << (gtUInt32)_leakingPBuffers[i];
    }

    // Write the Leaking Sync Objects:
    gtInt32 amountOfLeakingSyncObjects = (gtInt32)_leakingSyncObjects.size();
    ipcChannel << amountOfLeakingSyncObjects;

    for (int i = 0; i < amountOfLeakingSyncObjects; i++)
    {
        apLeakedSyncObjectID syncID = _leakingSyncObjects[i];
        ipcChannel << (gtInt32)syncID._syncID;
        ipcChannel << (gtUInt64)syncID._syncHandle;
    }

    // Write the Leaking CL Contexts:
    gtInt32 amountOfLeakingCLContexts = (gtInt32)m_leakingCLContexts.size();
    ipcChannel << amountOfLeakingCLContexts;

    for (int i = 0; i < amountOfLeakingCLContexts; i++)
    {
        ipcChannel << (gtUInt32)m_leakingCLContexts[i];
    }

    // Write the container object index:
    ipcChannel << (gtUInt32)_leakingObjectsContainerIndex;

    // Write the amount of leaks for each type:
    // GL context objects
    ipcChannel << (gtInt32)_numberOfLeakingTextures;
    ipcChannel << (gtInt32)m_numberOfLeakingGLSamplers;
    ipcChannel << (gtInt32)_numberOfLeakingRBOs;
    ipcChannel << (gtInt32)_numberOfLeakingFBOs;
    ipcChannel << (gtInt32)_numberOfLeakingVBOs;
    ipcChannel << (gtInt32)_numberOfLeakingShadingPrograms;
    ipcChannel << (gtInt32)_numberOfLeakingShaders;
    ipcChannel << (gtInt32)m_numberOfLeakingPipelines;
    ipcChannel << (gtInt32)_numberOfLeakingDisplayLists;
    // CL context objects
    ipcChannel << (gtInt32)_numberOfLeakingCommandQueues;
    ipcChannel << (gtInt32)m_numberOfLeakingEvents;
    ipcChannel << (gtInt32)_numberOfLeakingCLBuffers;
    ipcChannel << (gtInt32)_numberOfLeakingImages;
    ipcChannel << (gtInt32)_numberOfLeakingCLSamplers;
    ipcChannel << (gtInt32)_numberOfLeakingComputationPrograms;
    // CL program object
    ipcChannel << (gtInt32)_numberOfLeakingKernels;

    ipcChannel << (gtUInt64)_memoryLeakSize;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the memory leak attributes into the channel:
    gtInt32 leakTypeAsInt = 0;
    ipcChannel >> leakTypeAsInt;
    _leakType = (apMemoryLeakType)leakTypeAsInt;

    // Read the leak GL contexts:
    gtInt32 amountOfLeakingGLContexts = 0;
    ipcChannel >> amountOfLeakingGLContexts;

    for (int i = 0; i < amountOfLeakingGLContexts; i++)
    {
        gtUInt32 currentLeakContext = 0;
        ipcChannel >> currentLeakContext;
        m_leakingGLContexts.push_back((unsigned int)currentLeakContext);
    }

    // Read the leak PBuffers:
    gtInt32 amountOfLeakingPBuffers = 0;
    ipcChannel >> amountOfLeakingPBuffers;

    for (int i = 0; i < amountOfLeakingPBuffers; i++)
    {
        gtUInt32 currentLeakPBuffer = 0;
        ipcChannel >> currentLeakPBuffer;
        _leakingPBuffers.push_back((unsigned int)currentLeakPBuffer);
    }

    // Read the leak Sync Objects:
    gtInt32 amountOfLeakingSyncObjects = 0;
    ipcChannel >> amountOfLeakingSyncObjects;

    for (int i = 0; i < amountOfLeakingSyncObjects; i++)
    {
        apLeakedSyncObjectID currentSyncID;
        gtInt32 curretLeakSyncId = 0;
        ipcChannel >> curretLeakSyncId;
        currentSyncID._syncID = (int)curretLeakSyncId;

        gtUInt64 currentLeakSync = 0;
        ipcChannel >> currentLeakSync;
        currentSyncID._syncHandle = (oaGLSyncHandle)currentLeakSync;

        _leakingSyncObjects.push_back(currentSyncID);
    }

    // Read the leak CL contexts:
    gtInt32 amountOfLeakingCLContexts = 0;
    ipcChannel >> amountOfLeakingCLContexts;

    for (int i = 0; i < amountOfLeakingCLContexts; i++)
    {
        gtUInt32 currentLeakContext = 0;
        ipcChannel >> currentLeakContext;
        m_leakingCLContexts.push_back((unsigned int)currentLeakContext);
    }

    gtUInt32 leakingObjectContainerAsUInt32 = 0;
    ipcChannel >> leakingObjectContainerAsUInt32;
    _leakingObjectsContainerIndex = (unsigned int)leakingObjectContainerAsUInt32;

    // Read the amount of leaks for each type:
    // GL context objects:
    gtInt32 argAsInt32 = 0;
    ipcChannel >> argAsInt32;
    _numberOfLeakingTextures = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    m_numberOfLeakingGLSamplers = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    _numberOfLeakingRBOs = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    _numberOfLeakingFBOs = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    _numberOfLeakingVBOs = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    _numberOfLeakingShadingPrograms = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    _numberOfLeakingShaders = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    m_numberOfLeakingPipelines = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    _numberOfLeakingDisplayLists = (int)argAsInt32;

    // CL context objects:
    ipcChannel >> argAsInt32;
    _numberOfLeakingCommandQueues = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    m_numberOfLeakingEvents = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    _numberOfLeakingCLBuffers = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    _numberOfLeakingImages = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    _numberOfLeakingCLSamplers = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    _numberOfLeakingComputationPrograms = (int)argAsInt32;

    ipcChannel >> argAsInt32;
    _numberOfLeakingKernels = (int)argAsInt32;

    gtUInt64 argAsUInt64 = 0;
    ipcChannel >> argAsUInt64;
    _memoryLeakSize = (gtUInt64)argAsUInt64;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::~apMemoryLeakEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
apMemoryLeakEvent::~apMemoryLeakEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
apEvent::EventType apMemoryLeakEvent::eventType() const
{
    return apEvent::AP_MEMORY_LEAK;
}


// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
apEvent* apMemoryLeakEvent::clone() const
{
    apMemoryLeakEvent* pClone = new apMemoryLeakEvent;


    pClone->setLeakType(_leakType);

    switch (_leakType)
    {
        case AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK:
        {
            bool rc = true;
            int n = (int)m_leakingGLContexts.size();

            for (int i = 0; i < n; i++)
            {
                pClone->addLeakingRenderContext(m_leakingGLContexts[i]);
            }

            int amountOfPBuffers = (int)_leakingPBuffers.size();

            for (int i = 0; i < amountOfPBuffers; i++)
            {
                pClone->addLeakingPBuffer(_leakingPBuffers[i]);
            }

            int amountOfSyncObjects = (int)_leakingSyncObjects.size();

            for (int i = 0; i < amountOfSyncObjects; i++)
            {
                pClone->addLeakingSyncObject(_leakingSyncObjects[i]);
            }

            GT_ASSERT(rc);
        }
        break;

        case AP_INDEPENDENT_CL_ALLOCATED_OBJECT_LEAK:
        {
            bool rc = true;
            int n = (int)m_leakingCLContexts.size();

            for (int i = 0; i < n; i++)
            {
                pClone->addLeakingComputeContext(m_leakingCLContexts[i]);
            }

            GT_ASSERT(rc);
        }
        break;

        case AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK:
        {
            bool rc = pClone->setLeakingGLAllocatedObjects(_numberOfLeakingTextures, m_numberOfLeakingGLSamplers,
                                                           _numberOfLeakingRBOs, _numberOfLeakingFBOs,
                                                           _numberOfLeakingVBOs,
                                                           _numberOfLeakingShadingPrograms, _numberOfLeakingShaders,
                                                           m_numberOfLeakingPipelines,
                                                           _numberOfLeakingDisplayLists);

            unsigned int contextID = leakingObjectsRenderContextID();

            rc = pClone->setLeakingObjectsRenderContextID(contextID) && rc;
            GT_ASSERT(rc);
        }
        break;

        case AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK:
        {
            bool rc = pClone->setLeakingCLAllocatedObjects(_numberOfLeakingCommandQueues,
                                                           m_numberOfLeakingEvents,
                                                           _numberOfLeakingCLBuffers,
                                                           _numberOfLeakingImages,
                                                           _numberOfLeakingCLSamplers,
                                                           _numberOfLeakingComputationPrograms);

            unsigned int contextID = leakingObjectsComputeContextID();

            rc = pClone->setLeakingObjectsComputeContextID(contextID) && rc;
            GT_ASSERT(rc);
        }
        break;

        case AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK:
        {
            bool rc = pClone->setLeakingProgramAllocatedCLObjects(_numberOfLeakingKernels);
            unsigned int contextID = 0;
            unsigned int programID = 0;
            leakingObjectsComputationProgramID(contextID, programID);
            rc = pClone->setLeakingObjectsComputationProgramID(contextID, programID) && rc;
            GT_ASSERT(rc);
        }
        break;

        default:
        {
            // We added a new type of leak, but didn't implement it here:
            GT_ASSERT(false);
        }
        break;
    }

    // Add the memory leak size:
    pClone->addMemoryLeakSize(_memoryLeakSize);
    return pClone;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::setLeakType
// Description: Sets the leak type and resets the data
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
void apMemoryLeakEvent::setLeakType(apMemoryLeakType leakType)
{
    _leakType = leakType;

    // Reset the information and remove all irrelevant information:
    switch (leakType)
    {
        case AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK:
        case AP_INDEPENDENT_CL_ALLOCATED_OBJECT_LEAK:
        {
            m_leakingGLContexts.clear();
            _leakingPBuffers.clear();
            _leakingSyncObjects.clear();
            m_leakingCLContexts.clear();
            _leakingObjectsContainerIndex = 0;
            _numberOfLeakingTextures = -1;
            m_numberOfLeakingGLSamplers = -1;
            _numberOfLeakingRBOs = -1;
            _numberOfLeakingFBOs = -1;
            _numberOfLeakingVBOs = -1;
            _numberOfLeakingShadingPrograms = -1;
            _numberOfLeakingShaders = -1;
            m_numberOfLeakingPipelines = -1;
            _numberOfLeakingDisplayLists = -1;
            _numberOfLeakingCommandQueues = -1;
            m_numberOfLeakingEvents = -1;
            _numberOfLeakingCLBuffers = -1;
            _numberOfLeakingImages = -1;
            _numberOfLeakingCLSamplers = -1;
            _numberOfLeakingComputationPrograms = -1;
            _numberOfLeakingKernels = -1;
        }
        break;

        case AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK:
        {
            m_leakingGLContexts.clear();
            _leakingPBuffers.clear();
            _leakingSyncObjects.clear();
            m_leakingCLContexts.clear();
            _leakingObjectsContainerIndex = 0;
            _numberOfLeakingTextures = 0;
            m_numberOfLeakingGLSamplers = 0;
            _numberOfLeakingRBOs = 0;
            _numberOfLeakingFBOs = 0;
            _numberOfLeakingVBOs = 0;
            _numberOfLeakingShadingPrograms = 0;
            _numberOfLeakingShaders = 0;
            m_numberOfLeakingPipelines = 0;
            _numberOfLeakingDisplayLists = 0;
            _numberOfLeakingCommandQueues = -1;
            m_numberOfLeakingEvents = -1;
            _numberOfLeakingCLBuffers = -1;
            _numberOfLeakingImages = -1;
            _numberOfLeakingCLSamplers = -1;
            _numberOfLeakingComputationPrograms = -1;
            _numberOfLeakingKernels = -1;
        }
        break;

        case AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK:
        {
            m_leakingGLContexts.clear();
            _leakingPBuffers.clear();
            _leakingSyncObjects.clear();
            m_leakingCLContexts.clear();
            _leakingObjectsContainerIndex = 0;
            _numberOfLeakingTextures = -1;
            m_numberOfLeakingGLSamplers = -1;
            _numberOfLeakingRBOs = -1;
            _numberOfLeakingFBOs = -1;
            _numberOfLeakingVBOs = -1;
            _numberOfLeakingShadingPrograms = -1;
            _numberOfLeakingShaders = -1;
            m_numberOfLeakingPipelines = -1;
            _numberOfLeakingDisplayLists = -1;
            _numberOfLeakingCommandQueues = 0;
            m_numberOfLeakingEvents = 0;
            _numberOfLeakingCLBuffers = 0;
            _numberOfLeakingImages = 0;
            _numberOfLeakingCLSamplers = 0;
            _numberOfLeakingComputationPrograms = 0;
            _numberOfLeakingKernels = -1;
        }
        break;

        case AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK:
        {
            m_leakingGLContexts.clear();
            _leakingPBuffers.clear();
            _leakingSyncObjects.clear();
            m_leakingCLContexts.clear();
            _leakingObjectsContainerIndex = 0;
            _numberOfLeakingTextures = -1;
            m_numberOfLeakingGLSamplers = -1;
            _numberOfLeakingRBOs = -1;
            _numberOfLeakingFBOs = -1;
            _numberOfLeakingVBOs = -1;
            _numberOfLeakingShadingPrograms = -1;
            _numberOfLeakingShaders = -1;
            m_numberOfLeakingPipelines = -1;
            _numberOfLeakingDisplayLists = -1;
            _numberOfLeakingCommandQueues = -1;
            m_numberOfLeakingEvents = -1;
            _numberOfLeakingCLBuffers = -1;
            _numberOfLeakingImages = -1;
            _numberOfLeakingCLSamplers = -1;
            _numberOfLeakingComputationPrograms = -1;
            _numberOfLeakingKernels = 0;
        }
        break;

        default:
        {
            // We added a new type of leak, but didn't implement it here:
            GT_ASSERT(false);
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingRenderContextsAsString
// Description: Puts a list of leaking Render Context numbers into contextsList
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::leakingRenderContextsAsString(gtString& contextsList) const
{
    // If this isn't a context-independent object leak, this function is irrelevant.
    bool retVal = (_leakType == AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        contextsList.makeEmpty();
        int numberOfLeakingContexts = (int)m_leakingGLContexts.size();

        // Add the context numbers:
        for (int i = 0; i < numberOfLeakingContexts; i++)
        {
            contextsList.appendFormattedString(L", %d", m_leakingGLContexts[i]);
        }

        // Remove the comma and space before the first number:
        if (!contextsList.isEmpty())
        {
            contextsList.truncate(2, -1);
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingSyncObjectsAsString
// Description: Puts a list of leaking sync object handles into leakedSyncObjects
// Author:  AMD Developer Tools Team
// Date:        28/10/2009
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::leakingSyncObjectsAsString(gtString& leakedSyncObjects) const
{
    // If this isn't a context-independent object leak, this function is irrelevant.
    bool retVal = (_leakType == AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        leakedSyncObjects.makeEmpty();
        int numberOfLeakingSyncs = (int)_leakingSyncObjects.size();

        // Add the context numbers:
        for (int i = 0; i < numberOfLeakingSyncs; i++)
        {
            leakedSyncObjects.appendFormattedString(L", %d", _leakingSyncObjects[i]._syncID);
        }

        // Remove the comma and space before the first number:
        if (!leakedSyncObjects.isEmpty())
        {
            leakedSyncObjects.truncate(2, -1);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingRenderContexts
// Description: Return  a list of leaking Render Context numbers
// Author:  AMD Developer Tools Team
// Date:        28/10/2009
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::leakingRenderContexts(gtVector<unsigned int>& listOfRenderLeakingContext) const
{
    listOfRenderLeakingContext.clear();

    // If this isn't a context-independent object leak, this function is irrelevant.
    bool retVal = (_leakType == AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        int amountOfLeakingContexts = (int)m_leakingGLContexts.size();

        for (int i = 0; i < amountOfLeakingContexts; i++)
        {
            listOfRenderLeakingContext.push_back(m_leakingGLContexts[i]);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::numberOfLeakingRenderContexts
// Description: Returns the number of leaking render contexts
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
int apMemoryLeakEvent::numberOfLeakingRenderContexts() const
{
    int retVal = -1;

    if (_leakType == AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK)
    {
        retVal = (int)m_leakingGLContexts.size();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::addLeakingRenderContext
// Description: Adds a render context to the leaked contexts vector
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
void apMemoryLeakEvent::addLeakingRenderContext(unsigned int contextID)
{
    if (_leakType == AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK)
    {
        m_leakingGLContexts.push_back(contextID);
    }
}


// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingPBuffersAsString
// Description: Puts a list of leaking PBuffer numbers into buffersList
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::leakingPBuffersAsString(gtString& buffersList) const
{
    // If this isn't a context-independent object leak, this function is irrelevant.
    bool retVal = (_leakType == AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        buffersList.makeEmpty();
        int numberOfLeakingPBuffers = (int)_leakingPBuffers.size();

        // Add the buffer numbers:
        for (int i = 0; i < numberOfLeakingPBuffers; i++)
        {
            buffersList.appendFormattedString(L", %d", _leakingPBuffers[i]);
        }

        // Remove the comma and space before the first number:
        if (!buffersList.isEmpty())
        {
            buffersList.truncate(2, -1);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingPBuffers
// Description: Puts a list of leaking PBuffer numbers into buffersList
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::leakingPBuffers(gtVector<unsigned int>& leakingPBuffers) const
{
    leakingPBuffers.clear();

    // If this isn't a context-independent object leak, this function is irrelevant.
    bool retVal = (_leakType == AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        int amountOfLeakingPBOs = (int)_leakingPBuffers.size();

        for (int i = 0; i < amountOfLeakingPBOs; i++)
        {
            leakingPBuffers.push_back(_leakingPBuffers[i]);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::numberOfLeakingPBuffers
// Description: Returns the number of leaking Pixel Buffers
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
int apMemoryLeakEvent::numberOfLeakingPBuffers() const
{
    int retVal = -1;

    if (_leakType == AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK)
    {
        retVal = (int)_leakingPBuffers.size();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::addLeakingPBuffer
// Description: Adds a PBuffer to the leaked buffers vector
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
void apMemoryLeakEvent::addLeakingPBuffer(unsigned int bufferID)
{
    if (_leakType == AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK)
    {
        _leakingPBuffers.push_back(bufferID);
    }
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::addLeakingSyncObject
// Description: Adds a sync object to the leaked sync objects vector
// Author:  AMD Developer Tools Team
// Date:        28/10/2009
// ---------------------------------------------------------------------------
void apMemoryLeakEvent::addLeakingSyncObject(apLeakedSyncObjectID syncObjectID)
{
    if (_leakType == AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK)
    {
        _leakingSyncObjects.push_back(syncObjectID);
    }
}


// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::numberOfLeakingSyncObjects
// Description: Returns the number of leaking sync objects
// Author:  AMD Developer Tools Team
// Date:        28/10/2009
// ---------------------------------------------------------------------------
int apMemoryLeakEvent::numberOfLeakingSyncObjects() const
{
    int retVal = -1;

    if (_leakType == AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK)
    {
        retVal = (int)_leakingSyncObjects.size();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingSyncObjects
// Description: Puts a list of leaking sync objects handles into leakingSyncObjects
// Author:  AMD Developer Tools Team
// Date:        18/7/2009
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::leakingSyncObjects(gtVector<apLeakedSyncObjectID>& leakingSyncObjects) const
{
    leakingSyncObjects.clear();

    // If this isn't a context-independent object leak, this function is irrelevant.
    bool retVal = (_leakType == AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        int amountOfLeakingSyncs = (int)_leakingSyncObjects.size();

        for (int i = 0; i < amountOfLeakingSyncs; i++)
        {
            leakingSyncObjects.push_back(_leakingSyncObjects[i]);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingComputeContextsAsString
// Description: Puts a list of leaking Compute Context numbers into contextsList
// Author:  AMD Developer Tools Team
// Date:        23/07/2015
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::leakingComputeContextsAsString(gtString& contextsList) const
{
    // If this isn't a context-independent object leak, this function is irrelevant.
    bool retVal = (_leakType == AP_INDEPENDENT_CL_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        contextsList.makeEmpty();
        int numberOfLeakingContexts = (int)m_leakingCLContexts.size();

        // Add the context numbers:
        for (int i = 0; i < numberOfLeakingContexts; i++)
        {
            contextsList.appendFormattedString(L", %d", m_leakingCLContexts[i]);
        }

        // Remove the comma and space before the first number:
        if (!contextsList.isEmpty())
        {
            contextsList.truncate(2, -1);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingComputeContexts
// Description: Return  a list of leaking Compute Context numbers
// Author:  AMD Developer Tools Team
// Date:        23/07/2015
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::leakingComputeContexts(gtVector<unsigned int>& listOfComputeLeakingContext) const
{
    listOfComputeLeakingContext.clear();

    // If this isn't a context-independent object leak, this function is irrelevant.
    bool retVal = (_leakType == AP_INDEPENDENT_CL_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        int amountOfLeakingContexts = (int)m_leakingCLContexts.size();

        for (int i = 0; i < amountOfLeakingContexts; i++)
        {
            listOfComputeLeakingContext.push_back(m_leakingCLContexts[i]);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::numberOfLeakingComputeContexts
// Description: Returns the number of leaking compute contexts
// Author:  AMD Developer Tools Team
// Date:        23/07/2015
// ---------------------------------------------------------------------------
int apMemoryLeakEvent::numberOfLeakingComputeContexts() const
{
    int retVal = -1;

    if (_leakType == AP_INDEPENDENT_CL_ALLOCATED_OBJECT_LEAK)
    {
        retVal = (int)m_leakingCLContexts.size();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::addLeakingComputeContext
// Description: Adds a compute context to the leaked contexts vector
// Author:  AMD Developer Tools Team
// Date:        23/07/2015
// ---------------------------------------------------------------------------
void apMemoryLeakEvent::addLeakingComputeContext(unsigned int contextID)
{
    if (_leakType == AP_INDEPENDENT_CL_ALLOCATED_OBJECT_LEAK)
    {
        m_leakingCLContexts.push_back(contextID);
    }
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingAllocatedGLObjects
// Description: if this is a AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK, outputs the number of each
//              allocated object type which are leaking
// Arguments: leakTex - how many textures weren't deleted
//            leakRBO - how many render buffers weren't deleted
//            leakFBOs - how many frame buffers weren't deleted
//            leakVBO - how many vertex buffers weren't deleted
//            leakProg - how many shading programs weren't deleted
//            leakShad - how many shaders weren't deleted
//            leakList - how many display lists weren't deleted
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::leakingAllocatedGLObjects(int& leakTex, int& leakSmp, int& leakRBO, int& leakFBO, int& leakVBO, int& leakProg, int& leakShad, int& leakPipe, int& leakList) const
{
    bool retVal = (_leakType == AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        leakTex = _numberOfLeakingTextures;
        leakSmp = m_numberOfLeakingGLSamplers;
        leakRBO = _numberOfLeakingRBOs;
        leakFBO = _numberOfLeakingFBOs;
        leakVBO = _numberOfLeakingVBOs;
        leakProg = _numberOfLeakingShadingPrograms;
        leakShad = _numberOfLeakingShaders;
        leakPipe = m_numberOfLeakingPipelines;
        leakList = _numberOfLeakingDisplayLists;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::setLeakingGLAllocatedObjects
// Description: if this is a AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK, inputs the number of each
//              allocated object type which are leaking
// Arguments: leakTex - how many textures weren't deleted
//            leakRBO - how many render buffers weren't deleted
//            leakFBOs - how many frame buffers weren't deleted
//            leakVBO - how many vertex buffers weren't deleted
//            leakProg - how many shading programs weren't deleted
//            leakShad - how many shaders weren't deleted
//            leakList - how many display lists weren't deleted
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::setLeakingGLAllocatedObjects(int leakTex, int leakSmp, int leakRBO, int leakFBO, int leakVBO, int leakProg, int leakPipe, int leakShad, int leakList)
{
    bool retVal = (_leakType == AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        _numberOfLeakingTextures = leakTex;
        m_numberOfLeakingGLSamplers = leakSmp;
        _numberOfLeakingRBOs = leakRBO;
        _numberOfLeakingFBOs = leakFBO;
        _numberOfLeakingVBOs = leakVBO;
        _numberOfLeakingShadingPrograms = leakProg;
        _numberOfLeakingShaders = leakShad;
        m_numberOfLeakingPipelines = leakPipe;
        _numberOfLeakingDisplayLists = leakList;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingObjectsRenderContextID
// Description: Returns the context ID of the render context which was about
//              to be deleted with allocated objects still existing in it
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
unsigned int apMemoryLeakEvent::leakingObjectsRenderContextID() const
{
    unsigned int retVal = 0;

    if ((_leakType == AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK) && (m_leakingGLContexts.size() == 1))
    {
        retVal = m_leakingGLContexts[0];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::setLeakingObjectsRenderContextID
// Description: Input the context ID of the render context which was about
//              to be deleted with allocated objects still existing in it
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::setLeakingObjectsRenderContextID(unsigned int contextID)
{
    bool retVal = (_leakType == AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        // Erase any formerly set number:
        m_leakingGLContexts.clear();

        // Note the new number:
        m_leakingGLContexts.push_back(contextID);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingAllocatedCLObjects
// Description: if this is a AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK, outputs the number of each
//              allocated object type which are leaking
// Arguments: leakQue - how many command queues weren't deleted
//            leakBuf - how many buffers weren't deleted
//            leakImg - how many images weren't deleted
//            leakSmp - how many samplers weren't deleted
//            leakPrg - how many computation programs weren't deleted
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        23/6/2010
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::leakingAllocatedCLObjects(int& leakQue, int& leakEve, int& leakBuf, int& leakImg, int& leakSmp, int& leakPrg) const
{
    bool retVal = (_leakType == AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        leakQue = _numberOfLeakingCommandQueues;
        leakEve = m_numberOfLeakingEvents;
        leakBuf = _numberOfLeakingCLBuffers;
        leakImg = _numberOfLeakingImages;
        leakSmp = _numberOfLeakingCLSamplers;
        leakPrg = _numberOfLeakingComputationPrograms;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::setLeakingCLAllocatedObjects
// Description: if this is a AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK, inputs the number of each
//              allocated object type which are leaking
// Arguments: leakQue - how many command queues weren't deleted
//            leakBuf - how many buffers weren't deleted
//            leakImg - how many images weren't deleted
//            leakSmp - how many samplers weren't deleted
//            leakPrg - how many computation programs weren't deleted
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        23/6/2010
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::setLeakingCLAllocatedObjects(int leakQue, int leakEve, int leakBuf, int leakImg, int leakSmp, int leakPrg)
{
    bool retVal = (_leakType == AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        _numberOfLeakingCommandQueues = leakQue;
        m_numberOfLeakingEvents = leakEve;
        _numberOfLeakingCLBuffers = leakBuf;
        _numberOfLeakingImages = leakImg;
        _numberOfLeakingCLSamplers = leakSmp;
        _numberOfLeakingComputationPrograms = leakPrg;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingObjectsRenderContextID
// Description: Returns the context ID of the copmute context which was about
//              to be deleted with allocated objects still existing in it
// Author:  AMD Developer Tools Team
// Date:        23/6/2010
// ---------------------------------------------------------------------------
unsigned int apMemoryLeakEvent::leakingObjectsComputeContextID() const
{
    unsigned int retVal = 0;

    if ((_leakType == AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK) && (m_leakingCLContexts.size() == 1))
    {
        retVal = m_leakingCLContexts[0];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::setLeakingObjectsComputeContextID
// Description: Input the context ID of the compute context which was about
//              to be deleted with allocated objects still existing in it
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        23/6/2010
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::setLeakingObjectsComputeContextID(unsigned int contextID)
{
    bool retVal = (_leakType == AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        // Erase any formerly set number:
        m_leakingCLContexts.clear();

        // Note the new number:
        m_leakingCLContexts.push_back(contextID);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingProgramAllocatedCLObjects
// Description: if this is a AP_CL_Program_ALLOCATED_OBJECT_LEAK, outputs the number of each
//              allocated object type which are leaking
// Arguments: leakKer - how many kernels weren't deleted
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/7/2010
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::leakingProgramAllocatedCLObjects(int& leakKer) const
{
    bool retVal = (_leakType == AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        leakKer = _numberOfLeakingKernels;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::setLeakingProgramAllocatedCLObjects
// Description: if this is a AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK, inputs the number of each
//              allocated object type which are leaking
// Arguments: leakKer - how many kernels weren't deleted
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/7/2010
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::setLeakingProgramAllocatedCLObjects(int leakKer)
{
    bool retVal = (_leakType == AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        _numberOfLeakingKernels = leakKer;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::leakingObjectsComputationProgramID
// Description: Returns the context ID and program index of the computation program
//              which was about to be deleted with allocated objects still existing in it
// Author:  AMD Developer Tools Team
// Date:        28/7/2010
// ---------------------------------------------------------------------------
void apMemoryLeakEvent::leakingObjectsComputationProgramID(unsigned int& contextID, unsigned int& programID) const
{
    contextID = 0;
    programID = 0;

    GT_IF_WITH_ASSERT((_leakType == AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK) && (m_leakingCLContexts.size() == 1))
    {
        contextID = m_leakingCLContexts[0];
        programID = _leakingObjectsContainerIndex;
    }
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::setLeakingObjectsComputationProgramID
// Description: Input the context ID and program index of the computation program
//              which was about to be deleted with allocated objects still existing in it
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/7/2010
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::setLeakingObjectsComputationProgramID(unsigned int contextID, unsigned int programID)
{
    bool retVal = (_leakType == AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK);

    if (retVal)
    {
        // Erase any formerly set number:
        m_leakingCLContexts.clear();

        // Note the new number:
        m_leakingCLContexts.push_back(contextID);

        // Note the program index:
        _leakingObjectsContainerIndex = programID;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::openGLContextMemoryLeakAsString
// Description: Return a context memory leak as string
// Arguments:   gtString& memoryLeakStr
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/7/2010
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::openGLContextMemoryLeakAsString(gtString& memoryLeakStr) const
{
    bool retVal = true;

    // Calculate the total amount of leaking objects:
    int totalAmountOfLeakingObjects = _numberOfLeakingTextures + m_numberOfLeakingGLSamplers + _numberOfLeakingRBOs +
                                      _numberOfLeakingFBOs + _numberOfLeakingVBOs + _numberOfLeakingShadingPrograms +
                                      _numberOfLeakingShaders + m_numberOfLeakingPipelines + _numberOfLeakingDisplayLists;

    // Get the context ID:
    unsigned int contextID = leakingObjectsRenderContextID();

    // Empty the string:
    memoryLeakStr.makeEmpty();
    memoryLeakStr.append(AP_STR_MemoryLeakStart);
    memoryLeakStr.appendFormattedString(AP_STR_MemoryLeakAllocatedObjectsGLContext, totalAmountOfLeakingObjects, contextID);

    // Add the leak size to the message:
    if (_memoryLeakSize > 0)
    {
        gtString leakSizeAsString;
        leakSizeAsString.appendFormattedString(L"%llu", _memoryLeakSize);
        leakSizeAsString.addThousandSeperators();
        memoryLeakStr.appendFormattedString(AP_STR_MemoryLeakSizeMessage, leakSizeAsString.asCharArray());
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::openCLContextMemoryLeakAsString
// Description: Return a context memory leak as string
// Arguments:   gtString& memoryLeakStr
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/7/2010
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::openCLContextMemoryLeakAsString(gtString& memoryLeakStr) const
{
    bool retVal = true;

    // Calculate the total amount of leaking objects:
    int totalAmountOfLeakingObjects = _numberOfLeakingCommandQueues + m_numberOfLeakingEvents + _numberOfLeakingCLBuffers + _numberOfLeakingImages +
                                      _numberOfLeakingCLSamplers + _numberOfLeakingComputationPrograms;

    // Get the context ID:
    unsigned int contextID = leakingObjectsComputeContextID();

    // Empty the string:
    memoryLeakStr.makeEmpty();
    memoryLeakStr.append(AP_STR_MemoryLeakStart);
    memoryLeakStr.appendFormattedString(AP_STR_MemoryLeakAllocatedObjectsCLContext, totalAmountOfLeakingObjects, contextID);

    // Add the leak size to the message:
    if (_memoryLeakSize > 0)
    {
        gtString leakSizeAsString;
        leakSizeAsString.appendFormattedString(L"%llu", _memoryLeakSize);
        leakSizeAsString.addThousandSeperators();
        memoryLeakStr.appendFormattedString(AP_STR_MemoryLeakSizeMessage, leakSizeAsString.asCharArray());
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::openCLProgramMemoryLeakAsString
// Description: Return a program memory leak as string
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/7/2010
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::openCLProgramMemoryLeakAsString(gtString& memoryLeakStr) const
{
    bool retVal = true;

    // Calculate the total amount of leaking objects:
    int totalAmountOfLeakingObjects = _numberOfLeakingKernels;

    // Get the context ID:
    unsigned int contextID = 0;
    unsigned int programID = 0;
    leakingObjectsComputationProgramID(contextID, programID);

    // Empty the string:
    memoryLeakStr.makeEmpty();
    memoryLeakStr.append(AP_STR_MemoryLeakStart);
    memoryLeakStr.appendFormattedString(AP_STR_MemoryLeakAllocatedObjectsCLProgram, totalAmountOfLeakingObjects, contextID, programID + 1);

    // Add the leak size to the message:
    if (_memoryLeakSize > 0)
    {
        gtString leakSizeAsString;
        leakSizeAsString.appendFormattedString(L"%llu", _memoryLeakSize);
        leakSizeAsString.addThousandSeperators();
        memoryLeakStr.appendFormattedString(AP_STR_MemoryLeakSizeMessage, leakSizeAsString.asCharArray());
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::independentGLObjectMemoryLeakAsString
// Description: Return an independent memory leak as string
// Arguments:   gtString& memoryLeakStr
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/7/2010
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::independentGLObjectMemoryLeakAsString(gtString& memoryLeakStr) const
{
    bool retVal = true;

    // Get the amount of objects leaked:
    int numberOfLeakedRCs = (int)m_leakingGLContexts.size();
    int numberOfLeakedPBOs = (int)_leakingPBuffers.size();
    int numberOfLeakedSyncObjects = (int)_leakingSyncObjects.size();

    if ((numberOfLeakedRCs > 0) || (numberOfLeakedPBOs > 0) || (numberOfLeakedSyncObjects > 0))
    {
        // Add the start of the message:
        memoryLeakStr.append(AP_STR_MemoryLeakStart);

        // Add the RC info:
        if (numberOfLeakedRCs > 0)
        {
            // Build the leaking contexts string:
            gtString contextsLeakStr = (numberOfLeakedRCs > 1) ? AP_STR_MemoryLeakAppTerminationRenderContexts : AP_STR_MemoryLeakAppTerminationRenderContext;

            // Add the contexts string:
            gtString contextsList;
            bool rc = leakingRenderContextsAsString(contextsList);
            GT_IF_WITH_ASSERT(rc)
            {
                contextsLeakStr.append(AP_STR_Space);
                contextsLeakStr.append(contextsList);
            }

            memoryLeakStr.append(contextsLeakStr);
        }

        // Add the PB info:
        if (numberOfLeakedPBOs > 0)
        {
            // If we have a leak of two kinds or more, add the bridge here:
            if (numberOfLeakedRCs > 0)
            {
                if (numberOfLeakedSyncObjects == 0)
                {
                    memoryLeakStr.append(AP_STR_MemoryLeakAppTerminationLastBridge);
                }
                else // numberOfLeakedSyncObjects != 0
                {
                    memoryLeakStr.append(AP_STR_MemoryLeakAppTerminationMiddleBridge);
                }
            }

            // Build the leaking PBO string:
            gtString pboLeakStr = (numberOfLeakedRCs > 1) ? AP_STR_MemoryLeakAppTerminationPBOs : AP_STR_MemoryLeakAppTerminationPBO;

            // Add the contexts string:
            gtString pbosList;
            bool rc = leakingPBuffersAsString(pbosList);
            GT_IF_WITH_ASSERT(rc)
            {
                pboLeakStr.append(AP_STR_Space);
                pboLeakStr.append(pbosList);
            }

            memoryLeakStr.append(pboLeakStr);
        }

        // Add the Sync objects info:
        if (numberOfLeakedSyncObjects > 0)
        {
            // If we have a leak of two kinds or more, add the bridge here:
            if ((numberOfLeakedRCs > 0) || (numberOfLeakedPBOs > 0))
            {
                memoryLeakStr.append(AP_STR_MemoryLeakAppTerminationLastBridge);
            }

            // Build the leaking Sync string:
            gtString syncsLeakStr = (numberOfLeakedRCs > 1) ? AP_STR_MemoryLeakAppTerminationSyncObjects : AP_STR_MemoryLeakAppTerminationSyncObject;

            // Add the contexts string:
            gtString syncsList;
            bool rc = leakingSyncObjectsAsString(syncsList);
            GT_IF_WITH_ASSERT(rc)
            {
                syncsLeakStr.append(AP_STR_Space);
                syncsLeakStr.append(syncsList);
            }

            memoryLeakStr.append(syncsLeakStr);
        }

        // Add the appropriate end depending on how many leaked items we have:
        if ((numberOfLeakedPBOs + numberOfLeakedRCs + numberOfLeakedSyncObjects) <= 1)
        {
            memoryLeakStr.append(AP_STR_MemoryLeakAppTerminationEndSingular);
        }
        else // numberOfLeakedPBOs + numberOfLeakedRCs + numberOfLeakedSyncObjects > 1
        {
            memoryLeakStr.append(AP_STR_MemoryLeakAppTerminationEndPlural);
        }

        // Add the leak size to the message:
        if (_memoryLeakSize > 0)
        {
            gtString leakSizeAsString;
            leakSizeAsString.appendFormattedString(L"%llu", _memoryLeakSize);
            leakSizeAsString.addThousandSeperators();
            memoryLeakStr.appendFormattedString(AP_STR_MemoryLeakSizeMessage, leakSizeAsString.asCharArray());
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::independentCLObjectMemoryLeakAsString
// Description: Return an independent memory leak as string
// Arguments:   gtString& memoryLeakStr
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        23/07/2015
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::independentCLObjectMemoryLeakAsString(gtString& memoryLeakStr) const
{
    bool retVal = true;

    // Get the amount of objects leaked:
    int numberOfLeakedCtxs = (int)m_leakingCLContexts.size();

    if (numberOfLeakedCtxs > 0)
    {
        // Add the start of the message:
        memoryLeakStr.append(AP_STR_MemoryLeakStart);

        // Add the context info:
        if (numberOfLeakedCtxs > 0)
        {
            // Build the leaking contexts string:
            gtString contextsLeakStr = (numberOfLeakedCtxs > 1) ? AP_STR_MemoryLeakAppTerminationComputeContexts : AP_STR_MemoryLeakAppTerminationComputeContext;

            // Add the contexts string:
            gtString contextsList;
            bool rc = leakingComputeContextsAsString(contextsList);
            GT_IF_WITH_ASSERT(rc)
            {
                contextsLeakStr.append(AP_STR_Space);
                contextsLeakStr.append(contextsList);
            }

            memoryLeakStr.append(contextsLeakStr);
        }

        // Add the appropriate end depending on how many leaked items we have:
        if (numberOfLeakedCtxs <= 1)
        {
            memoryLeakStr.append(AP_STR_MemoryLeakAppTerminationEndSingular);
        }
        else // numberOfLeakedCtxs > 1
        {
            memoryLeakStr.append(AP_STR_MemoryLeakAppTerminationEndPlural);
        }

        // Add the leak size to the message:
        if (_memoryLeakSize > 0)
        {
            gtString leakSizeAsString;
            leakSizeAsString.appendFormattedString(L"%llu", _memoryLeakSize);
            leakSizeAsString.addThousandSeperators();
            memoryLeakStr.appendFormattedString(AP_STR_MemoryLeakSizeMessage, leakSizeAsString.asCharArray());
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::asString
// Description: Get the leak as string
// Arguments:   gtString& memoryLeakStr
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/7/2010
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::asString(gtString& memoryLeakStr) const
{
    bool retVal = false;

    // Check if the event contain a real memory leak:
    bool isMemoryLeakExists = memoryLeakExists();

    if (isMemoryLeakExists)
    {
        if (_leakType == AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK)
        {
            retVal = openGLContextMemoryLeakAsString(memoryLeakStr);
        }
        else if (_leakType == AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK)
        {
            retVal = openCLContextMemoryLeakAsString(memoryLeakStr);
        }
        else if (_leakType == AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK)
        {
            retVal = openCLProgramMemoryLeakAsString(memoryLeakStr);
        }
        else if (_leakType == AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK)
        {
            retVal = independentGLObjectMemoryLeakAsString(memoryLeakStr);
        }
        else if (_leakType == AP_INDEPENDENT_CL_ALLOCATED_OBJECT_LEAK)
        {
            retVal = independentCLObjectMemoryLeakAsString(memoryLeakStr);
        }
    }
    else
    {
        memoryLeakStr.append(AP_STR_MemoryLeakNone);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMemoryLeakEvent::memoryLeakExists
// Description: Returns true iff this event represents a deleted /unloaded object
//              that contains memory leaks.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/7/2010
// ---------------------------------------------------------------------------
bool apMemoryLeakEvent::memoryLeakExists() const
{
    // Check if memory leak exist according to memory leak type:
    bool retVal = false;

    switch (_leakType)
    {
        case AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK:
        {
            int numberOfLeakedRCs = (int)m_leakingGLContexts.size();
            int numberOfLeakedPBOs = (int)_leakingPBuffers.size();
            int numberOfLeakedSyncObjects = (int)_leakingSyncObjects.size();

            if ((numberOfLeakedRCs > 0) || (numberOfLeakedPBOs > 0) || (numberOfLeakedSyncObjects > 0))
            {
                retVal = true;
            }
        }
        break;

        case AP_INDEPENDENT_CL_ALLOCATED_OBJECT_LEAK:
        {
            int numberOfLeakedCtxs = (int)m_leakingCLContexts.size();

            if (0 < numberOfLeakedCtxs)
            {
                retVal = true;
            }
        }
        break;

        case AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK:
        {
            if ((_numberOfLeakingTextures > 0) || (0 < m_numberOfLeakingGLSamplers) || (_numberOfLeakingRBOs > 0) || (_numberOfLeakingFBOs > 0) ||
                (_numberOfLeakingVBOs > 0) || (_numberOfLeakingShadingPrograms > 0) || (_numberOfLeakingShaders > 0) || (0 < m_numberOfLeakingPipelines) || (_numberOfLeakingDisplayLists > 0))
            {
                retVal = true;
            }
        }
        break;

        case AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK:
        {
            if ((_numberOfLeakingCommandQueues > 0) || (0 < m_numberOfLeakingEvents) || (_numberOfLeakingCLBuffers > 0) || (_numberOfLeakingImages > 0) ||
                (_numberOfLeakingCLSamplers > 0) || (_numberOfLeakingComputationPrograms > 0))
            {
                retVal = true;
            }
        }
        break;

        case AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK:
        {
            if (_numberOfLeakingKernels > 0)
            {
                retVal = true;
            }
        }
        break;

        default:
        {
            GT_ASSERT_EX(false, L"Unknown leak type");
            break;
        }
    }

    return retVal;
}
