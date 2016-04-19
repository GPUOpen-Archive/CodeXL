//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afPluginConnectionManager.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>

// Static member initializations:
afPluginConnectionManager* afPluginConnectionManager::m_spMySingleInstance = nullptr;


// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::instance
// Description: Returns my single instance. Creates it if it doesn't exist yet
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
afPluginConnectionManager& afPluginConnectionManager::instance()
{
    if (nullptr == m_spMySingleInstance)
    {
        m_spMySingleInstance = new afPluginConnectionManager;

    }

    return *m_spMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::afPluginConnectionManager
// Description: Constructor
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
afPluginConnectionManager::afPluginConnectionManager()
{

}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::~afPluginConnectionManager
// Description: Destructor
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
afPluginConnectionManager::~afPluginConnectionManager()
{
    // Verify everyone unregistered from me:
    GT_ASSERT(0 == m_breakpointManagers.size());
    GT_ASSERT(0 == m_runModeManagers.size());
}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::registerBreakpointManager
// Description: Adds a breakpoint manager
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
void afPluginConnectionManager::registerBreakpointManager(afIBreakpointManager* piManager)
{
    GT_ASSERT(nullptr != piManager);

    // See if this manager was already registered:
    int numberOfManagers = (int)m_breakpointManagers.size();
    bool shouldAdd = (nullptr != piManager);

    for (int i = 0; i < numberOfManagers; i++)
    {
        if (piManager == m_breakpointManagers[i])
        {
            shouldAdd = false;
            break;
        }
    }

    // If this is a new manager:
    GT_IF_WITH_ASSERT(shouldAdd)
    {
        m_breakpointManagers.push_back(piManager);
    }
}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::unregisterBreakpointManager
// Description: Removes a breakpoint manager
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
void afPluginConnectionManager::unregisterBreakpointManager(afIBreakpointManager* piManager)
{
    GT_ASSERT(nullptr != piManager);

    // See if this manager was already registered:
    int numberOfManagers = (int)m_breakpointManagers.size();
    bool foundManager = false;

    for (int i = 0; i < numberOfManagers; i++)
    {
        if (foundManager)
        {
            // This can't happen on the first iteration. Move the manager back over the deleted on:
            m_breakpointManagers[i - 1] = m_breakpointManagers[i];
        }
        else if (piManager == m_breakpointManagers[i])
        {
            foundManager = true;
        }
    }

    GT_IF_WITH_ASSERT(foundManager)
    {
        // Remove the extra pointer:
        m_breakpointManagers.pop_back();
    }
}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::areBreakpointsSet
// Description: Quickly queries whether breakpoints are set in any of the managers
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
bool afPluginConnectionManager::areBreakpointsSet()
{
    bool retVal = false;

    // Go over all the managers:
    int numberOfManagers = (int)m_breakpointManagers.size();

    for (int i = 0; i < numberOfManagers; i++)
    {
        afIBreakpointManager* pCurrentManager = m_breakpointManagers[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentManager)
        {
            // If this breakpoint has any breakpoints:
            if (0 < pCurrentManager->numberOfBreakpoints())
            {
                retVal = true;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::getSetBreakpoints
// Description: Gets all the currently set breakpoints
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
void afPluginConnectionManager::getSetBreakpoints(gtVector<apBreakPoint*>& setBreakpoints)
{
    setBreakpoints.clear();

    // Go over all the managers:
    int numberOfManagers = (int)m_breakpointManagers.size();

    for (int i = 0; i < numberOfManagers; i++)
    {
        afIBreakpointManager* pCurrentManager = m_breakpointManagers[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentManager)
        {
            // Get each breakpoint in this manager:
            int currentNumberOfBreakpoints = pCurrentManager->numberOfBreakpoints();

            for (int j = 0; j < currentNumberOfBreakpoints; j++)
            {
                apBreakPoint* pCurrentBP = pCurrentManager->getBreakpoint(j);
                GT_IF_WITH_ASSERT(nullptr != pCurrentBP)
                {
                    // Add it to the vector:
                    setBreakpoints.push_back(pCurrentBP);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::setBreakpoint
// Description: Tries to find an appropriate manager and sets a breakpoint in it.
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
bool afPluginConnectionManager::setBreakpoint(const apBreakPoint& breakpoint)
{
    bool retVal = false;

    // Go over all the managers:
    int numberOfManagers = (int)m_breakpointManagers.size();

    for (int i = 0; i < numberOfManagers; i++)
    {
        afIBreakpointManager* pCurrentManager = m_breakpointManagers[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentManager)
        {
            // See if the breakpoint type matches this manager:
            if (pCurrentManager->isBreakpointSupported(breakpoint))
            {
                retVal = pCurrentManager->setBreakpoint(breakpoint);

                // Only allow one manager to control each breakpoint type:
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::removeBreakpoint
// Description: Tries to find an appropriate manager and removes a breakpoint from it.
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
bool afPluginConnectionManager::removeBreakpoint(const apBreakPoint& breakpoint)
{
    bool retVal = false;

    // Go over all the managers:
    int numberOfManagers = (int)m_breakpointManagers.size();

    for (int i = 0; i < numberOfManagers; i++)
    {
        afIBreakpointManager* pCurrentManager = m_breakpointManagers[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentManager)
        {
            // See if the breakpoint type matches this manager:
            if (pCurrentManager->isBreakpointSupported(breakpoint))
            {
                retVal = pCurrentManager->removeBreakpoint(breakpoint);

                // Only allow one manager to control each breakpoint type:
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        breakpointTypeFromSourcePath
// Description: See which breakpoint type matches a given source file and return it:
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
osTransferableObjectType afPluginConnectionManager::breakpointTypeFromSourcePath(const osFilePath& sourceFilePath)
{
    osTransferableObjectType retVal = OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT;

    // Go over all the managers:
    int numberOfManagers = (int)m_breakpointManagers.size();

    for (int i = 0; i < numberOfManagers; i++)
    {
        afIBreakpointManager* pCurrentManager = m_breakpointManagers[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentManager)
        {
            // If the manager knows this file path:
            osTransferableObjectType currentBreakpointType = pCurrentManager->breakpointTypeFromSourcePath(sourceFilePath);

            if (OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES != currentBreakpointType)
            {
                retVal = currentBreakpointType;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::doesBreakpointMatchFile
// Description: Returns true iff the manager controlling this breakpoint type
//              says that it matches the file.
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
bool afPluginConnectionManager::doesBreakpointMatchFile(const apBreakPoint& breakpoint, const osFilePath& sourceFilePath)
{
    bool retVal = false;

    // Go over all the managers:
    int numberOfManagers = (int)m_breakpointManagers.size();

    for (int i = 0; i < numberOfManagers; i++)
    {
        afIBreakpointManager* pCurrentManager = m_breakpointManagers[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentManager)
        {
            // See if the breakpoint type matches this manager:
            if (pCurrentManager->isBreakpointSupported(breakpoint))
            {
                retVal = pCurrentManager->doesBreakpointMatchFile(breakpoint, sourceFilePath);

                // Only allow one manager to control each breakpoint type:
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::registerRunModeManager
// Description: Adds a run mode manager
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
void afPluginConnectionManager::registerRunModeManager(afIRunModeManager* piManager)
{
    GT_ASSERT(nullptr != piManager);

    // See if this manager was already registered:
    int numberOfManagers = (int)m_runModeManagers.size();
    bool shouldAdd = (nullptr != piManager);

    for (int i = 0; i < numberOfManagers; i++)
    {
        if (piManager == m_runModeManagers[i])
        {
            shouldAdd = false;
            break;
        }
    }

    // If this is a new manager:
    GT_IF_WITH_ASSERT(shouldAdd)
    {
        m_runModeManagers.push_back(piManager);
    }
}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::unregisterRunModeManager
// Description: Removes a run mode manager
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
void afPluginConnectionManager::unregisterRunModeManager(afIRunModeManager* piManager)
{
    GT_ASSERT(nullptr != piManager);

    // See if this manager was already registered:
    int numberOfManagers = (int)m_runModeManagers.size();
    bool foundManager = false;

    for (int i = 0; i < numberOfManagers; i++)
    {
        if (foundManager)
        {
            // This can't happen on the first iteration. Move the manager back over the deleted on:
            m_runModeManagers[i - 1] = m_runModeManagers[i];
        }
        else if (piManager == m_runModeManagers[i])
        {
            foundManager = true;
        }
    }

    GT_IF_WITH_ASSERT(foundManager)
    {
        // Remove the extra pointer:
        m_runModeManagers.pop_back();
    }
}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::getCurrentRunModeMask
// Description: Gets the run modes mask from all plugins
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
afRunModes afPluginConnectionManager::getCurrentRunModeMask()
{
    afRunModes retVal = 0;

    // Go over all the managers:
    int numberOfManagers = (int)m_runModeManagers.size();

    for (int i = 0; i < numberOfManagers; i++)
    {
        afIRunModeManager* pCurrentManager = m_runModeManagers[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentManager)
        {
            retVal |= pCurrentManager->getCurrentRunModeMask();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::stopCurrentRun
// Description: Attempts to stop any existing run in any plugin
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
bool afPluginConnectionManager::stopCurrentRun(bool stopAndExit)
{
    bool retVal = true;

    // Go over all the managers:
    int numberOfManagers = (int)m_runModeManagers.size();

    for (int i = 0; i < numberOfManagers; i++)
    {
        afIRunModeManager* pCurrentManager = m_runModeManagers[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentManager)
        {
            // If this manager can change the run mode:
            bool canManagerStopRun = pCurrentManager->canStopCurrentRun();

            if (canManagerStopRun)
            {
                // Try to stop the current run:
                pCurrentManager->setStopAndExit(stopAndExit);
                bool stoppedSuccessfully = pCurrentManager->stopCurrentRun();

                // Report success only if all the current sessions were stopped:
                retVal = retVal && stoppedSuccessfully;
            }
        }
    }


    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::getExceptionEventDetails
// Description: Queries the run mode managers which one generated the exception event and gets the details from that one
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
void afPluginConnectionManager::getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce)
{
    // Go over all the managers:
    int numberOfManagers = (int)m_runModeManagers.size();

    for (int i = 0; i < numberOfManagers; i++)
    {
        afIRunModeManager* pCurrentManager = m_runModeManagers[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentManager)
        {
            bool rc = pCurrentManager->getExceptionEventDetails(exceptionEve, exceptionCallStack, openCLEnglineLoaded, openGLEnglineLoaded, kernelDebuggingEnteredAtLeastOnce);

            if (rc)
            {
                break;
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afPluginConnectionManager::bindProgramToBreakpoints
// Description: Bind a program to the existing breakpoints
// Arguments:   const apOpenCLProgramCreatedEvent& eve
// Author:      Sigal Algranaty
// Date:        19/7/2012
// ---------------------------------------------------------------------------
bool afPluginConnectionManager::bindProgramToBreakpoints(int contextId, int programIndex, bool unbind)
{
    bool retVal = true;

    // Go over all the managers:
    int numberOfManagers = (int)m_breakpointManagers.size();

    for (int i = 0; i < numberOfManagers; i++)
    {
        afIBreakpointManager* pCurrentManager = m_breakpointManagers[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentManager)
        {
            retVal = retVal && pCurrentManager->bindProgramToBreakpoints(contextId, programIndex, unbind);
        }
    }

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        onKernelSourceCodeUpdate
/// \brief Description: Is called when a program source code is updated
/// \param[in]          eve - the source code update event
/// \return void
/// -----------------------------------------------------------------------------------------------
bool afPluginConnectionManager::onKernelSourceCodeUpdate(const apKernelSourceBreakpointsUpdatedEvent& eve)
{
    bool retVal = true;

    // Go over all the managers:
    int numberOfManagers = (int)m_breakpointManagers.size();

    for (int i = 0; i < numberOfManagers; i++)
    {
        afIBreakpointManager* pCurrentManager = m_breakpointManagers[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentManager)
        {
            retVal = retVal && pCurrentManager->onKernelSourceCodeUpdate(eve);
        }
    }

    return retVal;

}

