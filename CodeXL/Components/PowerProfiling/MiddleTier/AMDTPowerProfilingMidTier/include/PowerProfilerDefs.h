//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfilerDefs.h
///
//==================================================================================

#ifndef _POWERPROFILERDEFS_H_
#define _POWERPROFILERDEFS_H_

// C++.
#include <memory>

// Infra.
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>

// Local.
#include <AMDTPowerProfilingMidTier/include/PPDevice.h>

enum AMDTPOWERPROFILINGMIDTIER_API PPResult
{
    // Success:
    PPR_NO_ERROR = 0,

    // Following describe warning returned from API:
    PPR_FIRST_WARNING = 1000,
    PPR_WARNING_SMU_DISABLED,
    PPR_WARNING_IGPU_DISABLED,

    // Following describe errors returned from API:
    PPR_FIRST_ERROR = 2000,
    PPR_NOT_SUPPORTED,
    PPR_COMMUNICATION_FAILURE,
    PPR_POLLING_THREAD_ALREADY_RUNNING,
    PPR_DB_CREATION_FAILURE,
    PPR_INVALID_SAMPLING_INTERVAL,
    PPR_DRIVER_ALREADY_IN_USE,
    PPR_DRIVER_VERSION_MISMATCH,
    PPR_REMOTE_ADDRESS_NOT_SET,
    PPR_REMOTE_CONNECTION_ERROR,
    PPR_REMOTE_HANDSHAKE_FAILURE,
    PPR_REMOTE_SESSION_CONFIGURATION_ERROR,
    PPR_REMOTE_APP_STOPPED,
    PPR_HYPERVISOR_NOT_SUPPORTED,
    PPR_COUNTERS_NOT_ENABLED,
    PPR_DB_MIGRATE_FAILURE,
    PPR_WRONG_PROJECT_SETTINGS,
    PPR_UNKNOWN_FAILURE
};

// FIXME: Is it OK to keep it here
struct AMDTPOWERPROFILINGMIDTIER_API PPSampledValuesBatch
{
    PPSampledValuesBatch() : m_quantizedTime(0), m_sampleValues() {}
    PPSampledValuesBatch(unsigned int quantizedTime) :  m_quantizedTime(quantizedTime), m_sampleValues() {}
    ~PPSampledValuesBatch() {}

    // The time when this batch of samples was taken.
    unsigned int m_quantizedTime;

    // The actual sample values.
    gtVector<double> m_sampleValues;
};

struct AMDTPOWERPROFILINGMIDTIER_API PwrProfCounterDesc
{
    AMDTUInt32           m_counterID;       /* Counter index */
    AMDTUInt32           m_deviceId;        /* Device Id */
    AMDTFloat64          m_minValue;        /* Minimum possible counter value */
    AMDTFloat64          m_maxValue;        /* Maximum possible counter value */
    std::string          m_name;            /* Name of the counter */
    std::string          m_description;     /* Description of the counter */
    std::string          m_categoryStr;     /* Power/Freq/Temperature */
    std::string          m_aggregationStr;  /* Single/Histogram/Cumulative */
    std::string          m_unitStr;         /* Seconds/MHz/Joules/Watts/Volt/Ampere */
};

struct AMDTPOWERPROFILINGMIDTIER_API HistogramBucket
{
    // The lower bound of the bucket's range.
    double m_lowerBound;

    // The upper bound of the bucket's range.
    double m_upperBound;

    // The value of the bucket.
    double m_value;
};

// Defines a callback which handles the new samples that were taken.
typedef AMDTPOWERPROFILINGMIDTIER_API void(*PPSamplesDataHandler)(std::shared_ptr<const gtMap<int, PPSampledValuesBatch>> pSampledDataPerCounter, void* pParams);

// Defines a callback for handling of fatal errors.
typedef AMDTPOWERPROFILINGMIDTIER_API void(*PPFatalErrorHandler)(PPResult error, void* pParams);

#endif // _POWERPROFILERDEFS_H_