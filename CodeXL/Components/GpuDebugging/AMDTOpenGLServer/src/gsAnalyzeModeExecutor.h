//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsAnalyzeModeExecutor.h
///
//==================================================================================

//------------------------------ gsAnalyzeModeExecutor.h ------------------------------

#ifndef __GSANALYZEMODEEXECUTOR
#define __GSANALYZEMODEEXECUTOR

class gsRenderContextMonitor;

// Infra:
// Forward declaration;
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apFunctionType.h>

// Local:
#include <src/gsStateVariablesSnapshot.h>

// ----------------------------------------------------------------------------------
// Struct Name:   gsAnalyzeModeExecutor
// General Description:
//   Handles redundant statistics change events.
//
// Author:               Sigal Algranaty
// Creation Date:        31/8/2008
// ----------------------------------------------------------------------------------
class gsAnalyzeModeExecutor
{
public:
    gsAnalyzeModeExecutor();
    virtual ~gsAnalyzeModeExecutor();

    // Events:
    void addFunctionCall(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList);
    bool initialize(gsRenderContextMonitor* pRenderContextMonitor);
    void onFirstTimeContextMadeCurrent();
    void afterMonitoredFunctionExecutionActions(apMonitoredFunctionId calledFunctionId);

private:
    bool takeStateVariablesSnapshotForStateChangeFunctionCall(apMonitoredFunctionId calledFunctionId, bool beforeFunctionCall);
    void startStateVariablesAnalyzeLogging(apMonitoredFunctionId calledFunctionId);

    void getRedundancyStatusWithinBeginEndBlock(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList);
    bool comparePreviousToCurrentStateVariablesSnapshots(bool& areValuesEqual);
    bool setFunctionRedundancyStatus(int callIndex, apMonitoredFunctionId calledFunctionID, apFunctionRedundancyStatus redundancyStatus);
    void initStaticVectors();

    void getAllBeginEndRelevantStateVariables(gtVector<apOpenGLStateVariableId>*& pStateVariablesForBeginEndBlock);
    bool getFunctionCallStateVariablesVector(apMonitoredFunctionId calledFunctionId, GLenum functionEnum, bool isEnumUsed , gtVector<apOpenGLStateVariableId>*& pFilteredStateVariableIds);
    bool isStateVariableComparisionNeededForFunctionInBeginEndBlock(int functionCallId, bool& shouldBeCompared);

    void removeStateVariablesFromVectorByImplementation(gtVector<apOpenGLStateVariableId>& stateVariablesVector);

private:
    // Holds a flag for each function call, which notes if the function is supported in begin-end
    // block, and if the function is a state change function:
    static bool* _isFunctionSupportedInBeginEndBlock;

    // State variables that can be changed while in begin end block:
    gtVector<apOpenGLStateVariableId> _beginEndStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glNormalStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glPixelStoreStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glViewportStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glTempStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glAlphaTestStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glAutoNormalStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glBlendStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glTextureStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glCullFaceStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glLightingStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glMatrixStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glTexCoordPointerStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glColorPointerStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glVertexPointerStateVariableIdsVec;
    gtVector<apOpenGLStateVariableId> _glNormalArrayStateVariableIds;
    gtVector<apOpenGLStateVariableId> _glFogCoordStateVariableIdsVec;

    // The id of this context:
    int _contextId;

    // A pointer to a render context monitor:
    gsRenderContextMonitor* _pRenderContextMonitor;

    // A snapshot of the render context state variables values before the last function call:
    // (Recorded only in analyze mode, before function calls)
    gsStateVariablesSnapshot* _pStateVaraiblesValues1;
    gsStateVariablesSnapshot* _pStateVaraiblesValues2;

    // Snapshot used for glBegin and glEnd calls. These snapshots help compare only the
    // relevant state variables for function supported in begin-end block;
    gsStateVariablesSnapshot _glBeginStateVariableSnapShot;

    // Redundancy status is computed inside begin end block before the function call, and used later after the function call:
    apFunctionRedundancyStatus _lastComputedRedundancyStatus;

    // Used to avoid a bug where switching modes causes the first function to always be flagged as redundant:
    bool _wasCalledBeforeFunction;
};


#endif  // __GSANALYZEMODEEXECUTOR 
