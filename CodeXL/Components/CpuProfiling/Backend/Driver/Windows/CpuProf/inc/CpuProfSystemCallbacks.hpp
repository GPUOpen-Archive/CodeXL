//=============================================================
// (c) 2012 Advanced Micro Devices, Inc.
//
/// \author franksw
/// \version $Revision: #1 $
/// \brief  Dynamic task info records
//
//=============================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/Driver/Windows/CpuProf/inc/CpuProfSystemCallbacks.hpp#1 $
// Last checkin:   $DateTime: 2014/05/11 06:15:06 $
// Last edited by: $Author: ekatz $
// Change list:    $Change: 495861 $
//=============================================================
#ifndef _CPUPROF_SYSTEMCALLBACKS_HPP_
#define _CPUPROF_SYSTEMCALLBACKS_HPP_
#pragma once

#include <stdlib.h>

namespace CpuProf {


//=============================================================================
/// Driver-supplied callback routine that is called whenever a process is
/// created or deleted.  This callback is hooked through
/// PsSetCreateProcessNotifyRoutine()
///
/// \param[in] hParentId The parent process id that created the process
/// \param[in] hProcessId The newly created process id
/// \param[in] bCreate Whether the process was created or destroyed
///
/// IRQL Level: PASSIVE_LEVEL
//=============================================================================
VOID CreateProcessCallback(IN HANDLE hParentId, IN HANDLE hProcessId, IN BOOLEAN bCreate);

//=============================================================================
/// Driver-supplied callback routine that is called whenever an image is
/// loaded for execution.  This callback is hooked through
/// PsSetLoadImageNotifyRoutine();
///
/// \param[in] pFullImageName The full image name
/// \param[in] hProcessId The process id that is loading a module
/// \param[in] pImageInfo Where the image is mapped
///
/// IRQL Level: >= PASSIVE_LEVEL
//=============================================================================
VOID LoadImageCallback(IN PUNICODE_STRING pFullImageName, IN HANDLE hProcessId, IN PIMAGE_INFO pImageInfo);

//=============================================================================
/// Driver-supplied callback routine that is subsequently notified whenever
/// an thread is created or deleted.  This callback is hooked through
/// PsSetCreateThreadNotifyRoutine();
///
/// \param[in] hProcessId The process id that is creating or destroying a thread
/// \param[in] hThreadId The thread id
/// \param[in] bCreate Whether the thread is created or destroyed
///
/// IRQL Level: PASSIVE_LEVEL
//=============================================================================
VOID CreateThreadCallback(IN HANDLE hProcessId, IN HANDLE hThreadId, IN BOOLEAN bCreate);

} // namespace CpuProf

#endif //_CPUPROF_SYSTEMCALLBACKS_HPP_
