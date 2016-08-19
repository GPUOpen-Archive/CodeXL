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

#ifndef _AMD_HSA_TOOLS_INTERFACES_H_
#define _AMD_HSA_TOOLS_INTERFACES_H_

#include "hsa.h"

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

/**
 * @brief Struct encapsulating pre and post dispatch callback
 * parameters. Users can query the boolean flag "pre_dispatch"
 * to distinguish if an instance is related to pre or post
 * dispatch. The flag is set to TRUE for pre dispatch callbacks.
 */
typedef struct hsa_dispatch_callback_s {
  
  // Set to true if instance represents a
  // pre dispatch callback parameter
  bool pre_dispatch;

  // Hsa Agent associated with dispatch
  hsa_agent_t agent;

  // Hsa Queue associated with dispatch
  hsa_queue_t *queue;

  // Handle of an instance of PM4Buffer class
  void *aql_translation_handle;

  // User supplied callback parameter.
  //
  // @note: Currently this is initialized via an
  // explicit Api. The existence of this parameter
  // suggests the current interface is not complete.
  void *correlation_handle;

  // Monotonically increasing numeric identifier of
  // a Aql Dispatch packet. It must be emphasized that
  // every Aql packet is associated with an Id, not just
  // those that dispatch a kernel.
  uint64_t packet_id;

  // Aql dispatch packet associated with dispatch
  hsa_kernel_dispatch_packet_t *aql_packet;

  // Hsa Completion Signal associated with dispatch
  hsa_signal_t signal;

  // Default constructor of structure
  hsa_dispatch_callback_s(const hsa_agent_t agent) : agent(agent) { }
 
} hsa_dispatch_callback_t;

//---------------------------------------------------------------------------//
// Pre-dispatch call back function signature. This is the call back function //
// before the kernel dispatch. The call back function is to indicate the     //
// start of the kernel launch. It is used by the debugger and profiler.      //
//---------------------------------------------------------------------------//
typedef void
  (*hsa_ext_tools_pre_dispatch_callback_function)
    (const hsa_dispatch_callback_t *hsart_param, void *user_args);

//---------------------------------------------------------------------------//
// Post-dispatch call back function signature. The post dispatch callback    //
// function is called after the kernel dispatch. The callback function is to //
// indicate the completion of the the kernel launch. It is used by the       //
// debugger and profiler.                                                    //
//---------------------------------------------------------------------------//
typedef void
  (*hsa_ext_tools_post_dispatch_callback_function)
    (const hsa_dispatch_callback_t *hsart_param, void *user_args);

//---------------------------------------------------------------------------//
// @brief Setup the callback function pointers                               //
// @param queue Pointer to queue that the callback functions are bind to     //
// @param pre_dispatch_function Pointer to predispatch callback function     //
// @param post_dispatch_function Pointer to postdispatch callback function   //
// @retval        - HSA_STATUS_SUCCESS                                       //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API hsa_ext_tools_set_callback_functions(
  hsa_queue_t *queue,
  hsa_ext_tools_pre_dispatch_callback_function pre_dispatch_function,
  hsa_ext_tools_post_dispatch_callback_function post_dispatch_function);

//---------------------------------------------------------------------------//
// @brief Query the callback function pointers                               //
// @param queue Pointer to queue that the callback functions are bind to     //
// @param pre_dispatch_function Pointer to predispatch callback function     //
// @param post_dispatch_function Pointer to postdispatch callback function   //
// @retval        - HSA_STATUS_SUCCESS                                       //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API hsa_ext_tools_get_callback_functions(
    hsa_queue_t *queue,
    hsa_ext_tools_pre_dispatch_callback_function &pre_dispatch_function,
    hsa_ext_tools_post_dispatch_callback_function &post_dispatch_function);

//---------------------------------------------------------------------------//
// @brief Setup the call back function argument pointers                     //
// @param queue Pointer to queue that the callback functions are bind to     //
// @param pre_dispatch_args Pointer to predispatch callback function         //
// arguments                                                                 //
// @param post_dispatch_args Pointer to postdispatch callback function       //
// arguments                                                                 //
// @retval        - HSA_STATUS_SUCCESS                                       //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API hsa_ext_tools_set_callback_arguments(
  hsa_queue_t *queue,
  void *pre_dispatch_args,
  void *post_dispatch_args);

//---------------------------------------------------------------------------//
// @brief Query the call back function argument pointers                     //
// @param queue Pointer to queue that the callback functions are bind to     //
// @param pre_dispatch_args Pointer to predispatch callback function         //
// arguments                                                                 //
// @param post_dispatch_args Pointer to postdispatch callback function       //
// arguments                                                                 //
// @retval        - HSA_STATUS_SUCCESS                                       //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API hsa_ext_tools_get_callback_arguments(
    hsa_queue_t *queue,
    void * &pre_dispatch_args,
    void * &post_dispatch_args);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // _AMD_HSA_TOOLS_INTERFACES_H_
