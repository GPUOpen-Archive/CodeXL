//=====================================================================
// Copyright (c) 2010 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Lib/AMD/OpenCLDebugAPI/1.3/Include/AMDOpenCLDebug.h $
/// \version $Revision: #9 $
/// \brief  Definitions for the AMD OpenCL debugger API.
//
//=====================================================================
// $Id: //devtools/main/Common/Lib/AMD/OpenCLDebugAPI/1.3/Include/AMDOpenCLDebug.h#9 $
// Last checkin:   $DateTime: 2014/11/26 12:01:40 $
// Last edited by: $Author: ushomron $
// Change list:    $Change: 511421 $
//=====================================================================

#ifndef _AMDCLDEBUG_H_
#define _AMDCLDEBUG_H_

#include <CL/cl.h>

// Include the external amdclIntercept calls
#include "CLWrappers.h"
// Include the internal amdclIntercept calls
#include "CLIntercept.h"
// Include the amdclDebugGetDispatchTable() and amdclDebugSetDispatchTable() functions
#include "CLDispatchTable.h"

#ifdef __cplusplus
extern "C" {
#endif


/// API Version identification
#define AMDCLDebugAPI_VersionMajor  1
#define AMDCLDebugAPI_VersionMinor  3

/// Additional OpenCL return codes from amdclDebugEnqueueNDRangeKernel()
/// when a kernel cannot be debugged.
#define CL_DEBUG_ERROR_AMD                          -5000
#define CL_UNSUPPORTED_PLATFORM_AMD                 -5001
#define CL_DEVICE_NOT_GPU_AMD                       -5002
#define CL_KERNEL_NOT_DEBUGGABLE_AMD                -5003
#define CL_COMMAND_QUEUE_NOT_INTERCEPTED_AMD        -5004

/// API return codes
typedef int amdclDebugError;
#define AMDCLDEBUG_UNINITIALIZED          -1
#define AMDCLDEBUG_SUCCESS                0
#define AMDCLDEBUG_ERROR                  1
#define AMDCLDEBUG_INVALID_CONTEXT        2
#define AMDCLDEBUG_INVALID_STATE          3
#define AMDCLDEBUG_INVALID_PC             4
#define AMDCLDEBUG_INVALID_LOCATOR        5
#define AMDCLDEBUG_BUFFER_SIZE            6
#define AMDCLDEBUG_OUT_OF_MEMORY          7
#define AMDCLDEBUG_BAD_EVENT              8
#define AMDCLDEBUG_NULL_POINTER           9
#define AMDCLDEBUG_UNSUPPORTED            10


/// events coming from the amdclDebugNDRangeKernel() callback function
typedef enum
{
   AMDCLDEBUG_EVENT_UNINITIALIZED = 0,    ///< Bug prevention, do not use.
   AMDCLDEBUG_EVENT_STARTED,              ///< The debugger has been invoked, but kernel
                                          ///<  execution has not started.
   AMDCLDEBUG_EVENT_HALTED,               ///< The debugger has halted at a specific kernel PC.
   AMDCLDEBUG_EVENT_FINISHED,             ///< The kernel being debugged has completed execution.
   AMDCLDEBUG_EVENT_ERROR,                ///< An error occcured in the debugger.
   AMDCLDEBUG_EVENT_MAX
} amdclDebugEvent;

/// Commands which can be given to the debugger
typedef enum
{
   AMDCLDEBUG_COMMAND_UNINITIALIZED = 0,  ///< bug prevention, do not use
   AMDCLDEBUG_COMMAND_CONTINUE,           ///< continue to next breakpoint
   AMDCLDEBUG_COMMAND_STEP,               ///< Step to next kernel PC
   AMDCLDEBUG_COMMAND_MAX
} amdclDebugCommand;

/// Debugger context
typedef void* amdclDebugContext;
/// User callback
typedef amdclDebugCommand (*amdclDebugCallback)(amdclDebugContext context,
                                                amdclDebugEvent debugEvent,
                                                void* user_data);
/// Kernel program counter.
typedef cl_uint amdclDebugPC;
/// Opaque register locator.
typedef cl_ulong amdclDebugRegisterLocator;
/// Opaque memory resource identifier.
typedef cl_uint amdclDebugMemoryResource;
/// Memory address.
typedef cl_ulong amdclDebugMemoryAddress;

typedef struct
{
   amdclDebugMemoryResource resource;   ///< Resource where this memory region resides.
   amdclDebugMemoryAddress  first;      ///< First address in memory region.
   amdclDebugMemoryAddress  last;       ///< Last address in memory region.
} amdclDebugMemoryRegion;


// GPU Debugger API calls

/// Get OpenCL GPU Debugger API library version identification
/// \param[out] major   Where to store the major version number
/// \param[out] minor   Where to store the minor version number
/// \param[out] build   Where to store the build number
/// \return             AMDCLDEBUG_SUCCESS on success
///                     AMDCLDEBUG_NULL_POINTER if any of major, minor or build is NULL
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugGetVersion(cl_uint* major,
                     cl_uint* minor,
                     cl_uint* build);

/// Enqueues a command to execute a kernel to be debugged on a device.
/// \param[in] command_queue           The command queue
/// \param[in] kernel                  Kernel object
/// \param[in] work_dim                Number of work dimensions
/// \param[in] global_work_offset      Array of offsets for computing Global ID
/// \param[in] global_work_size        Array of global work items
/// \param[in] local_work_size         Array of items making up a work-group
/// \param[in] num_events_in_wait_list Number of events prior to starting this kernel
/// \param[in] event_wait_list         The list of events
/// \param[in] event                   The completion event
/// \param[in] callback                Debug callback context
/// \param[in] user_data               User data to be passed to callback when debugging this kernel. May be NULL.
/// \return                            Same result as clEnqueueNDRangeKernel() or CL_DEBUG_ERROR_AMD
///                                    if the kernel cannot be debugged
extern CL_API_ENTRY cl_int CL_API_CALL
amdclDebugEnqueueNDRangeKernel(cl_command_queue   command_queue,
                               cl_kernel          kernel,
                               cl_uint            work_dim,
                               const size_t*      global_work_offset,
                               const size_t*      global_work_size,
                               const size_t*      local_work_size,
                               cl_uint            num_events_in_wait_list,
                               const cl_event*    event_wait_list,
                               cl_event*          event,
                               amdclDebugCallback callback,
                               void*              user_data ) CL_API_SUFFIX__VERSION_1_0;

/// Enqueues a command to execute a kernel to be debugged on a device.
/// \param[in] command_queue           The command queue
/// \param[in] kernel                  Kernel object
/// \param[in] work_dim                Number of work dimensions
/// \param[in] num_events_in_wait_list Number of events prior to starting this kernel
/// \param[in] event_wait_list         The list of events
/// \param[in] event                   The completion event
/// \param[in] callback                Debug callback context
/// \param[in] user_data               User data to be passed to callback when debugging this kernel. May be NULL.
/// \return                            Same result as clEnqueueTask()
extern CL_API_ENTRY cl_int CL_API_CALL
amdclDebugEnqueueTask(cl_command_queue   command_queue,
                      cl_kernel          kernel,
                      cl_uint            num_events_in_wait_list,
                      const cl_event*    event_wait_list,
                      cl_event*          event,
                      amdclDebugCallback callback,
                      void*              user_data ) CL_API_SUFFIX__VERSION_1_0;

/// Get the current program counter
/// \param[in] debugContext         Debug context
/// \param[in] progamCounter        Where to store the PC
/// \return                         AMDCLDEBUG_SUCCESS on success
///                                 AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                 AMDCLDEBUG_INVALID_PC if kernel not halted on a valid PC
///                                 AMDCLDEBUG_NULL_POINTER if programCounter == NULL
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugGetProgramCounter(amdclDebugContext debugContext,
                            amdclDebugPC*     programCounter);

/// Set a breakpoint at a Kernel PC.  If the caller tries to set a breakpoint on a PC with an existing breakpoint,
/// then the second call to set the breakpoint will be ignored, the actualPC will contain the PC where the breakpoint
/// has been set and the function will return AMDCLDEBUG_SUCCESS.
/// \param[in] debugContext         Debug context
/// \param[in] programCounter       Kernel program counter
/// \param[in] lookAhead            The maximum offset from the programCounter to search to for the next executable PC; 
///                                 Pass 0 to only set breakpoint at the exact PC
/// \param[out] actualPC            The actual PC that the breakpoint has been set to, pass NULL if the actual PC is not requested
/// \return                         AMDCLDEBUG_SUCCESS on success
///                                 AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                 AMDCLDEBUG_INVALID_PC if invalid PC specified
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugSetBreakpoint(amdclDebugContext  debugContext,
                        amdclDebugPC       programCounter,
                        cl_uint            lookAhead,
                        amdclDebugPC*      actualPC );

/// Clear a breakpoint at a Kernel PC
/// \param[in] debugContext         Debug context
/// \param[in] programCounter       Kernel program counter; this PC needs to be the actualPC as returned by the amdclDebugSetBreakpoint or
///                                 amdclDebugGetAllBreakpoints function.
/// \return                         AMDCLDEBUG_SUCCESS on success
///                                 AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                 AMDCLDEBUG_INVALID_PC if invalid PC specified
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugClearBreakpoint(amdclDebugContext   debugContext,
                          amdclDebugPC        programCounter);

/// Clear all breakpoints
/// \param[in] debugContext         Debug context
/// \return                         AMDCLDEBUG_SUCCESS on success
///                                 AMDCLDEBUG_INVALID_CONTEXT if bad context
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugClearAllBreakpoints(amdclDebugContext   debugContext);

/// Get the number of breakpoints
/// \param[in] debugContext         Debug context
/// \param[in] numBreakpoints       The number of breakpoints
/// \return                         AMDCLDEBUG_SUCCESS on success
///                                 AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                 AMDCLDEBUG_NULL_POINTER if numBreakpoints == NULL
///                                 AMDCLDEBUG_BUFFER_SIZE if inadequate space
///                                       was allocated to store the data
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugGetNumBreakpoints(amdclDebugContext    debugContext,
                            size_t*              numBreakpoints);

/// Get a list of all of the breakpoints
/// \param[in] debugContext         Debug context
/// \param[in] breakpointList       Where to store the PCs
/// \param[in] listLength           Length of breakpointList
/// \return                         AMDCLDEBUG_SUCCESS on success
///                                 AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                 AMDCLDEBUG_BUFFER_SIZE if inadequate space
///                                       was allocated to store the data
///                                 AMDCLDEBUG_NULL_POINTER if breakpointList == NULL
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugGetAllBreakpoints(amdclDebugContext    debugContext,
                            amdclDebugPC*        breakpointList,
                            size_t               listLength);

/// Get a the size of the kernel binary
/// \param[in] debugContext         Debug context
/// \param[in] debugBinarySize      Size of the kernel binary
/// \return                         AMDCLDEBUG_SUCCESS on success
///                                 AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                 AMDCLDEBUG_NULL_POINTER if debugBinarySize == NULL
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugGetKernelBinarySize(amdclDebugContext   debugContext,
                              size_t*             debugBinarySize);

/// Get a copy of the kernel binary
/// \param[in] debugContext         Debug context
/// \param[in] debugBinary          Where to store the copy of the kernel executable
/// \param[in] debugBinarySize      Size of the storage area
/// \return                         AMDCLDEBUG_SUCCESS on success
///                                 AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                 AMDCLDEBUG_BUFFER_SIZE if inadequate space
///                                       was allocated to store the data
///                                 AMDCLDEBUG_NULL_POINTER if debugBinarySize == NULL
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugGetKernelBinary(amdclDebugContext   debugContext,
                          void*               debugBinary,
                          size_t              debugBinarySize);

/// Get the kernel execution mask at the current PC.
/// \param[in] debugContext         Debug context
/// \param[in] executionMask        Where to store the PCs
/// \param[in] numElements          Number of elements in of executionMask
/// \return                         AMDCLDEBUG_SUCCESS on success
///                                 AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                 AMDCLDEBUG_INVALID_PC if kernel not halted on a valid PC
///                                 AMDCLDEBUG_NULL_POINTER if executionMask == NULL
///                                    or numElements == NULL
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugGetExecutionMask(amdclDebugContext  debugContext,
                           const cl_bool**    executionMask,
                           size_t*            numElements);

/// Set the list of registers to track while debugging.
/// \param[in]  debugContext             Debug context
/// \param[in]  debugRegisterLocators    Opaque register locations to track
/// \param[in]  numDebugRegisterLocators Number of opaque register locations to track
/// \return                              AMDCLDEBUG_SUCCESS on success
///                                      AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                      AMDCLDEBUG_INVALID_PC if kernel not in started state
///                                      AMDCLDEBUG_INVALID_LOCATOR if one of the specified register 
///                                         locators is invalid
///                                      AMDCLDEBUG_NULL_POINTER if debugRegisterLocators is NULL and
///                                         numDebugRegisterLocators is not zero
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugSetTrackingRegisters(amdclDebugContext                debugContext,
                               const amdclDebugRegisterLocator* debugRegisterLocators,
                               size_t                           numDebugRegisterLocators);

/// Set whether to track private memory or not.
/// \param[in]  debugContext             Debug context
/// \param[in]  track                    True to track private memory, false not to.
/// \return                              AMDCLDEBUG_SUCCESS on success
///                                      AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                      AMDCLDEBUG_INVALID_PC if kernel not in started state
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugSetTrackPrivateMemory(amdclDebugContext debugContext,
                                cl_bool           track);

/// Get the number of currently active registers.
/// \param[in] debugContext         Debug context
/// \param[in] numActiveRegisters   Where to store the number of active registers
/// \return                         AMDCLDEBUG_SUCCESS on success
///                                 AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                 AMDCLDEBUG_INVALID_PC if kernel not halted on a valid PC
///                                 AMDCLDEBUG_NULL_POINTER if numActiveRegisters is NULL
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugGetNumberOfActiveRegisters(amdclDebugContext   debugContext,
                                     size_t*             numActiveRegisters);
                                     
/// Get the list of currently active registers.
/// \param[in]  debugContext             Debug context
/// \param[in]  debugRegisterLocators    Active register locations
/// \param[in]  numActiveRegisters       Number of active registers
/// \return                              AMDCLDEBUG_SUCCESS on success
///                                      AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                      AMDCLDEBUG_INVALID_PC if kernel not halted on a valid PC
///                                      AMDCLDEBUG_BUFFER_SIZE if inadequate space
///                                         was allocated to store the data
///                                      AMDCLDEBUG_NULL_POINTER if debugRegisterLocators is NULL and
///                                         numActiveRegisters is not zero
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugGetActiveRegisters(amdclDebugContext          debugContext,
                             amdclDebugRegisterLocator* debugRegisterLocators,
                             size_t                     numActiveRegisters);

/// Get the register values
/// \param[in]  debugContext         Debug context
/// \param[in]  debugRegisterLocator Opaque register location of the variable
/// \param[out] outputBuffer         Where to store the values
/// \param[out] outputStride         The size of each element in the outputBuffer expressed as a number of bytes.  
/// \param[out] numElements          number of elements in the output buffer
/// \return                          AMDCLDEBUG_SUCCESS on success
///                                  AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                  AMDCLDEBUG_INVALID_PC if kernel not halted on a valid PC
///                                  AMDCLDEBUG_INVALID_LOCATOR if the specified register 
///                                    locator is invalid
///                                 AMDCLDEBUG_NULL_POINTER if outputBuffer or outputStride or numElements is NULL 
extern CL_API_ENTRY amdclDebugError CL_API_CALL
amdclDebugGetRegisterValues(amdclDebugContext         debugContext,
                            amdclDebugRegisterLocator debugRegisterLocator,
                            const void**              outputBuffer,
                            size_t*                   outputStride,
                            size_t*                   numElements);

/// Get the value stored in the OpenCL private memory region.
/// \param[in]  debugContext       Debug context
/// \param[in]  address            The memory address being requested.
/// \param[in]  count              The number of bytes to retrieve.
/// \param[out] buffer             A pointer to memory where the memory values being queried
///                                are returned. If buffer is NULL, it is ignored.
/// \param[out] outputSize         The actual size in bytes of data copied to buffer.
///                                If outputSize is NULL, it is ignored.
/// \return                        AMDCLDEBUG_SUCCESS on success
///                                AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                AMDCLDEBUG_INVALID_PC if kernel not halted on a valid PC
///                                AMDCLDEBUG_INVALID_LOCATOR if the specified memory 
///                                 locator is invalid (or memory region is not supported)
extern CL_API_ENTRY amdclDebugError CL_API_CALL 
amdclDebugGetPrivateMemoryValues(amdclDebugContext       debugContext,
                                 amdclDebugMemoryAddress address,
                                 size_t                  count,
                                 void*                   buffer,
                                 size_t*                 outputSize);

/// Get the value stored in the OpenCL global memory region.
/// \param[in]  debugContext       Debug context.
/// \param[in]  resource           The memory resource identifier.
/// \param[in]  address            The memory address being requested.
/// \param[in]  count              The number of bytes to retrieve.
/// \param[out] buffer             A pointer to memory where the memory values being queried
///                                are returned. If buffer is NULL, it is ignored.
/// \param[out] outputSize         The actual size in bytes of data copied to buffer.
///                                If outputSize is NULL, it is ignored.
/// \return                        AMDCLDEBUG_SUCCESS on success
///                                AMDCLDEBUG_INVALID_CONTEXT if bad context
///                                AMDCLDEBUG_INVALID_PC if kernel not halted on a valid PC
///                                AMDCLDEBUG_NULL_POINTER if buffer is NULL and count is non-zero.
///                                AMDCLDEBUG_INVALID_LOCATOR if the specified memory 
///                                 locator is invalid (or memory region is not supported)
extern CL_API_ENTRY amdclDebugError CL_API_CALL 
amdclDebugGetGlobalMemoryValues(amdclDebugContext        debugContext,
                                amdclDebugMemoryResource resource,
                                amdclDebugMemoryAddress  address,
                                size_t                   count,
                                void*                    buffer,
                                size_t*                  outputSize);

/// Get an array of the valid global memory regions that may be accessed
/// with amdclDebugGetGlobalMemoryValues().
/// \param[in]  debugContext       Debug context.
/// \param[in]  numRegions         Number of amdclDebugMemoryRegion structures pointed to by regions.
/// \param[out] regions            Pointer to an array of amdclDebugMemoryRegion structures.
///                                Can be NULL if numRegions is zero.
/// \param[out] numRegionsRet      The actual number of memory regions.
///                                If numRegionsRet is NULL, it is ignored.
/// \return                        AMDCLDEBUG_SUCCESS on success.
///                                AMDCLDEBUG_NULL_POINTER if regions is NULL and numRegions is non-zero.
///                                AMDCLDEBUG_BUFFER_SIZE if numRegions is not large enough
///                                  to store all the memory regions.
extern CL_API_ENTRY amdclDebugError CL_API_CALL 
amdclDebugGetValidGlobalMemoryRegions( amdclDebugContext       debugContext,
                                       size_t                  numRegions,
                                       amdclDebugMemoryRegion* regions,
                                       size_t*                 numRegionsRet );

/// Construct a register location from a register name string.
/// \param[in]  registerName         The register name string.
/// \param[out] debugRegisterLocator Pointer to opaque register location.
/// \return                          AMDCLDEBUG_SUCCESS on success.
///                                  AMDCLDEBUG_NULL_POINTER if registerName or debugRegisterLocator is NULL.
///                                  AMDCLDEBUG_INVALID_LOCATOR if the specified register 
///                                    name is invalid.
extern CL_API_ENTRY amdclDebugError CL_API_CALL 
amdclDebugUtilStringToDebugRegisterLocator( const cl_char*             registerName,
                                            amdclDebugRegisterLocator* debugRegisterLocator );

/// Construct a register name string from a register locator. The size of the registerNameBuffer must
/// be greater or equal to that returned in bufferSizeRet.
/// \param[in]  debugRegisterLocator Opaque register location.
/// \param[in]  bufferSize           Size in bytes of the memory pointed to by registerNameBuffer. Can be zero.
/// \param[out] registerNameBuffer   Pointer to location to store the register name. Can be NULL if bufferSize is zero.
/// \param[out] bufferSizeRet        The actual size in bytes of the register name string including NULL terminator.
///                                  If bufferSizeRet is NULL, it is ignored.
/// \return                          AMDCLDEBUG_SUCCESS on success.
///                                  AMDCLDEBUG_NULL_POINTER if registerNameBuffer is NULL and bufferSize is non-zero.
///                                  AMDCLDEBUG_INVALID_LOCATOR if the specified register 
///                                    locator is invalid.
///                                  AMDCLDEBUG_BUFFER_SIZE if inadequate space
///                                    was allocated to store the register name.
extern CL_API_ENTRY amdclDebugError CL_API_CALL 
amdclDebugUtilDebugRegisterLocatorToString( amdclDebugRegisterLocator debugRegisterLocator,
                                            size_t                    bufferSize,
                                            cl_char*                  registerNameBuffer,
                                            size_t*                   bufferSizeRet );

/// Get last error. The size of the buffer must be greater or equal to that returned in bufferSizeRet.
/// \param[in]  debugContext     Debug context.
/// \param[in]  bufferSize       Size in bytes of the memory pointed to by buffer. Can be zero.
/// \param[out] buffer           Pointer to a buffer to store the error string.
/// \param[out] bufferSizeRet    The actual size in bytes of the error string including NULL terminator.
///                              If bufferSizeRet is NULL, it is ignored.
/// \return                      AMDCLDEBUG_SUCCESS on success.
///                              AMDCLDEBUG_NULL_POINTER if buffer is NULL and bufferSize is non-zero.
///                              AMDCLDEBUG_BUFFER_SIZE if inadequate space
///                                was allocated to store error string.
extern CL_API_ENTRY amdclDebugError CL_API_CALL 
amdclDebugUtilGetLastError( amdclDebugContext      debugContext,
                            size_t                 bufferSize,
                            char*                  buffer,
                            size_t*                bufferSizeRet );

#ifdef __cplusplus
}
#endif

#endif   // _AMDCLDEBUG_H_
