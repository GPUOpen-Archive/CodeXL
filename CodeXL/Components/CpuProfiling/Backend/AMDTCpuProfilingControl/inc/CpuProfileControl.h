//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileControl.h
/// \brief AMD API to control profiling of either sampling or counting events.
/// \note This API is only valid for nodes with AMD processors.
//
// There are two main ways to use the profiling API: sampling and counting.
// Sampling will interrupt and take a sample of the current execution at the
// specified intervals. Counting will return the count of the specified events
// on command.
//
// When using this API, using any of the \ref sampling functions will result in
// a .prd file containing raw profiling data and a .ti file containing task
// information.  Using the \ref counting functions will not result in any files.
//
// Before any profiling is done, the \ref fnEnableProfiling function must be
// called.  When all profiling is finished, the \ref fnReleaseProfiling function
// must be called.
//
// If anything other than SUCCEEDED(HRESULT) was returned, the function \ref
// fnGetLastProfileError may give you some insight as to what the trouble was.
//
// There are three groups of functions: \ref profiling, \ref counting, and
// \ref sampling
//
// \defgroup profiling Profiling
// This part applies to all profiles, whether counting or sampling.  Profiles
// are exclusive, and different types may not be done simultaneously.
//
// \defgroup counting Counting
// A counting profile will let you count the number of specified events that
// occurred.
//
// There are two exclusive ways to use counting mode.  You can create a array
// of counters  to be counted or you can create the events and set them to
// indivdual cores  and counters yourself.
//
// The expected use of this would be to use \ref fnMakeProfileEvent to create
// several performance events and then set the desired events in a array or set
// them individually and start profiling with \ref fnStartProfiling.  During
// the profile, the counters can be read.
//
// \defgroup sampling Sampling
// A sampling profile will take a sample or interrupt whenever the period elapses.
//
// With timer-based sampling, the period is based on a time interval, in
// nanoseconds.  With event-based sampling, the period is based on accumulated
// performance events, on which the frequency depends on the selected event.
// With instruction-based sampling, the period can depend on ops and/or
// fetches.
//
// The expected use of this would be to use \ref fnSetProfileOutputFile to
// set an output file, set up one type of profiling configuration with \ref
// fnSetTimerConfiguration, \ref fnSetEventConfiguration, or \ref
// fnSetIbsConfiguration,   and then start profiling with \ref
// fnStartProfiling.
//
// After the profile has been stopped, you may use the
// \ref CaProfileDataAccessAPI.h to retrieve the data or import the .prd file
// into CodeAnalyst.  After the profile has been stopped, the configuration
// is not persistent, so you must set everything again to repeat the run.
///
//==================================================================================

#ifndef _CPUPROFILECONTROL_H_
#define _CPUPROFILECONTROL_H_

#include <AMDTCpuProfilingTranslation/inc/CPA_TIME.h>
#include <AMDTCpuProfilingRawData/inc/RunInfo.h>
#include "CpuProfilingControlDLLBuild.h"

/** This is a default key to use with \ref fnPauseProfiling and \ref
    fnResumeProfiling to pause and resume a profile started from the
    CodeAnalyst gui or the command line tool CaProfile
*/
const wchar_t CPU_PROFILE_PAUSE_KEY[] = L"CpuProfile";


/****************************************************************************/

/** This will set up the profiling driver for your profile
    \ingroup profiling
    \return The success of enabling the profiler
    \retval S_OK Success
    \retval S_FALSE The profiling has already been enabled
    \retval E_ACCESSDENIED The profiler was not available
    \retval E_UNEXPECTED There was an unexpected error
*/
CP_CTRL_API HRESULT fnEnableProfiling();


/** This will release profiling driver for other profiles
    \ingroup profiling
    \return The success of enabling the profiler
    \retval S_OK Success
    \retval S_FALSE The profiler doesn't need to be released or the profiler
        was stopped as part of the release
    \retval E_UNEXPECTED There was an unexpected error
*/
CP_CTRL_API HRESULT fnReleaseProfiling();


/** This will get the current version of the driver.
    Windows : This will return the caprof driver version.
    Linux   : This will return the version of Linux kernel (TBD)

    \note The typical version is shown as major.minor.build.

    \ingroup profiling
    @param[out] pMajor The major version number
    @param[out] pMinor The minor version number
    @param[out] pBuild The build version number
    \return The success of getting the driver version.
    \retval S_OK Success
    \retval E_INVALIDARG any of the arguments were NULL
    \retval E_ACCESSDENIED The profiler was not available
    \retval E_UNEXPECTED There was an unexpected error
*/
CP_CTRL_API HRESULT fnGetDriverVersion(
    /*out*/ unsigned int* pMajor,
    /*out*/ unsigned int* pMinor,
    /*out*/ unsigned int* pBuild);


/** This function allows you to build custom event configurations.
    It will check for the validity of the event select.  The performance event
    will be in the "Performance Event Select Register" format as defined in the
    relevant BKDG.

    \note Both guestOnlyEvents and hostOnlyEvents cannot be true.  Their use is
    reserved for a future version of the API.

    \ingroup profiling
    @param[in] eventSelect The selected event to profile.  These can be
        obtained from the CodeAnalyst application or the BKDG.
    @param[in] unitMask The mask of values appropriate to the eventSelect.  If
    a mask is available, at least one bit must be on.
    @param[in] edgeDetect each low-to-high transition of the event signal counts as
    an event, rather than the default level detection during clock cycles.
    @param[in] usrEvents user events will be counted
    @param[in] osEvents os events will be counted
    @param[in] guestOnlyEvents Rerserved for a future version.  On a vitualized
    system, only guest events will be counted
    @param[in] hostOnlyEvents Rerserved for a future version.  On a vitualized
    system, only host events will be counted
    @param[in] countingEvent true to use this performance event for \ref counting,
        false to use the event for \ref sampling
    @param[out] pPerformanceEvent The generated performance event
    \return The success of creating the performance event
    \retval S_OK Success
    \retval E_INVALIDARG One or more of the arguments were invalid
*/
CP_CTRL_API HRESULT fnMakeProfileEvent(
    /*in*/ unsigned int eventSelect,
    /*in*/ unsigned int unitMask,
    /*in*/ bool edgeDetect,
    /*in*/ bool usrEvents,
    /*in*/ bool osEvents,
    /*in*/ bool guestOnlyEvents,
    /*in*/ bool hostOnlyEvents,
    /*in*/ bool countingEvent,
    /*out*/ gtUInt64* pPerformanceEvent);


/** \struct EventConfiguration
    \brief All data needed for a single performance event.
    \ingroup profiling

    This is used to configure your profile.  It is used in the \ref
    fnSetCountingEvent, \ref fnSetCountingConfiguration, and \ref
    fnSetEventConfiguration.

    For counting, the value passed in is the starting count.

    For sampling, the value passed in is the event count to accumulate before
    generating a sample or the sample period. When doing sampling, this can
    drastically affect the size of the data file for common events.  Having too
    low of a sample period may cause the node to crash.  You can experiment
    with the period to get the amount of data that is most helpful to you.

    For the following events, we recommend the minimum period:
    0x76: 1000000
    0x40: 50000
    0xc0: 50000
    0xd1: 10000
    all others: 3000
*/
typedef struct EventConfiguration
{
    EventConfiguration()
    {
        performanceEvent = 0;
        value = 0;
        eventCounter = 0;
    };
    /** Performance event as created by \ref fnMakeProfileEvent*/
    gtUInt64 performanceEvent;
    /** Sample period for sampling or starting count for counting */
    gtUInt64 value;
    /** What event counter the event should use */
    unsigned int eventCounter;
}*  PEventConfiguration;


/** This enum represents the type of peformance monitoring counters (PMC).
    \ingroup profiling
*/
typedef enum
{
    PMC_CORE, /**< Core performance counter */
    PMC_NORTHBRIDGE, /**< Northbridge performance counter */
    PMC_L2I, /**< L2I peformance counter */
} PmcType;


/** This function retrieves the number of event counters per core by specified type.
    This affects the \ref counting in that if more events are counted than
    there are event counters available, then multiplexing is invoked.
    Multiplexing will proportionally reduce the samples and counts by the
    number of event sets that are necessary.
    \ingroup profiling
    @param[out] pEventCounterCount The number of event counters
    @param[in] type This specifies the type of counter to query.
    \return The success of getting the number of event counters
    \retval S_OK Success
    \retval E_INVALIDARG the argument was not a valid pointer
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnGetEventCounters(
    /*out*/ unsigned int* pEventCounterCount,
    /*in*/  PmcType type = PMC_CORE);


/** This function retrieves the availability of event counters per core by specified type.
    This affects the \ref counting in that if more events are counted than
    there are event counters available, then multiplexing is invoked.
    Multiplexing will proportionally reduce the samples and counts by the
    number of events per counter that are necessary.
    \ingroup profiling
    @param[in] type This specifies the type of counter to query.
    @param[in] performanceEvent The performance event
    @param[out] pAvailabilityMask The mask of available counters, where counter
    x is available if 1 << x is set
    \return The success of getting the availability of event counters
    \retval S_OK Success
    \retval E_INVALIDARG performanceEvent was not a valid event or
    pAvailabilityMask was not a valid pointer
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnGetEventCounterAvailability(
    /*in*/ gtUInt64 performanceEvent,
    /*out*/ unsigned int* pAvailabilityMask,
    /*in*/  PmcType type = PMC_CORE);


/** This enum represents the current state of the profiler.
    \ingroup profiling
*/
typedef enum
{
    ProfilingUnavailable, /**< The profiler is not available */
    ProfilingStopped,     /**< The profiler is not currently profiling */
    Profiling,            /**< The profiler is currently profiling */
    ProfilingPaused,      /**< The profiler is currently paused */
    ProfilingAborted      /**< The profiler encountered an error and aborted the profile*/
} ProfileState;


/** This will report on the current state of the profiler.

    \ingroup profiling
    @param[out] pProfileState This value will be updated with the current
    state of the profiler
    \return The success of getting the profile state
    \retval S_OK Success
    \retval E_INVALIDARG the argument was not a valid pointer
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnGetProfilerState(
    /*out*/ ProfileState* pProfileState);


/** If profiling is initialized and configured, this will start the profiler.

    \note The configurations cannot be changed while the profiler is profiling.

    \ingroup profiling
    @param[in] startPaused Should the profiler start in the \ref
    ProfilingPaused state
    @param[in] pauseKey If you want to pause the profile from another
    application, provide a string here, then use the same string with \ref
    fnPauseProfiling and \ref fnResumeProfiling to control the profile.  This
    parameter is optionaly, so can provide NULL.
    @param[out] pProfileState This optional value will be updated with the
    current state of the profiler
    \return The success of starting the profile
    \retval S_OK Success
    \retval E_ACCESSDENIED the profiler was not in the \ref ProfilingStopped
    state or if the profile was not enabled with \ref fnEnableProfiling
    \retval E_PENDING the output file was not set with \ref
    fnSetProfileOutputFile for a sampling profile or no configuration was set
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnStartProfiling(
    /*in*/ bool startPaused,
    /*in*/ bool pauseIndefinite,
    /*in*/ const wchar_t* pauseKey,
    /*out*/ ProfileState* pProfileState);


/** \typedef CaProfileCtrlKey
    \brief A key to identify which profile to pause or resume.
    \ingroup profiling

    The key is necessary for pausing or resuming sampling from a separate
    process via \ref fnPauseProfiling and \ref fnResumeProfiling.  The key is
    discovered with \ref fnGetProfileControlKey
*/
typedef void* CaProfileCtrlKey;


/** This will provide the profile control key for the pauseKey.  The profile
    control key is used for /ref fnPauseProfiling and /ref fnResumeProfiling

    \ingroup profiling
    @param[in] pauseKey If you want to pause a profile from another
    application, provide the same string as was provided to \ref
    fnStartProfiling to control the profile.  The pause key used by the
    CodeAnalyst Gui and CaProfile profiles is \ref CPU_PROFILE_PAUSE_KEY.
    \return The profile control key
    \retval NULL There was no active profile with the pause key
*/
CP_CTRL_API CaProfileCtrlKey fnGetProfileControlKey(
    /*in*/ const wchar_t* pauseKey);


/** If the profiler is running, and the state is \ref Profiling, this will
    pause the profiler, cause the state to go to \ref ProfilingPaused, and if
    sampling, no data while paused will be recorded.

    \ingroup profiling
    @param[in] profileKey If you want to pause a profile from another
    application, provide the \ref CaProfileCtrlKey as was provided by \ref
    fnGetProfileControlKey to control the profile.  This parameter is optional, so
    you can provide NULL to control the current profiling instance.
    \return The success of pausing the profile
    \retval S_OK the profiler was successfully paused
    \retval S_FALSE if the profiler was already in the \ref ProfilingPaused
    state
    \retval E_ACCESSDENIED the profiler was not in the \ref Profiling state or
    the profile key was not applicable for any current profile
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnPauseProfiling(
    /*in*/ CaProfileCtrlKey profileKey);


/** If the profiler is running, and the state is \ref ProfilingPaused, this
    will pause the profiler, cause the state to go to \ref Profiling, and
    resume recording data if sampling.

    \ingroup profiling
    @param[in] profileKey If you want to pause a profile from another
    application, provide the \ref CaProfileCtrlKey as was provided by \ref
    fnGetProfileControlKey to control the profile.  This parameter is optional, so
    you can provide NULL to control the current profiling instance.
    \return The success of resuming the profile
    \retval S_OK the profiling was successfully resumed
    \retval S_FALSE if the profiler was already in the \ref Profiling state
    \retval E_ACCESSDENIED the profiler was not in the \ref ProfilingPaused
    state or the profile key was not applicable for any current profile
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnResumeProfiling(
    /*in*/ CaProfileCtrlKey profileKey);


/** If the profiler is running, this will stop the profiler, and cause the
    state to go to \ref ProfilingStopped.  If sampling, no further data will
    be written to the profile files.

    \ingroup profiling
    \return The success of stopping the profile
    \retval S_OK the profiling was successfully stopped
    \retval E_ACCESSDENIED the profiler was not running
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnStopProfiling();


/** This will fill the provided buffer with a string explaining the last
    profiler error returned.

    \ingroup profiling

    @param[in] size The maximum size of the buffer to fill.
    @param[out] pErrorString The buffer to fill with an error string
    \return The success of filling the error string buffer.
    \retval S_OK Success
    \retval E_INVALIDARG the buffer was not valid pointer or the size is 0
    \retval S_FALSE the buffer was of insufficient size, but was filled as
        much as possible
*/
CP_CTRL_API HRESULT fnGetLastProfileError(
    /*in*/ unsigned int size,
    /*out*/ wchar_t* pErrorString);


/****************************************************************************/

/** This function sets the performance event at the eventCounterIndex on the
    core.  When the profiling is started wth \ref fnStartProfiling, the count
    can be read with \ref fnGetEventCount.  Valid cores depend on the node.
    The number of event counters available for a node can be found with
    \ref fnGetEventCounters.  This counting mode is exclusive with the \ref
    fnSetCountingConfiguration mode.  Event counting is also exclusive with
    \ref fnSetEventConfiguration.

    \note The performanceEvent.value is the starting count of the event.

    \ingroup counting
    @param[in] core The core of the node on which the event will be counted
    @param[in] eventCounterIndex Which counter to use, base 0.
    For Linux, eventCounterIndex will not be used.

    @param[in] performanceEvent The performance event to count
    \return The success of setting the configuration
    \retval S_OK Success
    \retval E_PENDING The profiler is currently profiling
    \retval E_ACCESSDENIED if another configuration has been set, or
    the profile was not enabled with \ref fnEnableProfiling.  Reset with
    \ref fnClearConfigurations.
    \retval E_INVALIDARG if either the core or index are invalid
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnSetCountingEvent(
    /*in*/ unsigned int core,
    /*in*/ unsigned int eventCounterIndex,
    /*in*/ EventConfiguration performanceEvent);


/** This function reads the performance event at the eventCounterIndex on the
    core.  The performance event must be set with \ref fnSetCountingEvent.
    Valid cores depend on the node.  The number of event counters
    available can be found with \ref fnGetEventCounters.

    \note For Linux, eventCounterIndex will not be used.

    \ingroup counting
    @param[in] core The core of the node on which the event will be counted
    @param[in] eventCounterIndex Which counter to use, base 0.
    @param[out] pEventCount The count of the performance event
    \return The success of setting the configuration
    \retval S_OK Success
    \retval S_FALSE \ref fnSetCountingEvent was not used to set the requested
    event
    \retval E_ACCESSDENIED if the profile was not enabled with \ref
    fnEnableProfiling
    \retval E_FAIL \ref fnSetCountingEvent was not used to set any events
    \retval E_PENDING if the profile is not currently running
    \retval E_INVALIDARG if either the core or index are invalid,
    or pEventCount is NULL
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnGetEventCount(
    /*in*/ unsigned int core,
    /*in*/ unsigned int eventCounterIndex,
    /*out*/ gtUInt64* pEventCount);


/** This function sets the performance event configuration on the given core
    mask.  When the profiling is started wth \ref fnStartProfiling, the count
    can be read with \ref fnGetAllEventCounts. If the array is longer than the
    number of event counters available on your node, then the mutliplexing
    will be used. The number of event counters available can be found with
    \ref fnGetEventCounters.  This counting mode is exclusive with the
    \ref fnSetCountingEvent mode.  Event counting is also exclusive with
    \ref fnSetEventConfiguration.

    \note Each performanceEvent.value is the starting count of the event.

    \note This function is deprecated.  It is limited to upto 64 cores.

    \ingroup counting
    @param[in] pPerformanceEvents The array of performance events
    @param[in] count The number of performance events
    @param[in] cpuCoreMask The optional CPU mask to configure performance
        events on certain cpu cores. A cpuCoreMask of 0x1011 indicates to set
        events on core 0, 1 and 3
    \return The success of setting the configuration
    \retval S_OK Success
    \retval S_FALSE It was configured correctly, execpt the group preferrences
        could not be honored.
    \retval E_ACCESSDENIED if another configuration has been set, or
    the profile was not enabled with \ref fnEnableProfiling.  Reset with
    \ref fnClearConfigurations.
    \retval E_PENDING The profiler is currently profiling
    \retval E_INVALIDARG if there are no performance events in the array, or if
    there are more than 32 events in the array
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnSetCountingConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ gtUInt64 cpuCoreMask = -1);


/** This function sets the performance event configuration on the given core
    mask.  When the profiling is started wth \ref fnStartProfiling, the count
    can be read with \ref fnGetAllEventCounts. If the array is longer than the
    number of event counters available on your node, then the mutliplexing
    will be used. The number of event counters available can be found with
    \ref fnGetEventCounters.  This counting mode is exclusive with the
    \ref fnSetCountingEvent mode.  Event counting is also exclusive with
    \ref fnSetEventConfiguration.

    \note Each performanceEvent.value is the starting count of the event.

    \ingroup counting
    @param[in] pPerformanceEvents The array of performance events
    @param[in] count The number of performance events
    @param[in] pCpuCoreMask The optional array of CPU mask to configure performance
    events on certain cpu cores. A pCpuCoreMask of an array of size two
    can specify CPU mask upto 128 cores.  For example, array of {0x1011,0x1}
    indicates to set events on core 0, 1, 3 and 64. NULL  will set all cores
    @param[in] cpuCoreMaskSize The number of cpu core mask in the array pointed
    to by pCpuCoremask. 0 will set all cores.
    \return The success of setting the configuration
    \retval S_OK Success
    \retval S_FALSE It was configured correctly, execpt the group preferrences
    could not be honored.
    \retval E_ACCESSDENIED if another configuration has been set, or
    the profile was not enabled with \ref fnEnableProfiling.  Reset with
    \ref fnClearConfigurations.
    \retval E_PENDING The profiler is currently profiling
    \retval E_INVALIDARG if there are no performance events in the array, or if
    there are more than 32 events in the array
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnSetCountingConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ const gtUInt64* pCpuCoreMask = NULL,
    /*in*/ unsigned int cpuCoreMaskSize = 0);


/** This function gets the count of the counting performance events that were
    previously set with \ref fnSetCountingConfiguration for the given core.

    \ingroup counting
    @param[in] core The core of the node of which to read
    @param[out] pCount The number of counting event counters that were set with
        \ref fnSetCountingConfiguration.
    \return The success of getting the count
    \retval S_OK Success
    \retval E_ACCESSDENIED if the profile was not enabled with
        \ref fnEnableProfiling.
    \retval E_INVALIDARG if the core is not valid or pCount is NULL
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnGetCountingEventCount(
    /*in*/ unsigned int core,
    /*in*/ unsigned int* pCount);


/** This function retrieves the performance event counts from the profiler.  The
    performance events must be set with \ref fnSetCountingConfiguration before
    the profiling starts.  This will only be valid after the profiling has been
    started with \ref fnStartProfiling.

    \ingroup counting
    @param[in] core The core of the node of which to read
    @param[in] size The size of the array passed in to receive the count
        information
    @param[out] pCounts The array of counts in the same order given for the
        configuration
    \return The success of getting the count
    \retval S_OK Success
    \retval E_ACCESSDENIED if the profile was not enabled with \ref
    fnEnableProfiling.
    \retval E_FAIL fnSetCountingConfiguration was not used to set configuration
    \retval E_INVALIDARG if any the core is invalid, size is 0, or pCounts is
        NULL
    \retval E_OUTOFMEMORY if the size is insufficeint for all the counters.
        The array should be at least the size given by \ref
        fnGetCountingEventCount.
    \retval E_PENDING if the profile is not currently running
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnGetAllEventCounts(
    /*in*/ unsigned int core,
    /*in*/ unsigned int size,
    /*out*/ gtUInt64* pCounts);


/****************************************************************************/

/** This function sets the path to the sampling output file. This function must be
    called before a sampling profile is started.

    Windows : If the file does not have a ".prd" extension, it will be added.
    Linux   : If the file does not have a ".caperf" extension, it will be added.

    \note This API is deprecated.  Please use \ref fnSetProfileOutputDirectory

    \ingroup sampling
    @param[in] pFileName The path and filename of the sampling output file
    \return The success of setting the configuration
    \retval S_OK Success
    \retval E_PENDING The profiler is currently profiling
    \retval E_INVALIDARG if pFileName is NULL
    \retval E_ACCESSDENIED if the path does not exist or is not writable, or
    the profile was not enabled with \ref fnEnableProfiling
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnSetProfileOutputFile(
    /*in*/ const wchar_t* pFileName);


/** This function sets the path to the sampling output directory.
    This function must be called before a sampling profile is started.

    \ingroup sampling
    @param[in] pDirectoryName The path to profile output directory
    \return The success of setting the configuration
    \retval S_OK Success
    \retval E_PENDING The profiler is currently profiling
    \retval E_INVALIDARG if pDirectoryName is NULL
    \retval E_ACCESSDENIED if the path does not exist or is not writable, or
    the profile was not enabled with \ref fnEnableProfiling
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnSetProfileOutputDirectory(
    /*in*/ const wchar_t* pDirectoryName);


//TODO: This API should be deprecated and change the FILETIME to CPA_TIME
//      since FILETIME is Windows-specific data structure.
///** This function gets a time mark for the node on which the profile is being
//  sampled.  This mark will be synchronized with all cores and the data in a
//  profile.  The idea with getting time marks at certain times is that you
//  can look at the sampling profile data during an interval of interest
//  between two time marks when using the \ref CaProfileDataAccessAPI.h.
//
//  \ingroup sampling
//  @param[out] pTimeMark The time mark for the node
//  \return The success of getting the time mark
//  \retval S_OK Success
//  \retval E_INVALIDARG if pTimeMark is NULL
//  \retval E_UNEXPECTED an unexpected error occurred
//*/
//CP_CTRL_API HRESULT fnGetCurrentTimeMark (
//  /*out*/ FILETIME *pTimeMark);


/** This function gets a time mark for the node on which the profile is being
    sampled.  This mark will be synchronized with all cores and the data in a
    profile.  The idea with getting time marks at certain times is that you
    can look at the sampling profile data during an interval of interest
    between two time marks when using the \ref CaProfileDataAccessAPI.h.

    \ingroup sampling
    @param[out] pTimeMark The time mark for the node
    \return The success of getting the time mark
    \retval S_OK Success
    \retval E_INVALIDARG if pTimeMark is NULL
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnGetCurrentTimeMark(
    /*out*/ CPA_TIME* pTimeMark);


/** This function will configure the timer-based sampling of the profile.  This
    function should be called before \ref fnStartProfiling.  The minimum value
    for the sampling period is 100 uS.  The actual resolution used will be
    returned in puSPeriod.

    \note Calling this twice before a profile will overwrite the previous
    configuration.  The set configuration will be persistent until the profile
    is started or when /ref fnClearConfigurations is called.

    \note This function is deprecated.  It is limited to 64 cores.

    \ingroup sampling
    @param[in] cpuCoreMask The mask of cores on which to sample the timer, -1
    will set it on all cores.
    @param[in,out] puSPeriod The number of microseconds between samples
    \return The success of configuring the timer-based sampling
    \retval S_OK Success
    \retval E_INVALIDARG if puSPeriod is not a valid pointer
    \retval E_ACCESSDENIED if another configuration has been set, or
    the profile was not enabled with \ref fnEnableProfiling.  Reset with
    \ref fnClearConfigurations.
    \retval S_FALSE Succeeded, but the resolution was changed
    \retval E_PENDING The profiler is currently profiling
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnSetTimerConfiguration(
    /*in*/ gtUInt64 cpuCoreMask,
    /*in/out*/ unsigned int* puSPeriod);


#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    CP_CTRL_API HRESULT fnSetThreadProfileConfiguration(bool isCSS);
#endif

/** This function will configure the timer-based sampling of the profile.  This
    function should be called before \ref fnStartProfiling.  The minimum value
    for the sampling period is 100 uS.  The actual resolution used will be
    returned in puSPeriod.

    \note Calling this twice before a profile will overwrite the previous
    configuration.  The set configuration will be persistent until the profile
    is started or when /ref fnClearConfigurations is called.

    \ingroup sampling
    @param[in,out] puSPeriod The number of microseconds between samples
    @param[in] pCpuCoreMask The optional array of CPU mask to configure performance
    events on certain cpu cores. A pCpuCoreMask of an array of size two
    can specify CPU mask upto 128 cores.  For example, array of {0x1011,0x1}
    indicates to set events on core 0, 1, 3 and 64. NULL  will set all cores
    @param[in] cpuCoreMaskSize The number of cpu core mask in the array pointed
    to by pCpuCoremask. 0 will set all cores.
    \return The success of configuring the timer-based sampling
    \retval S_OK Success
    \retval E_INVALIDARG if puSPeriod is not a valid pointer
    \retval E_ACCESSDENIED if another configuration has been set, or
    the profile was not enabled with \ref fnEnableProfiling.  Reset with
    \ref fnClearConfigurations.
    \retval S_FALSE Succeeded, but the resolution was changed
    \retval E_PENDING The profiler is currently profiling
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnSetTimerConfiguration(
    /*in/out*/ unsigned int* puSPeriod,
    /*in*/ const gtUInt64* pCpuCoreMask = NULL,
    /*in*/ unsigned int cpuCoreMaskSize = 0);


/** This function will configure the event-based sampling of the profile.  This
    function should be called before \ref fnStartProfiling.  The profiler can
    allow more events than there are performance counters, because of
    multiplexing.  The profiler will distribute the events across the available
    counters.

    \note Due to out-of-order execution, some of the ip addresses may not
    be from the actual instruction that caused the event sample.  Calling this
    twice before a profile will overwrite the previous configuration if the
    cpuCoreMask overlap.  The set configuration will be persistent until the
    profile is started or when /ref fnClearConfigurations is called. Event
    sampling is also exclusive with \ref fnSetCountingConfiguration and
    \ref fnSetCountingEvent.

    \note Each performanceEvent.value is the sample period of the event.

    \note This function is deprecated.  It is limited to upto 64 cores.

    \ingroup sampling
    @param[in] pPerformanceEvents The array of performance events
    @param[in] count The count of events in the array
    @param[in] cpuCoreMask The mask of cores on which to set the events, -1
    will set it on all cores.
    \return The success of configuring the event-based sampling
    \retval S_OK Success
    \retval E_ACCESSDENIED if another configuration has been set.  Reset with
    \ref fnClearConfigurations.
    \retval E_PENDING The profiler is currently profiling
    \retval E_INVALIDARG if the events configurations are not correct, one
    event is in the array multiple times with the same unit mask, or there
    are more than the maximum events in the array (8 * \ref fnGetEventCounters)
    \retval E_OUTOFMEMORY If required memory wasn't available
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnSetEventConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ gtUInt64 cpuCoreMask = -1);


/** This function will configure the event-based sampling of the profile.  This
    function should be called before \ref fnStartProfiling.  The profiler can
    allow more events than there are performance counters, because of
    multiplexing.  The profiler will distribute the events across the available
    counters.

    \note Due to out-of-order execution, some of the ip addresses may not
    be from the actual instruction that caused the event sample.  Calling this
    twice before a profile will overwrite the previous configuration if the
    pCpuCoreMask overlap.  The set configuration will be persistent until the
    profile is started or when /ref fnClearConfigurations is called. Event
    sampling is also exclusive with \ref fnSetCountingConfiguration and
    \ref fnSetCountingEvent.

    \note Each performanceEvent.value is the sample period of the event.

    \ingroup sampling
    @param[in] pPerformanceEvents The array of performance events
    @param[in] count The count of events in the array
    @param[in] pCpuCoreMask The optional array of CPU mask to configure performance
    events on certain cpu cores. A pCpuCoreMask of an array of size two
    can specify CPU mask upto 128 cores.  For example, array of {0x1011,0x1}
    indicates to set events on core 0, 1, 3 and 64. NULL  will set all cores
    @param[in] cpuCoreMaskSize The number of cpu core mask in the array pointed
    to by pCpuCoremask. 0 will set all cores.
    \return The success of configuring the event-based sampling
    \retval S_OK Success
    \retval E_ACCESSDENIED if another configuration has been set.  Reset with
    \ref fnClearConfigurations.
    \retval E_PENDING The profiler is currently profiling
    \retval E_INVALIDARG if the events configurations are not correct, one
    event is in the array multiple times with the same unit mask, or there
    are more than the maximum events in the array (8 * \ref fnGetEventCounters)
    \retval E_OUTOFMEMORY If required memory wasn't available
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnSetEventConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ const gtUInt64* pCpuCoreMask = NULL,
    /*in*/ unsigned int cpuCoreMaskSize = 0);


/** This function will get whether the node supports instruction-based
    sampling.

    \ingroup sampling
    @param[out] pIsIbsAvailable true if IBS is available on this node
    \retval S_OK Success
    \retval E_INVALIDARG if pIsIbsAvailable is NULL
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnGetIbsAvailable(
    /*out*/ bool* pIsIbsAvailable);


/** This function will configure the instruction-based sampling of the profile.
    This function should be called before \ref fnStartProfiling.
    Instruction-based sampling will 'tag' an instruction each time the period
    ends.  Different information is available from fetch tagged instructions
    than op tagged.  After a tagged instruction retires, information about its
    execution is collected as the sample.

    If either fetch samples or op samples are not desired, setting the period
    to 0 will disable that type of instruction-based sampling.  Otherwise, a
    valid period falls in the range of 50000 - 1048575.

    \note Calling this twice before a profile will overwrite the previous
    configuration.  The set configuration will be persistent until the
    profile is started or when /ref fnClearConfigurations is called.

    \note This function is deprecated.  It is limited to upto 64 cores.

    \ingroup sampling
    @param[in] cpuCoreMask The mask of cores on which to sample, -1 will set
    it on all cores.
    @param[in] fetchPeriod the period of fetched instructions between samples
    @param[in] opPeriod the period of micro-op instructions between samples
    @param[in] randomizeFetchSamples this will prevent loops from synchronizing
    with the sampling
    @param[in] useDispatchOps uses dispatched ops for the period rather than
    clock cycles
    \return The success of configuring the instruction-based sampling
    \retval S_OK Success
    \retval S_FALSE the dispatch ops feature was not available, but the profile
    is ready to go with a clock cycle opPeriod
    \retval E_PENDING The profiler is currently profiling
    \retval E_FAIL if the both fetchPeriod and opPeriod are 0
    \retval E_INVALIDARG if either period is out of the valid range and not 0
    or the cpuCoreMask is invalid
    \retval E_ACCESSDENIED if instruction-based profiling is not available, or
    if another configuration has been set.
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnSetIbsConfiguration(
    /*in*/ gtUInt64 cpuCoreMask,
    /*in*/ unsigned long fetchPeriod,
    /*in*/ unsigned long opPeriod,
    /*in*/ bool randomizeFetchSamples = true,
    /*in*/ bool useDispatchOps = false);


/** This function will configure the instruction-based sampling of the profile.
    This function should be called before \ref fnStartProfiling.
    Instruction-based sampling will 'tag' an instruction each time the period
    ends.  Different information is available from fetch tagged instructions
    than op tagged.  After a tagged instruction retires, information about its
    execution is collected as the sample.

    If either fetch samples or op samples are not desired, setting the period
    to 0 will disable that type of instruction-based sampling.  Otherwise, a
    valid period falls in the range of 50000 - 1048575.

    \note Calling this twice before a profile will overwrite the previous
    configuration.  The set configuration will be persistent until the
    profile is started or when /ref fnClearConfigurations is called.

    \ingroup sampling
    @param[in] fetchPeriod the period of fetched instructions between samples
    @param[in] opPeriod the period of micro-op instructions between samples
    @param[in] randomizeFetchSamples this will prevent loops from synchronizing
    with the sampling
    @param[in] useDispatchOps uses dispatched ops for the period rather than
    clock cycles
    @param[in] pCpuCoreMask The optional array of CPU mask to configure performance
    events on certain cpu cores. A pCpuCoreMask of an array of size two
    can specify CPU mask upto 128 cores.  For example, array of {0x1011,0x1}
    indicates to set events on core 0, 1, 3 and 64. NULL  will set all cores
    @param[in] cpuCoreMaskSize The number of cpu core mask in the array pointed
    to by pCpuCoremask. 0 will set all cores.
    \return The success of configuring the instruction-based sampling
    \retval S_OK Success
    \retval S_FALSE the dispatch ops feature was not available, but the profile
    is ready to go with a clock cycle opPeriod
    \retval E_PENDING The profiler is currently profiling
    \retval E_FAIL if the both fetchPeriod and opPeriod are 0
    \retval E_INVALIDARG if either period is out of the valid range and not 0
    or the pCpuCoreMask is invalid
    \retval E_ACCESSDENIED if instruction-based profiling is not available, or
    if another configuration has been set.
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnSetIbsConfiguration(
    /*in*/ unsigned long fetchPeriod,
    /*in*/ unsigned long opPeriod,
    /*in*/ bool randomizeFetchSamples = true,
    /*in*/ bool useDispatchOps = false,
    /*in*/ const gtUInt64* pCpuCoreMask = NULL,
    /*in*/ unsigned int cpuCoreMaskSize = 0);


/** This allows you to choose to only obtain profile data on the specified
    processes.  This function should be called before \ref fnStartProfiling.
    It will allow you to limit the overhead of data.  If the filter is not
    set, all processes will be sampled.

    The maximum number of process ids which can be filtered at one time is 256.

    \ingroup sampling
    @param[in] pProcessIds the processes to take profile data from
    @param[in] count the number of processes
    @param[in] ignoreChildren this will ignore the children of this process.
    By default, children of filtered processes will be included.
    \return The success of configuring the process id filters
    \retval S_OK Success
    \retval E_INVALIDARG If the arguement is invalid, empty, or has too many
    process ids
    \retval E_PENDING The profiler is currently profiling
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnSetFilterProcesses(
    /*in*/ unsigned int* pProcessIds,
    /*in*/ unsigned int count,
    /*in*/ bool systemWideProfile,
    /*in*/ bool ignoreChildren = false);


/** This is a CodeAnalyst internal function for the first version of the API.
    It allows you to flag one process for call-stack sampling for one profile.
    This function should be called before \ref fnStartProfiling. The unwind
    level is the depth in the call stack which will be attempted.  The sample
    period is the count of how many samples to be taken between call-stack
    sampling. It will allow you to limit the overhead and call-stack data.

    \note This will be in the Sampling group, when the call stack data is
    available to everyone via the \ref CaProfileDataAccessAPI.h.

    \note The system kernel processes are not valid for call stack sampling.

    \ingroup sampling
    @param[in] pProcessIds the array of processes to sample the call-stack from
    @param[in] count the count of processes in the array
    @param[in] unwindLevel how deep to unwind the call-stack
    @param[in] samplePeriod how many profile samples between call-stack samples
    @param[in] scope the sampling scope enum for user space and kernel space
    @param[in] captureVirtualStack whether to collect virtual stack or not
    \return The success of configuring the call-stack sampling
    \retval S_OK Success
    \retval E_PENDING The profiler is currently profiling
    \retval E_INVALIDARG Either the unwindLevel or samplePeriod were out of
    bounds, or the processIds were invalid.  The maximum unwindLevel is 392.
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnSetCallStackSampling(
    /*in*/ unsigned int* pProcessIds,
    /*in*/ unsigned int count,
    /*in*/ unsigned int unwindLevel,
    /*in*/ unsigned int samplePeriod,
    /*in*/ CpuProfileCssScope scope,
    /*in*/ bool captureVirtualStack);


/** This allows you to clear the currently set configuration(s).  \ref
    fnStartProfiling will again not be available until a configuration is set
    and a profile output file is set with \ref fnSetProfileOutputFile.

    \ingroup sampling
    \return The success of clearing the configurations
    \retval S_OK Success
    \retval S_FALSE if there was no configuration set
    \retval E_PENDING The profiler is currently profiling
    \retval E_ACCESSDENIED The profile was not enabled with \ref
    fnEnableProfiling
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnClearConfigurations();


/** This allows you to enable CPU utilization monitoring.

    \note This is currently not supported on Linux.  Calling this function
    will always return S_FALSE

    \ingroup sampling
    @param[in] utilization_interval in ms .
    @param[in] monitorFlag: 0x1 to monitor CPU utilization, 0x2 to monitor
        memory consumption.
    \return The success of enable the CPU utilization
    \retval S_OK Success
    \retval S_FALSE if failed
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnEnableCPUUtilization(
    /*in*/ unsigned int utilization_interval,
    /*in*/ unsigned int monitorFlag = 0x3);

/** This allows you to specify process id for CPU utilization monitoring.

    \note This is currently not supported on Linux.  Calling this function
    will always return S_FALSE

    \ingroup sampling
    @param[in] pid the process to be monitored for CPU Utilization.
    \return The success of adding process id into cpu utilization.
    \retval S_OK Success
    \retval S_FALSE if failed
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnAddCPUUtilizationProcessId(
    /* in */ unsigned pid);

/** This function will write the RI file with the provided data

    \ingroup sampling
    @param[in] pRIFilePath The file name to write to
    @param[in] pRunInfo The data need to be written into the file
    \return The success of writing the data into the RI file
    \retval S_OK Success
    \retval E_INVALIDARG if pRIFilePath or pRunInfo are not a valid pointer
    \retVal E_FAIL if failed to create or write the file pRIFilePath
    \retval E_UNEXPECTED an unexpected error occurred
*/
CP_CTRL_API HRESULT fnWriteRunInfo(
    /* in */ const wchar_t* pRIFilePath,
    /* in */ const RunInfo* pRunInfo);

/** This function will get whether predefined profiles are supported or not.

\ingroup profiling
@param[out] isPredefinedProfilesAvailable true if predefined profiles supported
\retval S_OK Success
\retval S_FALSE if failed
*/
CP_CTRL_API HRESULT fnGetPredefinedProfilesAvailable(/*out*/ bool& isPredefinedProfilesAvailable);

/*@}*/
#endif  // _CPUPROFILECONTROL_H_
