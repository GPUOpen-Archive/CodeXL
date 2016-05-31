//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suCallsStatisticsLogger.cpp
///
//==================================================================================

//------------------------------ suCallsStatisticsLogger.cpp ------------------------------

// ANSI C:
#include <memory.h>
#include <AMDTOSWrappers/Include/osStdLibIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTAPIClasses/Include/apOpenGLStateVariableId.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTServerUtilities/Include/suCallsStatisticsLogger.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>


// A calls statistics vector sized (measured in bytes)
int suCallsStatisticsLogger::_statisticsVectorSize = sizeof(gtUInt64) * apMonitoredFunctionsAmount;

// Static vector pointers initialization:
int* suCallsStatisticsLogger::_monitoredFuncIdToLoggedEnumFunc = NULL;
int* suCallsStatisticsLogger::_loggedEnumFuncToEnumPosInArgList = NULL;


// ---------------------------------------------------------------------------
// Name:        suCallsStatisticsLogger::suCallsStatisticsLogger
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        30/1/2006
// ---------------------------------------------------------------------------
suCallsStatisticsLogger::suCallsStatisticsLogger(): _fullFramesCount(0), _wasFirstFrameSkipped(false), _currentFunctionCallEnumValue(0),
    _currentFunctionCallDeprecationStatus(AP_DEPRECATION_NONE), _currentFunctionCallEnumType(OS_TOBJ_ID_INT_PARAMETER), _isInComputationFrame(true)
{
    // Initialize this class static vectors:
    initStaticVectors();

    // Initialize statistics arrays:
    ::memset(_currentFrameFunctionCallsAmount, 0, _statisticsVectorSize);

    // Initialize the global function calls amount vector:
    ::memset(_globalFunctionCallsAmount, 0, _statisticsVectorSize);

    // Initialize current frame statistics:
    ::memset(_fullFramesFunctionCallsAmount, 0, _statisticsVectorSize);

    // Initialize redundant calls statistics arrays:
    ::memset(_currentRedundantFunctionCallsAmount, 0, _statisticsVectorSize);

    // Initialize current frame statistics:
    ::memset(_fullFramesRedundantFunctionCallsAmount, 0, _statisticsVectorSize);

    // Initialize the current frame deprecation statistics:
    ::memset(_currentFrameDeprecationFunctionCallCounter, 0, _statisticsVectorSize * AP_DEPRECATION_STATUS_AMOUNT);

    // Initialize the deprecation statistics:
    ::memset(_fullFramesDeprecationFunctionCallCounter, 0, _statisticsVectorSize * AP_DEPRECATION_STATUS_AMOUNT);
}


// ---------------------------------------------------------------------------
// Name:        suCallsStatisticsLogger::~suCallsStatisticsLogger
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        30/1/2006
// ---------------------------------------------------------------------------
suCallsStatisticsLogger::~suCallsStatisticsLogger()
{
}

// ---------------------------------------------------------------------------
// Name:        suCallsStatisticsLogger::onFrameTerminatorCall
// Description: Is called when a frame terminator function is called.
// Author:      Yaki Tebeka
// Date:        30/1/2006
// ---------------------------------------------------------------------------
void suCallsStatisticsLogger::onFrameTerminatorCall()
{
    // Which deprecations to check. Note that the "top" is one higher than
    // the last value to be checked:
    int deprecationRangeBottom = AP_DEPRECATION_NONE;
    int deprecationRangeTop = AP_DEPRECATION_FULL + 1;

    // If we are in analyze mode, check all the deprecations:
    apExecutionMode currentExecutionMode = suDebuggedProcessExecutionMode();

    if (currentExecutionMode == AP_ANALYZE_MODE)
    {
        deprecationRangeBottom = 0;
        deprecationRangeTop = AP_DEPRECATION_STATUS_AMOUNT;
    }

    // Accumulate function calls amount in _fullFramesFunctionCallsAmount and _fullFramesRedundantFunctionCallsAmount:
    for (int i = 0; i < apMonitoredFunctionsAmount; i++)
    {
        _fullFramesFunctionCallsAmount[i] += _currentFrameFunctionCallsAmount[i];
        _fullFramesRedundantFunctionCallsAmount[i] += _currentRedundantFunctionCallsAmount[i];

        // OpenGL ES does not define a deprecation model:
#ifndef _GR_IPHONE_BUILD

        // Accumulate the deprecated function statistics:
        for (int j = deprecationRangeBottom; j < deprecationRangeTop; j++)
        {
            _fullFramesDeprecationFunctionCallCounter[j][i] += _currentFrameDeprecationFunctionCallCounter[j][i];
        }

#endif
    }

    // Clear current function calls counters:
    ::memset(_currentFrameFunctionCallsAmount, 0, _statisticsVectorSize);
    ::memset(_currentRedundantFunctionCallsAmount, 0, _statisticsVectorSize);
    // OpenGL ES does not define a deprecation model:
#ifndef _GR_IPHONE_BUILD
    ::memset(_currentFrameDeprecationFunctionCallCounter, 0, _statisticsVectorSize * AP_DEPRECATION_STATUS_AMOUNT);
#endif

    // Increase number of full frames:
    if (_wasFirstFrameSkipped)
    {
        _fullFramesCount++;
    }
    else
    {
        _wasFirstFrameSkipped = true;
    }

    // Iterate the functions to which we support enumerators logging:
    for (int i = 0; i < amountOfLoggedEnumFunctions; i++)
    {
        // Get the function's current and previous frame enumerators statistics holders:
        gtVector<apEnumeratorUsageStatistics>& currFuncFullFramesLoggedEnumsStat = _fullFramesEnumeratorsUsage[i];
        gtVector<apEnumeratorUsageStatistics>& currFuncCurrFrameLoggedEnumsStat = _currentFrameEnumeratorsUsage[i];
        gtVector<apEnumeratorUsageStatistics>& currFuncTotalLoggedEnumsStat = _totalEnumeratorsUsage[i];

        // Iterate the logged enumerators:
        int amountOfLoggedEnums = (int)currFuncCurrFrameLoggedEnumsStat.size();

        for (int j = 0; j < amountOfLoggedEnums; j++)
        {
            // Copy the current frame statistics to the previous frame statistics:
            currFuncFullFramesLoggedEnumsStat[j]._amountOfTimesUsed += currFuncCurrFrameLoggedEnumsStat[j]._amountOfTimesUsed;

            // Copy the current frame redundancy statistics to the previous frame statistics:
            currFuncFullFramesLoggedEnumsStat[j]._amountOfRedundantTimesUsed += currFuncCurrFrameLoggedEnumsStat[j]._amountOfRedundantTimesUsed;

            // Initialize current frame statistics:
            currFuncCurrFrameLoggedEnumsStat[j]._amountOfTimesUsed = 0;

            // Initialize current frame redundancy statistics:
            currFuncCurrFrameLoggedEnumsStat[j]._amountOfRedundantTimesUsed = 0;

            // At this moment, the total amount of usage is identical to full frame:
            currFuncTotalLoggedEnumsStat[j]._amountOfTimesUsed = currFuncFullFramesLoggedEnumsStat[j]._amountOfTimesUsed;
            currFuncTotalLoggedEnumsStat[j]._amountOfRedundantTimesUsed = currFuncFullFramesLoggedEnumsStat[j]._amountOfRedundantTimesUsed;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        suCallsStatisticsLogger::initStaticVectors
// Description: Initialize this class static vectors.
// Author:      Yaki Tebeka
// Date:        2/2/2006
// ---------------------------------------------------------------------------
void suCallsStatisticsLogger::initStaticVectors()
{
    // Contains true iff the static vectors were initialized:
    static bool stat_wereStaticVectorsInitialized = false;

    // A critical section that verifies that verifies that only one thread
    // can initialize this class vectors, and during initialization time no
    // other thread can exit this function:
    static osCriticalSection stat_vecInitializationCS;

    // Lock the access to the vectors initialization:
    osCriticalSectionLocker vecInitCSLocker(stat_vecInitializationCS);

    // If the vectors were not initialized:
    if (!stat_wereStaticVectorsInitialized)
    {
        // Allocate the static vectors:
        _monitoredFuncIdToLoggedEnumFunc = new int[apMonitoredFunctionsAmount];
        _loggedEnumFuncToEnumPosInArgList = new int[apMonitoredFunctionsAmount];

        GT_IF_WITH_ASSERT((_monitoredFuncIdToLoggedEnumFunc != NULL) && (_loggedEnumFuncToEnumPosInArgList != NULL))
        {
            // Mark that the static vectors were initialized:
            stat_wereStaticVectorsInitialized = true;

            // Initialize static vector values to default values:
            for (int i = 0; i < apMonitoredFunctionsAmount; i++)
            {
                _monitoredFuncIdToLoggedEnumFunc[i] = -1;
                _loggedEnumFuncToEnumPosInArgList[i] = -1;
            }

            // Update the static vectors with the data of the functions to which we monitor
            // enumerators values:
            // --------------------------------------------------------------------------

            // glEnable:
            _monitoredFuncIdToLoggedEnumFunc[ap_glEnable] = loggedEnumFunc_glEnable;
            _loggedEnumFuncToEnumPosInArgList[ap_glEnable] = 0;

            // glDisable:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDisable] = loggedEnumFunc_glDisable;
            _loggedEnumFuncToEnumPosInArgList[ap_glDisable] = 0;

            // glEnableClientState:
            _monitoredFuncIdToLoggedEnumFunc[ap_glEnableClientState] = loggedEnumFunc_glEnableClientState;
            _loggedEnumFuncToEnumPosInArgList[ap_glEnableClientState] = 0;

            // glDisableClientState:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDisableClientState] = loggedEnumFunc_glDisableClientState;
            _loggedEnumFuncToEnumPosInArgList[ap_glDisableClientState] = 0;

            // glBindBuffer:
            _monitoredFuncIdToLoggedEnumFunc[ap_glBindBuffer] = loggedEnumFunc_glBindBuffer;
            _loggedEnumFuncToEnumPosInArgList[ap_glBindBuffer] = 0;

            // glBindBufferARB:
            _monitoredFuncIdToLoggedEnumFunc[ap_glBindBufferARB] = loggedEnumFunc_glBindBufferARB ;
            _loggedEnumFuncToEnumPosInArgList[ap_glBindBufferARB] = 0;

            // glBindTexture:
            _monitoredFuncIdToLoggedEnumFunc[ap_glBindTexture] = loggedEnumFunc_glBindTexture;
            _loggedEnumFuncToEnumPosInArgList[ap_glBindTexture] = 0;

            // glTexParameteri:
            _monitoredFuncIdToLoggedEnumFunc[ap_glTexParameteri] = loggedEnumFunc_glTexParameteri;
            _loggedEnumFuncToEnumPosInArgList[ap_glTexParameteri] = 1;

            // glTexParameterf:
            _monitoredFuncIdToLoggedEnumFunc[ap_glTexParameterf] = loggedEnumFunc_glTexParameterf;
            _loggedEnumFuncToEnumPosInArgList[ap_glTexParameterf] = 1;

            // glTexParameteriv:
            _monitoredFuncIdToLoggedEnumFunc[ap_glTexParameteriv] = loggedEnumFunc_glTexParameteriv;
            _loggedEnumFuncToEnumPosInArgList[ap_glTexParameteriv] = 1;

            // glTexParameterfv:
            _monitoredFuncIdToLoggedEnumFunc[ap_glTexParameterfv] = loggedEnumFunc_glTexParameterfv;
            _loggedEnumFuncToEnumPosInArgList[ap_glTexParameterfv] = 1;

            // glBegin:
            _monitoredFuncIdToLoggedEnumFunc[ap_glBegin] = loggedEnumFunc_glBegin;
            _loggedEnumFuncToEnumPosInArgList[ap_glBegin] = 0;

            // glDrawArrays:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawArrays] = loggedEnumFunc_glDrawArrays;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawArrays] = 0;

            // glDrawArraysIndirect:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawArraysIndirect] = loggedEnumFunc_glDrawArraysIndirect;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawArraysIndirect] = 0;

            // glDrawArraysInstanced:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawArraysInstanced] = loggedEnumFunc_glDrawArraysInstanced;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawArraysInstanced] = 0;

            // glDrawArraysInstancedEXT:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawArraysInstancedEXT] = loggedEnumFunc_glDrawArraysInstancedEXT;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawArraysInstancedEXT] = 0;

            // glDrawArraysInstancedARB:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawArraysInstancedARB] = loggedEnumFunc_glDrawArraysInstancedARB;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawArraysInstancedARB] = 0;

            // glDrawArraysInstancedBaseInstance:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawArraysInstancedBaseInstance] = loggedEnumFunc_glDrawArraysInstancedBaseInstance;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawArraysInstancedBaseInstance] = 0;

            // glDrawElements:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawElements] = loggedEnumFunc_glDrawElements;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawElements] = 0;

            // glDrawElementsIndirect:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawElementsIndirect] = loggedEnumFunc_glDrawElementsIndirect;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawElementsIndirect] = 0;

            // glDrawElementsInstanced:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawElementsInstanced] = loggedEnumFunc_glDrawElementsInstanced;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawElementsInstanced] = 0;

            // glDrawElementsInstancedEXT:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawElementsInstancedEXT] = loggedEnumFunc_glDrawElementsInstancedEXT;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawElementsInstancedEXT] = 0;

            // glDrawElementsInstancedARB:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawElementsInstancedARB] = loggedEnumFunc_glDrawElementsInstancedARB;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawElementsInstancedARB] = 0;

            // glDrawElementsInstancedBaseInstance:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawElementsInstancedBaseInstance] = loggedEnumFunc_glDrawElementsInstancedBaseInstance;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawElementsInstancedBaseInstance] = 0;

            // glDrawElementsInstancedBaseVertexBaseInstance:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawElementsInstancedBaseVertexBaseInstance] = loggedEnumFunc_glDrawElementsInstancedBaseVertexBaseInstance;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawElementsInstancedBaseVertexBaseInstance] = 0;

            // glDrawRangeElements:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawRangeElements] = loggedEnumFunc_glDrawRangeElements;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawRangeElements] = 0;

            // glDrawTransformFeedback:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawTransformFeedback] = loggedEnumFunc_glDrawTransformFeedback;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawTransformFeedback] = 0;

            // glDrawTransformFeedbackStream:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawTransformFeedbackStream] = loggedEnumFunc_glDrawTransformFeedbackStream;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawTransformFeedbackStream] = 0;

            // glDrawTransformFeedbackInstanced:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawTransformFeedbackInstanced] = loggedEnumFunc_glDrawTransformFeedbackInstanced;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawTransformFeedbackInstanced] = 0;

            // glDrawTransformFeedbackStreamInstanced:
            _monitoredFuncIdToLoggedEnumFunc[ap_glDrawTransformFeedbackStreamInstanced] = loggedEnumFunc_glDrawTransformFeedbackStreamInstanced;
            _loggedEnumFuncToEnumPosInArgList[ap_glDrawTransformFeedbackStreamInstanced] = 0;

            // glMultiDrawArrays:
            _monitoredFuncIdToLoggedEnumFunc[ap_glMultiDrawArrays] = loggedEnumFunc_glMultiDrawArrays;
            _loggedEnumFuncToEnumPosInArgList[ap_glMultiDrawArrays] = 0;

            // glMultiDrawArraysEXT:
            _monitoredFuncIdToLoggedEnumFunc[ap_glMultiDrawArraysEXT] = loggedEnumFunc_glMultiDrawArraysEXT;
            _loggedEnumFuncToEnumPosInArgList[ap_glMultiDrawArraysEXT] = 0;

            // glMultiDrawArraysIndirect:
            _monitoredFuncIdToLoggedEnumFunc[ap_glMultiDrawArraysIndirect] = loggedEnumFunc_glMultiDrawArraysIndirect;
            _loggedEnumFuncToEnumPosInArgList[ap_glMultiDrawArraysIndirect] = 0;

            // glMultiDrawElements:
            _monitoredFuncIdToLoggedEnumFunc[ap_glMultiDrawElements] = loggedEnumFunc_glMultiDrawElements;
            _loggedEnumFuncToEnumPosInArgList[ap_glMultiDrawElements] = 0;

            // glMultiDrawElementsEXT:
            _monitoredFuncIdToLoggedEnumFunc[ap_glMultiDrawElementsEXT] = loggedEnumFunc_glMultiDrawElementsEXT;
            _loggedEnumFuncToEnumPosInArgList[ap_glMultiDrawElementsEXT] = 0;

            // glMultiDrawElementsIndirect:
            _monitoredFuncIdToLoggedEnumFunc[ap_glMultiDrawElementsIndirect] = loggedEnumFunc_glMultiDrawElementsIndirect;
            _loggedEnumFuncToEnumPosInArgList[ap_glMultiDrawElementsIndirect] = 0;

            // glIsEnabled:
            _monitoredFuncIdToLoggedEnumFunc[ap_glIsEnabled] = loggedEnumFunc_glIsEnabled;
            _loggedEnumFuncToEnumPosInArgList[ap_glIsEnabled] = 0;
        }
    }

    vecInitCSLocker.leaveCriticalSection();
}


// ---------------------------------------------------------------------------
// Name:        suCallsStatisticsLogger::getCurrentStatistics
// Description: Summarizes the current statistics, and put into apStatistics class
//              the function calls statistics details
// Arguments: apStatistics* pStatistics
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/7/2008
// ---------------------------------------------------------------------------
bool suCallsStatisticsLogger::getCurrentStatistics(apStatistics* pStatistics) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(pStatistics != NULL)
    {
        // Clear the statistics vector;
        pStatistics->clearFunctionCallsStatistics();

        // Calculate the number of function call during full frames:
        gtUInt64 amountOfFunctionCallsInFullFrames = 0;

        for (int i = 0; i < apMonitoredFunctionsAmount; i++)
        {
            amountOfFunctionCallsInFullFrames += _fullFramesFunctionCallsAmount[i];
        }

        // Set the full frames function calls amount:
        pStatistics->setAmountOfFunctionCallsInFullFrames(amountOfFunctionCallsInFullFrames);

        // Iterate the monitored functions:
        for (int i = 0; i < apMonitoredFunctionsAmount; i++)
        {
            // If the current monitored function was called in the previous frame:
            gtUInt64 curFuncCallsAmount = _fullFramesFunctionCallsAmount[i] + _currentFrameFunctionCallsAmount[i] + _globalFunctionCallsAmount[i];
            gtUInt64 curFuncFullFramesCallsAmount = _fullFramesFunctionCallsAmount[i];
            gtUInt64 curFuncRedundantCallsAmount = _fullFramesRedundantFunctionCallsAmount[i] + _currentRedundantFunctionCallsAmount[i];
            gtUInt64 curFuncFullFramesRedundantCallsAmount = _fullFramesRedundantFunctionCallsAmount[i];

            if (curFuncCallsAmount != 0)
            {
                // Create a function calls statistics object:
                apFunctionCallStatistics* pFuncCallStatistics = new apFunctionCallStatistics;

                // Fill the function details and statistics:
                pFuncCallStatistics->_functionId = (apMonitoredFunctionId)i;
                pFuncCallStatistics->_amountOfTimesCalled = curFuncCallsAmount;
                pFuncCallStatistics->_amountOfRedundantTimesCalled = curFuncRedundantCallsAmount;

                // Add the amount of deprecated functions:
                for (int j = 0; j < AP_DEPRECATION_STATUS_AMOUNT; j++)
                {
                    pFuncCallStatistics->_amountOfDeprecatedTimesCalled[j] = _fullFramesDeprecationFunctionCallCounter[j][i] + _currentFrameDeprecationFunctionCallCounter[j][i];
                }

                if (_fullFramesCount > 0)
                {
                    pFuncCallStatistics->_averageAmountPerFrame = (gtUInt64)curFuncFullFramesCallsAmount / _fullFramesCount;
                    pFuncCallStatistics->_averageRedundantAmountPerFrame = (gtUInt64)curFuncFullFramesRedundantCallsAmount / _fullFramesCount;

                    for (int j = 0; j < AP_DEPRECATION_STATUS_AMOUNT; j++)
                    {
                        pFuncCallStatistics->_averageDeprecatedAmountPerFrame[j] = _fullFramesDeprecationFunctionCallCounter[j][i] / _fullFramesCount;
                    }
                }
                else
                {
                    pFuncCallStatistics->_averageAmountPerFrame = 0;
                    pFuncCallStatistics->_averageRedundantAmountPerFrame = 0;

                    for (int j = 0; j < AP_DEPRECATION_STATUS_AMOUNT; j++)
                    {
                        pFuncCallStatistics->_averageDeprecatedAmountPerFrame[j] = 0;
                    }
                }

                // Fill the function enumerators statistics:
                fillFunctionEnumeratorsStatistics(*pFuncCallStatistics);

                // Add it to the output statistics:
                pStatistics->addFunctionCallStatistics(pFuncCallStatistics);
            }
        }

        // Set amount of full frames:
        pStatistics->setAmountOfFullFrames(_fullFramesCount);

        retVal = true;
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suCallsStatisticsLogger::addFunctionCall
// Description: Adds a function call to the collected statistics.
// Arguments: calledFunctionIndex - The called function.
//            argumentsAmount - The amount of function arguments.
//            pArgumentList - The function arguments, as a type, value pairs.
//                            Example: OS_TOBJ_ID_GL_INT_PARAMETER, 3,
//                                     OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4.4
// Author:      Yaki Tebeka
// Date:        30/1/2006
// ---------------------------------------------------------------------------
void suCallsStatisticsLogger::addFunctionCall(int calledFunctionIndex)
{
    if (_isInComputationFrame)
    {
        // Increase the current calls amount counter:
        _currentFrameFunctionCallsAmount[calledFunctionIndex]++;

        // Handle functions for which we log enumerators usage:
        // ---------------------------------------------------
        // Get the _currentFrameEnumeratorsUsage index of the called function:
        int funVecIndex = _monitoredFuncIdToLoggedEnumFunc[calledFunctionIndex];

        // If it is a function for which we log enumerators:
        if (funVecIndex != -1)
        {
            // Get the function enumerators statistics vector:
            gtVector<apEnumeratorUsageStatistics>& enumsStatisticsVec = _currentFrameEnumeratorsUsage[funVecIndex];
            gtVector<apEnumeratorUsageStatistics>& totalEnumStatisticsVec = _totalEnumeratorsUsage[funVecIndex];
            gtVector<apEnumeratorUsageStatistics>& fullFramesEnumStatisticsVec = _fullFramesEnumeratorsUsage[funVecIndex];

            // Look for the used enumerator statistics handler:
            bool haveEnumStatisticHandler = false;
            int amountOfEnumeratorsUsed = (int)enumsStatisticsVec.size();

            for (int i = 0; i < amountOfEnumeratorsUsed; i++)
            {
                // If we already have an handler for the used enumerator:
                apEnumeratorUsageStatistics& curEnumStatisticsHolder = enumsStatisticsVec[i];
                apEnumeratorUsageStatistics& totalEnumStatisticsHolder = totalEnumStatisticsVec[i];

                if (curEnumStatisticsHolder._enum == _currentFunctionCallEnumValue)
                {
                    // Increment the enum statistics:
                    (curEnumStatisticsHolder._amountOfTimesUsed)++;

                    // Increment the total usage count of the enum:
                    (totalEnumStatisticsHolder._amountOfTimesUsed)++;

                    // Mark that we found the enum statistics handler:
                    haveEnumStatisticHandler = true;
                    break;
                }
            }

            // If we don't have an handler for the used enumerator:
            if (!haveEnumStatisticHandler)
            {
                // Implementation note:
                // We add the enumerator handler to full frames vector, current frame vector, and total vector
                // This makes the three vectors indices similar (the same index refers to the same
                // enumerator in both vectors).

                apEnumeratorUsageStatistics enumStatisticsHandler;

                enumStatisticsHandler._enum = _currentFunctionCallEnumValue;
                enumStatisticsHandler._enumType = _currentFunctionCallEnumType;

                enumStatisticsHandler._amountOfTimesUsed = 1;

                // a. Add it to the full frames statistics:
                fullFramesEnumStatisticsVec.push_back(enumStatisticsHandler);

                // b. Add it to the total and full frames counters:
                totalEnumStatisticsVec.push_back(enumStatisticsHandler);

                // c. Add it to the current frame statistics:
                enumsStatisticsVec.push_back(enumStatisticsHandler);
            }
        }
    }
    else
    {
        // Handle global function calls amount:
        _globalFunctionCallsAmount[calledFunctionIndex]++;
    }

    // OpenGL ES does not define a deprecation model:
#ifndef _GR_IPHONE_BUILD
    // Increase the current calls deprecation counter:
    _currentFrameDeprecationFunctionCallCounter[_currentFunctionCallDeprecationStatus][calledFunctionIndex]++;
#endif
}



// ---------------------------------------------------------------------------
// Name:        SU_GET_FIRST_ARG_AS_ENUM_VAL
// Description: Retrieves the first argument value (as a GLenum).
// Arguments: enumVal - GLenum that will get the first argument value.
//            argumentType - osTransferableObjectType that will get the first
//                              argument type
//            pArgumentList - A function's argument list.
// Author:      Yaki Tebeka
// Date:        30/1/2006
// ---------------------------------------------------------------------------
#define SU_GET_FIRST_ARG_AS_ENUM_VAL(argumentType, enumVal, pArgumentList)                                  \
    va_list pCurrentArgument;                                                                               \
    va_copy(pCurrentArgument, pArgumentList);                                                               \
    argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument, int));                               \
    enumVal = va_arg(pCurrentArgument , unsigned long);                                                     \
    va_end(pCurrentArgument);


// ---------------------------------------------------------------------------
// Name:        SU_GET_SECOND_ARG_AS_ENUM_VAL
// Description: Retrieves the second argument value (as a GLenum).
// Arguments: enumVal - GLenum that will get the second argument value.
//            secondArgumentType - osTransferableObjectType that will get the
//                                  second argument type
//            pArgumentList - A function's argument list.
//
// Implementation notes:
//   We assume that the first parameter is also an enum !!!
//   (Otherwise, we would need to implement a mechanism that detects the first
//    parameter type. This mechanism in not currently needed and will probably reduce
//    performance)
//  The first parameter is simply skipped (rgorton: 28-Aug-2012)
//
// Author:      Yaki Tebeka
// Date:        5/2/2006
// ---------------------------------------------------------------------------
#define SU_GET_SECOND_ARG_AS_ENUM_VAL(secondArgumentType, enumVal, pArgumentList)                               \
    va_list pCurrentArgument;                                                                                   \
    va_copy(pCurrentArgument, pArgumentList);                                                                   \
    osTransferableObjectType firstArgumentType = (osTransferableObjectType)(va_arg(pCurrentArgument, int));     \
    GT_ASSERT(firstArgumentType == OS_TOBJ_ID_GL_ENUM_PARAMETER);                                               \
    \
    (void) va_arg(pCurrentArgument , unsigned long);                                            \
    secondArgumentType = (osTransferableObjectType)(va_arg(pCurrentArgument, int));                             \
    enumVal = va_arg(pCurrentArgument , unsigned long);                                                         \
    va_end(pCurrentArgument);




// ---------------------------------------------------------------------------
// Name:        suCallsStatisticsLogger::saveCurrentFunctionCallAttributes
// Description: The function saves attributes of the function call that can be
//              extracted before the function call, and used after the call is done.
//              Extracts the function enumerator out of the function arguments,
//              and save if for after the function call - in order to add the function correctly
//              to the function statistics.
// Arguments: apMonitoredFunctionId calledFuncitonId - The called function id.
//            argumentsAmount - The amount of function arguments.
//            pArgumentList - The function arguments, as a type, value pairs.
//                            Example: OS_TOBJ_ID_GL_INT_PARAMETER, 3,
//                                     OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4.4
//            apFunctionDeprecationStatus deprecationStatus
// Author:      Sigal Algranaty
// Date:        17/8/2008
// ---------------------------------------------------------------------------
void suCallsStatisticsLogger::saveCurrentFunctionCallAttributes(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus deprecationStatus)
{
    (void)(argumentsAmount); // unused
    // Save the function deprecation status:
    _currentFunctionCallDeprecationStatus = deprecationStatus;

    // Handle functions for which we log enumerators usage:
    // ---------------------------------------------------

    // Get the _currentFrameEnumeratorsUsage index of the called function:
    int funVecIndex = _monitoredFuncIdToLoggedEnumFunc[calledFunctionId];

    // If it is a function for which we log enumerators:
    if (funVecIndex != -1)
    {
        // Get the enum index in the function arguments list:
        int enumIndex = _loggedEnumFuncToEnumPosInArgList[calledFunctionId];

        if (enumIndex == 0)
        {
            // Get the enum value:
            SU_GET_FIRST_ARG_AS_ENUM_VAL(_currentFunctionCallEnumType, _currentFunctionCallEnumValue, pArgumentList);
        }
        else if (enumIndex == 1)
        {
            // Get the enum value:
            SU_GET_SECOND_ARG_AS_ENUM_VAL(_currentFunctionCallEnumType, _currentFunctionCallEnumValue, pArgumentList);
        }
        else
        {
            // An error occurred:
            gtString errorString = SU_STR_UnsupportedParamIndex;
            errorString.appendFormattedString(L"%d", enumIndex);
            GT_ASSERT_EX(0, errorString.asCharArray());
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        suCallsStatisticsLogger::clearFunctionCallsStatistics
// Description: Clear the function calls statistics
// Arguments:
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/7/2008
// ---------------------------------------------------------------------------
bool suCallsStatisticsLogger::clearFunctionCallsStatistics()
{
    bool retVal = true;

    // Initialize statistics arrays:
    ::memset(_currentFrameFunctionCallsAmount, 0, _statisticsVectorSize);

    // Initialize statistics arrays:
    ::memset(_globalFunctionCallsAmount, 0, _statisticsVectorSize);

    // Initialize current frame statistics:
    ::memset(_fullFramesFunctionCallsAmount, 0, _statisticsVectorSize);

    // Initialize statistics arrays:
    ::memset(_currentRedundantFunctionCallsAmount, 0, _statisticsVectorSize);

    // Initialize current frame statistics:
    ::memset(_fullFramesRedundantFunctionCallsAmount, 0, _statisticsVectorSize);

    // Initialize the current frame deprecation statistics:
    ::memset(_currentFrameDeprecationFunctionCallCounter, 0, _statisticsVectorSize * AP_DEPRECATION_STATUS_AMOUNT);

    // Initialize the deprecation statistics:
    ::memset(_fullFramesDeprecationFunctionCallCounter, 0, _statisticsVectorSize * AP_DEPRECATION_STATUS_AMOUNT);

    // Iterate the functions to which we support enumerators logging:
    for (int i = 0; i < amountOfLoggedEnumFunctions; i++)
    {
        // Get the function's current and previous frame enumerators statistics holders:
        gtVector<apEnumeratorUsageStatistics>& currFuncFullFramesLoggedEnumsStat = _fullFramesEnumeratorsUsage[i];
        gtVector<apEnumeratorUsageStatistics>& currFuncCurrFrameLoggedEnumsStat = _currentFrameEnumeratorsUsage[i];
        gtVector<apEnumeratorUsageStatistics>& currTotalFrameLoggedEnumsStat = _totalEnumeratorsUsage[i];

        // Iterate the logged enumerators:
        int amountOfLoggedEnums = (int)currFuncCurrFrameLoggedEnumsStat.size();

        for (int j = 0; j < amountOfLoggedEnums; j++)
        {
            // Copy the current frame statistics to the previous frame statistics:
            currFuncFullFramesLoggedEnumsStat[j]._amountOfTimesUsed = 0;
            currFuncFullFramesLoggedEnumsStat[j]._amountOfRedundantTimesUsed = 0;

            // Initialize current frame statistics:
            currFuncCurrFrameLoggedEnumsStat[j]._amountOfTimesUsed = 0;
            currFuncCurrFrameLoggedEnumsStat[j]._amountOfRedundantTimesUsed = 0;

            // Initialize total statistics:
            currTotalFrameLoggedEnumsStat[j]._amountOfTimesUsed = 0;
            currTotalFrameLoggedEnumsStat[j]._amountOfRedundantTimesUsed = 0;

        }
    }

    // Clear full frames count:
    _fullFramesCount = 0;
    _wasFirstFrameSkipped = false;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suCallsStatisticsLogger::accumulateFunctionCallRedundancyStatus
// Description: Accumulate a given function call redundancy status
// Arguments: calledFunctionID - The id of the called function.
//            redundancyStatus - The redundancy status to be accumulated.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/7/2008
// ---------------------------------------------------------------------------
bool suCallsStatisticsLogger::accumulateFunctionCallRedundancyStatus(apMonitoredFunctionId calledFunctionID, apFunctionRedundancyStatus redundancyStatus)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((calledFunctionID < apMonitoredFunctionsAmount) && (calledFunctionID > 0))
    {
        if (redundancyStatus == AP_REDUNDANCY_REDUNDANT)
        {
            // If the function redundancy status is redundant, increase the amount
            //  of redundant calls of this callIndex:
            _currentRedundantFunctionCallsAmount[calledFunctionID]++;
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suCallsStatisticsLogger::fillFunctionEnumeratorsStatistics
// Description: Fills a given function enumerators usage statistics.
// Arguments: funcStatisticsHolder - A function statistics holder, containing
//                                   the queried function id, that will be filled
//                                   with the function enumerators usage information.
// Author:      Yaki Tebeka
// Date:        2/2/2006
// ---------------------------------------------------------------------------
void suCallsStatisticsLogger::fillFunctionEnumeratorsStatistics(apFunctionCallStatistics& funcStatisticsHolder) const
{
    // Get the queried function id:
    int queriedFuncId = funcStatisticsHolder._functionId;

    // Get its loggedEnumeratorsFunctions enum value:
    int loggedEnumFuncId = _monitoredFuncIdToLoggedEnumFunc[queriedFuncId];

    // If this is a function to which we log enumerators:
    if (loggedEnumFuncId != -1)
    {
        // Get the function enumerators statistics vector:
        const gtVector<apEnumeratorUsageStatistics>& enumsTotalStatisticsVec = _totalEnumeratorsUsage[loggedEnumFuncId];

        // Iterate the used enumerators statistics handlers:
        int amountOfEnumeratorsUsed = (int)enumsTotalStatisticsVec.size();

        for (int i = 0; i < amountOfEnumeratorsUsed; i++)
        {
            // Get the current enumerator statistics holder:
            const apEnumeratorUsageStatistics& enumTotalStatisticsHolder = enumsTotalStatisticsVec[i];

            // If the enumerator was used in the previous frame:
            if (0 < enumTotalStatisticsHolder._amountOfTimesUsed)
            {
                // Add it to the output function statistics:
                funcStatisticsHolder._usedEnumerators.push_back(enumTotalStatisticsHolder);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        suCallsStatisticsLogger::getCurrentFunctionCallEnumValue
// Description: Returns the last function call enum value
// Arguments:
//              apMonitoredFunctionsAmount calledFunctionId - the current function call id
//              GLenum& currentFunctionCallEnumValue
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/9/2008
// ---------------------------------------------------------------------------
bool suCallsStatisticsLogger::getCurrentFunctionCallEnumValue(apMonitoredFunctionId calledFunctionId, GLenum& currentFunctionCallEnumValue)const
{
    bool retVal = false;
    int funVecIndex = _monitoredFuncIdToLoggedEnumFunc[calledFunctionId];

    if (funVecIndex != -1)
    {
        currentFunctionCallEnumValue = _currentFunctionCallEnumValue;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suCallsStatisticsLogger::onComputationFrameStarted
// Description: Is handling computation frame started event
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        6/4/2010
// ---------------------------------------------------------------------------
void suCallsStatisticsLogger::onComputationFrameStarted()
{
    // Mark we are in a frame:
    _isInComputationFrame = true;
}

// ---------------------------------------------------------------------------
// Name:        suCallsStatisticsLogger::onComputationFrameEnded
// Description: Is handling computation frame ended event
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        6/4/2010
// ---------------------------------------------------------------------------
void suCallsStatisticsLogger::onComputationFrameEnded()
{
    // Mark we are not in a frame:
    _isInComputationFrame = false;
}
