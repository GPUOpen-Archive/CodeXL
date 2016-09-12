//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief GPUPerfAPI Utilities
//==============================================================================

#ifndef _GPA_UTILS_H_
#define _GPA_UTILS_H_

#include "GPUPerfAPI.h"
#include "GPACounterGenerator.h"
#include "GPUPerfAPICounters.h"
#include "DeviceInfo.h"
#include "GPUPerfAPILoader.h"
#include <map>
#include <vector>
#include <string>

#include "ADLUtil/ADLUtil.h"
#include <AMDTBaseTools/Include/gtString.h>

typedef decltype(GPA_GetAvailableCounters)* GPA_GetAvailableCountersForDeviceProc;
typedef decltype(GPA_GetAvailableCountersByGeneration)* GPA_GetAvailableCountersByGenerationProc;

typedef std::vector<std::string> CounterList;
typedef std::map<GPA_HW_GENERATION, CounterList> HWCounterMap;
typedef std::map<gpa_uint32, CounterList> HWCounterDeviceMap;

#define GPA_INFINITE_PASS 0xFFFFFFFF

/// \addtogroup Common
// @{

typedef void (*CounterNameMappingProc)(CounterList& counters);

/// This class handles the interaction with the GPUPerfAPI library.
class GPAUtils
{
public:

    struct CounterPassInfo
    {
        int vendorId;
        int deviceId;
        int revId;
        CounterList listofCounter;
        unsigned int numberOfPass;
    };

    /// Constructor
    GPAUtils();

    /// Destructor
    ~GPAUtils() {}

    /// Accessor to the GPA Loader
    GPUPerfAPILoader& GetGPALoader()
    {
        return m_GPALoader;
    }

    /// Initialize GPA with a context (command queue or ID3D11Device)
    /// \param context Context object
    /// \return true if successful
    bool Open(void* context);


    /// Close the current context in GPA
    /// \return true if successful
    bool Close();

    /// Accessor to whether or not GPA has been loaded
    /// \return true if GPA dll has been loaded; false otherwise
    bool Loaded()
    {
        return m_GPALoader.Loaded();
    }

    /// Filters non-compute counters out of the specified Counter List (used for DirectCompute only)
    /// \param[in] gen the hardware generation whose counters are being filtered
    /// \param[out] counterList the list of counters to filter
    /// \param counterListWithDescription to specify counterlist contains description or not
    void FilterNonComputeCounters(GPA_HW_GENERATION gen, CounterList& counterList, bool counterListWithDescription = false);

    /// Filters non-compute counters out of the specified Counter List (used for DirectCompute only)
    /// \param[in] gen the hardware generation whose counters are being filtered
    /// \param[out] counterList the list of counters to filter
    void FilterNonComputeCountersGdt(GDT_HW_GENERATION gen, CounterList& counterList);

    /// Get enabled counter names, only used by DCServer to generate table column names
    /// \param enabledCounters Output counter list
    /// \return the number of enabled counters
    gpa_uint32 GetEnabledCounterNames(CounterList& enabledCounters);

    /// Get available counters from GPA offline
    /// \param generation Hardware generation
    /// \param availableCounters Output list of counters (optionally with descriptions)
    /// \param shouldIncludeCounterDescriptions optional param to add description to the output list of the counters
    /// \return true if successful
    bool GetAvailableCounters(GPA_HW_GENERATION generation, CounterList& availableCounters, const bool shouldIncludeCounterDescriptions = false);

    /// Get available counters from GPA offline
    /// \param generation Hardware generation
    /// \param availableCounters Output list of counters
    /// \return true if successful
    bool GetAvailableCountersGdt(GDT_HW_GENERATION generation, CounterList& availableCounters);

    /// Get available counters from GPA offline
    /// \param uDeviceId Device Id
    /// \param uRevisionId Revision Id
    /// \param nMaxPass Maximum number of passes allowed
    /// \param availableCounters Output list of counters
    /// \return true if successful
    bool GetAvailableCountersForDevice(gpa_uint32 uDeviceId, gpa_uint32 uRevisionId, size_t nMaxPass, CounterList& availableCounters);

    /// Sets the list of enabled counters. Can be called after InitGPA to modify the set of enabled counters
    /// \param countersToEnable list of counters to enable
    /// \return true if successful
    bool SetEnabledCounters(const CounterList& countersToEnable);

    /// If SetEnabledCounters is called, selected counters will be enabled, if not, all counters are enabled
    /// \return true if successful, false otherwise
    bool EnableCounters();

    /// Gives the number of passes required by the given counter list - enables the counter too
    /// \param counterList list of the counters
    /// \param[out] counterPassInfoList list containing device info, counter list and number of pass
    /// \return true if successful, false otherwise
    bool GetNumberOfPass(const CounterList counterList, std::vector<GPAUtils::CounterPassInfo>& counterPassInfoList);

    /// Load GPA Dll, load counter files if specified
    /// \param api the API to initialize
    /// \param strDLLPath GPA DLL Path
    /// \param strError Error string output
    /// \param pszCounterFile Counter file path
    /// \param[out] pSelectedCounters the list of selected counters
    /// \param nMaxPass Maximum number of passes allowed
    /// \return true if GPAUtils has been initialized.
    bool InitGPA(GPA_API_Type api,
                 const gtString& strDLLPath,
                 std::string& strError,
                 const char* pszCounterFile = NULL,
                 CounterList* pSelectedCounters = NULL,
                 size_t nMaxPass = GPA_INFINITE_PASS);

    /// unloads the currently loaded GPA dll
    void Unload()
    {
        m_GPALoader.Unload();
        m_HWCounterMap.clear();
        m_bInit = false;
    }

    /// Is GPAUtils initialized
    /// \return true if GPAUtils is initialized
    bool IsInitialized() const
    {
        return m_bInit;
    }

    /// Check GPA status code
    /// \param status GPA status code
    /// \return the passed-in status code
    GPA_Status StatusCheck(GPA_Status status);

    /// Get counter value
    /// \param uSessionID Session ID
    /// \param counterDataTable counter result array
    /// \param sampleCount Output sample count
    /// \param count Output Output Number of enabled counters
    void GetCounterValues(gpa_uint32 uSessionID, CounterList& counterDataTable, gpa_uint32& sampleCount, gpa_uint32& count);

    /// Verify counter name - This is used to remove unknown counter name specified in the counter file
    /// \param strCounter Counter name
    /// \param generation Hardware generation
    void VerifyCounter(const std::string& strCounter, GPA_HW_GENERATION& generation);

private:
    /// Disable copy constructor
    GPAUtils(const GPAUtils& gpautils);

    /// Disable assignment operator
    GPAUtils& operator= (const GPAUtils& gpautils);

    /// Get counter by generation, non-thread safe
    /// \param generation Hardware generation
    /// \param shouldIncludeCounterDescriptions optional param to add description to the output list of the counters
    /// \return the list of counters for thie specified generation
    CounterList& GetCounters(GPA_HW_GENERATION generation, const bool shouldIncludeCounterDescriptions = false);

    /// Get counters by deviceID, non-thread safe
    /// \param uDeviceid the device id to get counters for
    /// \param uRevisionid the revision id to get counters for
    /// \param nMaxPass Maximum number of passes allowed
    /// \return the list of counters for thie specified device
    CounterList& GetCountersForDevice(gpa_uint32 uDeviceid, gpa_uint32 uRevisionid, size_t nMaxPass);

    /// Helper function to enable list of counters
    /// \param selectedCounterNames Selected counter names
    /// \return true if successful
    bool EnableCounterSet(const CounterList& selectedCounterNames);

    /// Convert GDT HW genration enum to GPA HW generation enum
    /// \param[in] gdtHwGen The GDT HW generation value
    /// \return GPA Hw generation that matches the input GDT HW generation
    GPA_HW_GENERATION GdtHwGenToGpaHwGen(const GDT_HW_GENERATION gdtHwGen);

    /// Logging callback function for GPA
    /// \param messageType the type of message being logged
    /// \param message the message to log
    static void GPALogCallback(GPA_Logging_Type messageType, const char* message);

private:
    GPUPerfAPILoader       m_GPALoader;                      ///< the loader that is used to load GPA dlls
    unsigned int           m_nMaxNumCounter;                 ///< Maximum number of counters selected
    CounterList            m_selectedCounters;               ///< List of selected counters - NOTE: If counter file not specified, list is empty and all counter will be enabled.
    GPA_GetAvailableCountersByGenerationProc m_pGetAvailableCountersByGen;     ///< Function to retrieve list of counters for a particular hw generation
    GPA_GetAvailableCountersForDeviceProc    m_pGetAvailableCountersForDevice; ///< Function to retrieve list of counters for a particular device
    HWCounterMap           m_HWCounterMap;                   ///< Hardware generation to counter list map
    HWCounterDeviceMap     m_HWCounterDeviceMap;             ///< Device Id to counter list map
    GPA_API_Type           m_API;                            ///< API type
    bool                   m_bInit;                          ///< A flag indicating whether or not GPAUtils is initialized
    size_t                 m_nMaxPass;                       ///< Max pass allowed
};

// @}

#endif //_GPA_UTILS_H_
