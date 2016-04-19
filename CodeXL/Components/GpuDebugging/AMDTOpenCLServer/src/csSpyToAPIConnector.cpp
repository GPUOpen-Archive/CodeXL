//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csSpyToAPIConnector.cpp
///
//==================================================================================

//------------------------------ csSpyToAPIConnector.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/osDebugLog.h>
#include <AMDTOSWrappers/osPipeSocketClient.h>
#include <AMDTAPIClasses/Include/apAPIFunctionId.h>

// Local:
#include <inc/csAPICallsHandlingThread.h>
#include <inc/csSpyToAPIConnector.h>
#include <inc/csAPIMainLoop.h>

// TO_DO: For testing - do not commit me uncommented:
// #define VS_INTEGRATION 1


// Static members initializations:
csSpyToAPIConnector* csSpyToAPIConnector::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        csSpyToAPIConnector::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
csSpyToAPIConnector& csSpyToAPIConnector::instance()
{
    // If my single instance was not created yet - create it:
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new csSpyToAPIConnector;
        GT_ASSERT_ALLOCATION(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        csSpyToAPIConnector::csSpyToAPIConnector
// Description: Constructor.
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
csSpyToAPIConnector::csSpyToAPIConnector()
    : _pSocketClient(NULL), _pAPICallsHandlingThread(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        csSpyToAPIConnector::~csSpyToAPIConnector
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
csSpyToAPIConnector::~csSpyToAPIConnector()
{
}


// ---------------------------------------------------------------------------
// Name:        csSpyToAPIConnector::initialize
// Description: Initialize a TCP / IP  OpenCL Spy <-> API connection.
// Arguments:   portAddress - The port address to which the Spy will connect to.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool csSpyToAPIConnector::initialize(const osPortAddress& portAddress)
{
    bool retVal = false;

    // TCP / IP sockets are currently supported on Windows only:
    /*
    #if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        // Create a TCP / IP socket client:
        _pSocketClient = new osTCPSocketClient;

        if(_pSocketClient)
        {
            // Open the client side of the socket:
            bool rc = _pSocketClient->open();
            if (rc)
            {
                // Connect it to the socket server:
                rc = ((osTCPSocketClient*)_pSocketClient)->connect(portAddress);

                if (rc)
                {
                    // Instead of having a thread that listens to the API socket,
                    // The Visual Studio Package calls gsProcessSingleAPICall() to process
                    // API calls.
                    #ifdef VS_INTEGRATION
                    {
                        retVal = true;
                        }
                    #else
                    {
                        // Wait for the debugger to initialize the API:
                        handleAPIInitializationCalls();

                        // Create and run the API calls handling thread:
                        bool rc2 = createAndRunTheAPIHandlingThread();
                        GT_IF_WITH_ASSERT(rc2)
                        {
                            retVal = true;
                        }
                    }
                    #endif
                }
            }
        }

    #endif
    */

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csSpyToAPIConnector::initialize
// Description: Initialize a shared memory Spy <-> API connection.
// Arguments:   sharedMemoryObjectName - The shared memory object name, that is
//                                       used for the connection.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool csSpyToAPIConnector::initialize(const gtString& sharedMemoryObjectName)
{
    bool retVal = false;

    // Create the socket client:
    _pSocketClient = new osPipeSocketClient(sharedMemoryObjectName);
    GT_ASSERT_ALLOCATION(_pSocketClient);

    // Connect the socket:
    bool rc = _pSocketClient->open();
    GT_IF_WITH_ASSERT(rc)
    {
        // Wait for the debugger to initialize the API:

        // TO_DO: OpenCL!!!!
        // handleAPIInitializationCalls();

        // Create and run the API calls handling thread:
        bool rc2 = createAndRunTheAPIHandlingThread();
        GT_IF_WITH_ASSERT(rc2)
        {
            retVal = true;
        }
    }


    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csSpyToAPIConnector::terminate
// Description: Terminates the API <-> Spy connection.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool csSpyToAPIConnector::terminate()
{
    bool retVal = false;

    bool rc1 = true;
    bool rc2 = true;

    if (_pAPICallsHandlingThread)
    {
        // Terminate and delete the API requests handling thread:
        rc2 = _pAPICallsHandlingThread->terminate();

        delete _pAPICallsHandlingThread;
        _pAPICallsHandlingThread = NULL;
    }

    if (_pSocketClient)
    {
        // Close and delete the socket client:
        rc1 = _pSocketClient->close();
        delete _pSocketClient;
        _pSocketClient = NULL;
    }

    // TO_DO: We need to check why does the sockets close() function fails (rc2) !!
    retVal = rc2;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csSpyToAPIConnector::handleAPIInitializationCalls
// Description: Wait for the debugger API initialization calls and handle them.
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
void csSpyToAPIConnector::handleAPIInitializationCalls()
{
    // Handle the API functions initialization calls:
    csAPIMainLoop(*_pSocketClient);
}


// ---------------------------------------------------------------------------
// Name:        csSpyToAPIConnector::createAndRunTheAPIHandlingThread
// Description: Creates and runs the API calls handling thread.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool csSpyToAPIConnector::createAndRunTheAPIHandlingThread()
{
    bool retVal = false;

    // If the socket client exists:
    if (_pSocketClient)
    {
        // Create the thread that will handle incoming API calls:
        _pAPICallsHandlingThread = new csAPICallsHandlingThread(*_pSocketClient);

        if (_pAPICallsHandlingThread)
        {
            // Start the thread run:
            retVal = _pAPICallsHandlingThread->execute();
        }
    }

    return retVal;
}



