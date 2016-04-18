//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdPackageConnector.cpp
///
//==================================================================================

//------------------------------ vsdPackageConnector.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTProcessDebugger/Include/pdProcessDebuggersManager.h>

// Local:
#include <CodeXLVSPackageDebugger/Include/vsdPackageConnector.h>
#include <src/vsdDebugProcess.h>
#include <src/vsdProcessDebugger.h>

// Static members initializations:
vsdPackageConnector* vsdPackageConnector::ms_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        vsdPackageConnector::instance
// Description: Returns a reference to the single instance of this class.
//              The first call to this function creates this single instance.
// Author:      Uri Shomroni
// Date:        25/12/2011
// ---------------------------------------------------------------------------
vsdPackageConnector& vsdPackageConnector::instance()
{
    // If my single instance was not created yet - create it:
    if (ms_pMySingleInstance == NULL)
    {
        ms_pMySingleInstance = new vsdPackageConnector;
    }

    // Return my single instance:
    return *ms_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        vsdPackageConnector::vsdPackageConnector
// Description: Constructor
// Author:      Uri Shomroni
// Date:        25/12/2011
// ---------------------------------------------------------------------------
vsdPackageConnector::vsdPackageConnector()
    : m_initialized(false), m_piNativeDebugEngine(NULL), m_pVSProcessDebugger(NULL), m_piDebuggedProcess(NULL)
{

}

// ---------------------------------------------------------------------------
// Name:        vsdPackageConnector::~vsdPackageConnector
// Description: Destructor
// Author:      Uri Shomroni
// Date:        25/12/2011
// ---------------------------------------------------------------------------
vsdPackageConnector::~vsdPackageConnector()
{
    terminatePackageConnection();
}

// ---------------------------------------------------------------------------
// Name:        vsdPackageConnector::initializeWithPackage
// Description: Initializes the package connector using the interfaces from the
//              package
// Author:      Uri Shomroni
// Date:        25/12/2011
// ---------------------------------------------------------------------------
void vsdPackageConnector::initializeWithPackage(IDebugEngine2* piNativeDebugEngine, IDebugProgramProvider2* piNativeProgramProvider)
{
    // Clean up any previous connection:
    terminatePackageConnection();

    GT_IF_WITH_ASSERT((piNativeDebugEngine != NULL) && (piNativeProgramProvider != NULL))
    {
        // Get the debug engine and add to its reference count:
        m_piNativeDebugEngine = piNativeDebugEngine;
        m_piNativeDebugEngine->AddRef();

        // Create the vsdProcessDebugger:
        m_pVSProcessDebugger = new vsdProcessDebugger(*m_piNativeDebugEngine, *piNativeProgramProvider);

        // Register it in the process debuggers manager:
        pdProcessDebuggersManager::instance().setProcessDebuggerInSlot(*m_pVSProcessDebugger, PD_VISUAL_STUDIO_PROCESS_DEBUGGER);

        // Mark we are initialized:
        m_initialized = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdPackageConnector::terminatePackageConnection
// Description: Terminates and cleans up the interfaces obtained in initializeWithPackage()
// Author:      Uri Shomroni
// Date:        25/12/2011
// ---------------------------------------------------------------------------
void vsdPackageConnector::terminatePackageConnection()
{
    if (m_piNativeDebugEngine != NULL)
    {
        m_piNativeDebugEngine->Release();
        m_piNativeDebugEngine = NULL;
    }

    if (m_pVSProcessDebugger != NULL)
    {
        // Unregister the process debugger from the process debuggers manager:
        pdProcessDebuggersManager::instance().removeProcessDebuggerFromSlot(*m_pVSProcessDebugger, PD_VISUAL_STUDIO_PROCESS_DEBUGGER);

        delete m_pVSProcessDebugger;
        m_pVSProcessDebugger = NULL;
    }

    if (m_piDebuggedProcess != NULL)
    {
        m_piDebuggedProcess->Release();
        m_piDebuggedProcess = NULL;
    }

    m_initialized = false;
}

// ---------------------------------------------------------------------------
// Name:        vsdPackageConnector::setDebugPort
// Description: Sets the debug port, which is used for process manipulation
// Author:      Uri Shomroni
// Date:        27/12/2011
// ---------------------------------------------------------------------------
void vsdPackageConnector::setDebugPort(IDebugPort2* piDebugPort)
{
    if (m_initialized)
    {
        GT_IF_WITH_ASSERT(m_pVSProcessDebugger != NULL)
        {
            m_pVSProcessDebugger->setDebugPort(piDebugPort);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdPackageConnector::getWrappedDebugPort
// Description: Get the debug port, as it is wrapped by the vsdProcessDebugger
// Author:      Uri Shomroni
// Date:        22/1/2012
// ---------------------------------------------------------------------------
IDebugPort2* vsdPackageConnector::getWrappedDebugPort()
{
    IDebugPort2* retVal = NULL;

    if (m_initialized)
    {
        GT_IF_WITH_ASSERT(m_pVSProcessDebugger != NULL)
        {
            retVal = m_pVSProcessDebugger->getWrappedDebugPort();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        vsdPackageConnector::debuggedProcess
// Description: Returns the debugged process interface
// Author:      Uri Shomroni
// Date:        25/1/2012
// ---------------------------------------------------------------------------
IDebugProcess2* vsdPackageConnector::debuggedProcess()
{
    return m_piDebuggedProcess;
}

// ---------------------------------------------------------------------------
// Name:        vsdPackageConnector::setEnumProgramsToBeControlledByDebuggedProcess
// Description: Sets an IEnumDebugPrograms2 interface to be controlled by m_piDebuggedProcess.
// Author:      Uri Shomroni
// Date:        25/1/2012
// ---------------------------------------------------------------------------
void vsdPackageConnector::setProgramToBeEnumeratedByDebuggedProcess(IDebugProgram2* piDebugProgram)
{
    if (m_initialized)
    {
        GT_IF_WITH_ASSERT(m_piDebuggedProcess != NULL)
        {
            vsdCSingleProgramEnumDebugPrograms* pEnum = new vsdCSingleProgramEnumDebugPrograms(piDebugProgram);
            // Not sure if this is needed:
            m_piDebuggedProcess->setControlledEnumPrograms(pEnum);
            pEnum->Release();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdPackageConnector::setDebuggedProcess
// Description: Set the debugged process interface, used by the SDM and the native
//              debug engine to control the application
// Author:      Uri Shomroni
// Date:        4/1/2012
// ---------------------------------------------------------------------------
void vsdPackageConnector::setDebuggedProcess(vsdCDebugProcess* piDebuggedProcess)
{
    if (m_initialized)
    {
        if (m_piDebuggedProcess != NULL)
        {
            m_piDebuggedProcess->Release();
            m_piDebuggedProcess = NULL;
        }

        m_piDebuggedProcess = piDebuggedProcess;

        if (m_piDebuggedProcess != NULL)
        {
            m_piDebuggedProcess->AddRef();
        }
    }
}


