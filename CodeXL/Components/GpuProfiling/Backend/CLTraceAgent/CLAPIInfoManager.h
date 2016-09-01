//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages pointers to each saved API object for API tracing.
//==============================================================================

#ifndef _CL_API_INFO_MANAGER_H_
#define _CL_API_INFO_MANAGER_H_

/// \defgroup CLAPIInfoManager CLAPIInfoManager
/// This module maintains pointers to each saved API object.
///
/// \ingroup CLTraceAgent
// @{

#include <CL/opencl.h>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "APIInfoManagerBase.h"
#include "CLAPIDefs.h"
#include "CLEnqueueAPIDefs.h"
#include "../CLCommon/CLFunctionDefs.h"
#include "../Common/GlobalSettings.h"
#include "../Common/ProfilerTimer.h"

typedef std::map<osThreadId, CLAPI_clGetEventInfo*> PreviousGEIMap;
typedef std::pair<osThreadId, CLAPI_clGetEventInfo*> PreviousGEIMapPair;
typedef std::map<const cl_command_queue, std::list<const CLAPI_clCreateCommandQueueBase*> > CLCommandQueueMap;
typedef std::pair<const cl_command_queue, std::list<const CLAPI_clCreateCommandQueueBase*> > CLCommandQueueMapPair;
typedef std::map<const cl_context, std::list<const CLAPI_clCreateContextBase*> > CLContextMap;
typedef std::pair<const cl_context, std::list<const CLAPI_clCreateContextBase*> > CLContextMapPair;
typedef std::map<const cl_kernel, std::string> CLKernelMap;
typedef std::pair<const cl_kernel, std::string> CLKernelMapPair;
typedef std::vector<cl_kernel> EnqueuedTaskList;

/// Handle the response on the end of the timer
/// \param timerType type of the ending timer for which response have to be executed
void CLAPITraceAgentTimerEndResponse(ProfilerTimerType timerType);

void TimerThread(void* param);

//------------------------------------------------------------------------------------
/// CLAPIInfoManager manages captured api calls
//------------------------------------------------------------------------------------
class CLAPIInfoManager : public APIInfoManagerBase, public TSingleton<CLAPIInfoManager>
{
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<CLAPIInfoManager>;
    FRIENDTESTCASE(CLAPIInfoManager);
public:
    /// Destructor
    ~CLAPIInfoManager();

    /// Add APIInfo to the list
    /// \param en APIInfo entry
    void AddAPIInfoEntry(APIBase* en);

    /// Release all entries in m_CLAPIInfoMap
    void Release();

    /// Save trace data to tmp file
    /// \param bForceFlush Force to write all data out no matter it's ready or not - used in Detach() only
    void FlushTraceData(bool bForceFlush = false);

    /// Is API in API filter list
    /// \param type cl function enum
    /// \return True if API is in filter list
    bool IsInFilterList(CL_FUNC_TYPE type);

    /// Check if the specified API should be intercepted
    /// \param type HSA function type
    /// \return true if API should be intercepted
    bool ShouldIntercept(const char* strAPIName);

    /// Add cl_command_queue, CLAPI_clCreateCommandQueue* pair to m_CLCommandQueueMap
    /// \param cmdQueue OpenCL command queue object
    /// \param cmdQueueAPIObj CLAPI_clCreateCommandQueueBase pointer
    void AddToCommandQueueMap(const cl_command_queue cmdQueue, const CLAPI_clCreateCommandQueueBase* cmdQueueAPIObj);

    /// Add cl_context, CLAPI_clCreateContextBase* pair to m_CLContextMap
    /// \param context OpenCL context
    /// \param contextAPIObj CLAPI_clCreateContextBase pointer
    void AddToContextMap(const cl_context context, const CLAPI_clCreateContextBase* contextAPIObj);

    /// Get clCreateCommandQueueAPIObj from OpenCL command queue object
    /// \param cmdQueue OpenCL command queue object
    /// \return CLAPI_clCreateCommandQueueBase pointer
    const CLAPI_clCreateCommandQueueBase* GetCreateCommandQueueAPIObj(const cl_command_queue cmdQueue);

    /// Get clCreateContextAPIObj from OpenCL context object
    /// \param context OpenCL context object
    /// \return CLAPI_clCreateContextBase pointer
    const CLAPI_clCreateContextBase* GetCreateContextAPIObj(const cl_context context);

    /// Add cl_kernel, name string pair to m_CLCommandQueueMap
    /// \param kernel OpenCL kernel object
    /// \param szName kernel name string
    void AddToKernelMap(const cl_kernel kernel, const char* szName);

    /// Get kernel name string from cl_kernel object
    /// \param kernel OpenCL kernel object
    /// \return kernel name string
    std::string& GetKernelName(const cl_kernel kernel);

    /// Save to Atp File
    void SaveToOutputFile();

    /// Query CPU timestamps as well as user PMCs before API call
    /// \param[inout] pEntry API object
    /// \return CPU timestamps
    ULONGLONG GetTimeNanosStart(CLAPIBase* pEntry = NULL);

    /// Query CPU timestamps as well as user PMCs after API call
    /// \param[inout] pEntry API object
    /// \return CPU timestamps
    ULONGLONG GetTimeNanosEnd(CLAPIBase* pEntry = NULL);

    /// Return true if max number of APIs are traced.
    /// \return true if max number of APIs are traced.
    bool IsCapReached() const
    {
        return m_uiLineNum >= GlobalSettings::GetInstance()->m_params.m_uiMaxNumOfAPICalls;
    }

    /// Add an enqueued task's kernel to the list of kernels
    /// \param kernel the kernel object for an enqueued task
    void AddEnqueuedTask(const cl_kernel kernel);

    /// Check and remove an kernel from the list of kernels
    /// \param kernel the kernel to check and remove from the list
    /// \return true if the specified kernel is in the list, false otherwise
    bool CheckEnqueuedTask(const cl_kernel kernel);

    /// Enables or Disables the profiler delay
    /// \param doEnable true for enable and false for disable
    /// \param delayInSeconds seconds to delay the profiler
    void EnableProfileDelayStart(bool doEnable, unsigned int delayInSeconds = 0);

    /// Enables or Disables the profiler duration
    /// \param doEnable true for enable and false for disable
    /// \param durationInSeconds profiler duration in seconds
    void EnableProfileDuration(bool doEnable, unsigned int durationInSeconds = 0);

    /// Indicates whether profiler should run after delay or not
    /// \param delayInSeconds to return the amount by which profile set to be delayed
    /// \returns true if delay is enabled
    bool IsProfilerDelayEnabled(unsigned int& delayInSeconds);

    /// Indicates whether profiler should run only for set duration or not
    /// \param durationInSeconds to return the amount by which profile set to run
    /// \returns true if duration of the profiler is enabled
    bool IsProfilerDurationEnabled(unsigned int& durationInSeconds);

    /// Assigns the call back function
    /// \param timerType type of the timer
    /// \param timerEndHandler call back function pointer
    void SetTimerFinishHandler(ProfilerTimerType timerType, TimerEndHandler timerEndHandler);

    /// Creates the Profiler Timer
    /// \param timerType timer type of the starting timer
    /// \param timeIntervalInSeconds profiler duration or profiler delay in seconds
    void CreateTimer(ProfilerTimerType timerType, unsigned int timeIntervalInSeconds);

    /// Starts the timer
    /// \param timerType Type of the the timer
    void startTimer(ProfilerTimerType timerType);

protected:
    /// Add the specified api to the list of APIs to filter
    /// \param strAPIName the name of the API to add to the filter
    void AddAPIToFilter(const std::string& strAPIName);

private:

    /// Constructor
    CLAPIInfoManager();

    /// Update m_ullStart, m_ullEnd from m_APITimeInfoMap
    void Update();

    /// Disable copy constructor
    /// \param obj obj
    CLAPIInfoManager(const CLAPIInfoManager& obj);

    /// Disable assignment operator
    /// \param obj obj
    /// \return lhs
    CLAPIInfoManager& operator = (const CLAPIInfoManager& obj);

private:
    unsigned int            m_uiLineNum;                         ///< number of lines output to file
    PreviousGEIMap          m_previousGEIMap;                    ///< stl map that contains the previous CLAPI_clGetEventInfo instance for each thread
    CLCommandQueueMap       m_clCommandQueueMap;                 ///< stl map that maps from cl_command_queue to CLAPI_clCreateCommandQueue*
    CLContextMap            m_clContextMap;                      ///< stl map that maps from cl_context to CLAPI_clCreateContextBase*
    CLKernelMap             m_clKernelMap;                       ///< stl map that maps from cl_kernel to string
    EnqueuedTaskList        m_enqueuedTasks;                     ///< stl vector containing the clEnqueueTask apis
    AMDTMutex               m_mtxPreviousGEI;                    ///< mutex used to lock access to m_PreviousGEIMap
    AMDTMutex               m_mtxEnqueuedTask;                   ///< mutex used to lock access to m_enqueuedTasks
    std::set<CL_FUNC_TYPE>  m_filterAPIs;                        ///< OpenCL APIs that are disabled in the trace, if API is not in m_MustInterceptAPIs list, the API will not be intercept,
    ///< otherwise, API is intercepted but it won't show up in trace file.
    std::set<CL_FUNC_TYPE>  m_mustInterceptAPIs;                 ///< Some of the APIs like clCreateContext, clCreateCommandQueue and etc are not able to be disabled.
    std::list<ITraceEntry*> m_mustInterceptAPIList;              ///< List containing the must-intercept API objects, if they are not actually traced.  If not tracing the APIs, we need to store the must-intercept API objects somewhere
    bool                    m_bDelayStartEnabled;                ///< flag indicating whether or not the profiler should start with delay or not
    bool                    m_bProfilerDurationEnabled;          ///< Flag indiacating whether profiler should only run for certain duration
    unsigned int            m_secondsToDelay;                    ///< Seconds to delay for profiler to start
    unsigned int            m_profilerShouldRunForSeconds;       ///< Duration in seconds for which Profiler should run
    ProfilerTimer*          m_delayTimer;                        ///< timer for handling delay timer for the profile agent
    ProfilerTimer*          m_durationTimer;                     ///< timer for handling duration timer for the profile agent
};

// @}

#endif //_CL_API_INFO_MANAGER_H_
