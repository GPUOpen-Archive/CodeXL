//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csAMDKernelDebuggingManager.h
///
//==================================================================================

//------------------------------ csAMDKernelDebuggingManager.h ------------------------------

#ifndef __CSAMDKERNELDEBUGGINGMANAGER_H
#define __CSAMDKERNELDEBUGGINGMANAGER_H

// Forward declarations:
class osCallStack;
class osFilePath;
class csDWARFParser;
struct csDWARFProgram;
struct csDWARFVariable;

// AMD Server Utilities:
#include <AMDTServerUtilities/Include/suIKernelDebuggingManager.h>

// AMD CL Debugging API:
#include <AMDOpenCLDebug.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTAPIClasses/Include/apBreakReason.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apKernelDebuggingCommand.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>

// ----------------------------------------------------------------------------------
// Class Name:          csAMDKernelDebuggingSession
// General Description: Contains all the information needed for kernel debugging.
//                      This struct is created at kernel enqueuement and is passed
//                      as the kernel debugging function's user data.
// Author:              Uri Shomroni
// Creation Date:       26/4/2011
// ----------------------------------------------------------------------------------
struct csAMDKernelDebuggingSession
{
public:
    enum csAMDKernelDebuggingSessionReason
    {
        CS_STEP_IN_COMMAND,
        CS_KERNEL_FUNCTION_NAME_BREAKPOINT,
        CS_KERNEL_SOURCE_CODE_BREAKPOINT
    };

public:
    csAMDKernelDebuggingSession(oaCLKernelHandle kernel, apMonitoredFunctionId kernelDebuggingFunction, suIKernelDebuggingManager::KernelDebuggingSessionReason reason, unsigned int workDimension, const gtSize_t* globalWorkOffset, const gtSize_t* globalWorkSize, const gtSize_t* localWorkSize);
    ~csAMDKernelDebuggingSession();

public:
    // The DWARF parser:
    csDWARFParser* _pDWARFParser;

    // Used to handle the deletion of this struct, to compensate for the options of synchronous and asynchronous debugging:
    bool _canDebugSessionBeDeleted;

    // The debugging context and kernel:
    apContextID _debuggedKernelComputeContextId;
    oaCLKernelHandle _debuggedKernelHandle;

    // The debugging program
    oaCLProgramHandle _debuggedKernelContainingProgramHandle;
    gtString _debuggedKernelContainingProgramSourceFilePath;

    // The function used to enqueue the kernel:
    apMonitoredFunctionId _debuggingKernelFunction;
    suIKernelDebuggingManager::KernelDebuggingSessionReason _kernelDebuggingReason;

    // The global and local work location:
    int _debuggedKernelGlobalWorkOffset[3];
    int _debuggedKernelGlobalWorkSize[3];
    int _debuggedKernelLocalWorkSize[3];
    int _debuggedKernelTotalGlobalWorkSize;

    // The kernel binary:
    gtUByte* _debuggedKernelBinary;
    gtSize_t _debuggedKernelBinarySize;

    // Details related to the debug API and the client API:
    amdclDebugContext _kernelDebugContext;
    int _lastDebuggedKernelLineNumber;
    bool* _debuggedKernelValidWorkItems;
    int _steppingWorkItem[3];
    amdclDebugPC _debuggedKernelProgramCounter;
    amdclDebugPC _debuggedKernelLastProgramCounter;
    amdclDebugPC _debuggedKernelLastStoppedProgramCounter;
    bool _debuggedKernelFirstCallback;
    bool _debuggedKernelFirstBreakpoint;
    apBreakReason _continuedStepBreakReason;
    gtMap<amdclDebugPC, amdclDebugPC> _debuggedKernelBoundToRequestedBreakpoints;
    gtMap<amdclDebugPC, amdclDebugPC> _debuggedKernelRequestedToBoundBreakpoints;

    // Emulate breakpoints:
    gtVector<amdclDebugPC> _debuggedKernelAllPCsWithLineNumbers;
    gtVector<amdclDebugPC> _debuggedKernelCurrentBreakpoints;
    bool _areStepBreakpointsSet;
    bool _stepBreakpointsSetSuccessfully;

    // The next command we will send to the kernel debugging API:
    amdclDebugCommand _nextDebuggingCommand;
    bool _kernelDebuggingCallbackWaiting;
};

// ----------------------------------------------------------------------------------
// Class Name:          csAMDKernelDebuggingManager
// General Description: Manages connections to the AMD kernel debugging API and DWARF
// Author:              Uri Shomroni
// Creation Date:       21/11/2010
// ----------------------------------------------------------------------------------
class csAMDKernelDebuggingManager : public suIKernelDebuggingManager
{
public:
    csAMDKernelDebuggingManager();
    ~csAMDKernelDebuggingManager();
    static csAMDKernelDebuggingManager& instance();

    // General kernel debugging API functions:
    virtual bool initialize();
    virtual bool terminate();
    virtual bool programBuildFlagsSupported(const char* buildFlags, gtString& failureReason);
    virtual KernelDebuggingManagerType kernelDebuggerType();

    // Functions used in debugging kernels:
    virtual oaCLKernelHandle currentlyDebuggedKernel();
    virtual bool getGlobalWorkGeometry(int& o_workDimensions, int o_globalWorkSize[3], int o_localWorkSize[3], int o_globalWorkOffset[3]);
    virtual void setNextKernelDebuggingCommand(apKernelDebuggingCommand command);
    virtual bool setSteppingWorkItem(const int coordinate[3]);
    virtual bool isWorkItemValid(const int coordinate[3]);
    virtual bool getFirstValidWorkItem(int wavefrontIndex /*= -1*/, int o_coordinate[3]);
    virtual void releaseCallbackToNextCommand();

    // Aid functions:
    virtual bool getCurrentKernelCallStack(const int coordinate[3], osCallStack& kernelStack);
    virtual int getCurrentKernelDebugLineNumber(const int coordinate[3]);
    virtual bool doesVariableExistInCurrentScope(const gtString& variableName, const int coordinate[3]);
    virtual bool getVariableValueString(const gtString& variableName, const int coordinate[3], gtString& valueString, gtString& valueStringHex, gtString& variableType);
    virtual bool getVariableMembers(const gtString& variableName, const int coordinate[3], gtVector<gtString>& memberNames);
    virtual bool getAvailableVariables(const int coordinate[3], gtVector<gtString>& variableNames, bool getLeaves, int stackFrameDepth);
    virtual bool getExecutionMask(bool*& executionMask, int& maskSize);
    virtual bool getAmountOfActiveWavefronts(int& amountOfWavefronts);
    virtual bool getActiveWavefrontID(int wavefrontIndex, int& wavefrontId);
    virtual bool getWavefrontIndex(const int coordinate[3], int& o_wavefrontIndex);
    virtual bool exportVariableValuesToFile(const gtString& variableName, bool& variableTypeSupported, osFilePath& variableDataFilePath);
    virtual bool getKernelSourceBreakpointResolution(oaCLProgramHandle programHandle, int requestedLineNumber, int& resolvedLineNumber);

    // Note that this only synchronizes with this kernel debugging callback:
    static void synchronizeWithCSKernelDebuggingCallback();

    // The callback function used in debugging:
    static amdclDebugCommand csAMDclDebugCallback(amdclDebugContext debugContext, amdclDebugEvent debugEvent, void* user_data);

private:
    friend class csSingletonsDelete;

private:
    int lineNumberFromKernelProgramCounter(amdclDebugContext debugContext, amdclDebugPC programCounter, gtString& sourceFilePath);
    void kernelProgramCountersFromLineNumber(amdclDebugContext debugContext, const osFilePath& filePath, int lineNumber, gtVector<amdclDebugPC>& programCounters);
    bool getVariableValuesInCurrentLocation(const gtString& variableName, int variableIndex, const void*& variableValues, unsigned int& valueStride, csDWARFVariable& variableData, bool& wasAllocated);
    void releaseVariableValues(const void* variableValues, bool wasAllocated);

    void onKernelDebuggingStartedBreakpoint(amdclDebugContext debugContext);
    void onKernelDebuggingFinishedBreakpoint(amdclDebugContext debugContext);
    void onKernelDebuggingBreakpointHit(amdclDebugContext debugContext, apBreakReason brkReason);
    void cleanupOnKernelDebuggingEnded();

    void updateValidWorkItems();
    bool shouldContinueStepping(apBreakReason brkReason);
    void updateKernelBreakpoints(amdclDebugContext debugContext);
    int currentlyDebuggedKernelWorkDimension();
    const csDWARFVariable* getVariableDetailsInCurrentLocation(const gtString& variableName);

    void emulateStepCommand(amdclDebugCommand& debugCommand, amdclDebugContext debugContext);
    void setBreakpointsOnAllPCs(amdclDebugContext debugContext);
    void setBreakpointsFromCurrentPC(amdclDebugContext debugContext, bool includeCurrentScope);
    void addBreakpointsFromProgram(amdclDebugContext debugContext, const csDWARFProgram& program);
    void restoreRealBreakpoints(amdclDebugContext debugContext);

    static bool debugLastErrorString(amdclDebugContext debugContext, gtString& failureString);

private:
    static csAMDKernelDebuggingManager* _pMySingleInstance;

    // A handle to the module which contains the OpenCL Debugging API:
    osModuleHandle _hCLKernelDebuggingModule;

    // Holds the kernel debugging session while the callback exists:
    csAMDKernelDebuggingSession* _pCurrentKernelDebuggingSession;
};

// This specific global variable should not be used by other classes, we simply declare it privately as extern.
// Note it is initialized in csGlobalVariables.cpp, since it is used to initialize cs_stat_pIKernelDebuggingManager,
// which IS the public API:
extern csAMDKernelDebuggingManager& cs_stat_amdKernelDebuggingManager;

#endif //__CSAMDKERNELDEBUGGINGMANAGER_H

