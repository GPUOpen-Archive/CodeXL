//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afUnhandledExceptionHandler.cpp
///
//==================================================================================

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osCallsStackReader.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afUnhandledExceptionHandler.h>

// AMDTApplicationComponents:
#include <AMDTApplicationComponents/Include/acMessageBox.h>



// ---------------------------------------------------------------------------
// Name:        afUnhandledExceptionHandler::afUnhandledExceptionHandler
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        21/4/2009
// ---------------------------------------------------------------------------
afUnhandledExceptionHandler::afUnhandledExceptionHandler() : m_shouldStopWaiting(false)
{
}


// ---------------------------------------------------------------------------
// Name:        afUnhandledExceptionHandler::~afUnhandledExceptionHandler
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        21/4/2009
// ---------------------------------------------------------------------------
afUnhandledExceptionHandler::~afUnhandledExceptionHandler()
{
}


// ---------------------------------------------------------------------------
// Name:        afUnhandledExceptionHandler::instance
// Description: Returns this class' single instance
// Author:      Yaki Tebeka
// Date:        21/4/2009
// ---------------------------------------------------------------------------
afUnhandledExceptionHandler& afUnhandledExceptionHandler::instance()
{
    // If my single instance was not already created:
    afUnhandledExceptionHandler* pSignleInstance = (afUnhandledExceptionHandler*)osUnhandledExceptionHandler::instance();

    if (pSignleInstance == nullptr)
    {
        pSignleInstance = new afUnhandledExceptionHandler;
        osUnhandledExceptionHandler::registerSingleInstance((osUnhandledExceptionHandler&)*pSignleInstance);
    }

    return *pSignleInstance;
}


// ---------------------------------------------------------------------------
// Name:        afUnhandledExceptionHandler::onUnhandledException
// Description: Is called when an unhandled exception is caught.
// Arguments: exceptionCode - The exception code.
//            pExceptionContext - A pointer to the exception context (cast into void*)
// Author:      Yaki Tebeka
// Date:        21/4/2009
// ---------------------------------------------------------------------------
void afUnhandledExceptionHandler::onUnhandledException(osExceptionCode exceptionCode, void* pExceptionContext)
{
    // Notify the handlers about an unhandled exception scenario.
    emit UnhandledExceptionSignal(exceptionCode, pExceptionContext);

    // Wait for the handler to inform us that it is finished and that we can and should exit.
    osWaitForFlagToTurnOn(m_shouldStopWaiting, ULONG_MAX);
}

void afUnhandledExceptionHandler::StopWaiting()
{
    m_shouldStopWaiting = true;
}


