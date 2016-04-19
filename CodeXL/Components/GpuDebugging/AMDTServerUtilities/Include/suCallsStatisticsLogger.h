//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suCallsStatisticsLogger.h
///
//==================================================================================

//------------------------------ suCallsStatisticsLogger.h ------------------------------

#ifndef __SUCALLSSTATISTICSLOGGER_H
#define __SUCALLSSTATISTICSLOGGER_H

// Standard C:
#include <stdarg.h>

// Infra:
#include <AMDTAPIClasses/Include/apStatistics.h>
#include <AMDTAPIClasses/Include/apFunctionCallStatistics.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>

// Local
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:   suCallsStatisticsLogger
// General Description:  Logs monitored function calls statistics.
// Author:               Sigal Algranaty
// Creation Date:        22/3/2010
// ----------------------------------------------------------------------------------
class SU_API suCallsStatisticsLogger
{
public:
    suCallsStatisticsLogger();
    virtual ~suCallsStatisticsLogger();

    // Virtual functions:
    virtual void onFrameTerminatorCall();
    virtual void addFunctionCall(int calledFunctionIndex);
    virtual void saveCurrentFunctionCallAttributes(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus deprecationStatus);
    virtual void onComputationFrameStarted();
    virtual void onComputationFrameEnded();

    bool getCurrentStatistics(apStatistics* pStatistics) const;
    bool accumulateFunctionCallRedundancyStatus(apMonitoredFunctionId calledFunctionID, apFunctionRedundancyStatus redundancyStatus);
    bool clearFunctionCallsStatistics();

    bool getCurrentFunctionCallEnumValue(apMonitoredFunctionId calledFunctionId, GLenum& currentFunctionCallEnumValue) const ;

private:
    void fillFunctionEnumeratorsStatistics(apFunctionCallStatistics& funcStatisticsHolder) const;
    void initStaticVectors();
    void summarizeFunctionCallsStatistics();

private:
    // Enumerates functions to which we log enumerators usage statistics:
    // !! Notice - when adding values here - add them also to initStaticVectors() !!
    enum loggedEnumeratorsFunctions
    {
        loggedEnumFunc_glEnable,
        loggedEnumFunc_glDisable,
        loggedEnumFunc_glEnableClientState,
        loggedEnumFunc_glDisableClientState,
        loggedEnumFunc_glBindBuffer,
        loggedEnumFunc_glBindBufferARB,
        loggedEnumFunc_glBindTexture,
        loggedEnumFunc_glTexParameteri,
        loggedEnumFunc_glTexParameterf,
        loggedEnumFunc_glTexParameteriv,
        loggedEnumFunc_glTexParameterfv,
        loggedEnumFunc_glBegin,
        loggedEnumFunc_glDrawArrays,
        loggedEnumFunc_glDrawArraysIndirect,
        loggedEnumFunc_glDrawArraysInstanced,
        loggedEnumFunc_glDrawArraysInstancedEXT,
        loggedEnumFunc_glDrawArraysInstancedARB,
        loggedEnumFunc_glDrawArraysInstancedBaseInstance,
        loggedEnumFunc_glDrawElements,
        loggedEnumFunc_glDrawElementsIndirect,
        loggedEnumFunc_glDrawElementsInstanced,
        loggedEnumFunc_glDrawElementsInstancedEXT,
        loggedEnumFunc_glDrawElementsInstancedARB,
        loggedEnumFunc_glDrawElementsInstancedBaseInstance,
        loggedEnumFunc_glDrawElementsInstancedBaseVertexBaseInstance,
        loggedEnumFunc_glDrawRangeElements,
        loggedEnumFunc_glDrawTransformFeedback,
        loggedEnumFunc_glDrawTransformFeedbackStream,
        loggedEnumFunc_glDrawTransformFeedbackInstanced,
        loggedEnumFunc_glDrawTransformFeedbackStreamInstanced,
        loggedEnumFunc_glMultiDrawArrays,
        loggedEnumFunc_glMultiDrawArraysEXT,
        loggedEnumFunc_glMultiDrawArraysIndirect,
        loggedEnumFunc_glMultiDrawElements,
        loggedEnumFunc_glMultiDrawElementsEXT,
        loggedEnumFunc_glMultiDrawElementsIndirect,
        loggedEnumFunc_glIsEnabled,

        amountOfLoggedEnumFunctions
    };

    // Maps apMonitoredFunctionId to loggedEnumeratorsFunctions:
    static int* _monitoredFuncIdToLoggedEnumFunc;

    // Maps loggedEnumeratorsFunctions enum to the the logged enum position
    // in the function's arguments list:
    static int* _loggedEnumFuncToEnumPosInArgList;

    // Logs the amount of calls the occurred in the current frame:
    gtUInt64 _currentFrameFunctionCallsAmount[apMonitoredFunctionsAmount];

    // Logs the amount of calls occurred in the global scope (when no frame is active):
    gtUInt64 _globalFunctionCallsAmount[apMonitoredFunctionsAmount];

    // Logs the amount of calls the occurred in the previous frame:
    gtUInt64 _fullFramesFunctionCallsAmount[apMonitoredFunctionsAmount];

    // Logs the amount of redundant calls the occurred in the current frame:
    gtUInt64 _currentRedundantFunctionCallsAmount[apMonitoredFunctionsAmount];

    // Logs the amount of redundant calls the occurred in the previous frame:
    gtUInt64 _fullFramesRedundantFunctionCallsAmount[apMonitoredFunctionsAmount];

    // Logs the amount of deprecated function calls occurred in the current frame:
    gtUInt64 _currentFrameDeprecationFunctionCallCounter[AP_DEPRECATION_STATUS_AMOUNT][apMonitoredFunctionsAmount];

    // Logs the amount of deprecated function calls occurred until now:
    gtUInt64 _fullFramesDeprecationFunctionCallCounter[AP_DEPRECATION_STATUS_AMOUNT][apMonitoredFunctionsAmount];

    // Contain the number of full frames counted since the last clear call:
    gtUInt64 _fullFramesCount;

    // The first frame is always skipped at count, since we do not want the results for the first 3 frames to be twisted:
    bool _wasFirstFrameSkipped;

    // A calls statistics vector sized (measured in bytes)
    static int _statisticsVectorSize;

    // Logs current frame enumerators usage:
    gtVector<apEnumeratorUsageStatistics> _currentFrameEnumeratorsUsage[amountOfLoggedEnumFunctions];

    // Logs full frames enumerators usage:
    gtVector<apEnumeratorUsageStatistics> _fullFramesEnumeratorsUsage[amountOfLoggedEnumFunctions];

    // Logs total (full frames + current) enumerators usage:
    // We need this, since once we summarize for get statistics, we do not want to create another apEnumeratorUsageStatistics object:
    gtVector<apEnumeratorUsageStatistics> _totalEnumeratorsUsage[amountOfLoggedEnumFunctions];

    // Stores the currently logged function enumeration value:
    GLenum _currentFunctionCallEnumValue;

    // Stores the currently logged function deprecation status:
    apFunctionDeprecationStatus _currentFunctionCallDeprecationStatus;

    // Stores the currently logged function enumeration type:
    osTransferableObjectType _currentFunctionCallEnumType;

    // cl_gremedy_computation_frame:
    bool _isInComputationFrame;

};

#endif //__SUCALLSSTATISTICSLOGGER_H
