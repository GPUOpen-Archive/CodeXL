//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atEventObserver.cpp
///
//==================================================================================

//------------------------------ atEventObserver.cpp ------------------------------

// Standard C++:
#include <iostream>

// Google test:
#include "gtest/gtest.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apKernelDebuggingCommand.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreationFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <inc/atEventObserver.h>

// Static members initialization:
atEventObserver* atEventObserver::_pMyStaticInstance;

// TO_DO: Sigal - move to GRApiFunctions and share
// ---------------------------------------------------------------------------
// Name:        gdGetCurrentBreakpointFunction
// Description: Inputs a breakpoint event and output formatted strings that contain
//              the function name and its arguments.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        1/7/2004
// ---------------------------------------------------------------------------
bool gdGetCurrentBreakpointFunction(const apFunctionCall* pBreakedOnFunctionCall, gtString& funcName, gtString& funcArgs)
{
    bool retVal = false;

    if (pBreakedOnFunctionCall)
    {
        // Get the function name:
        apMonitoredFunctionId funcId = pBreakedOnFunctionCall->functionId();
        bool rc = gaGetMonitoredFunctionName(funcId, funcName);

        if (rc)
        {
            // Get the function Arguments:
            funcArgs += L"(";

            // Get the function arguments:
            const gtList<const apParameter*>& funcArguments = pBreakedOnFunctionCall->arguments();

            // Iterate them:
            gtList<const apParameter*>::const_iterator iter = funcArguments.begin();
            gtList<const apParameter*>::const_iterator endIter = funcArguments.end();

            gtString currentArgumentValueAsString;

            while (iter != endIter)
            {
                // Get the current argument value (as a string):
                (*(*iter)).valueAsString(currentArgumentValueAsString);

                // Add it to the dialog:
                funcArgs += currentArgumentValueAsString;

                iter++;

                // Add the "," only if it is NOT the last parameter
                if (iter != endIter)
                {
                    funcArgs += L" , ";
                }
            }
        }

        funcArgs += L")";

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        atPendingEventNotificationFunc
// Description: Is called when a debugged process event occurs. The function is
///             moving the event handling to the event handler
// Arguments:   const apEvent& pendingEvent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        1/12/2011
// ---------------------------------------------------------------------------
void atPendingEventNotificationFunc(const apEvent& pendingEvent)
{
    GT_UNREFERENCED_PARAMETER(&pendingEvent);

    atEventObserver& theATEventObserver = atEventObserver::instance();
    bool canSet = theATEventObserver.beforeWritingEventsFlag();

    if (canSet)
    {
        // Turn on the flag that notifies the main thread that there are pending debug events:
        theATEventObserver.setPendingDebugEvents(true);
        theATEventObserver.afterAccessingEventsFlag();
    }
}

// ---------------------------------------------------------------------------
// Name:        atEventObserver::atEventObserver
// Description: Constructor
// Author:      Merav Zanany
// Date:        30/11/2011
// ---------------------------------------------------------------------------
atEventObserver::atEventObserver(): _waitingForEventsToArrive(false), _pCurrentTestData(NULL)
{
    // Register atPendingEventNotificationFunc as the event notification function:
    apEventsHandler& theEventsHandler = apEventsHandler::instance();
    theEventsHandler.registerPendingEventNotificationCallback(&atPendingEventNotificationFunc);

    // Register as an events observer:
    theEventsHandler.registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
    theEventsHandler.registerEventsRegistrationObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        atEventObserver::~atEventObserver
// Description: Destructor
// Author:      Merav Zanany
// Date:        30/11/2011
// ---------------------------------------------------------------------------
atEventObserver::~atEventObserver()
{
    // Register as an events observer:
    apEventsHandler& theEventsHandler = apEventsHandler::instance();
    theEventsHandler.unregisterEventsObserver(*this);
    theEventsHandler.unregisterEventsRegistrationObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        atEventObserver::onEvent
// Description: Overrides base class onEvent
// Arguments:   const apEvent& eve
//              bool& vetoEvent
// Author:      Merav Zanany
// Date:        30/11/2011
// ---------------------------------------------------------------------------
void atEventObserver::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {
        case apEvent::AP_BREAKPOINT_HIT:
        {
            onBreakpointHit((const apBreakpointHitEvent&)(eve));
        }
        break;

        case apEvent::AP_AFTER_KERNEL_DEBUGGING_EVENT:
        {
            bool rcContinue = shouldWaitForMoreBreakpoints();

            if (!rcContinue)
            {
                // Terminate the process since the test is done:
                gaTerminateDebuggedProcess();
            }

            _currentExecutedKernelName = L"";
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            // Make sure that the logging is clear:
            GT_ASSERT(_testLogStrings.size() == 0);
            _testLogStrings.clear();
            _currentExecutedKernelName = L"";

            // Add header for the current test:
            initializeCurrentTest();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            // Clear the executed kernels map:
            _executedKernels.clear();

            // Mark the process as terminated:
            m_hasProcessEnded = true;
        }
        break;

        case apEvent::AP_EXCEPTION:
        {
            const apExceptionEvent& exceptionEvent = (const apExceptionEvent&)eve;

            if (exceptionEvent.isSecondChance())
            {
                gtString logMsg;

                // The debugged application crashed!
                osCallStack crashStack;
                osThreadId crashedThreadId = exceptionEvent.triggeringThreadId();
                bool rcStk = gaGetThreadCallStack(crashedThreadId, crashStack, false);
                GT_IF_WITH_ASSERT(rcStk)
                {
                    gtString ignored;
                    bool ignored2 = false;
                    crashStack.asString(ignored, logMsg, ignored2, true);
                }

                logMsg.prepend(L"Debugged application crashed! Terminating test. Crash call stack:\n");

                // Terminate it:
                gaTerminateDebuggedProcess();

                outputLogString(logMsg);
            }
        }
        break;

        default:
        {
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        atEventObserver::onEventRegistration
// Description: Called when an event is registered. Used to monitor whether
//              the process was successfully created.
// Author:      Uri Shormoni
// Date:        18/11/2014
// ---------------------------------------------------------------------------
void atEventObserver::onEventRegistration(apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
            m_wasProcessCreated = true;
            break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            const apDebuggedProcessCreationFailureEvent& failureEve = (const apDebuggedProcessCreationFailureEvent&)eve;
            ASSERT_TRUE(m_wasProcessCreated) << "Could not create debugged process. Failure Code: " << failureEve.processCreationFailureReason() << ". Failure message: " << failureEve.processCreationError().asASCIICharArray();
            m_hasProcessEnded = true;
        }
        break;

        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        atEventObserver::onBreakpointHit
// Description: Build a string describing the breakpoint and output
// Arguments:   const apBreakpointHitEvent& event
// Author:      Sigal Algranaty
// Date:        21/12/2011
// ---------------------------------------------------------------------------
void atEventObserver::onBreakpointHit(const apBreakpointHitEvent& event)
{
    gtString funcName;
    gtString funcArgs;
    gtString outputStr;
    int lineNumber = -1;

    // Get the breakpoint function name and arguments
    bool rc = gdGetCurrentBreakpointFunction(event.breakedOnFunctionCall(), funcName, funcArgs);
    GT_ASSERT(rc);

    // Check variable values:
    const gtString& testType = _pCurrentTestData->_testType;
    bool breakedInKernelDebugging = gaIsInKernelDebugging();

    // Get the break reason:
    apBreakReason breakReason = event.breakReason();

    switch (breakReason)
    {
        case AP_MONITORED_FUNCTION_BREAKPOINT_HIT:
        {
            // Monitored function break point:
            outputStr = L"AP_MONITORED_FUNCTION_BREAKPOINT_HIT:";
            outputStr.append(funcName);
        }
        break;

        case AP_KERNEL_SOURCE_CODE_BREAKPOINT_HIT:
        {
            // Kernel source breakpoint:
            outputStr = L"AP_KERNEL_SOURCE_CODE_BREAKPOINT_HIT:";

            // Get the currently debugged kernel function name:
            apCLKernel currentlyDebuggedKernel(OA_CL_NULL_HANDLE, 0, OA_CL_NULL_HANDLE, L"");
            bool rcKer = gaGetCurrentlyDebuggedKernelDetails(currentlyDebuggedKernel);
            GT_IF_WITH_ASSERT(rcKer)
            {
                funcName = currentlyDebuggedKernel.kernelFunctionName();

                // Append the function name to the output string:
                outputStr.append(funcName);

                // Interpret "step over" as a kernel step:
                bool rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_OVER);
                GT_ASSERT(rcStp);

                _executedKernels[funcName] = true;
                _currentExecutedKernelName = funcName;
            }
        }
        break;

        case AP_KERNEL_FUNCTION_NAME_BREAKPOINT_HIT:
        {
            // Kernel source breakpoint:
            outputStr = L"AP_KERNEL_FUNCTION_NAME_BREAKPOINT_HIT:";

            // Get the currently debugged kernel function name:
            apCLKernel currentlyDebuggedKernel(OA_CL_NULL_HANDLE, 0, OA_CL_NULL_HANDLE, L"");
            bool rcKer = gaGetCurrentlyDebuggedKernelDetails(currentlyDebuggedKernel);
            GT_IF_WITH_ASSERT(rcKer)
            {
                funcName = currentlyDebuggedKernel.kernelFunctionName();

                // Append the function name to the output string:
                outputStr.append(funcName);

                // Interpret "step over" as a kernel step:
                bool rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_OVER);
                GT_ASSERT(rcStp);

                _executedKernels[funcName] = true;
                _currentExecutedKernelName = funcName;
            }
        }
        break;

        case AP_NEXT_MONITORED_FUNCTION_BREAKPOINT_HIT:
        {
            // "Step" command break point:
            outputStr = L"AP_NEXT_MONITORED_FUNCTION_BREAKPOINT_HIT:";
            outputStr.append(funcName);
        }
        break;

        case AP_DRAW_MONITORED_FUNCTION_BREAKPOINT_HIT:
        {
            // "Draw Step" command break point:
            outputStr = L"AP_DRAW_MONITORED_FUNCTION_BREAKPOINT_HIT:";
            outputStr.append(funcName);
        }
        break;

        case AP_FRAME_BREAKPOINT_HIT:
        {
            // "Frame Step" command break point:
            outputStr = L"AP_FRAME_BREAKPOINT_HIT:";
            outputStr.append(funcName);
        }
        break;

        case AP_STEP_IN_BREAKPOINT_HIT:
        {
            // "Step in" command break point:
            outputStr = L"AP_STEP_IN_BREAKPOINT_HIT";

            // Interpret "step out" as a kernel step:
            bool rcStp = false;

            if (L"StepIntoOutTest" == testType)
            {
                rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_OUT);
            }
            else
            {
                rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_OVER);
            }

            GT_ASSERT(rcStp);
        }
        break;

        case AP_STEP_OVER_BREAKPOINT_HIT:
        {
            // "Step over" command break point:
            outputStr = L"AP_STEP_OVER_BREAKPOINT_HIT";

            bool rcStp = false;

            // Interpret "step over" as a kernel step:
            rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_OVER);
        }
        break;

        case AP_STEP_OUT_BREAKPOINT_HIT:
        {
            // "Step out" command break point:
            outputStr = L"AP_STEP_OUT_BREAKPOINT_HIT";

            bool rcStp = false;

            if (breakedInKernelDebugging)
            {
                // Continue stepping out:
                rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_OUT);
            }
            else
            {
                // Kernel debugging ended, exit the application:
                rcStp = gaTerminateDebuggedProcess();
            }

            GT_ASSERT(rcStp);
        }
        break;

        case AP_BREAK_COMMAND_HIT:
        {
            // "Break" command break point:
            outputStr = L"AP_STEP_OVER_BREAKPOINT_HIT:";
        }
        break;

        default:
        {
            // Unexpected break reason:
            GT_ASSERT(false);
        }
        break;
    }

    // If we are in kernel debugging, get the kernel call stack to find the current line number:
    if (breakedInKernelDebugging)
    {
        osCallStack kernelStack;
        bool rcCS = gaGetCurrentlyDebuggedKernelCallStack(kernelStack);

        if (rcCS)
        {
            outputStr.append(':');
            storeCurrentLineNumber(kernelStack, outputStr, lineNumber);
        }
    }

    // Output this string:
    outputLogString(outputStr);

    if ((L"StepTest" == testType) || (L"WorkItemsTest" == testType))
    {
        rc = testVariables(lineNumber);
        GT_ASSERT(rc);
    }

    // Check Locals in Kernel
    if ((L"LocalsTest" == testType))
    {
        rc = outputAllLocals(lineNumber);
        GT_ASSERT(rc);
    }

    // Test the "step into" op
    if ((L"StepIntoTest" == testType) || (L"StepIntoOutTest" == testType))
    {
        rc = testStepInto(lineNumber);
        GT_ASSERT(rc);
    }

    // Test the values of a variable for all work items
    if ((L"AllValuesTest" == testType))
    {
        rc = testValueForAllWorkItems(lineNumber);
        GT_ASSERT(rc);
    }

    // Set the kernel stepping work item, if needed
    setSteppingWorkItem();

    // Resume the debugged process and continue the test:
    rc = gaResumeDebuggedProcess();
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        atEventObserver::storeCurrentLineNumber
// Description: Append the current line number to the output string and store it in the integer parameter
// Return Val:  void
// Author:      Doron Ofek
// Date:        Jan-25, 2014
// ---------------------------------------------------------------------------
void atEventObserver::storeCurrentLineNumber(const osCallStack& kernelStack, gtString& outputStr, int& lineNumber)
{
    if (0 < kernelStack.amountOfStackFrames())
    {
        const osCallStackFrame* pKernelStackTopFrame = kernelStack.stackFrame(0);

        if (NULL != pKernelStackTopFrame)
        {
            lineNumber = pKernelStackTopFrame->sourceCodeFileLineNumber();
            outputStr.appendFormattedString(L"%d", lineNumber);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        atEventObserver::instance
// Description: Single instance creation
// Return Val:  atEventObserver&
// Author:      Sigal Algranaty
// Date:        21/12/2011
// ---------------------------------------------------------------------------
atEventObserver& atEventObserver::instance()
{
    if (_pMyStaticInstance == NULL)
    {
        _pMyStaticInstance = new atEventObserver;
    }

    return *_pMyStaticInstance;
}

// ---------------------------------------------------------------------------
// Name:        atEventObserver::initializeCurrentTest
// Description: Add header for the current test
// Author:      Sigal Algranaty
// Date:        1/1/2012
// ---------------------------------------------------------------------------
void atEventObserver::initializeCurrentTest()
{
    // Sanity check
    GT_IF_WITH_ASSERT(_pCurrentTestData != NULL)
    {
        // Initialize the current test kernels status:
        for (int i = 0; i < (int)_pCurrentTestData->_testedKernels.size(); i++)
        {
            // Get the current kernel test:
            atTestData::atKernelTest* pCurrentKernelTest = _pCurrentTestData->_testedKernels[i];

            if (pCurrentKernelTest != NULL)
            {
                _executedKernels[pCurrentKernelTest->_kernelName] = false;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        atEventObserver::shouldWaitForMoreBreakpoints
// Description: Should we continue test?
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/1/2012
// ---------------------------------------------------------------------------
bool atEventObserver::shouldWaitForMoreBreakpoints()
{
    bool retVal = false;

    // Iterate the map and look for non executed kernel:
    gtMap<gtString, bool>::iterator iter = _executedKernels.begin();

    for (; iter != _executedKernels.end(); iter++)
    {
        if (!(*iter).second)
        {
            retVal = true;
            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        atEventObserver::setCurrentTest
// Description: Set the current test data
// Arguments:   const atTestData* pTestData
// Author:      Sigal Algranaty
// Date:        1/1/2012
// ---------------------------------------------------------------------------
void atEventObserver::setCurrentTest(const atTestData* pTestData)
{
    GT_IF_WITH_ASSERT(pTestData != NULL)
    {
        _pCurrentTestData = pTestData;
    }
}


// ---------------------------------------------------------------------------
// Name:        atEventObserver::testVariables
// Description: Test the current kernel variable values for the current line number
// Arguments:   int lineNumber
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/1/2012
// ---------------------------------------------------------------------------
bool atEventObserver::testVariables(int lineNumber)
{
    bool retVal = true;

    // Sanity check
    GT_IF_WITH_ASSERT(_pCurrentTestData != NULL)
    {
        // Check if we're in the middle of kernel debugging session:
        if ((lineNumber >= 0) && (!_currentExecutedKernelName.isEmpty()))
        {
            // Go through the kernel tests, and check if there are tests for the current line:
            for (int i = 0 ; i < (int)_pCurrentTestData->_testedKernels.size(); i++)
            {
                // Get the current kernel test:
                atTestData::atKernelTest* pKernelTest = _pCurrentTestData->_testedKernels[i];

                if (pKernelTest != NULL)
                {
                    if (pKernelTest->_kernelName == _currentExecutedKernelName)
                    {
                        // Go through the variables requested for this kernel:
                        for (int k = 0; k < (int)pKernelTest->_variablesToRead.size(); k++)
                        {
                            // Get the current variable:
                            atTestData::atKernelTest::atVarData currentVar = pKernelTest->_variablesToRead[k];

                            if (currentVar.lineNumber == lineNumber)
                            {
                                gtString outputStr;

                                // Get the variable work items values
                                int workItem[3] = {0, -1, -1};

                                for (int j = 0; j < 3; j++)
                                {
                                    workItem[j] = currentVar.workItem[j];
                                }

                                gtString variableValue;
                                gtString variableValueHex;
                                gtString variableType;
                                bool rc = gaGetKernelDebuggingVariableValueString(currentVar.varName, workItem, variableValue, variableValueHex, variableType);

                                outputStr.appendFormattedString(L"Work Item (%d, %d, %d): ", workItem[0], workItem[1], workItem[2]);

                                if (rc)
                                {
                                    outputStr.appendFormattedString(L"Variable value: %ls=%ls. Variable type: %ls", currentVar.varName.asCharArray(), variableValue.asCharArray(), variableType.asCharArray());
                                }
                                else
                                {
                                    outputStr.appendFormattedString(L"Failed to get variable %ls", currentVar.varName.asCharArray());
                                }

                                retVal = retVal && rc;

                                // Output the variable value to the log file:
                                outputLogString(outputStr);
                            }
                        }
                    }
                }
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        atEventObserver::outputAllLocals
// Description: Test all the locals for the current kernel (output to a file)
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        21/2/2012
// ---------------------------------------------------------------------------
bool atEventObserver::outputAllLocals(int lineNumber)
{
    bool retVal = true;

    // Sanity check
    GT_IF_WITH_ASSERT(_pCurrentTestData != NULL)
    {
        // Check if we're in the middle of kernel debugging session:
        if (!_currentExecutedKernelName.isEmpty())
        {
            // Go through the kernel tests, and check if there are tests for the current line:
            for (int i = 0 ; i < (int)_pCurrentTestData->_testedKernels.size(); i++)
            {
                // Get the current kernel test:
                atTestData::atKernelTest* pKernelTest = _pCurrentTestData->_testedKernels[i];

                if (pKernelTest != NULL)
                {
                    if (pKernelTest->_kernelName == _currentExecutedKernelName)
                    {
                        for (int j = 0; j < (int)pKernelTest->_localsInLine.size(); j++)
                        {
                            if (pKernelTest->_localsInLine[j] == lineNumber)
                            {
                                gtVector<gtString> tempVariableNames;
                                gtString outputStr = L"";

                                bool rc = gaGetKernelDebuggingAvailableVariables(tempVariableNames, false, 0);

                                for (int k = 0; k < (int)tempVariableNames.size(); k++)
                                {
                                    outputStr.append(tempVariableNames[k]);
                                    outputStr.append(L"\n");
                                }

                                // Output the variable value to the log file:
                                outputLogString(outputStr);

                                retVal = retVal && rc;
                            }
                        }
                    }
                }
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        atEventObserver::testStepInto
// Description: Test stepping into a function inside a kernel
// Arguments:   int lineNumber
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        11/6/2012
// ---------------------------------------------------------------------------
bool atEventObserver::testStepInto(int lineNumber)
{
    bool retVal = true;

    // Sanity check
    GT_IF_WITH_ASSERT(_pCurrentTestData != NULL)
    {
        // Check if we're in the middle of kernel debugging session:
        if (!_currentExecutedKernelName.isEmpty())
        {
            // Go through the kernel tests, and check if there are tests for the current line:
            for (int i = 0 ; i < (int)_pCurrentTestData->_testedKernels.size(); i++)
            {
                // Get the current kernel test:
                atTestData::atKernelTest* pKernelTest = _pCurrentTestData->_testedKernels[i];

                if (pKernelTest != NULL)
                {
                    if (pKernelTest->_kernelName == _currentExecutedKernelName)
                    {
                        if (pKernelTest->m_stepIntoLine == lineNumber)
                        {
                            // Interpret "step into" as a kernel step:
                            bool rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_IN);
                            GT_ASSERT(rcStp);
                        }
                    }
                }
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        atEventObserver::testValueForAllWorkItems
// Description: Test value of a variable for all work items' values
// Arguments:   int lineNumber
// Return Val:  bool - Success / failure.
// Author:      Merav Zanany
// Date:        20/6/2012
// ---------------------------------------------------------------------------
bool atEventObserver::testValueForAllWorkItems(int lineNumber)
{
    bool retVal = true;

    // Sanity check
    GT_IF_WITH_ASSERT(_pCurrentTestData != NULL)
    {
        // Check if we're in the middle of kernel debugging session:
        if ((lineNumber >= 0) && (!_currentExecutedKernelName.isEmpty()))
        {
            // Go through the kernel tests, and check if there are tests for the current line:
            for (int i = 0 ; i < (int)_pCurrentTestData->_testedKernels.size(); i++)
            {
                // Get the current kernel test:
                atTestData::atKernelTest* pKernelTest = _pCurrentTestData->_testedKernels[i];

                if (pKernelTest != NULL)
                {
                    if (pKernelTest->_kernelName == _currentExecutedKernelName)
                    {
                        // Go through the variables requested for this kernel:
                        for (int k = 0; k < (int)pKernelTest->_variablesToRead.size(); k++)
                        {
                            // Get the current variable:
                            atTestData::atKernelTest::atVarData currentVar = pKernelTest->_variablesToRead[k];

                            if (currentVar.lineNumber == lineNumber)
                            {
                                gtString outputStr;
                                gtString variableValue, variableValueHex, variableType;

                                // Get the variable work items values
                                int workItem[3] = {0, -1, -1};
                                int xVal = currentVar.workItem[0];
                                int yVal = currentVar.workItem[1];
                                int zVal = currentVar.workItem[2];

                                for (int j = 0; j < xVal; j++)
                                {
                                    workItem[0] = (xVal == -1 ? -1 : j);
                                    workItem[1] = (yVal == -1 ? -1 : j);
                                    workItem[2] = (zVal == -1 ? -1 : j);

                                    bool rc = gaGetKernelDebuggingVariableValueString(currentVar.varName, workItem, variableValue, variableValueHex, variableType);

                                    outputStr.appendFormattedString(L"X:%d Y:%d Z:%d. ", workItem[0], workItem[1], workItem[2]);

                                    if (rc)
                                    {
                                        outputStr.appendFormattedString(L"Variable value: %ls=%ls\n", currentVar.varName.asCharArray(), variableValue.asCharArray());
                                    }
                                    else
                                    {
                                        outputStr.appendFormattedString(L"Failed to get variable %ls\n", currentVar.varName.asCharArray());
                                    }

                                    retVal = retVal && rc;
                                }

                                bool rcStp = gaSetKernelDebuggingCommand(AP_KERNEL_STEP_IN);
                                GT_ASSERT(rcStp);

                                // Output the variable value to the log file:
                                outputLogString(outputStr);
                            }
                        }
                    }
                }
            }
        }
    }
    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        atEventObserver::setSteppingWorkItem
// Description: Sets the stepping work item if needed
// Author:      Uri Shomroni
// Date:        3/7/2013
// ---------------------------------------------------------------------------
void atEventObserver::setSteppingWorkItem()
{
    // Sanity check:
    if (NULL != _pCurrentTestData)
    {
        // If we are in kernel debugging:
        if (!_currentExecutedKernelName.isEmpty())
        {
            // Find the kernel test we are currently in:
            const gtPtrVector<atTestData::atKernelTest*>& kernels = _pCurrentTestData->_testedKernels;
            int numberOfKernels = (int)kernels.size();

            for (int i = 0; i < numberOfKernels; i++)
            {
                // Sanity check:
                const atTestData::atKernelTest* pCurrentKernel = kernels[i];
                GT_IF_WITH_ASSERT(NULL != pCurrentKernel)
                {
                    // Is this the correct kernel?
                    if (pCurrentKernel->_kernelName == _currentExecutedKernelName)
                    {
                        // If this test has a work item defined:
                        if (-1 < pCurrentKernel->m_workItemCoord[0])
                        {
                            // Set its value:
                            bool rcWI = gaSetKernelSteppingWorkItem(pCurrentKernel->m_workItemCoord);
                            GT_ASSERT(rcWI);
                        }

                        // We found the correct kernel, we can stop looking:
                        break;
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        atEventObserver::outputLogString
// Description: Output the requested string
// Arguments:   gtString &outputStr
// Author:      Sigal Algranaty
// Date:        2/1/2012
// ---------------------------------------------------------------------------
void atEventObserver::outputLogString(gtString& outputStr)
{
    // Output to cout:
    std::cout << outputStr.asASCIICharArray();
    std::cout << "\n";

    // Add the string to the breakpoints vector:
    _testLogStrings.push_back(outputStr);
}

