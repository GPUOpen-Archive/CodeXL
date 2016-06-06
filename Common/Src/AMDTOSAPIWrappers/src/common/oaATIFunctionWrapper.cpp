//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaATIFunctionWrapper.cpp
///
//=====================================================================

//------------------------------ oaATIFunctionWrapper.cpp ------------------------------
// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osModule.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaATIFunctionWrapper.h>

#ifdef OA_DEBUGGER_USE_AMD_GPA

// Static members initializations:
oaATIFunctionWrapper* oaATIFunctionWrapper::_pMySingleGLInstance = NULL;
oaATIFunctionWrapper* oaATIFunctionWrapper::_pMySingleCLInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        oaATIFunctionWrapper::oaATIFunctionWrapper
// Description:
// Return Val:
// Author:      AMD Developer Tools Team
// Date:        24/2/2010
// ---------------------------------------------------------------------------
oaATIFunctionWrapper::oaATIFunctionWrapper():
    _GPA_Initialize(NULL),
    _GPA_Destroy(NULL),
    _GPA_OpenContext(NULL),
    _GPA_CloseContext(NULL),
    _GPA_SelectContext(NULL),
    _GPA_GetNumCounters(NULL),
    _GPA_GetCounterName(NULL),
    _GPA_GetCounterDescription(NULL),
    _GPA_GetCounterDataType(NULL),
    _GPA_GetCounterUsageType(NULL),
    _GPA_GetDataTypeAsStr(NULL),
    _GPA_GetUsageTypeAsStr(NULL),
    _GPA_EnableCounter(NULL),
    _GPA_DisableCounter(NULL),
    _GPA_GetEnabledCount(NULL),
    _GPA_GetEnabledIndex(NULL),
    _GPA_IsCounterEnabled(NULL),
    _GPA_EnableCounterStr(NULL),
    _GPA_DisableCounterStr(NULL),
    _GPA_EnableAllCounters(NULL),
    _GPA_DisableAllCounters(NULL),
    _GPA_GetCounterIndex(NULL),
    _GPA_GetPassCount(NULL),
    _GPA_BeginSession(NULL),
    _GPA_EndSession(NULL),
    _GPA_BeginPass(NULL),
    _GPA_EndPass(NULL),
    _GPA_BeginSample(NULL),
    _GPA_EndSample(NULL),
    _GPA_GetSampleCount(NULL),
    _GPA_IsSampleReady(NULL),
    _GPA_IsSessionReady(NULL),
    _GPA_GetSampleUInt64(NULL),
    _GPA_GetSampleUInt32(NULL),
    _GPA_GetSampleFloat64(NULL),
    _GPA_GetSampleFloat32(NULL),
    _GPA_GetStatusAsStr(NULL)
{
}

// ---------------------------------------------------------------------------
// Name:        oaATIFunctionWrapper::gl_instance
// Description: Returns a reference to the single instance of this class.
//              The first call to this function creates this single instance.
// Author:      AMD Developer Tools Team
// Date:        24/2/2010
// ---------------------------------------------------------------------------
oaATIFunctionWrapper& oaATIFunctionWrapper::gl_instance()
{
    // If my single instance was not created yet - create it:
    if (_pMySingleGLInstance == NULL)
    {
        _pMySingleGLInstance = new oaATIFunctionWrapper;


        // Initialize with ATI Perf OpenGL File name:
        _pMySingleGLInstance->initialize(OS_ATI_PERF_OPENGL_MODULE_NAME);
    }

    // Return my single instance:
    return *_pMySingleGLInstance;
}

// ---------------------------------------------------------------------------
// Name:        oaATIFunctionWrapper::cl_instance
// Description: Returns a reference to the single instance of this class.
//              The first call to this function creates this single instance.
// Author:      AMD Developer Tools Team
// Date:        25/2/2010
// ---------------------------------------------------------------------------
oaATIFunctionWrapper& oaATIFunctionWrapper::cl_instance()
{
    // If my single instance was not created yet - create it:
    if (_pMySingleCLInstance == NULL)
    {
        _pMySingleCLInstance = new oaATIFunctionWrapper;


        // Initialize with the OpenCL DLL:
        _pMySingleCLInstance->initialize(OS_ATI_PERF_OPENCL_MODULE_NAME);
    }

    // Return my single instance:
    return *_pMySingleCLInstance;
}

// ---------------------------------------------------------------------------
// Name:        oaATIFunctionWrapper::initialize
// Description: Initializes the ATI function pointers
// Arguments: const gtString& atiDllFileName - the ATI dll file name
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/2/2010
// ---------------------------------------------------------------------------
bool oaATIFunctionWrapper::initialize(const gtString& atiDllFileName)
{
    bool retVal = false;

    // Set the ATI file path:
    bool rcAppPath = osGetCurrentApplicationPath(_atiDllFilePath);
    GT_IF_WITH_ASSERT(rcAppPath)
    {
        // Check if we got here from GUI or from spy:
        gtString applicationFileName;
        _atiDllFilePath.getFileName(applicationFileName);
        applicationFileName.toLowerCase();

        if ((applicationFileName.find(L"gdebugger") == -1) && (applicationFileName.find(L"devenv") == -1))
        {
            // Get the environment spies directory:
            gtString spiesDirectory;
            bool rc = osGetCurrentProcessEnvVariableValue(OS_STR_envVar_spiesDirectory, spiesDirectory);
            GT_IF_WITH_ASSERT(rc)
            {
                // Make sure that the last character in the spies directory is '\', otherwise the path will not be
                // identified correctly by osDirectory:
                int lastCharPos = spiesDirectory.length() - 1;

                if (spiesDirectory[lastCharPos] != '\\')
                {
                    spiesDirectory.append(L"\\");
                }

                _atiDllFilePath.setFileDirectory(spiesDirectory);
                osDirectory spiesDir(spiesDirectory);
                spiesDir.upOneLevel();
                _atiDllFilePath = spiesDir.directoryPath();
            }
        }

        // Set the file name:
        _atiDllFilePath.setFileName(atiDllFileName);
        _atiDllFilePath.setFileExtension(L"dll");

        // Make sure that the ATI file exist:
        bool rcFileExist = _atiDllFilePath.exists();

        if (rcFileExist)
        {
            // Get the ATI module handle:
            gtString dbgMsg;
            dbgMsg.appendFormattedString(L"Loading GPU DLL from path: %ls ", _atiDllFilePath.asString().asCharArray());
            OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

            osModuleHandle atiModuleHandle = NULL;
            bool rcLoad = osLoadModule(_atiDllFilePath, atiModuleHandle);
            GT_IF_WITH_ASSERT(rcLoad)
            {
                retVal = true;
                // Get the function pointers;
                osProcedureAddress pFunctionHandler = NULL;
                bool rc = osGetProcedureAddress(atiModuleHandle, "GPA_Initialize", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_Initialize = (PFNGPA_INITIALIZEPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_Destroy", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_Destroy = (PFNGPA_DESTROYPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_OpenContext", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_OpenContext = (PFNGPA_OPENCONTEXTPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_CloseContext", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_CloseContext = (PFNGPA_CLOSECONTEXTPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_SelectContext", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_SelectContext = (PFNGPA_SELECTCONTEXTPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetNumCounters", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetNumCounters = (PFNGPA_GETNUMCOUNTERSPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetCounterName", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetCounterName = (PFNGPA_GETCOUNTERNAMEPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetCounterDescription", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetCounterDescription = (PFNGPA_GETCOUNTERDESCRIPTIONPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetCounterDataType", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetCounterDataType = (PFNGPA_GETCOUNTERDATATYPEPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetCounterUsageType", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetCounterUsageType = (PFNGPA_GETCOUNTERUSAGETYPEPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetDataTypeAsStr", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetDataTypeAsStr = (PFNGPA_GETDATATYPEASSTRPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetUsageTypeAsStr", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetUsageTypeAsStr = (PFNGPA_GETUSAGETYPEASSTRPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_EnableCounter", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_EnableCounter = (PFNGPA_ENABLECOUNTERPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_DisableCounter", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_DisableCounter = (PFNGPA_DISABLECOUNTERPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }
                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetEnabledCount", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetEnabledCount = (PFNGPA_GETENABLEDCOUNTPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetEnabledIndex", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetEnabledIndex = (PFNGPA_GETENABLEDINDEXPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_IsCounterEnabled", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_IsCounterEnabled = (PFNGPA_ISCOUNTERENABLEDPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_EnableCounterStr", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_EnableCounterStr = (PFNGPA_ENABLECOUNTERSTRPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_DisableCounterStr", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_DisableCounterStr = (PFNGPA_DISABLECOUNTERSTRPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_EnableAllCounters", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_EnableAllCounters = (PFNGPA_ENABLEALLCOUNTERSPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_DisableAllCounters", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_DisableAllCounters = (PFNGPA_DISABLEALLCOUNTERSPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetCounterIndex", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetCounterIndex = (PFNGPA_GETCOUNTERINDEXPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetPassCount", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetPassCount = (PFNGPA_GETPASSCOUNTPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_BeginSession", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_BeginSession = (PFNGPA_BEGINSESSIONPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_EndSession", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_EndSession = (PFNGPA_ENDSESSIONPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_BeginPass", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_BeginPass = (PFNGPA_BEGINPASSPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_EndPass", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_EndPass = (PFNGPA_ENDPASSPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_BeginSample", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_BeginSample = (PFNGPA_BEGINSAMPLEPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_EndSample", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_EndSample = (PFNGPA_ENDSAMPLEPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetSampleCount", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetSampleCount = (PFNGPA_GETSAMPLECOUNTPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_IsSampleReady", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_IsSampleReady = (PFNGPA_ISSAMPLEREADYPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_IsSessionReady", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_IsSessionReady = (PFNGPA_ISSESSIONREADYPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetSampleUInt64", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetSampleUInt64 = (PFNGPA_GETSAMPLEUINT64PROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetSampleUInt32", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetSampleUInt32 = (PFNGPA_GETSAMPLEUINT32PROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetSampleFloat64", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetSampleFloat64 = (PFNGPA_GETSAMPLEFLOAT64PROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetSampleFloat32", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetSampleFloat32 = (PFNGPA_GETSAMPLEFLOAT32PROC)pFunctionHandler;
                    retVal = retVal && rc;
                }

                rc = osGetProcedureAddress(atiModuleHandle, "GPA_GetStatusAsStr", pFunctionHandler);
                GT_IF_WITH_ASSERT(rc)
                {
                    _GPA_GetStatusAsStr = (PFNGPA_GETSTATUSASSTRPROC)pFunctionHandler;
                    retVal = retVal && rc;
                }
            }
        }
        else
        {
            // Output an error to the logfile:
            gtString errorMessage;
            errorMessage.appendFormattedString(L"Cannot find ATI dll: %ls", _atiDllFilePath.asString().asCharArray());
            OS_OUTPUT_DEBUG_LOG(errorMessage.asCharArray(), OS_DEBUG_LOG_ERROR);

        }
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_Initialize()
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_Initialize != NULL)
    {
        retVal = _GPA_Initialize();
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_Destroy()
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_Destroy != NULL)
    {
        retVal = _GPA_Destroy();
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_OpenContext(void* context)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_OpenContext != NULL)
    {
        retVal = _GPA_OpenContext(context);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_CloseContext()
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_CloseContext != NULL)
    {
        retVal = _GPA_CloseContext();
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_SelectContext(void* context)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_SelectContext != NULL)
    {
        retVal = _GPA_SelectContext(context);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_GetNumCounters(gpa_uint32* count)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetNumCounters != NULL)
    {
        retVal = _GPA_GetNumCounters(count);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_GetCounterName(gpa_uint32 index, const char** name)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetCounterName != NULL)
    {

        retVal = _GPA_GetCounterName(index, name);
    }
    return retVal;
}
GPA_Status oaATIFunctionWrapper::GPA_GetCounterDescription(gpa_uint32 index, const char** description)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetCounterDescription != NULL)
    {

        retVal = _GPA_GetCounterDescription(index, description);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_GetCounterDataType(gpa_uint32 index, GPA_Type* counterDataType)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetCounterDataType != NULL)
    {

        retVal = _GPA_GetCounterDataType(index, counterDataType);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_GetCounterUsageType(gpa_uint32 index, GPA_Usage_Type* counterUsageType)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetCounterUsageType != NULL)
    {

        retVal = _GPA_GetCounterUsageType(index, counterUsageType);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_GetDataTypeAsStr(GPA_Type counterDataType, const char** typeStr)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetDataTypeAsStr != NULL)
    {

        retVal = _GPA_GetDataTypeAsStr(counterDataType, typeStr);
    }
    return retVal;
}


GPA_Status oaATIFunctionWrapper::GPA_GetUsageTypeAsStr(GPA_Usage_Type counterUsageType, const char** usageTypeStr)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetUsageTypeAsStr != NULL)
    {

        retVal = _GPA_GetUsageTypeAsStr(counterUsageType, usageTypeStr);
    }
    return retVal;
}


GPA_Status oaATIFunctionWrapper::GPA_EnableCounter(gpa_uint32 index)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_EnableCounter != NULL)
    {
        retVal = _GPA_EnableCounter(index);
    }
    return retVal;
}
GPA_Status oaATIFunctionWrapper::GPA_DisableCounter(gpa_uint32 index)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_DisableCounter != NULL)
    {
        retVal = _GPA_DisableCounter(index);
    }
    return retVal;
}
GPA_Status oaATIFunctionWrapper::GPA_GetEnabledCount(gpa_uint32* count)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetEnabledCount != NULL)
    {
        retVal = _GPA_GetEnabledCount(count);
    }
    return retVal;
}
GPA_Status oaATIFunctionWrapper::GPA_GetEnabledIndex(gpa_uint32 enabledNumber, gpa_uint32* enabledCounterIndex)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetEnabledIndex != NULL)
    {
        retVal = _GPA_GetEnabledIndex(enabledNumber, enabledCounterIndex);
    }
    return retVal;
}
GPA_Status oaATIFunctionWrapper::GPA_IsCounterEnabled(gpa_uint32 counterIndex)
{
    return _GPA_IsCounterEnabled(counterIndex);
}

GPA_Status oaATIFunctionWrapper::GPA_EnableCounterStr(const char* counter)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_EnableCounterStr != NULL)
    {
        retVal = _GPA_EnableCounterStr(counter);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_DisableCounterStr(const char* counter)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_DisableCounterStr != NULL)
    {
        retVal = _GPA_DisableCounterStr(counter);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_EnableAllCounters()
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_EnableAllCounters != NULL)
    {
        retVal = _GPA_EnableAllCounters();
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_DisableAllCounters()
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_DisableAllCounters != NULL)
    {
        retVal = _GPA_DisableAllCounters();
    }
    return retVal;

}

GPA_Status oaATIFunctionWrapper::GPA_GetCounterIndex(const char* counter, gpa_uint32* index)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetCounterIndex != NULL)
    {
        retVal = _GPA_GetCounterIndex(counter, index);
    }
    return retVal;
}


GPA_Status oaATIFunctionWrapper::GPA_GetPassCount(gpa_uint32* numPasses)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetPassCount != NULL)
    {
        retVal = _GPA_GetPassCount(numPasses);
    }
    return retVal;

}
GPA_Status oaATIFunctionWrapper::GPA_BeginSession(gpa_uint32* sessionID)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_BeginSession != NULL)
    {
        retVal = _GPA_BeginSession(sessionID);
    }
    return retVal;

}

GPA_Status oaATIFunctionWrapper::GPA_EndSession()
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_EndSession != NULL)
    {
        retVal = _GPA_EndSession();
    }
    return retVal;

}

GPA_Status oaATIFunctionWrapper::GPA_BeginPass()
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_BeginPass != NULL)
    {
        retVal = _GPA_BeginPass();
    }
    return retVal;

}

GPA_Status oaATIFunctionWrapper::GPA_EndPass()
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_EndPass != NULL)
    {
        retVal = _GPA_EndPass();
    }
    return retVal;

}

GPA_Status oaATIFunctionWrapper::GPA_BeginSample(gpa_uint32 sampleID)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_BeginSample != NULL)
    {
        retVal = _GPA_BeginSample(sampleID);
    }
    return retVal;

}
GPA_Status oaATIFunctionWrapper::GPA_EndSample()
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_EndSample != NULL)
    {
        retVal = _GPA_EndSample();
    }
    return retVal;

}

GPA_Status oaATIFunctionWrapper::GPA_GetSampleCount(gpa_uint32 sessionID, gpa_uint32* samples)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetSampleCount != NULL)
    {
        retVal = _GPA_GetSampleCount(sessionID, samples);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_IsSampleReady(bool* readyResult, gpa_uint32 sessionID, gpa_uint32 sampleID)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_IsSampleReady != NULL)
    {
        retVal = _GPA_IsSampleReady(readyResult, sessionID, sampleID);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_IsSessionReady(bool* readyResult, gpa_uint32 sessionID)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_IsSessionReady != NULL)
    {
        retVal = _GPA_IsSessionReady(readyResult, sessionID);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_GetSampleUInt64(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterID, gpa_uint64* result)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetSampleUInt64 != NULL)
    {
        retVal = _GPA_GetSampleUInt64(sessionID, sampleID, counterID, result);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_GetSampleUInt32(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterIndex, gpa_uint32* result)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetSampleUInt32 != NULL)
    {
        retVal = _GPA_GetSampleUInt32(sessionID, sampleID, counterIndex, result);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_GetSampleFloat64(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterIndex, gpa_float64* result)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetSampleFloat64 != NULL)
    {
        retVal = _GPA_GetSampleFloat64(sessionID, sampleID, counterIndex, result);
    }
    return retVal;
}

GPA_Status oaATIFunctionWrapper::GPA_GetSampleFloat32(gpa_uint32 sessionID, gpa_uint32 sampleID, gpa_uint32 counterIndex, gpa_float32* result)
{
    GPA_Status retVal = GPA_STATUS_ERROR_COUNTERS_NOT_OPEN;
    GT_IF_WITH_ASSERT(_GPA_GetSampleFloat32 != NULL)
    {
        retVal = _GPA_GetSampleFloat32(sessionID, sampleID, counterIndex, result);
    }
    return retVal;
}
const char*  oaATIFunctionWrapper::GPA_GetStatusAsStr(GPA_Status status)
{
    const char* retVal = NULL;
    GT_IF_WITH_ASSERT(_GPA_GetStatusAsStr != NULL)
    {
        retVal = _GPA_GetStatusAsStr(status);
    }
    return retVal;
}

#endif // OA_DEBUGGER_USE_AMD_GPA

