////////////////////////////////////////////////////////////////////////////////
//
// The University of Illinois/NCSA
// Open Source License (NCSA)
//
// Copyright (c) 2014-2015, Advanced Micro Devices, Inc. All rights reserved.
//
// Developed by:
//
//                 AMD Research and AMD HSA Software Development
//
//                 Advanced Micro Devices, Inc.
//
//                 www.amd.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal with the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
//  - Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimers.
//  - Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimers in
//    the documentation and/or other materials provided with the distribution.
//  - Neither the names of Advanced Micro Devices, Inc,
//    nor the names of its contributors may be used to endorse or promote
//    products derived from this Software without specific prior written
//    permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS WITH THE SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _HSA_EXT_DEBUGGER_H_
#define _HSA_EXT_DEBUGGER_H_

#include "hsa.h"
#include "amd_hsa_tools_interfaces.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Hash define for exporting the API functions.
 */
#if defined _WIN32 || defined __CYGWIN__
#ifdef __GNUC__
#define HSA_TOOLS_API __attribute__ ((dllexport))
#else
#define HSA_TOOLS_API __declspec(dllexport)  // Note: actually gcc seems
                                             // to also supports this
                                             // syntax.
#endif
#ifndef DLL_LOCAL
#define DLL_LOCAL
#endif

#else  // defined _WIN32 || defined __CYGWIN__
#if __GNUC__ >= 4
#define HSA_TOOLS_API __attribute__ ((visibility("default")))
#ifndef DLL_LOCAL
#define DLL_LOCAL  __attribute__ ((visibility("hidden")))
#endif
#else
#define HSA_TOOLS_API
#ifndef DLL_LOCAL
#define DLL_LOCAL
#endif
#endif
#endif  // defined _WIN32 || defined __CYGWIN__

//---------------------------------------------------------------------------//
// @brief Wave actions used to control the wave execution on the hardware    //
// @details The wave action enumerations are used to specify the desired     //
// behavior when calling the wave control function. Overall, there are       //
// five types of operations that can be specified.                           //
//---------------------------------------------------------------------------//
typedef enum hsa_ext_tools_wave_action_s {
  // Suspend the wave execution on the hardware
  HSA_EXT_TOOLS_WAVE_ACTION_HALT = 1,

  // Resume the kernel execution. When a kernel is halted, calling the
  // wave control function with this operation will resume the wave
  // execution, and the wave is resumed at the instruction following
  // the exception PC. This is the default behavior.
  HSA_EXT_TOOLS_WAVE_ACTION_RESUME = 2,

  // Terminate the kernel execution. The wave is killed at the executing
  // instruction.
  HSA_EXT_TOOLS_WAVE_ACTION_KILL = 3,

  // Cause the kernel execution to be in the SQ debug mode. That is, the
  // trap handler is triggered after the execution of every instruction
  // in the kernel. This action is used by the debugger to debug the
  // kernel execution.
  HSA_EXT_TOOLS_WAVE_ACTION_DEBUG = 4,

  // This action causes a host trap for the kernel execution. As a result,
  // a trap handler will be triggered. This action is mainly used
  // by the debugger.
  HSA_EXT_TOOLS_WAVE_ACTION_TRAP = 5,

  HSA_EXT_TOOLS_WAVE_ACTION_MAX
} hsa_ext_tools_wave_action_t;

//---------------------------------------------------------------------------//
// Host actions when encountering an exception in the kernel. It is to       //
// specify the desired host response in the event thatn a agent kernel       //
// exception is encountered.                                                 //
//---------------------------------------------------------------------------//
typedef enum hsa_ext_tools_host_action_s {
  // Ignore the kernel exception. The host application will ignore any kernel
  // exceptions. This is the default action on the host side.
  HSA_EXT_TOOLS_HOST_ACTION_IGNORE = 1,

  // Exit the host application on a kernel exception.  The host application
  // will call exit() in the event of a kernel exception.
  HSA_EXT_TOOLS_HOST_ACTION_EXIT = 2,

  // Report the exception information to users. The host application will print
  // out the exception information when an exception happens in the kernel.
  HSA_EXT_TOOLS_HOST_ACTION_NOTIFY = 4
} hsa_ext_tools_host_action_t;

//---------------------------------------------------------------------------//
// @brief Mode of the wave action when calling the wave control function     //
// @details The wave mode enumerations are used to specify the desired       //
//          broadcast level when calling the wave control function.          //
//---------------------------------------------------------------------------//
typedef enum hsa_ext_tools_wave_mode_s {
  // Apply the wave action on a single wave
  HSA_EXT_TOOLS_WAVE_MODE_SINGLE = 0,

  // Apply the wave action on all waves in the current process
  HSA_EXT_TOOLS_WAVE_MODE_BROADCAST_PROCESS = 2,

  // Apply the wave action on all waves within current compute unit.
  HSA_EXT_TOOLS_WAVE_MODE_BROADCAST_PROCESS_CU = 3,

  HSA_EXT_TOOLS_WAVE_MODE_MAX
} hsa_ext_tools_wave_mode_t;

//---------------------------------------------------------------------------//
// Trap types supported in the RT system                                     //
// It is used to indicate which type of trap handler is used.                //
// Current implementation only supports debugger trap handler.               //
//---------------------------------------------------------------------------//
typedef enum hsa_ext_tools_trap_type_s {
  // Debug Trap handler type
  HSA_EXT_TOOLS_DEBUG_TRAP = 1,

  HSA_EXT_TOOLS_MAX_TRAP
} hsa_ext_tools_trap_type_t;

//---------------------------------------------------------------------------//
// Enumeration of address watch mode, it indicates the different             //
// modes of address watch.                                                   //
//---------------------------------------------------------------------------//
typedef enum hsa_ext_tools_address_watch_mode_s
{
  // Read operations only
  HSA_EXT_TOOLS_ADDRESS_WATCH_READ = 0,

  // Write or Atomic operations only
  HSA_EXT_TOOLS_ADDRESS_WATCH_NON_READ = 1,

  // Atomic Operations only
  HSA_EXT_TOOLS_ADDRESS_WATCH_ATOMIC = 2,

  // Read, Write or Atomic operations
  HSA_EXT_TOOLS_ADDRESS_WATCH_ALL = 3,

  // Number of address watch modes
  HSA_EXT_TOOLS_ADDRESS_WATCH_NUM
} hsa_ext_tools_address_watch_mode_t;

//---------------------------------------------------------------------------//
// Enumeration of debugger event status, it indicates the return value       //
// of the event wait function.                                               //
// Note: HSA runtime spec does not have the timeout status, so I need to add //
//       one to be used by the tools                                         //
//---------------------------------------------------------------------------//
typedef enum hsa_ext_tools_event_wait_status_s
{
  // Event wait function returns success
  HSA_EXT_TOOLS_EVENT_WAIT_SUCCESS = HSA_STATUS_SUCCESS,

  // Error happened in event Wait
  HSA_EXT_TOOLS_EVENT_WAIT_ERROR = HSA_STATUS_ERROR, 

  // Event wait function timeout
  HSA_EXT_TOOLS_EVENT_WAIT_TIMEOUT = 0x14003
} hsa_ext_tools_event_wait_status_t;

//---------------------------------------------------------------------------//
// Dispatch exception policy descriptor. It is to define the expected        //
// exception policy in the event an exception is encountered on the          //
// associated dispatch.                                                      //
//---------------------------------------------------------------------------//
typedef struct hsa_ext_tools_exception_policy_s {
  // Mask of exception types to handle, only the last 7 bits (or 9 bits on CI)
  // are used. Each bit is for a different exception type.
  uint32_t exception_mask;

  // Expected wave action to take in the event of an exception.  The default
  // wave action is to resume the wave (kHsaWaveActionResume) in the event of
  // an exception during the execution of a kernel.
  hsa_ext_tools_wave_action_t wave_action;

  // Expected host action to take in the event of an exception. The default
  // host action is to report the exceptions info in the GPU.
  hsa_ext_tools_host_action_t host_action;

  // Mode used for calling the wavecontrol function to control the kernel
  // execution.
  hsa_ext_tools_wave_mode_t wave_mode;
} hsa_ext_tools_exception_policy_t;

//---------------------------------------------------------------------------//
// This structure is for the debug info in each kernel dispatch. Right       //
// now, it contains the memory descriptor information of the scratch memory  //
// and the global memory.                                                    //
//---------------------------------------------------------------------------//
typedef struct hsa_ext_tools_dispatch_debug_info_s {
  // Scratch memory descriptor
  uint32_t scratch_memory_descriptor[4];

  // Global memory descriptor
  uint32_t global_memory_descriptor[4];
} hsa_ext_tools_dispatch_debug_info_t;

//---------------------------------------------------------------------------//
// Message used by the KFD wave control for CI                               //
// This structure indicates the various information used by the wave         //
// control function.                                                         //
//---------------------------------------------------------------------------//
typedef struct hsa_ext_tools_wave_msg_amd_gen2_s {
  union {
    struct {
      // User data
      unsigned int user_data      : 8;

      // Shader array
      unsigned int shader_array   : 1;

      // Privileged
      unsigned int priv           : 1;

      // Virtual Memory id
      unsigned int vm_id          : 4;

      // Wave id
      unsigned int wave_id        : 4;

      // SIMD id
      unsigned int simd           : 2;

      // Compute unit
      unsigned int hsa_cu         : 4;

      // Shader engine
      unsigned int shader_engine  : 2;

      // see HSA_DBG_WAVEMSG_TYPE
      unsigned int message_type   : 2;

      // Reserved bits
      unsigned int reserved1      : 4;
    } ui32;
    uint32_t value;
  } ui32;
  uint32_t reserved0;
} hsa_ext_tools_wave_msg_amd_gen2_t;

//---------------------------------------------------------------------------//
// Message used for calling wave control. It can support multiple            //
// generations of GPUs with more union items defined. Currently, only the CI //
// GPU is supported. More items are to be added later.                       //
//---------------------------------------------------------------------------//
typedef struct hsa_ext_tools_wave_control_message_s {
  // pointer to associated host-accessible data
  void *memory_va;
  union {
    hsa_ext_tools_wave_msg_amd_gen2_t wave_msg_info_gen2;
  };
} hsa_ext_tools_wave_control_message_t;

//---------------------------------------------------------------------------//
// Structure for kernel execution mode. This structure is used to control    //
// the kernel execution mode. The following aspects are included in this     //
// structure:                                                                //
//   1. Regular execution or debug mode (0: regular execution (default),     //
//                                       1: debug mode)                      //
//   2. SQ debugger mode on/off                                              //
//   3. Disable L1 scalar cache (0: enable (default), 1: disable)            //
//   4. Disable L1 vector cache (0: enable (default), 1: disable)            //
//   5. Disable L2 cache (0: enable (default), 1: disable)                   //
//   6. Num of CUs reserved for display (0 (default), 7: max)                //
//---------------------------------------------------------------------------//
typedef struct hsa_ext_tools_kernel_execution_mode_s {
  union {
    struct {
      // Normal execution, or either debugger or profiler is ON.
      uint32_t monitor_mode         : 1;

      // SQ debug mode is on/off, indicate a trap handler is triggered
      // after the execution is each instruction.
      uint32_t gpu_single_step_mode : 1;

      // Disable L1 scalar cache
      uint32_t disable_l1_scalar    : 1;

      // Disable L1 vector cache
      uint32_t disable_l1_vector    : 1;

      // Disable L2 cache
      uint32_t disable_l2_cache     : 1;

      // Number of CUs reserved
      uint32_t reserved_cu_num      : 3;

      // Reserved
      uint32_t reserved            : 24;
    } ui32;
    uint32_t ui32_all;
  } ui32;
} hsa_ext_tools_kernel_execution_mode_t;

//---------------------------------------------------------------------------//
// Cache flush options                                                       //
//---------------------------------------------------------------------------//
typedef union hsa_ext_tools_cache_flush_options_s {
  struct
  {
    unsigned int sq_icache     :  1;
    unsigned int sq_kcache     :  1;
    unsigned int tc_l1         :  1;
    unsigned int tc_l2         :  1;
    unsigned int reserved      : 28;
  } ci;
} hsa_ext_tools_cache_flush_options_t;

//---------------------------------------------------------------------------//
// Debugger event handle                                                     //
//---------------------------------------------------------------------------//
typedef uint64_t  hsa_ext_tools_event_t;

//---------------------------------------------------------------------------//
// @brief Setup the ocl event handle to correlate debugger to HSA dispatch   //
// @param agent Pointer to hsa agent                                         //
// @param ocl_event_handle OpenCL event handle                               //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if ocl event handle is NULL   //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_set_correlation_handler(const hsa_agent_t agent, 
                                        void *correlation_handle);

//---------------------------------------------------------------------------//
// @brief Control the execution of wavefront on the GPU                      //
// @param agent Pointer to hsa agent                                         //
// @param action Actions to be taken on the wavefront                        //
// @param mode How the actions are taken, single wave, broadcast, etc.       //
// @param trap_id This is used for just the action of h_trap, in which       //
//        a trap ID is needed.                                               //
// @param msg_ptr Pointer to a message indicate various information.         //
//        see the KFD design for specific information.                       //
// @retval        - HSA_STATUS_SUCCESS if success                            //
//                  HSA_STATUS_ERROR_INVALID_ARGUMENT if any argument is NULL//
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_wave_control(const hsa_agent_t agent,
                             hsa_ext_tools_wave_action_t action,
                             hsa_ext_tools_wave_mode_t mode,
                             uint32_t trap_id,
                             void *msg_ptr);

//---------------------------------------------------------------------------//
// @brief Invalidate all cache on the agent.                                 //
// @details This HSA interface is used to flush the caches on a agent.       //
// All caches will be flushed.                                               //
// @param agent Pointer to hsa agent                                         //
// @retval  - HSA_STATUS_SUCCESS if success                                  //
//            HSA_STATUS_ERROR_INVALID_ARGUMENT if agent pointer is NULL     //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_flush_cache(const hsa_agent_t agent, 
                            hsa_ext_tools_cache_flush_options_t options);

//---------------------------------------------------------------------------//
// @brief Install trap handler and corresponding trap buffer.                //
// @details This HSA interface is used to install various trap handler and   //
//          the corresponding trap buffer if there is any. If no trap buffer //
//          is used, the corresponding pointer should be NULL.               //
// @param agent Pointer to hsa agent                                         //
// @param trap_type Type of trap handler to be installed                     //
// @param trap_handler Pointer to the trap handler                           //
// @param trap_buffer Pointer to the trap handler                            //
// @param trap_handler_size Size of the buffer to store the trap handler     //
// @param trap_buffer_size Size of the buffer to store the trap handler      //
// @retval  - HSA_STATUS_SUCCESS if success                                  //
//            HSA_STATUS_ERROR_INVALID_ARGUMENT if agent pointer is NULL     //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_install_trap(const hsa_agent_t agent, 
                             hsa_ext_tools_trap_type_t type,
                             void *trap_handler,
                             void *trap_buffer,
                             size_t trap_handler_size,
                             size_t trap_buffer_size);

//---------------------------------------------------------------------------//
// @brief Set up an exception policy in the trap handler object              //
// @param agent Pointer to hsa agent                                         //
// @param exception_policy Exception policy structure                        //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the exception policy is    //
//           invalid                                                         //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_set_exception_policy(const hsa_agent_t agent, 
                                      hsa_ext_tools_exception_policy_t
                                      exception_policy);

//---------------------------------------------------------------------------//
// @brief Get the exception policy from the trap handler object              //
// @param agent Pointer to hsa agent                                         //
// @param exception_policy Pointer to an exception policy structure          //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the pointer to the         //
//           exception structure is NULL                                     //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_exception_policy(const hsa_agent_t agent, 
                                        hsa_ext_tools_exception_policy_t
                                        *exception_policy);

//---------------------------------------------------------------------------//
// @brief Set up the kernel execution mode in the trap handler object        //
// @param agent Pointer to hsa agent                                         //
// @param mode Mode to be set for the kernel execution                       //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if either agent or mode       //
//           value is incorrect                                              //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_set_kernel_execution_mode(
    const hsa_agent_t agent, 
    hsa_ext_tools_kernel_execution_mode_t mode);

//---------------------------------------------------------------------------//
// @brief Get the kernel execution from the trap handler object              //
// @param agent Pointer to hsa agent                                         //
// @param exception_policy Pointer to an exception policy structure          //
// @retval        - HSA_STATUS_SUCCESS if success                            //
//                  HSA_STATUS_ERROR_INVALID_ARGUMENT if pointer to the      //
//                  mode is NULL                                             //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_kernel_execution_mode(
    const hsa_agent_t agent, 
    hsa_ext_tools_kernel_execution_mode_t *mode);

//---------------------------------------------------------------------------//
// @brief Register the debugger on a agent.                                  //
// @details This function is to register the debugger on the agent when a    //
// kernel is in the debug mode. Applications are responsible in ensuring     //
// that they perform Registration only ONCE - a second attempt is illegal    //
// @param agent Pointer to hsa agent                                         //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the agent pointer is NULL  //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_register(const hsa_agent_t agent);

//---------------------------------------------------------------------------//
// @brief Unregister the debugger on a agent.                                //
// @details This function is to unregister the debugger on the agent. It is  //
// illegal to unregister without having successfully registered before.      //
// Once an Agent has successfully unregistered, it is illegal to perform a   //
// second unregistration.                                                    //
// @param agent Pointer to hsa agent                                         //
// @retval        - HSA_STATUS_SUCCESS if success                            //
//                  HSA_STATUS_ERROR_INVALID_ARGUMENT if the agent pointer   //
//                  is NULL                                                  //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_unregister(const hsa_agent_t agent);

//---------------------------------------------------------------------------//
// @brief Debug address watch.                                               //
// @details This function is to sets watch points on memory address ranges   //
// to generate exception events when the watched addresses are accessed.     //
// Overall, it just passes all the arguments to the driver API to perform    //
// the actual operations.                                                    //
// @param agent Pointer to a hsa agent                                       //
// @param num_watch_points number of points to be watched.                   //
// @param watch_mode Pointer to the array of modes for each watch point      //
// @param watch_address Pointer to the array of addresses to be watched      //
// @param watch_mask Pointer to the array of masks for each watch point      //
// @param watch_event Pointer to the array of watch events corresponding     //
// to each watch point. This parameter is optional and needs to be NULL      //
// in the current implementation.                                            //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if pointer to the mode        //
//           is NULL                                                         //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_address_watch(const hsa_agent_t agent,
                              uint32_t num_watch_points,
                              hsa_ext_tools_address_watch_mode_t *mode,
                              void **watch_address,
                              uint64_t *watch_mask,
                              hsa_ext_tools_event_t *watch_event);

//---------------------------------------------------------------------------//
// @brief Function for getting the dispatch debug info, which includes       //
//        the scratch memory descriptor and the global meomry descriptor     //
// @note In Debug mode the memory used by kernel for various entities        //
// should be of type system memory since it should be accessible to          //
// both Cpu and Gpu.                                                         //
// @param agent Pointer to hsa agent                                         //
// @param info - Pointer to the structor containing the dispatch debug       //
//        info. In GPU, the pointer points to a structure DispatchDebugInfo  //
//        as defined at the begining.                                        //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if debug_info pointer is NULL //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_dispatch_debug_info(
    const hsa_agent_t agent, 
    hsa_ext_tools_dispatch_debug_info_t *info);

//---------------------------------------------------------------------------//
// @brief Function to copy data from one memory location to another memory   //
// without regard to the source and destination being device local memory or //
// host accessible system memory. If either the source or destination        //
// buffers are in device local memory then they must be accessible to the    //
// Hsa agent given as input parameter.                                       //
//                                                                           //
// @param agent Pointer to a agent hsa agent structure                       //
//                                                                           //  
// @param src_addr Pointer to the buffer containing data to be copied from   //
//                                                                           //
// @param dst_addr Pointer to the buffer for the data to be copied to        //
//                                                                           //
// @param size number of bytes to be copied from source buffer               //
//                                                                           //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if any argument is invalid    //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API
  hsa_ext_tools_dmacopy(const hsa_agent_t agent,
                        uint32_t *src_addr,
                        uint32_t *dst_addr, uint32_t size);

//---------------------------------------------------------------------------//
// @brief Function to create a debug event. In the current implementation,   //
// only one debug event is supported per process. Creation of multiple debug //
// events will fail, and corrsponding error message is reported.             //
// @param agent Pointer to a agent hsa agent structure                       //
// @param auto_reset Whether an event is automatically reset after it is     //
//        signaled.                                                          //
// @param event - Pointer to the debugger event handle                       //
// @param event - Pointer to the debugger event ID                           //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if any argument is NULL       //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API
  hsa_ext_tools_create_event(const hsa_agent_t agent,
                                bool auto_reset,
                                hsa_ext_tools_event_t *event,
                                uint32_t *event_id);

//---------------------------------------------------------------------------//
// @brief Wait on a debug to be signaled.                                    //
// @details This is a blocking wait on an event to complete.                 //
// @param timeout The timeout period of this functional call.                //
// @param event Event handle                                                 //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if any argument is invalid    //
//---------------------------------------------------------------------------//
hsa_ext_tools_event_wait_status_t HSA_TOOLS_API 
  hsa_ext_tools_wait_event(int32_t timeout, hsa_ext_tools_event_t event);

//---------------------------------------------------------------------------//
// @brief Destroy the debug event object                                     //
// @param event Handle of the debug event                                    //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the event is NULL          //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_destroy_event(hsa_ext_tools_event_t event);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // _HSA_EXT_DEBUGGER_H_
