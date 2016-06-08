//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaATIFunctionTypes.h
///
//=====================================================================

//------------------------------ oaATIFunctionTypes.h ------------------------------

#ifndef __OAATIFUNCTIONTYPES_H
#define __OAATIFUNCTIONTYPES_H

#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#ifdef OA_DEBUGGER_USE_AMD_GPA


    #include <GPUPerfAPIFunctionTypes.h>

    typedef GPA_Status(*PFNGPA_INITIALIZEPROC)();
    typedef GPA_Status(*PFNGPA_DESTROYPROC)();
    typedef GPA_Status(*PFNGPA_OPENCONTEXTPROC)(void* context);
    typedef GPA_Status(*PFNGPA_CLOSECONTEXTPROC)();
    typedef GPA_Status(*PFNGPA_SELECTCONTEXTPROC)(void* context);

    typedef GPA_Status(*PFNGPA_SELECTCONTEXTPROC)(void* context);
    typedef GPA_Status(*PFNGPA_GETNUMCOUNTERSPROC)(gpa_uint32* count);
    typedef GPA_Status(*PFNGPA_GETCOUNTERNAMEPROC)(gpa_uint32 index, const char** name);
    typedef GPA_Status(*PFNGPA_GETCOUNTERDESCRIPTIONPROC)(gpa_uint32 index, const char** description);
    typedef GPA_Status(*PFNGPA_GETCOUNTERDATATYPEPROC)(gpa_uint32 index, GPA_Type* counterDataType);
    typedef GPA_Status(*PFNGPA_GETCOUNTERUSAGETYPEPROC)(gpa_uint32 index, GPA_Usage_Type* counterUsageType);
    typedef GPA_Status(*PFNGPA_GETDATATYPEASSTRPROC)(GPA_Type counterDataType, const char** typeStr);
    typedef GPA_Status(*PFNGPA_GETUSAGETYPEASSTRPROC)(GPA_Usage_Type counterUsageType, const char** usageTypeStr);
    typedef GPA_Status(*PFNGPA_ENABLECOUNTERPROC)(gpa_uint32 index);
    typedef GPA_Status(*PFNGPA_DISABLECOUNTERPROC)(gpa_uint32 index);
    typedef GPA_Status(*PFNGPA_GETENABLEDCOUNTPROC)(gpa_uint32* count);
    typedef GPA_Status(*PFNGPA_GETENABLEDINDEXPROC)(gpa_uint32 enabledNumber, gpa_uint32* enabledCounterIndex);
    typedef GPA_Status(*PFNGPA_ISCOUNTERENABLEDPROC)(gpa_uint32 counterIndex);
    typedef GPA_Status(*PFNGPA_ENABLECOUNTERSTRPROC)(const char* counter);
    typedef GPA_Status(*PFNGPA_DISABLECOUNTERSTRPROC)(const char* counter);
    typedef GPA_Status(*PFNGPA_ENABLEALLCOUNTERSPROC)();
    typedef GPA_Status(*PFNGPA_DISABLEALLCOUNTERSPROC)();
    typedef GPA_Status(*PFNGPA_GETCOUNTERINDEXPROC)(const char* counter, gpa_uint32* index);
    typedef GPA_Status(*PFNGPA_GETPASSCOUNTPROC)(gpa_uint32* numPasses);
    typedef GPA_Status(*PFNGPA_BEGINSESSIONPROC)(gpa_uint32* sessionID);
    typedef GPA_Status(*PFNGPA_ENDSESSIONPROC)();
    typedef GPA_Status(*PFNGPA_BEGINPASSPROC)();
    typedef GPA_Status(*PFNGPA_ENDPASSPROC)();
    typedef GPA_Status(*PFNGPA_BEGINSAMPLEPROC)(gpa_uint32 sampleID);
    typedef GPA_Status(*PFNGPA_ENDSAMPLEPROC)();
    typedef GPA_Status(*PFNGPA_GETSAMPLECOUNTPROC)(gpa_uint32 sessionID, gpa_uint32* samples);
    typedef GPA_Status(*PFNGPA_ISSAMPLEREADYPROC)(bool* readyResult, gpa_uint32 sessionID, gpa_uint32 sampleID);
    typedef GPA_Status(*PFNGPA_ISSESSIONREADYPROC)(bool* readyResult, gpa_uint32 sessionID);

    typedef GPA_Status(*PFNGPA_GETSAMPLEUINT64PROC)(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterID, gpa_uint64* result);
    typedef GPA_Status(*PFNGPA_GETSAMPLEUINT32PROC)(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterIndex, gpa_uint32* result);
    typedef GPA_Status(*PFNGPA_GETSAMPLEFLOAT64PROC)(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterIndex, gpa_float64* result);
    typedef GPA_Status(*PFNGPA_GETSAMPLEFLOAT32PROC)(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterIndex, gpa_float32* result);

    typedef const char* (*PFNGPA_GETSTATUSASSTRPROC)(GPA_Status status);

#endif // OA_DEBUGGER_USE_AMD_GPA

#endif //__OAATIFUNCTIONTYPES_H

