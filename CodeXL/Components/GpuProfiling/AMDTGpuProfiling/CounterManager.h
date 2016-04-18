//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CounterManager.h $
/// \version $Revision: #35 $
/// \brief :  This file contains CounterManager class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CounterManager.h#35 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================
#ifndef _COUNTERMANAGER_H_
#define _COUNTERMANAGER_H_

#ifdef _WIN32
    #pragma warning(push, 1)
#endif
#include <QtCore>
#include <QtWidgets>
#ifdef _WIN32
    #pragma warning(pop)
#endif

#include <AMDTOSWrappers/Include/osModule.h>

#include <TSingleton.h>
#include <GPACounterGenerator.h>
#include <GPUPerfAPICounters.h>

#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>

#include <AMDTGpuProfiling/Util.h>
#include <AMDTGpuProfiling/CounterGroup.h>


/// enum for hardware families for which there are distinct counters
enum HardwareFamily
{
    /// Unassigned hardware
    NA_HARDWARE_FAMILY = 0x0,

    /// Southern Islands hardware
    SOUTHERN_ISLANDS_FAMILY = 0x1,

    /// Sea Islands hardware
    SEA_ISLANDS_FAMILY = 0x2,

    /// Volcanic Islands hardware
    VOLCANIC_ISLANDS_FAMILY = 0x4
};

/// A singleton class to manage the counter settings
class AMDT_GPU_PROF_API CounterManager : public TSingleton<CounterManager>
{
    // provide access to private constructor
    friend class TSingleton<CounterManager>;

public:
    /// Get the total count of the counters related to the HW counters for the specified hardware family.
    /// \param hardwareFamily the hardware family whose counter count is needed
    /// \return the total count
    int GetHWCounterCount(HardwareFamily hardwareFamily);

    /// Check whether the counter name is related to the HW counters.
    /// \param strCounterName the counter name
    /// \return true if the counter is related to the HW counters, false otherwise
    bool IsHWCounterName(const QString& strCounterName);

    /// Get counter description from the counter name for the specified hardware family
    /// \param hardwareFamily the hardware family whose counter description is needed
    /// \param strCounterName counter name
    /// \param[out] strCounterDesc the counter description
    /// \return true if the counter name is found and has a description, false otherwise
    bool GetCounterDesc(HardwareFamily hardwareFamily, const QString& strCounterName, QString& strCounterDesc);

    /// Gets a flag indicating whether or not a counter represents a percentage value.
    /// \param hardwareFamily the hardware family whose counter is being checked
    /// \param strCounterName counterName Name of the counter.
    /// \param[out] isPercentage true if counter represents a percentage value else false
    /// \return true if counter name is found, false otherwise
    bool IsCounterTypePercentage(HardwareFamily hardwareFamily, const QString& strCounterName, bool& isPercentage);

    /// Get the counter name related to the HW counters for a specific index.
    /// \param hardwareFamily the hardware family whose counter is required
    /// \param index an index to the HW counter name list
    /// \param strHWCounterName the output HW counter name
    /// \return true if successful, false otherwise
    bool GetHWCounterName(HardwareFamily hardwareFamily, int index, QString& strHWCounterName);

    /// Set the specific counter of the counter capture option.
    /// \param hardwareFamily the hardware family whose counter is required
    /// \param strCounterName counter name
    /// \param capture boolean setting
    /// \return true if successful, false otherwise
    bool SetHWCounterCapture(HardwareFamily hardwareFamily, const QString& strCounterName, bool capture);

    /// Get the value from a specific index of the counter capture option.
    /// \param hardwareFamily the hardware family whose counters are required
    /// \param index a specific index to the counter capture option
    /// \param captureOut the output setting
    /// \return true if successful, false otherwise
    bool GetHWCounterCapture(HardwareFamily hardwareFamily, int index, bool& captureOut);

    /// Get the total group count.
    /// \param hardwareFamily the hardware family whose group count is needed
    /// \return total group
    int GetHWCounterGroupCount(HardwareFamily hardwareFamily);

    /// Get the group name given an index to the group
    /// \param hardwareFamily the hardware family whose group is needed
    /// \param  index the input index to the group
    /// \param  strCounterGroupName the output group name
    /// \return true if successful, false otherwise
    bool GetHWCounterGroupName(HardwareFamily hardwareFamily, int index, QString& strCounterGroupName);

    /// Get the counter name for the specified hardware family given a group index and the counter index.
    /// \param hardwareFamily the hardware family whose counter is being checked
    /// \param groupIndex the input group index
    /// \param counterIndex the input counter index in the group
    /// \param[out] strCounterName the output counter name
    /// \return true if successful, false otherwise
    bool GetHWCounterNameInGroup(HardwareFamily hardwareFamily, int groupIndex, int counterIndex, QString& strCounterName);

    /// Get the total count in a group given the group index.
    /// \param hardwareFamily the hardware family whose group is being checked
    /// \param index the group index
    /// \return the count in a group if successful, 0 otherwise
    int GetHWCounterCountInGroup(HardwareFamily hardwareFamily, int index);

    /// Generate counter files
    /// \param strCounterFile Counter file name
    /// \return True on success
    bool GenerateCounterFile(QString& strCounterFile);

    /// Indicates whether or not the specified hardware family is supported by the current machine
    /// \param hardwareFamily the hardware family to check
    /// \return true if the specified hardware family is supported on the current machine, false otherwise
    bool IsHardwareFamilySupported(HardwareFamily hardwareFamily);

    /// Gets the display name for the specified hardware family
    /// \param hardwareFamily the hardware family whose display name is required
    /// \param[out] strDisplayName the display name for the specified hardware family
    /// \return true if the hardware family has a display name, false otherwise
    bool GetHardwareFamilyDisplayName(HardwareFamily hardwareFamily, QString& strDisplayName);

    /// Gets the displayable list of the names of available devices for the specified hardware family
    /// \param hardwareFamily the hardware family whose devices display list is required
    /// \param[out] strDevicesDisplayList the list of device names for the specified hardware family
    void GetHardwareFamilyDevicesDisplayList(HardwareFamily hardwareFamily, QString& strDevicesDisplayList);

    /// Gets the displayable list of pass counts for the available devices for the specified hardware family, given the list of enabled counters
    /// \param hardwareFamily the hardware family whose pass counts are required
    /// \param enabledCounters the list of enabled counters for this hardware generation
    /// \param[out] strPassCountDisplayList the list of pass counts for the specified hardware family
    /// \return true if the hardware family has a list of pass counts, false otherwise
    bool GetHardwareFamilyPassCountDisplayList(HardwareFamily hardwareFamily,
                                               const QStringList& enabledCounters,
                                               const bool isGpuTimeCollected,
                                               QList<int>& numReqPassesForDevs);

    /// Gets the displayable list of pass counts for the available devices for the specified hardware family, given the list of enabled counters
    /// \param hardwareFamily the hardware family whose pass counts are required
    /// \param countersList the list of requested counters for this hardware generation
    /// \param numRequiredPassesForDevs the maximum number of passes that can be used
    /// \param enabledCounters[out] the maximal sub-set of counter names from countersList which require not more then numRequiredPassesForDevs
    /// \return true if the hardware family has a list of pass counts, false otherwise
    bool GetHardwareFamilyPassCountLimitedCountersList(HardwareFamily hardwareFamily,
                                                       const QStringList& countersList,
                                                       unsigned int numRequiredPassesForDevs,
                                                       QStringList& enabledCounters);


    void AdjustCounterManagerState(bool isRemoteSession);

    /// is devices and families list is dummy(happens for example when catalyst is not installed)
    /// the list will contain a sample device for each hardware family
    /// \returns bool if the list is dummy
    bool IsDummyDevicesAdded() const { return m_isDummyDevicesAdded; }

private:
    static const QString ms_SI_FAMILY_ENV_VAR_VALUE;  ///< string constant for the "CodeXLGPUProfilerHardwareFamily" env var value for southern islands
    static const QString ms_CI_FAMILY_ENV_VAR_VALUE;  ///< string constant for the "CodeXLGPUProfilerHardwareFamily" env var value for sea islands
    static const QString ms_VI_FAMILY_ENV_VAR_VALUE;  ///< string constant for the "CodeXLGPUProfilerHardwareFamily" env var value for volcanic islands
    static const QString ms_SI_FAMILY_NAME;           ///< string constant for the southern islands hardware family name
    static const QString ms_CI_FAMILY_NAME;           ///< string constant for the sea islands hardware family name
    static const QString ms_VI_FAMILY_NAME;           ///< string constant for the volcanic islands hardware family name
    static const int     ms_SI_PLACEHOLDER_DEVICE_ID; ///< placeholder device id for a southern islands device
    static const int     ms_CI_PLACEHOLDER_DEVICE_ID; ///< placeholder device id for a sea islands device
    static const int     ms_VI_PLACEHOLDER_DEVICE_ID; ///< placeholder device id for a volcanic islands device
    static const int     ms_UNSPECIFIED_REV_ID;       ///< unspecified device revision id
    static const int     ms_AMD_VENDOR_ID;            ///< AMD vendor id

    /// typedef used for counter indices
    struct CounterIndices
    {
        /// Constructor
        /// \param actualIndex the actual index within our counter lists (m_counterCaptureOptionHW and m_counterNamesHW)
        /// \param internalIndex the index of the counter according to GPA
        CounterIndices(int actualIndex, int internalIndex) : m_actualIndex(actualIndex), m_internalIndex(internalIndex) {}

        int m_actualIndex;   ///< the actual index within our counter lists (m_counterCaptureOptionHW and m_counterNamesHW)
        int m_internalIndex; ///< the index of the counter according to GPA
    };

    typedef QHash<QString, QString>             CounterData;               ///< typedef used for counter data
    typedef QHash<QString, CounterIndices*>     CounterNameToIndex;        ///< typedef used for counter data
    typedef decltype(GPA_GetAvailableCounters)* GPA_GetAvailableCountersProc;

    /// Initializes singleton instance of CounterManager
    CounterManager();

    void Init(bool isRemoteSession);
    void Reset();
    void FreeResources(bool isReleaseModulesRequired);

    /// Destroys singleton instance of CounterManager
    ~CounterManager();

    /// Loads the GPA Counters module and initializes the GetAvailableCounters entrypoint
    void LoadCountersModule();

    /// Setup counters: add counter names and descriptions to a dictionary
    void SetupCounterData();

    /// Indicates whether or not the specified counter is a public counter
    /// \param hardwareFamily the hardware family which should be checked for the counter
    /// \param strCounterName the name of the counter to check
    /// \return true if the counter is a public counter, false otherwise
    bool IsPublicCounter(HardwareFamily hardwareFamily, const QString& strCounterName);

    /// Indicates whether or not the specified counter is an "ignored" counter -- Ignored means that GPA exposes it, but the profiler should not
    /// \param strCounterName the name of the counter to check
    /// \return true if the counter should be ignored, false otherwise
    bool IsIgnoredCounter(const QString& strCounterName);

    /// Get the HW counter index from the counter name.
    /// \param hardwareFamily the hardware family whose counter is required
    /// \param internalIndex flag indicating whether or not the internal index (true) or public index (false) should be returned
    /// \param strCounterName the input counter name
    /// \param[out] index the output index to the counter name array
    /// \return true if successful, false otherwise
    bool GetHWCounterIndexFromName(HardwareFamily hardwareFamily, bool internalIndex, const QString& strCounterName, int& index);

    /// Gets the counter info (name/description) for the specified hardware family
    /// \param hardwareFamily the hardware family whose counters are needed
    /// \param pCounterAccessor the counter accessor object to be used to get the counter information
    void GetCounterInfoFromAccessor(HardwareFamily hardwareFamily, GPA_ICounterAccessor* pCounterAccessor);

    /// Adds the specified device id to the list of devices for its hardware family.
    /// If the family can not be gleaned from the device id, then it will be added to the
    /// NA_HARDWARE_FAMILY family
    /// \param deviceId the device id to add to the hardware family
    /// \param revId the revision id to add to the hardware family
    void AddDeviceId(int deviceId, int revId);

    /// Adds the specified device id to the list of devices for the specified hardware family
    /// \param hardwareFamily the hardware family to which the device should be added
    /// \param deviceId the device id to add to the hardware family
    /// \param revId the revision id to add to the hardware family
    void AddDeviceIdToFamily(HardwareFamily hardwareFamily, int deviceId, int revId);

    /// Updates the m_counterIndices after the counters have been sorted
    /// \param counterNames the sorted list of counters
    /// \param counterIndexMap the counter Index map whose actual indices should be updated
    void UpdateCounterIndexMap(const QStringList& counterNames, const CounterNameToIndex& counterIndexMap);

    // structure for holding device id and rev id for installed devices
    struct DeviceAndRevInfo
    {
        int m_deviceID; ///< device id
        int m_revID;    ///< rev id
    };

    osModuleHandle                                 m_gpaCountersModuleHandle;         ///< module handle for GPACounters DLL
    GPA_GetAvailableCountersProc                   m_gpa_GetAvailableCountersFuncPtr; ///< pointer to GPA_GetAvailableCounters method in GPA

    QMap<HardwareFamily, CounterData>              m_counterData;             ///< pairs of counter name and the counter description per hardware family
    QMap<HardwareFamily, QList<QString> >          m_counterNamesHW;          ///< a list of HW counter names per hardware family
    QMap<HardwareFamily, CounterNameToIndex>       m_counterIndices;          ///< map of counter name to counter index per hardware family.  Provides fast lookups in GetHWCounterIndexFromName
    QMap<HardwareFamily, QList<bool> >             m_counterCaptureOptionHW;  ///< the capture option for the HW counters per hardware family
    QMap<HardwareFamily, QList<CounterGroup*> >    m_counterGroupHW;          ///< a list of counter groups per hardware family
    QMap<HardwareFamily, int>                      m_publicCounterCount;      ///< number of public counters per hardware family
    QMap<HardwareFamily, QList<DeviceAndRevInfo> > m_availableDevices;        ///< a list of the available devices per hardware family
    QList<HardwareFamily>                          m_dummyDeviceAdded;        ///< list of hardware families for which a dummy device was added

    bool m_isDummyDevicesAdded;                 /// is true if dummy devices list was saved for case of remote session
};


#endif // _COUNTERMANAGER_H_
