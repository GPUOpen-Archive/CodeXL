//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTPowerProfileDataAccess.h
///
//==================================================================================

#ifndef _AMDTPOWERPROFILEDATAACCESS_H_
#define _AMDTPOWERPROFILEDATAACCESS_H_
#include <AMDTPowerProfileInternal.h>

//AMDTPwrTimeLineOpt: Structure for time line option parameter
struct AMDTPwrTimeLineOpt
{
    AMDTUInt64 m_sampleId;
    AMDTUInt64 m_flag;
};

/** This would open the profile data file initialize internal structures.
       In case driver fails to open the file it will return error.

    \Note: A generic raw record file to support both Linux and Windows.

    \ingroup profiling
    @param[in] pFileName The path and filename of the profiling output file.
    @param[in] pParam for future use
    \return The success of setting the configuration
    \retval AMDT_STATUS_OK Success
    \retval AMDT_STATUS_PENDING The profiler is currently profiling ( offline mode only)
    \retval AMDT_ERROR_INVALIDARG if pFileName is NULL and isOnline = false
    \retval AMDT_ERROR_ACCESSDENIED If file or buffer is not accessible
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/
AMDTResult AMDTPwrOpenOfflineDataAccess(
    /*in*/ const wchar_t* pFileName, void* pParam);

/** This would open the profile data buffer initialize internal structures. Buffer will be
       used instead of file in case of online mode.

    \ingroup profiling
    @param[in] pParam Place holder at this moment.
    \return The success of setting the configuration
    \retval AMDT_STATUS_OK Success
    \retval AMDT_STATUS_PENDING If profile is not currently active/running
    \retval AMDT_ERROR_INVALIDARG if pFileName is NULL and isOnline = false
    \retval AMDT_ERROR_ACCESSDENIED If file or buffer is not accessible
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/
AMDTResult AMDTPwrOpenOnlineDataAccess(void* pParam);

/** Releases all resources appropriately.
    \ingroup profiling
    \return The success of closing the profile
    \retval AMDT_STATUS_OK Success
    \retval AMDT_ERROR_FAIL if fnInitializeDataAccess is not called before
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/
AMDTResult AMDTPwrCloseDataAccess();

/** This would provide the current configuration set by the client.

    \ingroup profiling
    @param[out] pCfgInfo pointer to the configuration information structure
    \return The success of setting the configuration
    \retval AMDT_STATUS_OK Success
    \retval AMDT_ERROR_INVALIDARG if pCfgInfo is NULL
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/
AMDTResult AMDTPwrGetProfileConfig(
    /*in*/ AMDTPwrProfileConfig** pCfgInfo);

/** This would provide the profile duration in millisecond.

    \ingroup profiling
    @param[out] pTime duration in millisecond
    \return The success of setting the configuration
    \retval AMDT_STATUS_OK Success
    \retval AMDT_STATUS_PENDING The profiler is currently busy ( offline mode only)
    \retval AMDT_ERROR_INVALIDARG if pTime is NULL
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/
AMDTResult AMDTPwrGetProfileTimeStamps(
    /*in*/ AMDTUInt64* pStartTime, /*in*/ AMDTUInt64* pEndTime);

// AMDTGetCounterValues: Get profile time line counter values.
AMDTResult AMDTGetCounterValues(AMDTPwrProcessedDataRecord* pData);

// AMDTGetCummulativePidProfData: Get Process profile infornation list.
// This is a list of PIDs and their corresponding power indicators
AMDTResult AMDTGetCummulativePidProfData(AMDTUInt32* pPIDCount, AMDTPwrProcessInfo** ppData, AMDTUInt32 pidVal, bool reset);

// AMDTGetInstrumentedData: Get the accumulated instrumented data
AMDTResult AMDTGetInstrumentedData(AMDTUInt32 markerId, PwrInstrumentedPowerData** ppData);
// PwrGetModuleProfileData: module level profiling data.
// this api can be called at any point of profile state from start to end of the session
AMDTResult PwrGetModuleProfileData(AMDTPwrModuleData** ppData, AMDTUInt32* pModuleCount, AMDTFloat32* pPower);


#endif //_AMDTPOWERPROFILEDATAACCESS_H_

