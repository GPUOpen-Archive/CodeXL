//==============================================================================
// Copyright (c) 2010-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file is for internal use to define the function types.
//==============================================================================

#ifndef _GPUPERFAPI_FUNCTION_TYPES_INTERNAL_H_
#define _GPUPERFAPI_FUNCTION_TYPES_INTERNAL_H_

#include "GPUPerfAPIFunctionTypes.h"

// Util
typedef GPA_Status(*GPA_EnableCountersFromFilePtrType)(const char* pFile, gpa_uint32* pCountersRead);

// logging
typedef void(*GPA_LoggingDebugCallbackPtrType)(GPA_Log_Debug_Type messageType, const char* pMessage);
typedef GPA_Status(*GPA_RegisterLoggingDebugCallbackPtrType)(GPA_Log_Debug_Type loggingType, GPA_LoggingDebugCallbackPtrType pCallbackFuncPtr);


// internal functions
typedef GPA_Status(*GPA_InternalProfileStartPtrType)();
typedef GPA_Status(*GPA_InternalProfileStopPtrType)(const char* pFilename);

/// For internal purposes only -- not needed for normal operation of GPUPerfAPI
typedef GPA_Status(*GPA_InternalSetDrawCallCountsPtrType)(const int iCounts);

#endif // _GPUPERFAPI_FUNCTION_TYPES_INTERNAL_H_
