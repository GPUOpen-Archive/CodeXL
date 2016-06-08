//==============================================================================
// Copyright (c) 2011-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Interface from Developer Tools to ADL.
//==============================================================================

#ifndef _ADL_UTIL_H_
#define _ADL_UTIL_H_

#ifdef _LINUX
    #ifndef LINUX
        #define LINUX
    #endif
#endif

#include "AMDTMutex.h"

#include "adl_sdk.h"
// This is from ADL's include directory.
#include "customer/oem_structures.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <TSingleton.h>

#ifdef __GNUC__
    #define ADLUTIL_DEPRECATED __attribute__((deprecated))
    typedef void* ADLModule;
#elif _WIN32
    #define ADLUTIL_DEPRECATED __declspec(deprecated)
    typedef HINSTANCE ADLModule;
#else
    #define ADLUTIL_DEPRECATED
#endif

/// Stores ASIC information that is parsed from data supplied by ADL
struct ADLUtil_ASICInfo
{
    std::string adapterName;      ///< description of the adapter ie "ATI Radeon HD 5800 series"
    std::string deviceIDString;   ///< string version of the deviceID (for easy comparing since the deviceID is hex, but stored as int)
    int vendorID;                 ///< the vendor ID
    int deviceID;                 ///< the device ID (hex value stored as int)
    int revID;                    ///< the revision ID (hex value stored as int)
    unsigned int gpuIndex;        ///< GPU index in the system
#ifdef _WIN32
    std::string registryPath;     ///< Adapter registry path
    std::string registryPathExt;  ///< Adapter registry path
#endif // _WIN32
};

typedef std::vector<ADLUtil_ASICInfo> AsicInfoList;

/// Return values from the ADLUtil
enum ADLUtil_Result
{
    ADL_RESULT_NONE,                     ///< Undefined ADL result
    ADL_SUCCESS = 1,                     ///< Data was retrieved successfully.
    ADL_NOT_FOUND,                       ///< ADL DLLs were not found.
    ADL_MISSING_ENTRYPOINTS,             ///< ADL did not expose necessary entrypoints.
    ADL_INITIALIZATION_FAILED,           ///< ADL could not be initialized.
    ADL_GET_ADAPTER_COUNT_FAILED,        ///< ADL was unable to return the number of adapters.
    ADL_GET_ADAPTER_INFO_FAILED,         ///< ADL was unable to return adapter info.
    ADL_GRAPHICS_VERSIONS_GET_FAILED,    ///< ADL was unable to return graphics versions info.
    ADL_WARNING,                         ///< ADL Operation succeeded, but generated a warning.
    ADL_CHANGING_POWER_MODE_FAILED       ///< ADL was unable to sucessfully modify OD5 power tables
};

/// Uses ADL to obtain information about the available ASICs. This is deprecated -- use AMDTADLUtils::Instance()->GetAsicInfoList() instead.
/// \param   asicInfoList A list to populate with the available ASICs.
/// \returns              an enum ADLUtil_Result status code.
ADLUTIL_DEPRECATED ADLUtil_Result ADLUtil_GetASICInfo(AsicInfoList& asicInfoList);

/// Uses ADL to obtain version information about installed drivers. This is deprecated -- use AMDTADLUtils::Instance()->GetADLVersionsInfo() instead
/// \param   info The version information.
/// \returns      an enum ADLUtil_Result status code.
ADLUTIL_DEPRECATED ADLUtil_Result ADLUtil_GetVersionsInfo(struct ADLVersionsInfo& info);

// Typedefs of the ADL function pointers. If additional entry points are needed, add them here
typedef int(*ADL_Main_Control_Create_fn)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int(*ADL_Main_Control_Destroy_fn)();
typedef int(*ADL2_Main_Control_Create_fn)(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);
typedef int(*ADL2_Main_Control_Destroy_fn)(ADL_CONTEXT_HANDLE);

typedef int(*ADL_Adapter_NumberOfAdapters_Get_fn)(int*);
typedef int(*ADL_Adapter_AdapterInfo_Get_fn)(LPAdapterInfo, int);
typedef int(*ADL2_Adapter_NumberOfAdapters_Get_fn)(ADL_CONTEXT_HANDLE, int*);
typedef int(*ADL2_Adapter_AdapterInfo_Get_fn)(ADL_CONTEXT_HANDLE, LPAdapterInfo, int);

typedef int(*ADL_Graphics_Versions_Get_fn)(struct ADLVersionsInfo*);
typedef int(*ADL2_Graphics_Versions_Get_fn)(ADL_CONTEXT_HANDLE, struct ADLVersionsInfo*);

typedef int(*ADL_Overdrive5_ODParameters_Get_fn)(int iAdapterIndex, ADLODParameters* lpOdParameters);
typedef int(*ADL_Overdrive5_ODPerformanceLevels_Set_fn)(int iAdapterIndex, ADLODPerformanceLevels* lpOdPerformanceLevels);
typedef int(*ADL_Overdrive5_ODPerformanceLevels_Get_fn)(int iAdapterIndex, int iDefault, ADLODPerformanceLevels* lpOdPerformanceLevels);
typedef int(*ADL2_Overdrive5_ODParameters_Get_fn)(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODParameters* lpOdParameters);
typedef int(*ADL2_Overdrive5_ODPerformanceLevels_Set_fn)(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODPerformanceLevels* lpOdPerformanceLevels);
typedef int(*ADL2_Overdrive5_ODPerformanceLevels_Get_fn)(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDefault, ADLODPerformanceLevels* lpOdPerformanceLevels);


// Table of ADL entry points. If additional entry points are needed, add them to this table and the code will automatically initialize them
#define ADL_INTERFACE_TABLE \
    X(ADL_Main_Control_Create) \
    X(ADL_Main_Control_Destroy) \
    X(ADL2_Main_Control_Create) \
    X(ADL2_Main_Control_Destroy) \
    X(ADL_Adapter_NumberOfAdapters_Get) \
    X(ADL_Adapter_AdapterInfo_Get) \
    X(ADL2_Adapter_NumberOfAdapters_Get) \
    X(ADL2_Adapter_AdapterInfo_Get) \
    X(ADL_Graphics_Versions_Get) \
    X(ADL2_Graphics_Versions_Get) \
    X(ADL_Overdrive5_ODParameters_Get) \
    X(ADL_Overdrive5_ODPerformanceLevels_Set) \
    X(ADL_Overdrive5_ODPerformanceLevels_Get) \
    X(ADL2_Overdrive5_ODParameters_Get) \
    X(ADL2_Overdrive5_ODPerformanceLevels_Set) \
    X(ADL2_Overdrive5_ODPerformanceLevels_Get)

//------------------------------------------------------------------------------------
/// Singleton class to provide caching of values returned by various ADLUtil functions
//------------------------------------------------------------------------------------
class AMDTADLUtils : public TSingleton<AMDTADLUtils>
{
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<AMDTADLUtils>;

public:
    /// Loads ADL library, initializes the function entry points, and calls ADL2_Main_Control_Create
    ADLUtil_Result LoadAndInit();

    /// Calls ADL2_Main_Control_Destroy, unloads ADL library, and clears the function entry points
    ADLUtil_Result Unload();

    /// Get the AsicInfoList from ADL. The value is cached so that multiple calls don't need to requery ADL
    /// \param[out] asicInfoList the AsicInfoList from ADL
    /// \returns    an enum ADLUtil_Result status code.
    ADLUtil_Result GetAsicInfoList(AsicInfoList& asicInfoList);

    /// Get the Catalyst version info from ADL. The value is cached so that multiple calls don't need to requery ADL
    /// \param[out] adlVersionInfo the Catalyst version info from ADL
    /// \returns    an enum ADLUtil_Result status code.
    ADLUtil_Result GetADLVersionsInfo(ADLVersionsInfo& adlVersionInfo);

    /// Gets the major, minor and subminor number of the driver version. For
    /// instance, if the driver version string is 14.10.1005-140115n-021649E-ATI,
    /// the major number is "14", the minor is "10", and the sub-minor is "1005".
    /// If minor or subminor do not exist in the driver string, 0 will be returned for those values.
    /// TODO: we could add a version struct and return that, and then provide operator overloads to compare version numbers (see http://adt-review.amd.com/r/6002/)
    /// \param[out] majorVer the major version number
    /// \param[out] minorVer the minor version number
    /// \param[out] subMinorVer the sub-minor version number
    /// \return an enum ADLUtil_Result status code.
    ADLUtil_Result GetDriverVersion(unsigned int& majorVer, unsigned int& minorVer, unsigned int& subMinorVer);

    /// Updates the Power tables so that the GPU clock in all power states is set to the highest power level in the power table
    /// \param[in] gpuIndex the index of the GPU whose power table should be updated. If -1, then the power table for all GPUs will be updated
    /// \returns    an enum ADLUtil_Result status code.
    ADLUtil_Result UseHighPowerMode(unsigned int gpuIndex = (unsigned int) - 1);

    /// Updates the Power tables so that the GPU clock in all power states is set back to the default value
    /// \param[in] gpuIndex the index of the GPU whose power table should be updated. If -1, then the power table for all GPUs will be updated
    /// \returns    an enum ADLUtil_Result status code.
    ADLUtil_Result ResumeNormalPowerMode(unsigned int gpuIndex = (unsigned int) - 1);

    /// Resets the singleton data so that the next call with requery the data rather than using any cached data
    void Reset();

private:
    /// constructor
    AMDTADLUtils();

    /// destructor
    ~AMDTADLUtils();

    /// Helper function used by UseHighPowerMode and ResumeNormalPowerMode
    /// \param[in] forceHigh if true, then the GPU clock in all entries in the Power table will be set to the highest clock rate.  If false, then Power tables are restored back to their original, default values
    /// \param[in] gpuIndex the index of the GPU whose power table should be updated. If -1, then the power table for all GPUs will be updated
    ADLUtil_Result ForceGPUClock(bool forceHigh, unsigned int gpuIndex);

    ADLModule          m_libHandle;          ///< Handle to ADL Module
    ADL_CONTEXT_HANDLE m_adlContext;         ///< ADL Context for use with ADL2 functions
    AMDTMutex          m_asicInfoMutex;      ///< Mutex to protect access to the m_asicInfoList
    AMDTMutex          m_adlVersionsMutex;   ///< Mutex to protect access to the m_adlVersionsInfo
    AMDTMutex          m_powerTableMutex;    ///< Mutex to protect access to the overdrive power tables set by ForceGPUClock
    AsicInfoList       m_asicInfoList;       ///< the ADL ASIC list
    ADLVersionsInfo    m_adlVersionsInfo;    ///< the ADL Version info

    ADLUtil_Result     m_asicInfoListRetVal; ///< the result of the AsicInfoList query
    ADLUtil_Result     m_versionRetVal;      ///< the result of the ADL Version query

    /// typedef for a map used to save off original power table information
    typedef std::unordered_map<unsigned int, ADLODPerformanceLevels*> PowerTableMap;

    PowerTableMap          m_origODPerformanceLevels; ///< map of the original power tables per GPU
    std::unordered_set<unsigned int> m_highPowerGpuSpeedSet;    ///< set indicating whether the power tables have been changed for a given GPU index

#define X(SYM) SYM##_fn m_##SYM; ///< ADL entry point
    ADL_INTERFACE_TABLE;
#undef X

};
#endif //_ADL_UTIL_H_
