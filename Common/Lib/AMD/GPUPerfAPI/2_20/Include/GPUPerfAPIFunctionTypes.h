//==============================================================================
// Copyright (c) 2010-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines function types to make it easier to dynamically load
///         different GPUPerfAPI DLLs into an application that supports multiple APIs.
///         Applications which statically link to GPUPerfAPI do not need to include
///         this file.
//==============================================================================

#ifndef _GPUPERFAPI_FUNCTION_TYPES_H_
#define _GPUPERFAPI_FUNCTION_TYPES_H_

#include "GPUPerfAPITypes.h"

typedef void(*GPA_LoggingCallbackPtrType)(GPA_Logging_Type messageType, const char* pMessage);

typedef GPA_Status(*GPA_RegisterLoggingCallbackPtrType)(GPA_Logging_Type loggingType, GPA_LoggingCallbackPtrType pCallbackFuncPtr);

// Startup / exit
typedef GPA_Status(*GPA_InitializePtrType)();
typedef GPA_Status(*GPA_DestroyPtrType)();

// Context
typedef GPA_Status(*GPA_OpenContextPtrType)(void* pContext);
typedef GPA_Status(*GPA_CloseContextPtrType)();
typedef GPA_Status(*GPA_SelectContextPtrType)(void* pCcontext);

// Counter Interrogation
typedef GPA_Status(*GPA_GetNumCountersPtrType)(gpa_uint32* pCount);
typedef GPA_Status(*GPA_GetCounterNamePtrType)(gpa_uint32 index, const char** ppName);
typedef GPA_Status(*GPA_GetCounterDescriptionPtrType)(gpa_uint32 index, const char** ppDescription);
typedef GPA_Status(*GPA_GetCounterDataTypePtrType)(gpa_uint32 index, GPA_Type* pCounterDataType);
typedef GPA_Status(*GPA_GetCounterUsageTypePtrType)(gpa_uint32 index, GPA_Usage_Type* pCounterUsageType);
typedef GPA_Status(*GPA_GetDataTypeAsStrPtrType)(GPA_Type counterDataType, const char** ppTypeStr);
typedef GPA_Status(*GPA_GetUsageTypeAsStrPtrType)(GPA_Usage_Type counterUsageType, const char** ppTypeStr);
typedef const char* (*GPA_GetStatusAsStrPtrType)(GPA_Status status);

typedef GPA_Status(*GPA_EnableCounterPtrType)(gpa_uint32 index);
typedef GPA_Status(*GPA_DisableCounterPtrType)(gpa_uint32 index);
typedef GPA_Status(*GPA_GetEnabledCountPtrType)(gpa_uint32* pCount);
typedef GPA_Status(*GPA_GetEnabledIndexPtrType)(gpa_uint32 enabledNumber, gpa_uint32* pEnabledCounterIndex);

typedef GPA_Status(*GPA_IsCounterEnabledPtrType)(gpa_uint32 counterIndex);

typedef GPA_Status(*GPA_EnableCounterStrPtrType)(const char* pCounter);
typedef GPA_Status(*GPA_DisableCounterStrPtrType)(const char* pCounter);

typedef GPA_Status(*GPA_EnableAllCountersPtrType)();
typedef GPA_Status(*GPA_DisableAllCountersPtrType)();
typedef GPA_Status(*GPA_GetCounterIndexPtrType)(const char* pCounter, gpa_uint32* pIndex);

typedef GPA_Status(*GPA_GetPassCountPtrType)(gpa_uint32* pNumPasses);

typedef GPA_Status(*GPA_BeginSessionPtrType)(gpa_uint32* pSessionID);
typedef GPA_Status(*GPA_EndSessionPtrType)();

typedef GPA_Status(*GPA_BeginPassPtrType)();
typedef GPA_Status(*GPA_EndPassPtrType)();

typedef GPA_Status(*GPA_BeginSamplePtrType)(gpa_uint32 sampleID);
typedef GPA_Status(*GPA_EndSamplePtrType)();

typedef GPA_Status(*GPA_GetSampleCountPtrType)(gpa_uint32 sessionID, gpa_uint32* pSamples);

typedef GPA_Status(*GPA_IsSampleReadyPtrType)(bool* pReadyResult, gpa_uint32 sessionID, gpa_uint32 sampleID);
typedef GPA_Status(*GPA_IsSessionReadyPtrType)(bool* pReadyResult, gpa_uint32 sessionID);
typedef GPA_Status(*GPA_GetSampleUInt64PtrType)(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterID, gpa_uint64* pResult);
typedef GPA_Status(*GPA_GetSampleUInt32PtrType)(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterIndex, gpa_uint32* pResult);
typedef GPA_Status(*GPA_GetSampleFloat32PtrType)(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterIndex, gpa_float32* pResult);
typedef GPA_Status(*GPA_GetSampleFloat64PtrType)(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterIndex, gpa_float64* pResult);

typedef GPA_Status(*GPA_GetDeviceIDPtrType)(gpa_uint32* pDeviceID);
typedef GPA_Status(*GPA_GetDeviceDescPtrType)(const char** ppDesc);

#endif // _GPUPERFAPI_FUNCTION_TYPES_H_
