//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The interface for the GPA helper class.
//==============================================================================

#ifndef _CL_GPAPROFILER_H_
#define _CL_GPAPROFILER_H_

#include <map>
#include <vector>
#include <string>
#include <cmath>
#include <set>

#include <CL/opencl.h>

// ADL headers
#include <ADLUtil.h>
#include <customer/oem_structures.h>

#include "../Common/GPAUtils.h"
#include "../Common/Defs.h"
#include "../Common/FileUtils.h"
#include "../Common/LocaleSetting.h"
#include "../Common/Logger.h"
#include "../CLCommon/CLPlatformInfo.h"
#include "CLContextManager.h"
#include "CLKernelAssembly.h"
#include "KernelStats.h"
#include "CLDeferredKernel.h"
#include "CLUserEvent.h"
#include "DeviceInfoUtils.h"
#include "../Common/KernelProfileResultManager.h"
#include "../Common/ProfilerTimer.h"


/// Handle the response on the end of the timer
/// \param timerType type of the ending timer for which response have to be executed
void CLGPAProfilerTimerEndResponse(ProfilerTimerType timerType);


/// \addtogroup CLProfileAgent
// @{
class CLUserEvent;

typedef std::vector<CLUserEvent*> UserEventList;

//------------------------------------------------------------------------------------
/// This class encapsulates GPUPerfAPI as used by the OpenCL profiler
//------------------------------------------------------------------------------------
class CLGPAProfiler
{
public:
    /// Constructor
    CLGPAProfiler();
    ~CLGPAProfiler();

    /// Accessor to the GPA Loader
    GPUPerfAPILoader& GetGPALoader()
    {
        //return m_GPALoader;
        return m_GPAUtils.GetGPALoader();
    }

    /// Initialize GPA with a context (command queue)
    /// \param commandQueue the command queue for the context
    bool Open(cl_command_queue commandQueue);

    /// Close the current context in GPA
    bool Close();

    /// Enable GPA counters
    /// \param commandQueue the command queue for the context
    /// \return true if succesfull, false otherwise (counters have been enabled)
    bool EnableCounters(cl_command_queue commandQueue);

    /// Profile an OpenCL kernel call with the full set of public counters
    bool FullProfile(
        cl_command_queue commandQueue,
        cl_kernel        kernel,
        cl_uint          uWorkDim,
        const size_t*    pGlobalWorkOffset,
        const size_t*    pGlobalWorkSize,
        const size_t*    pLocalWorkSize,
        cl_uint          uEventWaitList,
        const cl_event*  pEventWaitList,
        cl_event*        pEvent,
        cl_int&          nResultOut,
        gpa_uint32&      uSessionIDOut,
        double&          dKernelTimeOut);

    /// Check whether we need to generate the kernel source, IL or ISA (based on the kernel name and
    /// the pointer value).  Generate those files if they haven't been generated yet.
    /// \param commandQueue    the CL command queue handle
    /// \param kernel          the CL kernel handle
    /// \param strKernelName   the kernel function name
    /// \param strKernelHandle the unique string identifying a kernel
    /// \return true if successful, false otherwise
    bool GenerateKernelAssembly(const cl_command_queue& commandQueue,
                                const cl_kernel&        kernel,
                                const std::string&      strKernelName,
                                const std::string&      strKernelHandle);

    /// Get the kernel information from kernel assembly.
    /// \return the kernel information from the kernel assembly
    const KernelInfo& GetKernelInfoFromKernelAssembly(std::string& strKernelName) const;

    /// Initialize PMC profiler
    /// \param params Parameters pass from CodeXLGpuProfiler
    /// \param strErrorOut Error message if init failed
    /// \return true if succeeded
    bool Init(const Parameters& params, std::string& strErrorOut);

    /// Output the counter result for the specified session to a csv file
    /// \param uSessionID  the GPA profiling session ID
    /// \param kernelStats contains the kernel statistics
    /// \return false if GPA is not loaded, true if successful
    bool DumpSession(gpa_uint32 uSessionID, const KernelStats& kernelStats);

    /// A helper function to print out the statistics in the kernel stats' structure.
    /// \param kernelStats  the kernel stats' structure
    void DumpKernelStats(const KernelStats& kernelStats);

    /// Accessor to whether or not GPA has been loaded
    /// \return true if GPA dll has been loaded; false otherwise
    bool Loaded()
    {
        return m_GPAUtils.Loaded();
    }

    /// unloads the currently loaded GPA dll
    void Unload()
    {
        m_GPAUtils.Unload();
    }

    /// Converts the status into a string that can be used in error messages
    /// \param status the status to convert
    /// \return a string version of the status
    std::string GetStatusString(GPA_Status status) const;

    /// Save the context to the contextManager
    /// \param context the CL context to be saved
    /// \return true if successful, false otherwise (context had been added)
    bool AddContext(const cl_context context);

    /// Remove the context from the contextManager
    /// \param context the CL context to be saved
    /// \return true if successful, false otherwise
    bool RemoveContext(const cl_context context);

    /// Save kernel to the contextManager
    /// \param kernel the CL kernel to be saved
    /// \return true is successful, false otherwise
    bool AddKernel(const cl_kernel kernel);

    /// Add user event
    /// \param userEvent user event object
    void AddUserEvent(cl_event userEvent);

    /// Remove kernel from the contextManager
    /// \param kernel the CL kernel to be removed
    /// \return true is successful, false otherwise
    bool RemoveKernel(const cl_kernel kernel);

    /// Save a kernel arg to the contextManager if the arg value is of type cl_buffer created with CL_MEM_READ_WRITE flag
    /// \param kernel    the CL kernel
    /// \param argIdx argument binding index
    /// \param pArgValue the argument to the kernel
    /// \return true if the kernel arg is saved successfully, false otherwise
    bool AddKernelArg(const cl_kernel kernel, cl_uint argIdx, const void* pArgValue);

    /// Flag the specified kernel arg as an SVM kernel arg
    /// \param kernel    the CL kernel
    /// \param argIdx the index of the kernel arg which is an SVM pointer
    /// \return true if the kernel arg is saved successfully, false otherwise
    bool AddKernelArgSVMPointer(const cl_kernel kernel, cl_uint argIdx);

    /// Check whether the kernel uses SVM Pointer args
    /// \param kernel    the CL kernel
    /// \return true if the kernel uses SVM Pointer kernel args
    bool HasKernelArgSVMPointer(const cl_kernel kernel);

    /// Flag the specified kernel arg as a pipe arg
    /// \param kernel    the CL kernel
    /// \param argIdx the index of the kernel arg which is a pipe
    /// \return true if the kernel arg is saved successfully, false otherwise
    bool AddKernelArgPipe(const cl_kernel kernel, cl_uint argIdx);

    /// Check whether the kernel uses pipe args
    /// \param kernel    the CL kernel
    /// \return true if the kernel uses pipe kernel args
    bool HasKernelArgPipe(const cl_kernel kernel);

    /// Save buffer to the kernel list
    /// \param context    the CL context
    /// \param buffer     the CL buffer to be saved
    /// \param flags      the flags parameter when the buffer was created
    /// \param bufferSize the size of the CL buffer
    /// \param pHost      the host pointer (may be NULL) for the buffer when it was created
    /// \return true if successful, false otherwise
    bool AddBuffer(const cl_context& context,
                   const cl_mem&     buffer,
                   cl_mem_flags      flags,
                   size_t            bufferSize,
                   void*             pHost);

    /// Save the subBuffer to the parentBuffer's context
    /// \param parentBuffer the parent buffer from which the sub buffer was created
    /// \param subBuffer the sub buffer to be saved
    /// \param flags      the flags parameter when the buffer was created
    /// \param bufferSize the size of the CL buffer
    /// \return true if successful, false otherwise
    bool AddSubBuffer(const cl_mem& parentBuffer,
                      const cl_mem& subBuffer,
                      cl_mem_flags  flags,
                      size_t        bufferSize);

    /// Save pipe to the kernel list
    /// \param context    the CL context
    /// \param pipe       the CL pipe to be saved
    /// \return true if successful, false otherwise
    bool AddPipe(const cl_context context,
                 const cl_mem     pipe);

    /// Remove mem object
    /// \param mem       the CL mem object
    /// \return true if successful, false otherwise
    bool RemoveMemObject(const cl_mem& mem);

    /// Set the GPU flag to true or false depending whether a GPU device is detected or not
    /// \param bFlag the flag
    void SetGPUFlag(bool bFlag) { m_bGPU = bFlag; }

    /// Get the GPU flag
    /// \return the GPU flag (true if a GPU device has been detected, false otherwise)
    bool GetGPUFlag() const { return m_bGPU; }

    /// Check whether the list contains user events
    /// \param wait_list cl_event list to be checked whether it has user events
    /// \param num number of items in wait_list
    /// \param event event object that created by enqueue command, it might be added to CLUserEvent.dependentEventList
    /// \return true if it has user events
    bool HasUserEvent(const cl_event* wait_list, int num, cl_event* event);

    /// Check whether the list contains user events
    /// \return true if it has user events
    CLUserEvent* HasUserEvent(const cl_event* wait_list, int num);

    /// Remove user event when clRelease event is called or clSetUserEvent is called and status = CL_COMPLETE
    /// \param event user event object
    void RemoveUserEvent(cl_event event);

    /// Determines whether the max number of kernels have been profiled already
    /// \return true if the max number of kernels have already been profiled
    bool HasKernelMaxBeenReached() const { return m_uiCurKernelCount >= m_uiMaxKernelCount; }

    /// Indicates whether or not profiling is currently enabled
    /// \return true if profiling is enabled, false otherwise
    bool IsProfilingEnabled() const { return m_bIsProfilingEnabled; }

    /// Enable to disable profiling
    /// \param doEnable, flag indicating whether to enable (true) or disable (false) profiling
    void EnableProfiling(bool doEnable) { m_bIsProfilingEnabled = doEnable; }

    /// Indicates whether profiler should run after delay or not
    /// \param delayInMilliseconds to return the amount by which profile set to be delayed
    /// \returns true if delay is enabled otherwise false
    bool IsProfilerDelayEnabled(unsigned long& delayInMilliseconds);

    /// Indicates whether profiler should run only for set duration or not
    /// \param durationInMilliseconds to return the amount by which profile set to run
    /// \returns true if duration of the profiler is enabled
    bool IsProfilerDurationEnabled(unsigned long& durationInMilliseconds);

    /// Assigns the call back function
    /// \param timerType type of the timer
    /// \param timerEndHandler call back function pointer
    void SetTimerFinishHandler(ProfilerTimerType timerType, TimerEndHandler timerEndHandler);

    /// Creates the Profiler Timer
    /// \param timerType timer type of the starting timer
    /// \param timeIntervalInMilliseconds profiler duration or profiler delay in milliseconds
    void CreateTimer(ProfilerTimerType timerType, unsigned long timeIntervalInMilliseconds);

    /// Starts the timer
    /// \param timerType timer type of the starting timer
    void StartTimer(ProfilerTimerType timerType);

private:
    /// Set the output file path
    /// \param strOutputFile  the output file path
    void SetOutputFile(const std::string& strOutputFile);

    /// Add header to profile result mgr, setup default columns
    void InitHeader();

    /// Given the command queue, get the GPU device name
    /// \param pszDeviceName  the resulting GPU device name
    /// \param commandQueue   the cl command queue
    /// \return false if it is not a GPU device, true if successful
    bool GetDeviceName(char* pszDeviceName, cl_command_queue commandQueue) const;

    /// Gets the device id from the given device name, using ADLUtils' asicInfoList
    /// \param pszDeviceName the device name whose device id is wanted
    /// \param asicInfoList the list of available ASICs from ADL
    /// \param[out] deviceId the device id for the specified device
    /// \param[out] revisionId the revision id for the specified device
    bool GetAvailableDeviceIdFromDeviceNameAndAsicInfoList(const char* pszDeviceName, const AsicInfoList asicInfoList, int& deviceId, int& revisionId);

    GPAUtils           m_GPAUtils;                          ///< common GPA utility functions
    bool               m_isGPAOpened;                       ///< flag indicating if a GPA Context is currently opened
    std::string        m_strDeviceName;                     ///< the device name
    unsigned int       m_uiCurKernelCount;                  ///< number of kernels that have been profiled.
    unsigned int       m_uiMaxKernelCount;                  ///< max number of kernels to profile.
    unsigned int       m_uiOutputLineCount;                 ///< number of items written to the output file
    bool               m_bIsProfilingEnabled;               ///< flag indicating if profiling is currently enabled
    bool               m_bGPU;                              ///< true if profiling a GPU device (false is the default)
    CLContextManager   m_contextManager;                    ///< manages all the CL contexts for arena support

    KernelAssembly     m_KernelAssembly;                    ///< manages retrieving the CL source, IL and ISA from the CL run-time
    std::string        m_strOutputFile;                     ///< the output file path
    std::string        m_strOccupancyFile;                  ///< the output kernel occupancy file
    UserEventList      m_userEventList;                     ///< user event list
    CLPlatformSet      m_platformList;                      ///< Platform list

    bool               m_bForceSinglePass;                  ///< flag indicating whether or not the profiler should only allow a single pass
    bool               m_bCollectGPUTime;                   ///< flag indicating whether or not the profiler should collect gpu time when collecting perf counters
    bool               m_bDelayStartEnabled;                ///< flag indicating whether or not the profiler should start with delay or not
    bool               m_bProfilerDurationEnabled;          ///< flag indiacating whether profiler should only run for certain duration
    unsigned long      m_delayInMilliseconds;               ///< milliseconds to delay for profiler to start
    unsigned long      m_durationInMilliseconds;            ///< duration in milliseconds for which Profiler should run
    ProfilerTimer*     m_delayTimer;                        ///< timer for handling delay timer for the profile agent
    ProfilerTimer*     m_durationTimer;                     ///< timer for handling duration timer for the profile agent
};

// @}

#endif //_CL_GPAPROFILER_H_
