//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileDataAccess.h
/// \brief CPU profile translated data access interface.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingTranslation/src/CpuProfileDataAccess.h#6 $
// Last checkin:   $DateTime: 2016/04/14 01:44:54 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569055 $
//=====================================================================

#ifndef _CPUPROFILEDATAACCESS_H_
#define _CPUPROFILEDATAACCESS_H_

// Suppress Qt header warnings
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning(push)
    #pragma warning(disable : 4127 4718)
#endif
#include <QSet>
#include <QMutex>
#include <QMap>
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning(pop)
#endif

#include <CpuProfileDataTranslation.h>
#include <AMDTCpuPerfEventUtils/inc/EventsFile.h>
#include <AMDTCpuProfilingRawData/inc/CpuProfileReader.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    #include <AMDTCpuProfilingTranslation/inc/Windows/TaskInfoInterface.h>
    #include <AMDTCpuProfilingRawData/inc/Windows/PrdReader.h>
    #include <AMDTProfilingAgentsData/inc/JavaJncReader.h>
    #include <AMDTProfilingAgentsData/inc/Windows/ClrJncReader.h>

    //Windows registry path
    #define CPA_EVENTS_INSTALL_DIR_REG      L"SOFTWARE\\AMD"
    //Windows registry key
    // Note that the path stored in this key needs to end in a slash
    #define CPA_EVENTS_KEY_REG              L"CpuEventPath"

#else


#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

#if SUPPORT_CSS
    #include "ModAnalyzer.h"
#endif

const gtUInt64 HASHSIZE = 4096ULL;


typedef gtMap<SampleDatumKey, gtUInt64> SampleDataMap;
/** \struct ModAggregate
    \brief This structure allows module data aggregation
*/
struct ModAggregate
{
    ModuleDataType modData;
    SampleDataMap sampleData;
};

/** \struct InstAggregate
    \brief This structure allows instruction data aggregation
*/
struct InstAggregate
{
    InstructionDataType instData;
    SampleDataMap sampleData;
};

/** The less than function for the \ref InstAggregate
*/
inline bool operator< (const InstAggregate& temp1, const InstAggregate& temp2)
{
    return (temp1.instData < temp2.instData);
}

/** \struct RawAggregate
    \brief This structure allows raw data aggregation
*/
struct RawAggregate
{
    RawDataType rawData;
    SampleDataMap sampleData;
};

/** \struct InstructionKey
    \brief This structure holds the key to the instruction aggregation map
*/
struct InstructionKey
{
    gtVAddr offset;
    gtUInt64 addr; // load address
    gtUInt32 pid;
    gtUInt32 thread_id;
};

/** This function has to imply that !(a < b) is (a >= b), for the QMap sorting
    to work
*/
inline bool operator< (const InstructionKey& temp1, const InstructionKey& temp2)
{
    if (temp1.addr < temp2.addr)
    {
        return true;
    }
    else if (temp1.addr > temp2.addr)
    {
        return false;
    }
    else if (temp1.offset < temp2.offset)
    {
        return true;
    }
    else if (temp1.offset > temp2.offset)
    {
        return false;
    }
    else if (temp1.pid < temp2.pid)
    {
        return true;
    }
    else if (temp1.pid > temp2.pid)
    {
        return false;
    }
    else if (temp1.thread_id < temp2.thread_id)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/** This class represents the data for one module.  It holds a list of sample
    data for pids as well as instruction information for the module.

    After the aggregation is finished, \ref aggregateHashes will collect and
    append the hashed data into a single list for performance.

*/
class ModuleAggregator
{
public:
    /** constructor */
    ModuleAggregator();
    /** Destructor */
    ~ModuleAggregator();

    /** This also allocates the hash array based on the size of the named
    module
    @param[in] pData The info with which to init
    */
    HRESULT Initialize(const RawAggregate* pData);

    /** This adds the sample to the process map and the appropriate hash */
    void AddSample(const RawAggregate* pData);

    /** Since the aggregation is finished, this takes the results from the
    hashed maps and puts them into the list.  This will also free the
    allocated hash array afterwards */
    void AggregrateHashes();

    /** This fills the given list with the instruction data */
    HRESULT FillInstructionList(gtUInt64 coreRule,
                                const QSet<gtUInt64>& rulePid,
                                const QSet<gtUInt64>& ruleTid,
                                gtList<InstAggregate>* pInstructionData,
                                CpuProfileReader* pReader);

    /** This fills the given list with the process data for this module */
    HRESULT FillModuleList(gtUInt64 coreRule, const QSet<gtUInt64>& rulePid, gtList<ModAggregate>* pSystemData);

    //these are public to allow for appending data
public:
    /** Module name */
    wchar_t m_moduleName[OS_MAX_PATH + 1];
    /** Module load address */
    gtUInt64 m_address;
    /** Module size */
    gtUInt64 m_size;
    /** Whether the module is 64-bit */
    bool m_is64Bit;
    /** What type of module is it */
    ModuleType m_type;
    /** This is the instruction list for the module, filled in after the
    aggregation is finished */
    gtList<InstAggregate> m_dataList;
    /** This holds samples for each of the different pids that were sampled in
    this module */
    QMap<gtUInt32, ModAggregate> m_aggProcessData;
private:
    /** Each array entry is associated with a certain range of addresses, thus
    reducing the search time required by a single huge map for large modules */
    QMap<InstructionKey, InstAggregate>* m_hashArray;
    /** count of hash array */
    int m_hashCount;
};

/** This class represents the data for one data set.  It holds a list of
    intervals, a list of process information, and a map of modules by name.

    After the aggregation is finished, \ref aggregateMaps will collect and
    append the hashed data into a single list for performance.

*/
class ProfileDataSet
{
public:
    /** constructor */
    ProfileDataSet();
    /** destructor */
    ~ProfileDataSet();

    /** This writes the lists of data to the given path name
    @param[in] pPathName The profile data file to write
    @param[in] pAggDir The directory to get the jit and css files from
    @param[in] missedCount The number of samples missed during the profile
    @param[in] profileCoreMask The cores which were valid for the profile
    @param[in] cpuFamily The family the profile was taken on
    @param[in] cpuModel The model the profile was taken on
    @param[in] eventList The list of events in the profile
    @param[in] periodList The list of periods of events
    @param[in] groupCount The number of groups in the profile
    @param[in] pCssPids The set of pids with css info
    @param[in] pTopology The topology map
    \note This requires appropriate supporting files if applicable */
    HRESULT WriteToFile(const wchar_t* pPathName,
                        const wchar_t* pAggDir,
                        gtUInt64 missedCount,
                        gtUInt64 profileCoreMask,
                        int cpuFamily,
                        int cpuModel,
                        const gtList<gtUInt64>& eventList,
                        const gtList<gtUInt64>& periodList,
                        int groupCount,
                        QSet<gtUInt64>* pCssPids,
                        CoreTopologyMap* pTopMap);

    /** This reads the data from an existing profile data file
    @param[in] pReader The profile reader to get data from
    */
    HRESULT ReadFromFile(CpuProfileReader* pReader);

    /** This adds the sample to the process map and the appropriate module */
    void AddSample(const RawAggregate* pData, bool checkInterval);

    /** Since the aggregation is finished, this takes the results from the
    map and puts them into the list.  */
    void AggregrateMaps();

    /** clears the partial data for the set away */
    void Clear();

    /** This gets the count of the system list appropriate to the rules
        The module data is cached for the \ref fillModuleData call

    @param[in] coreRule The rule about which core data to use
    @param[in] m_rulePid The rule about which process data to use
    */
    size_t GetModuleDataCount(gtUInt64 coreRule, const QSet<gtUInt64>& rulePid);

    /** This fills the system list appropriate to the rules
    @param[in] coreRule The rule about which core data to use
    @param[in] m_rulePid The rule about which process data to use
    @param[in] maxSize The maximum number of data to fill
    @param[out] pSystemData The data to return
    */
    HRESULT FillModuleData(gtUInt64 coreRule, const QSet<gtUInt64>& rulePid, unsigned int maxSize, ModuleDataType* pSystemData);

    /** This gets the count of the system list appropriate to the rules
        The process data is cached for the \ref fillProcessData call

    @param[in] coreRule The rule about which core data to use
    @param[in] m_rulePid The rule about which process data to use
    */
    size_t GetProcessDataCount(gtUInt64 coreRule, const QSet<gtUInt64>& rulePid);

    /** This fills the process list appropriate to the rules
    @param[in] coreRule The rule about which core data to use
    @param[in] m_rulePid The rule about which process data to use
    @param[in] maxSize The maximum number of data to fill
    @param[out] pProcessData The data to return
    */
    HRESULT FillProcessData(gtUInt64 coreRule, const QSet<gtUInt64>& rulePid, unsigned int maxSize, ModuleDataType* pProcessData);

    /** This gets the count of the instruction list appropriate to the rules
        The process data is cached for the \ref fillInstructionData call

    @param[in] coreRule The rule about which core data to use
    @param[in] m_rulePid The rule about which process data to use
    @param[in] m_ruleTid The rule about which thread data to use
    @param[in] modulePath Which module on which to return data
    @param[out] pCount The count of data
    \retval E_INVALIDARG if modulePath was not a valid module for the data set
    */
    HRESULT GetInstructionDataCount(gtUInt64 coreRule,
                                    const QSet<gtUInt64>& rulePid,
                                    const QSet<gtUInt64>& ruleTid,
                                    const wchar_t* modulePath,
                                    unsigned int* pCount);

    /** This fills the instruction list appropriate to the rules and given
    module
    @param[in] coreRule The rule about which core data to use
    @param[in] m_rulePid The rule about which process data to use
    @param[in] m_ruleTid The rule about which thread data to use
    @param[in] modulePath Which module on which to return data
    @param[in] maxSize The maximum number of data to fill
    @param[out] pInstructionData The data to return
    */
    HRESULT FillInstructionData(gtUInt64 coreRule,
                                const QSet<gtUInt64>& rulePid,
                                const QSet<gtUInt64>& ruleTid,
                                const wchar_t* modulePath,
                                unsigned int maxSize,
                                InstructionDataType* pInstructionData,
                                gtUInt64* pModuleLoadAddress,
                                ModuleType* pModuleType);

    /** This retrieves the jit data for the given address and jnc data file
    @param[in] address The address to get jit info for
    @param[in] pJncDataFile The jit data file to use
    @param[in] pJncDir The directory in which the jnc file should be
    @param[in] jitType Whether the module is Java or Managed
    @param[out] pJitData The data to return
    */
    HRESULT GetJitData(gtUInt64 address, const wchar_t* pJncDataFile, const wchar_t* pJncDir, int jitType, JITDataType* pJitData);

    /** Is aggregated data ready for this data set?
    \return Whether the data set is ready
    */
    bool IsReady() const;

    /** Checks to see the 'bitness' of a module.  If the name is unknown,
    the address contains clues.
    \return Whether the module is 64-bits
    */
    static bool IsModule64Bit(wchar_t* pModuleName, gtUInt64 address, gtUInt32 pid);

    /** Checks to see if the time is in the interval covered by the data set
    \return Whether the time is in the data set's interval
    */
    bool IsInInterval(const CPA_TIME* pTime) const;

    /** This compare 2 CPA_TIME
    @param[in] pTime1 the first time
    @param[in] pTime2 the second time
    \return
    \retval -2 means pTime1 or pTime2 is NULL
    \retval -1 means 1st time is earlier than the 2nd
    \retval  0 means 1st time is equal to the 2nd
    \retval  1 means 1st time is later than the 2nd
    */
    int Compare_CPA_TIME(const CPA_TIME* pTime1, const CPA_TIME* pTime2) const;

    //these are public to allow for appending data
public:
    /** The intervals of interest to the set
    \note There may be a better way to track intervals, like a binary tree?
    This currently needs to be searched for each sample for each data set.
    */
    gtList<ProfileDataSetInterval> m_intervals;
    /** The map used to aggregate module and instruction data */
    QMap<QString, ModuleAggregator> m_aggModuleData;
    /** The map used to aggregate process data */
    QMap<gtUInt32, ModAggregate> m_aggProcessData;
private:
    /** Whether the data has been aggregated in this set, yet */
    bool m_ready;
    CpuProfileReader* m_pProfileReader;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    /** This maps java jnc file names to the jnc reader */
    QMap<QString, JavaJncReader> m_JncMap;
    /** This maps clr jnc file names to the jnc reader */
    QMap<QString, ClrJncReader> m_ClrMap;
#else
    //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    gtUInt64 m_cacheCoreRule;
    QSet<gtUInt64> m_cacheRulePid;
    QSet<gtUInt64>  m_cacheRuleTid;
    QString m_cacheModName;
    gtList<ModAggregate> m_moduleCache;
    gtList<ModAggregate> m_processCache;
    gtList<InstAggregate> m_instructionCache;
    gtList<SampleDatumKey*> m_keyCache;
    gtList<gtUInt64*> m_dataCache;
    gtList<wchar_t*> m_stringCache;
};

/** \struct CssDataInfo
    The info stored from a previous record to describe a css record
*/
struct CssDataInfo
{
    /** is css stack data available */
    bool dataAvailable;
    /** process id */
    gtUInt64 pid;
    /** thread id */
    gtUInt64 tid;
    /** time stamp of the css record */
    gtUInt64 timeStamp;
    /** address at which the css is traced */
    gtUInt64 addr;
    /** on which core was the css traced */
    int core;
    /** the css stack */
    gtList <gtUInt64> stack;
};

#if SUPPORT_CSS
    /** This typedef holds the module analysis for css profiles */
    typedef QMap<QString, CModAnalyzer*> ModAnalysisMap;
#endif

/** This class is the internal version of the data access API.

    It should handle all the requests from the API as well as any internal
    requests
*/
class CpuProfileDataAccess
{
public:
    /** Creator */
    CpuProfileDataAccess();

    /** Destructor, frees any remaining allocated memory */
    ~CpuProfileDataAccess();

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    /** This function will open a raw profile data file and read the headers.
        @param[in] pPrdFile The profile data file of interest
        \note This function assumes that the file exists

        \return The success of opening the profile
        \retval S_OK Success
        \retval E_ACCESSDENIED The file wasn't able to be opened
        \retval E_OUTOFMEMORY The system is unable to allocate memory
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT OpenPrdFile(const wchar_t* pPrdFile);
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    /** This function checks to see if a prd file was opened with the \ref
        OpenPrdFile function.

        \return Whether a prd file was opened for the data access
        \retval true The profile data is available
        \retval false The profile data is not available
    */
    bool IsProfileDataAvailable() const;

    /** This function will open an aggregated profile data file and import the
        data into a data set.
        @param[in] pPofileFile The profile data file of interest
        \note This function assumes that the file exists

        \return The success of opening the profile
        \retval S_OK Success
        \retval E_ACCESSDENIED The file wasn't able to be opened
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT ImportDataSet(const wchar_t* pProfileFile);

    /** This function will return whether the data set has already been defined.
        @param[in] pDataSetName The data set name

        \return Whether the data set has been defined
        \retval true The data set is already present
        \retval false The data has not been defined yet
    */
    bool IsDataSetExists(const wchar_t* pDataSetName) const;

    /** This function will use the JIT data files from the specified directory
        during the data aggregation.  This has to be called before a profile with
        JIT data is aggregated for the JIT information to be used.

        @param[in] pJitDirName The JIT data file directory of interest, NULL to use
        the default
        \return The success of setting the directory to JIT data files of interest
        \retval S_OK Success
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT SetJitDir(const wchar_t* pJitDirName);

    /** This will return the time mark of the start of the profile
        @param[out] pTimeMark The time mark at the start of the profile
        \return The success of getting the time mark
        \retval S_OK Success
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT GetStartTimeMark(CPA_TIME* pTimeMark) const;

    /** This will return the time mark of the end of the profile
        @param[out] pTimeMark The time mark at the end of the profile
        \return The success of getting the time mark
        \retval S_OK Success
        \retval E_FAIL The profile data was aggregated, and this is not available
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT GetEndTimeMark(CPA_TIME* pTimeMark) const;

    /** This will define a new data set by the intervals.
    @param[in] intervals The list of intervals that defines the set
    @param[in] count The number of intervals in the array
    @param[in] pDataSetName The data set name for later retrieval
    \return The success of adding the data set
    \retval S_OK Success
    \retval E_FAIL The profile data was aggregated, and this is not available
    \retval E_OUTOFMEMORY no more memory is available
    \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT AddDataSet(const ProfileDataSetInterval* pIntervals, unsigned int count, const wchar_t* pDataSetName);

    /** This will provide a list of the defined data sets

        @param[out] pDataSets The list of names of data sets
        \return The success of getting the data sets
        \retval S_OK Success
        \retval E_OUTOFMEMORY no more memory is available
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT GetDataSets(gtList<wchar_t*>* pDataSets);

    /** This will remove the named data set, except for the default.
        @param[in] pDataSetName The data set name to remove
        \return The success of removing the data set
        \retval S_OK Success
        \retval E_INVALIDARG an invalid data set name was provided
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT RemoveDataSet(const wchar_t* pDataSetName);

    /** The plan is to split up the prd file up and have one thread each read a
        bit. The flow of a thread would be to read data, determine which data sets
        it applies to, search the module information, and aggregate it.
        To aggregate the data into module instructions, we will use an array of maps.
        The number of maps will depend on the module's size and which map is used
        depends on the hash of the address offset.

        We will determine the most advantageous number of prd file chunks and size
        hash by experimentation.

        @param[in] pCancel an optional way to cancel the processing from another
        thread.
        @param[in] pJitDataDirectory The directory, if any, to write the JIT data
        files to.
        @param[out] pPercentComplete an optional way to track the progress of the
        processing

        \return The success of processing
        \retval S_OK Success
        \retval S_FALSE You cancelled the processing
        \retval E_INVALIDARG pJitDataDirectory was not NULL or writable
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT AggregateDataSets(const bool* pCancel, const wchar_t* pJitDataDirectory, float* pPercentComplete);

    /** This returns the current aggregation status (aggregating or not aggregating).
        Aggregation may be initiated in its own thread. This function returns the
        current state of aggregation. Not recommended for synchronization.

        \return Whether aggregation is progress or not.
        \retval true Aggregation is in progress
        \retval false Aggregation is not in progress
    */
    bool IsAggregationInProgress() const { return m_aggregating; }

    /** This will return whether data has been aggregated for the specified data
    set.

    @param[in] pDataSetName The data set name to check
    \return Whether the data has been aggregated for the data set
    \retval true The data is available
    \retval false The profile data was not yet aggregated
    */
    bool IsDataSetReady(const wchar_t* pDataSetName) const;

    /** This will write the named data set to a file, including all necessary
        call-stack and JIT data files in the same directory.  No filters will be
        applied to the written data.

        @param[in] pDataSetName The data set name to write
        @param[in] pFileName The file name to write to
        \return The success of removing the data set
        \retval S_OK Success
        \retval E_ACCESSDENIED the access to the path was denied
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT WriteSetToFile(const wchar_t* pDataSetName, const wchar_t* pFileName);

    /** This will append the source data set into the destination data set.

        @param[in] pDestinationDataSet The destination data set which will hold the
            aggregated data
        @param[in] pSourceDataSet The source data set to append
        \retval S_OK Success
        \retval E_FAIL The data set intervals overlapped
        \retval E_OUTOFMEMORY no more memory is available
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT AppendDataSets(const wchar_t* pDestinationDataSet, const wchar_t* pSourceDataSet);

    /** This function will retrieve the available core mask from the profile.

        \note This assumes that \ref IsProfileDataAvailable is true

        @param[out] pCoreMask The core mask of the profile
        \return The success of getting the available core mask
        \retval S_OK Success
        \retval E_INVALIDARG an invalid pointer was given
    */
    HRESULT GetAvailableCoreData(gtUInt64* pCoreMask) const;

    /** This function will retrieve the available core count from the profile.

        \note This assumes that \ref IsProfileDataAvailable is true

        @param[out] pCoreCount The core count of the profile
        \return The success of getting the available core count
        \retval S_OK Success
        \retval E_INVALIDARG an invalid pointer was given
    */
    HRESULT GetAvailableCoreCount(unsigned int* pCoreCount) const;

    /** This function will set the rule for the returned data to the queries.

        @param[in] coreMask The core data to show.  It must be greater than 0.

        \return The success of setting the rule
        \retval S_OK Success
        \retval E_INVALIDARG an invalid core mask was given
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT SetRuleCoreData(gtUInt64 coreMask);

    /** This will set the rule for the returned data to the queries to limit data
        returned to only the selected processes.

        @param[in] count The number of process ids in the array
        @param[in] processIdList The list of processes ids to show.
        \return The success of setting the rule
        \retval S_OK Success
        \retval E_INVALIDARG an invalid list was given
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT SetRuleForProcesses(unsigned int count, const unsigned int* pProcessIdList);

    /** This will set the rule for the returned data to the queries to limit data
        returned to only the selected threads.

        @param[in] count The number of process ids in the array
        @param[in] threadIdList The list of thread ids to show.
        \return The success of setting the rule
        \retval S_OK Success
        \retval E_INVALIDARG an invalid list was given
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT SetRuleForThreads(unsigned int count, const unsigned int* pThreadIdList);

    /** This will reset all rules back to their defaults.

        \return The success of resetting the rules
        \retval S_OK Success
    */
    HRESULT ResetRules();

    /** This will get the count of the performance events in the profile

        \return The count of the performance events in the profile
    */
    size_t GetDataEventCount() const;

    /** This will get the performance events in the profile

        @param[in] maxSize The maximum number of data to fill
        @param[out] pPerformanceEvents The list of performance events, in the same
        order as the data values
        @param[out] pDataLabels The list of human-readable names of performance
        events, in the same order as the data values
        @param[out] pSampIntvl The list of sample intervals of performance events,
        in the same order as the data values

        \return The success of retrieving the data events
        \retval S_OK Success
        \retval E_INVALIDARG All lists were NULL
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT GetDataEvents(unsigned int maxSize, gtUInt64* pPerformanceEvents, wchar_t** ppDataLabels, gtUInt64* pSampIntvls);

    /** This will aggregate the sample data from source into the destination.

        @param[in,out] pDestination The data with the resulting aggregation
        @param[in] pSource The data which will be aggregated
        \return The success of aggregating the data
        \retval S_OK Success
        \retval E_INVALIDARG either pDestination or pSource were NULL
        \retval E_UNEXPECTED an unexpected error occurred
    */
    static HRESULT Aggregate(SampleDataMap* pDestination, const SampleDataMap* pSource);

    /** This function will retrieve the count of system-wide list of modules for
        the given data set, depending on the rules set. If NULL is given for the
        data set, then the default will be used.

        @param[in] pDataSetName which data set to retrieve data from

        \return The count of data for the data set
    */
    size_t GetModuleDataCount(const wchar_t* pDataSetName);

    /** This function will retrieve the system-wide list of modules and data for
    the given data set, depending on the rules set. If NULL is given for the
    data set, then the default will be used.

    @param[in] pDataSetName which data set to retrieve data from
    @param[in] maxSize The maximum number of data to fill
    @param[out] pSystemData The list of data

    \return The success of retrieving the data
    \retval S_OK Success
    \retval E_INVALIDARG pSystemData was NULL
    \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT GetModuleData(const wchar_t* pDataSetName, unsigned int maxSize, ModuleDataType* pSystemData);

    /** This function will retrieve the count of system-wide list of processes for
        the given data set, depending on the rules set. If NULL is given for the
        data set, then the default will be used.

        @param[in] pDataSetName which data set to retrieve data from

        \return The count of data for the data set
    */
    size_t GetProcessDataCount(const wchar_t* pDataSetName);

    /** This function will retrieve the system-wide list of processes and data for
        the given data set, depending on the rules set. If NULL is given for the
        data set, then the default will be used.

        @param[in] pDataSetName which data set to retrieve data from
        @param[in] maxSize The maximum number of data to fill
        @param[out] pProcessData The list of data

        \return The success of retrieving the data
        \retval S_OK Success
        \retval E_INVALIDARG pSystemData was NULL
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT GetProcessData(const wchar_t* pDataSetName, unsigned int maxSize, ModuleDataType* pProcessData);

    /** This function will retrieve the count of system-wide list of processes for
        the given data set, depending on the rules set. If NULL is given for the
        data set, then the default will be used.
        @param[in] pDataSetName which data set to retrieve data from
        @param[in] modulePath The module to retrieve instructions for
        @param[out] pCount The count

        \return The count of data for the data set
        \retval E_INVALIDARG if modulePath was not a valid module for the data set
    */
    HRESULT GetInstructionDataCount(const wchar_t* pDataSetName, const wchar_t* modulePath, unsigned int* pCount);

    /** This function will retrieve the list of instruction addresses and data for
        the given module in the given data set, depending on the rules set. If
        NULL is given for the data set, then the default will be used.

        @param[in] pDataSetName which data set to retrieve data from
        @param[in] modulePath The list of data
        @param[in] maxSize The maximum number of data to fill
        @param[out] pInstructionData The list of data

        \return The success of retrieving the data
        \retval S_OK Success
        \retval E_INVALIDARG pModuleData was NULL, or modulePath was not a valid
        module for the data set
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT GetInstructionData(const wchar_t* pDataSetName,
                               const wchar_t* modulePath,
                               unsigned int maxSize,
                               InstructionDataType* pInstructionData,
                               gtUInt64* pModuleLoadAddress,
                               ModuleType* pModuleType);

    /** This function will retrieve the saved JIT data given the address and JNC
        file name.

        @param[in] pDataSetName which data set to retrieve data from
        @param[in] address The address to report on
        @param[in] pJncDataFile The jnc file of JIT data
        @param[in] jitType Whether the module is Java or Managed
        @param[out] pJitData The JIT data

        \return The success of retrieving the data
        \retval S_OK Success
        \retval E_INVALIDARG pModuleData was NULL, the address was not found in
        the provided jnc file, or the jnc file was not found
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT GetJitData(const wchar_t* pDataSetName, gtUInt64 address, const wchar_t* pJncDataFile, int jitType, JITDataType* pJitData);

    /** This function will get the first raw (un-aggregated) record from the
        profile intervals defined by the data set. If NULL is given for the data
        set, then the default will be used.

        @param[in] pDataSetName which data set to retrieve data from
        @param[out] pData the first raw sample data

        \return The success of retrieving the data
        \retval S_OK Success
        \retval E_INVALIDARG pDataSetName was not a valid name, or pData was NULL
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT GetFirstRawRecord(const wchar_t* pDataSetName, RawDataType* pData);

    /** This function will get the next raw (un-aggregated) record from the
        profile intervals defined by the data set.

        @param[out] pData the first raw sample data
        @param[out] pPercentComplete the optional percentage complete
        \return The success of retrieving the data
        \retval S_OK Success
        \retval S_FALSE There are no more records available
        \retval E_POINTER \ref fnGetFirstRawRecord has not been called yet
        \retval E_INVALIDARG pData was NULL
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT GetNextRawRecord(RawDataType* pData, float* pPercentComplete);

    /** This function will get the profile type
    \return The profile type as defined in PRDReader.h
    */
    unsigned int GetProfileType() const;
    /** This function will get the ibs config
    \note This may be expanded to include the fetch randomization and op
    dispatch
    @param[out] pFetchCount the fetch count
    @param[out] pOpsCount the ops count
    */
    void GetIbsConfig(gtUInt64* pFetchCount, gtUInt64* pOpsCount) const;

    int GetCpuFamily() const;
    int GetCpuModel() const;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    const wchar_t* GetPrdFilePath() const { return  m_inputFile; }
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

private: //helper functions
    /** This function will set the jit dir to read to the default registry */
    void SetDefaultJitDir();

    /** This function will add all ibs fetch events to the lists */
    void AddIbsFetchEvents();

    /** This function will add all ibs op events to the lists */
    void AddIbsOpEvents();

    /** This function helps add the event */
    void AddOneIbsEvent(unsigned int eventSelect);

    /** Checks the rules to see if the record is valid */
    bool IsRawOkayByRules(gtUInt64 pid, gtUInt64 tid, int core, CPA_TIME* pTime, bool checkInterval) const;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    void GetModInfoHelper(const void* pVoid, TiModuleInfo* pModInfo, unsigned char dataType);

    void TranslateIbsFetch(const IBSFetchRecordData& ibsFetchRec, const TiModuleInfo* pModInfo, RawAggregate* pData);

    void TranslateIbsOp(const IBSOpRecordData& ibsOpRec, const TiModuleInfo* pModInfo, RawAggregate* pData);
#else
    //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    void ProcessCssData();

    HRESULT OpenEventsFile(int cpuFamily, int cpuModel);

    CPA_TIME SynchronizeCPA(const CPA_TIME& start, gtUInt64 deltaTick, int core, unsigned int extraMs = 0U);

    /** This function will get the first raw (aggregated) record from the
        profile intervals defined by the data set. If NULL is given for the data
        set, then the default will be used.

        @param[in] pDataSetName which data set to retrieve data from
        @param[out] pData the first raw sample data

        \return The success of retrieving the data
        \retval S_OK Success
        \retval E_INVALIDARG pDataSetName was not a valid name, or pData was NULL
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT GetFirstRawAggregateRecord(const wchar_t* pDataSetName, RawAggregate* pData);

    /** This function will get the next raw (aggregated) record from the
        profile intervals defined by the data set.

        @param[out] pData the first raw sample data
        @param[out] pPercentComplete the optional percentage complete
        \return The success of retrieving the data
        \retval S_OK Success
        \retval S_FALSE There are no more records available
        \retval E_POINTER \ref fnGetFirstRawRecord has not been called yet
        \retval E_INVALIDARG pData was NULL
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT GetNextRawAggregateRecord(RawAggregate* pData, float* pPercentComplete);

    /** This function will get the events file install directory

        @param[out] path array containing events file directory
        \return The success of retrieving the directory
        \retval S_OK Success
        \retval E_UNEXPECTED an unexpected error occurred
    */
    HRESULT GetEventPathFromRegistryKey(wchar_t* path);

    HRESULT SetupProfileReader(const wchar_t* pFile);
    void CleanupProfileReader();

private:
    /** Whether the data has been aggregated in some sets, yet */
    bool m_ready;
    /** The possible time marks in a profile */
    ProfileDataSetInterval m_entire;
    /** The data sets holding all the data, by name */
    QMap<QString, ProfileDataSet> m_dataSets;
    /** The available core mask of the profile */
    gtUInt64 m_profileCoreMask;
    /** The rule on what data is copied to the lists for cores */
    gtUInt64 m_ruleCoreMask;
    /** The rule on what data is copied to the lists for processes */
    QSet<gtUInt64> m_ruleProcessIdList;
    /** The rule on what data is copied to the lists for threads */
    QSet<gtUInt64> m_ruleThreadIdList;

    /** The cpu family of the profile */
    int m_cpuFamily;
    /** The cpu model of the profile */
    int m_cpuModel;
    /** The core count of the profile */
    unsigned int m_coreCount;
    /** Whether the prd interval is useful */
    bool m_checkInterval;

    /** Used to write profile files */
    int m_groupCount;
    /** The collection of events */
    gtList<gtUInt64> m_events;
    EventsFile m_eventsFile;
    /** The collection of event names */
    gtList<QString> m_labels;
    /** The collection of event periods, useful for determining norms */
    gtList<gtUInt64> m_sampIntvls;

    gtList<wchar_t*> m_stringCache;
    gtList<SampleDatumKey*> m_keyCache;
    gtList<gtUInt64*> m_dataCache;
    //aggregation member variables

    /** The mutex to make the aggregation thread safe */
    QMutex m_mutex;
    /** The jit input directory */
    wchar_t m_jitDir[OS_MAX_PATH];
    /** The aggregate output file directory */
    wchar_t m_aggOutDir[OS_MAX_PATH];
    /** The data set's intervals to use during aggregation */
    ProfileDataSet* m_pRawDataSet;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    /** The prd reader to use during aggregation */
    PrdReader* m_pPrdReader;
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    /** The input filename to use */
    wchar_t m_inputFile[OS_MAX_PATH];
    /** The approximate number of records in a prd file */
    gtUInt64 m_approxRecords;
    /** The known progress through the records in a prd file */
    gtUInt64 m_rawProgress;
    /** Whether we're currently aggregating */
    bool m_aggregating;
#if SUPPORT_CSS
    /** The modules analyzed for the css analysis */
    ModAnalysisMap m_CssMap;
#endif
    /** The list of css process ids */
    QSet<gtUInt64> m_cssPids;
    /** The accumulative css identifying data from the previous record */
    CssDataInfo m_rawCss;
    /** The number of missed samples within the prd file */
    gtUInt64 m_missedCount;
    /** Used to identify the worst event*/
    gtUInt64 m_worstCount;
    /** Used to identify the worst event*/
    int m_worstGroup;
    /** Used to identify the worst event*/
    int m_worstCounter;
    /** Stores the CPU topology */
    CoreTopologyMap m_topMap;
};

#endif // _CPUPROFILEDATAACCESS_H_
