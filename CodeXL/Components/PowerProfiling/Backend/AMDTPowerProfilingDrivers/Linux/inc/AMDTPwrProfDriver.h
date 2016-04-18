//===============================================================================
//
// Copyright(c) 2015 Advanced Micro Devices, Inc.All Rights Reserved
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//=================================================================================

#ifndef _PWRPROFDRIVER_H
#define _PWRPROFDRIVER_H

#include <linux/ioctl.h>
#include <AMDTPwrProfAttributes.h>

#define PWR_PROF_MAJOR_NUMBER  0xCC

/// \def IOCTL_GET_VERSION returns the version encoded in a ULONG64 *
/// Use InvokeOut
/// Handled by \ref IoctlGetVersionHandler
#define IOCTL_GET_VERSION \
    _IOWR(PWR_PROF_MAJOR_NUMBER, 1,unsigned long*)

/// \def IOCTL_REGISTER_CLIENT obtains the client id used for interactions,
/// both IOCTL and shared memory
/// Use InvokeOut and a ULONG *
/// Handled by \ref IoctlRegisterClient
#define IOCTL_REGISTER_CLIENT \
    _IOWR(PWR_PROF_MAJOR_NUMBER, 2,unsigned long*)


/// \def IOCTL_SET_OUTPUT_FILE sets the file path and prd header for the
/// sampling output files
/// Use InvokeInOut and \ref OUTPUT_FILE_DESCRIPTOR
/// Handled by \ref IoctlSetOutputFileHandler
#define IOCTL_SET_OUTPUT_FILE   \
    _IOWR(PWR_PROF_MAJOR_NUMBER,3,POUTPUT_FILE_DESCRIPTOR)


/// \def IOCTL_ADD_PROF_CONFIGS sets the profiling configurations to profile when
/// the profiler starts.
/// Use InvokeInOut and \ref PROF_CONFIGS
/// Handled by \ref IoctlAddProfConfigsHandler
#define IOCTL_ADD_PROF_CONFIGS \
    _IOWR(PWR_PROF_MAJOR_NUMBER, 4, PPROF_CONFIGS)


/// \def IOCTL_START_PROFILER starts the profile.
/// Use InvokeInOut and \ref PROFILER_PROPERTIES
/// Handled by \ref IoctlStartProfilerHandler
#define IOCTL_START_PROFILER \
    _IOWR(PWR_PROF_MAJOR_NUMBER,5, PPROFILER_PROPERTIES)


/// \def IOCTL_PAUSE_PROFILER pauses the profile.
/// A faster method is to set the appropriate (1 << clientId) shared memory bit
/// or to set the shared memory key
/// Use InvokeInOut and ULONG client id in, ULONG profile state out
/// Handled by \ref IoctlPauseProfilerHandler
#define IOCTL_PAUSE_PROFILER \
    _IOWR(PWR_PROF_MAJOR_NUMBER, 6, PPROFILER_PROPERTIES)


/// \def IOCTL_RESUME_PROFILER resumes the profile.
/// A faster method is to clear the appropriate (1 << clientId) shared memory
/// bit or to clear the shared memory key
/// Use InvokeInOut and ULONG client id in, ULONG profile state out
/// Handled by \ref IoctlResumeProfilerHandler
#define IOCTL_RESUME_PROFILER   \
    _IOWR(PWR_PROF_MAJOR_NUMBER,7 , PPROFILER_PROPERTIES)


/// \def IOCTL_GET_FILE_HEADER_BUFFER is to get the file header data
/// Use InvokeInOut and \ref FILE_HEADER
/// Handled by \ref IoctlGetFileHeaderBufferHandler
#define IOCTL_GET_FILE_HEADER_BUFFER \
    _IOWR(PWR_PROF_MAJOR_NUMBER, 8, PFILE_HEADER)

/// \def IOCTL_GET_DATA_BUFFER is to get the sample data buffer
/// Use InvokeInOut and \ref DATA_BUFFER
/// Handled by \ref IoctlGetDataBufferHandler
#define IOCTL_GET_DATA_BUFFER \
    _IOWR(PWR_PROF_MAJOR_NUMBER, 9,PDATA_BUFFER)


/// \def IOCTL_STOP_PROFILER stops the profile.  It will also clear the state
/// of the clientId.
/// Use InvokeInOut and ULONG client id in, ULONG profile status out
/// Handled by \ref IoctlStopProfilerHandler
#define IOCTL_STOP_PROFILER \
    _IOWR(PWR_PROF_MAJOR_NUMBER, 10,PPROFILER_PROPERTIES)


/// \def IOCTL_UNREGISTER_CLIENT frees the client id used for interactions,
/// it will immediately stop and finish any current profile
/// Use InvokeIn and a ULONG *
/// Handled by \ref IoctlUnegisterClient
#define IOCTL_UNREGISTER_CLIENT \
    _IOWR(PWR_PROF_MAJOR_NUMBER, 11, PPROFILER_PROPERTIES)

// \def IOCTL_ACCESS_PCI_DEVICE provides access to PCI devices,
/// Use InvokeInOut and \ref ACCESS_PCI
/// Handled by \ref IoctlAccessPciDevice
#define IOCTL_ACCESS_PCI_DEVICE \
    _IOWR(PWR_PROF_MAJOR_NUMBER, 15, PACCESS_PCI)

/// \def IOCTL_ACCESS_MSR provides access to MSR
/// Use InvokeInOut and \ref ACCESS_MSR
/// Handled by \ref IoctlAccessMSR
#define IOCTL_ACCESS_MSR \
    _IOWR(PWR_PROF_MAJOR_NUMBER, 16, PACCESS_MSR)

/// \def IOCTL_ACCESS_MMIO provides access to MMIO address space
/// Use InvokeInOut and \ref ACCESS_MMIO
/// Handled by \ref IoctlAccessMMIO
#define IOCTL_ACCESS_MMIO \
    _IOWR(PWR_PROF_MAJOR_NUMBER, 17, PACCESS_MMIO)

/// \def IOCTL_SET_AND_GET_FD create an annonymous inode and
/// return a  file descriptor for the file.
#define IOCTL_SET_AND_GET_FD \
    _IOWR(PWR_PROF_MAJOR_NUMBER, 18, int*)



#endif  //_PWRPROFDRIVER_H

