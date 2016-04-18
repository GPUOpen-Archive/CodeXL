//------------------------------ kaSingletonsDelete.cpp ------------------------------

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/src/kaGlobalVariableManager.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaSingletonsDelete.h>

// The static instance of this class. When the process that contains this file exits,
// the destructor of this single instance will be called. This destructor releases all
// the existing singleton objects.
static kaSingletonsDelete instance;


// ---------------------------------------------------------------------------
// Name:        kaSingletonsDelete::~kaSingletonsDelete
// Description: Destructor - release all the existing singleton objects.
// Author:      Yaki Tebeka
// Date:        7/12/2003
// ---------------------------------------------------------------------------
kaSingletonsDelete::~kaSingletonsDelete()
{

    // Delete the kaGlobalVariableManager instance:
    OS_OUTPUT_DEBUG_LOG(L"Before deleting kaGlobalVariableManager", OS_DEBUG_LOG_DEBUG);
    delete kaGlobalVariableManager::m_psMySingleInstance;
    kaGlobalVariableManager::m_psMySingleInstance = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting kaGlobalVariableManager", OS_DEBUG_LOG_DEBUG);

    // Delete the kaBackendManager instance:
    OS_OUTPUT_DEBUG_LOG(L"Before deleting kaBackendManager", OS_DEBUG_LOG_DEBUG);
    delete kaBackendManager::m_psMySingleInstance;
    kaBackendManager::m_psMySingleInstance = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting kaBackendManager", OS_DEBUG_LOG_DEBUG);

    // Delete the kaApplicationTreeHandler instance:
    OS_OUTPUT_DEBUG_LOG(L"Before deleting kaApplicationTreeHandler", OS_DEBUG_LOG_DEBUG);
    delete kaApplicationTreeHandler::m_psMySingleInstance;
    kaApplicationTreeHandler::m_psMySingleInstance = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting kaApplicationTreeHandler", OS_DEBUG_LOG_DEBUG);

    // Delete the kaApplicationCommands instance:
    OS_OUTPUT_DEBUG_LOG(L"Before deleting kaApplicationCommands", OS_DEBUG_LOG_DEBUG);
    delete kaApplicationCommands::m_psMySingleInstance;
    kaApplicationCommands::m_psMySingleInstance = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting kaApplicationCommands", OS_DEBUG_LOG_DEBUG);
}
