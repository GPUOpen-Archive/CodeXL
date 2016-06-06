//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaATIFunctionWrapper.h
///
//=====================================================================

//------------------------------ oaATIFunctionWrapper.h ------------------------------

#ifndef __OAATIFUNCTIONWRAPPER
#define __OAATIFUNCTIONWRAPPER

#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#ifdef OA_DEBUGGER_USE_AMD_GPA

// ATI:
#include <GPUPerfAPIFunctionTypes.h>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaOSAPIWrappersDLLBuild.h>
#include <AMDTOSAPIWrappers/Include/oaATIFunctionTypes.h>

// ----------------------------------------------------------------------------------
// Class Name:           OA_API oaATIFunctionWrapper
// General Description: Wraps ATI functions for performance counters monitoring
// Author:      AMD Developer Tools Team
// Creation Date:        24/2/2010
// ----------------------------------------------------------------------------------
class OA_API oaATIFunctionWrapper
{
public:

    // An instance for the ATI OpenGL function wrapper:
    static oaATIFunctionWrapper& gl_instance();

    // An instance for the ATI OpenCL function wrapper:
    static oaATIFunctionWrapper& cl_instance();

    // osSingeltonsDelete will delete the above static members:
    friend class oaSingeltonsDelete;

    bool initialize(const gtString& atiDllFileName);

private:
    // Do not allow the use of my default constructor:
    oaATIFunctionWrapper();

public:

    GPA_Status GPA_Initialize();
    GPA_Status GPA_Destroy();
    GPA_Status GPA_OpenContext(void* context);
    GPA_Status GPA_CloseContext();
    GPA_Status GPA_SelectContext(void* context);
    GPA_Status GPA_GetNumCounters(gpa_uint32* count);
    GPA_Status GPA_GetCounterName(gpa_uint32 index, const char** name);
    GPA_Status GPA_GetCounterDescription(gpa_uint32 index, const char** description);
    GPA_Status GPA_GetCounterDataType(gpa_uint32 index, GPA_Type* counterDataType);
    GPA_Status GPA_GetCounterUsageType(gpa_uint32 index, GPA_Usage_Type* counterUsageType);
    GPA_Status GPA_GetDataTypeAsStr(GPA_Type counterDataType, const char** typeStr);
    GPA_Status GPA_GetUsageTypeAsStr(GPA_Usage_Type counterUsageType, const char** usageTypeStr);
    GPA_Status GPA_EnableCounter(gpa_uint32 index);
    GPA_Status GPA_DisableCounter(gpa_uint32 index);
    GPA_Status GPA_GetEnabledCount(gpa_uint32* count);
    GPA_Status GPA_GetEnabledIndex(gpa_uint32 enabledNumber, gpa_uint32* enabledCounterIndex);
    GPA_Status GPA_IsCounterEnabled(gpa_uint32 counterIndex);
    GPA_Status GPA_EnableCounterStr(const char* counter);
    GPA_Status GPA_DisableCounterStr(const char* counter);
    GPA_Status GPA_EnableAllCounters();
    GPA_Status GPA_DisableAllCounters();
    GPA_Status GPA_GetCounterIndex(const char* counter, gpa_uint32* index);
    GPA_Status GPA_GetPassCount(gpa_uint32* numPasses);
    GPA_Status GPA_BeginSession(gpa_uint32* sessionID);
    GPA_Status GPA_EndSession();
    GPA_Status GPA_BeginPass();
    GPA_Status GPA_EndPass();
    GPA_Status GPA_BeginSample(gpa_uint32 sampleID);
    GPA_Status GPA_EndSample();
    GPA_Status GPA_GetSampleCount(gpa_uint32 sessionID, gpa_uint32* samples);
    GPA_Status GPA_IsSampleReady(bool* readyResult, gpa_uint32 sessionID, gpa_uint32 sampleID);
    GPA_Status GPA_IsSessionReady(bool* readyResult, gpa_uint32 sessionID);
    GPA_Status GPA_GetSampleUInt64(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterID, gpa_uint64* result);
    GPA_Status GPA_GetSampleUInt32(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterIndex, gpa_uint32* result);
    GPA_Status GPA_GetSampleFloat64(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterIndex, gpa_float64* result);
    GPA_Status GPA_GetSampleFloat32(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterIndex, gpa_float32* result);
    const char* GPA_GetStatusAsStr(GPA_Status status);



private:

    // The ATI DLL path:
    osFilePath _atiDllFilePath;

    // Is the function wrapper initialized:
    bool _isInitialized;

    PFNGPA_INITIALIZEPROC _GPA_Initialize;
    PFNGPA_DESTROYPROC _GPA_Destroy;
    PFNGPA_OPENCONTEXTPROC _GPA_OpenContext;
    PFNGPA_CLOSECONTEXTPROC _GPA_CloseContext;
    PFNGPA_SELECTCONTEXTPROC _GPA_SelectContext;
    PFNGPA_GETNUMCOUNTERSPROC _GPA_GetNumCounters;
    PFNGPA_GETCOUNTERNAMEPROC _GPA_GetCounterName;
    PFNGPA_GETCOUNTERDESCRIPTIONPROC _GPA_GetCounterDescription;
    PFNGPA_GETCOUNTERDATATYPEPROC _GPA_GetCounterDataType;
    PFNGPA_GETCOUNTERUSAGETYPEPROC _GPA_GetCounterUsageType;
    PFNGPA_GETDATATYPEASSTRPROC _GPA_GetDataTypeAsStr;
    PFNGPA_GETUSAGETYPEASSTRPROC _GPA_GetUsageTypeAsStr;
    PFNGPA_ENABLECOUNTERPROC _GPA_EnableCounter;
    PFNGPA_DISABLECOUNTERPROC _GPA_DisableCounter;
    PFNGPA_GETENABLEDCOUNTPROC _GPA_GetEnabledCount;
    PFNGPA_GETENABLEDINDEXPROC _GPA_GetEnabledIndex;
    PFNGPA_ISCOUNTERENABLEDPROC _GPA_IsCounterEnabled;
    PFNGPA_ENABLECOUNTERSTRPROC _GPA_EnableCounterStr;
    PFNGPA_DISABLECOUNTERSTRPROC _GPA_DisableCounterStr;
    PFNGPA_ENABLEALLCOUNTERSPROC _GPA_EnableAllCounters;
    PFNGPA_DISABLEALLCOUNTERSPROC _GPA_DisableAllCounters;
    PFNGPA_GETCOUNTERINDEXPROC _GPA_GetCounterIndex;
    PFNGPA_GETPASSCOUNTPROC _GPA_GetPassCount;
    PFNGPA_BEGINSESSIONPROC _GPA_BeginSession;
    PFNGPA_ENDSESSIONPROC _GPA_EndSession;
    PFNGPA_BEGINPASSPROC _GPA_BeginPass;
    PFNGPA_ENDPASSPROC _GPA_EndPass;
    PFNGPA_BEGINSAMPLEPROC _GPA_BeginSample;
    PFNGPA_ENDSAMPLEPROC _GPA_EndSample;
    PFNGPA_GETSAMPLECOUNTPROC _GPA_GetSampleCount;
    PFNGPA_ISSAMPLEREADYPROC _GPA_IsSampleReady;
    PFNGPA_ISSESSIONREADYPROC _GPA_IsSessionReady;
    PFNGPA_GETSAMPLEUINT64PROC _GPA_GetSampleUInt64;
    PFNGPA_GETSAMPLEUINT32PROC _GPA_GetSampleUInt32;
    PFNGPA_GETSAMPLEFLOAT64PROC _GPA_GetSampleFloat64;
    PFNGPA_GETSAMPLEFLOAT32PROC _GPA_GetSampleFloat32;
    PFNGPA_GETSTATUSASSTRPROC _GPA_GetStatusAsStr;

    // This class single instance:
    static oaATIFunctionWrapper* _pMySingleGLInstance;
    static oaATIFunctionWrapper* _pMySingleCLInstance;

};

#endif // OA_DEBUGGER_USE_AMD_GPA

#endif  // __OSATIFUNCTIONWRAPPER


