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

#ifndef _HSA_EXT_PROFILER_H_
#define _HSA_EXT_PROFILER_H_

#include "hsa.h"
#include "hsa_ext_amd.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

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
// @brief Enumeration of various information that is set for a counter.      //
// @detail This enumeration defines the various counter info that could be   //
//         used in a counter. This is used by a counter object to specify    //
//         its type and other conditions that are needed to retrieve a       //
//         counter value.                                                    //
//---------------------------------------------------------------------------//
typedef enum hsa_ext_tools_counter_parameter_s {
  // Event index of a counter
  HSA_EXT_TOOLS_COUNTER_PARAMETER_EVENT_INDEX = 0,

  // Simd mask of a counter
  HSA_EXT_TOOLS_COUNTER_PARAMETER_SIMD_MASK = 1,

  // Shader engine mask of a counter
  HSA_EXT_TOOLS_COUNTER_PARAMETER_SHADER_MASK = 2,

  // Max counter info index
  HSA_EXT_TOOLS_COUNTER_PARAMETER_INFO_MAX
} hsa_ext_tools_counter_parameter_t;

//---------------------------------------------------------------------------//
// @brief Enumeration of counter block type mask                             //
// @details This enumeration define the bit mask representing types of       //
// counter broup supported by HSA. This is used by counter block object to   //
// specify its type.                                                         //
//---------------------------------------------------------------------------//
typedef enum hsa_ext_tools_counter_block_type_s {
  // Unknown counter block type
  HSA_EXT_TOOLS_COUNTER_BLOCK_TYPE_UNKNOWN = 0,

  // The CounterBlock of this type can be access at anytime.
  // note Examples are software Counters and CPU Counters.
  HSA_EXT_TOOLS_COUNTER_BLOCK_TYPE_SYNC = 1,

  // The CounterBlock type can be access asynchronously.
  // It is required that the Counter must be stopped
  // before accessing.
  HSA_EXT_TOOLS_COUNTER_BLOCK_TYPE_ASYNC = 2,

  // The CounterBlock of this counter block is used for generating
  // trace.
  HSA_EXT_TOOLS_COUNTER_BLOCK_TYPE_TRACE = 3,

  // Max CounterBlock type
  HSA_EXT_TOOLS_COUNTER_BLOCK_TYPE_MAX
} hsa_ext_tools_counter_block_type_t;

//---------------------------------------------------------------------------//
// @brief Enumeration of various information that is set for a counter block.//
// @detail This enumeration defines the various info that could be used      //
// in a counter block. This is used by a counter object to specify its type  //
// and other conditions that are needed for a counter block.                 //
//---------------------------------------------------------------------------//
typedef enum hsa_ext_tools_counter_block_info_s {
  // Index of a counter block
  HSA_EXT_TOOLS_COUNTER_BLOCK_INFO_EVENT_INDEX = 0,

  // Shader bits of a counter block
  HSA_EXT_TOOLS_COUNTER_BLOCK_INFO_SHADER_BITS = 1,

  // Simd mask of a counter
  HSA_EXT_TOOLS_COUNTER_BLOCK_INFO_CONTROL_METHOD = 2,

  // Max index of counter block info
  HSA_EXT_TOOLS_COUNTER_BLOCK_INFO_MAX
} hsa_ext_tools_counter_block_info_t;

//---------------------------------------------------------------------------//
// Enumeration for the methods used to index into the correct registers.    //
//---------------------------------------------------------------------------//
typedef enum hsa_ext_tools_counter_index_method_s {
  // No index
  HSA_EXT_TOOLS_COUNTER_INDEX_METHOD_BY_NONE = 0,

  // Index by block instance
  HSA_EXT_TOOLS_COUNTER_INDEX_METHOD_BY_INSTANCE = 1,

  // Index by shader engine
  HSA_EXT_TOOLS_COUNTER_INDEX_METHOD_BY_SHADER_ENGINE = 2,

  // Index by shader and instance
  HSA_EXT_TOOLS_COUNTER_INDEX_METHOD_BY_SHADER_ENGINE_ANDINSTANCE = 3
} hsa_ext_tools_counter_index_method_t;

//---------------------------------------------------------------------------//
// Enumeration for the HSAPerf generic error codes                           //
//---------------------------------------------------------------------------//
typedef enum hsa_ext_tools_error_codes_s {
  // Successful
  HSA_EXT_TOOLS_ERROR_CODE_OK = 0,

  // Generic error code
  HSA_EXT_TOOLS_ERROR_CODE_ERROR,

  // Generic invalid HSAPerf API arguments
  HSA_EXT_TOOLS_ERROR_CODE_INVALID_ARGS,

  // The operation is not permit due to currently in the unmodifiable
  // HSAPerf state .
  HSA_EXT_TOOLS_ERROR_CODE_UNMODIFIABLE_STATE,

  // The hsa_ext_tools_set_pmu_parameter() or 
  // hsa_ext_tools_get_pmu_parameter() API contains invalid parameter value.
  HSA_EXT_TOOLS_ERROR_CODE_INVALID_PARAM,

  // The hsa_ext_tools_set_pmu_parameter() or 
  // hsa_ext_tools_get_pmu_parameter() API contains invalid parameter size
  // or return size.
  HSA_EXT_TOOLS_ERROR_CODE_INVALID_PARAM_SIZE,

  // The hsa_ext_tools_set_pmu_parameter() or 
  // hsa_ext_tools_get_pmu_parameter() API contains invalid
  // pointer (e.g. NULL).
  HSA_EXT_TOOLS_ERROR_CODE_INVALID_PARAM_DATA,

  // The hsa_ext_tools_get_pmu_info() API contains invalid info value.
  HSA_EXT_TOOLS_ERROR_CODE_INVALID_INFO,

  // The hsa_ext_tools_get_pmu_info() API contains invalid info 
  // size (e.g. zero).
  HSA_EXT_TOOLS_ERROR_CODE_INVALID_INFO_SIZE,

  // The hsa_ext_tools_get_pmu_info() API contains invalid 
  // data (e.g. NULL).
  HSA_EXT_TOOLS_ERROR_CODE_INVALID_INFO_DATA
} hsa_ext_tools_error_codes_t;

//---------------------------------------------------------------------------//
// Enumeration for Pmu profiling state                                       //
//---------------------------------------------------------------------------//
typedef enum hsa_ext_tools_pmu_state_s {
  // Profiling idle. In this state, changes can be made to
  // the PMU, counter blocks, counters. This state can represent
  // the moment prior to calling begin or after calling
  // hsa_ext_tools_pmu_wait_for_completion().
  HSA_EXT_TOOLS_PMU_STATE_IDLE,

  // Profiling start. In this state, changes cannot be made to
  // the PMU, counter block, counters. The PMU is collecting
  // performance counter data. This state represents
  // the moment after calling hsa_ext_tools_pmu_begin() and before calling
  // hsa_ext_tools_pmu_end()
  HSA_EXT_TOOLS_PMU_STATE_START,

  // Profiling stop. In this state, changes cannot be made to
  // the PMU, counter blocks, Counters. PMU has stopped the
  // performance counter data collection. However, the result
  // might not yet be available. This state represents
  // the moment after calling hsa_ext_tools_pmu_end() and before the call
  // to hsa_ext_tools_pmu_wait_for_completion() has returned success.
  HSA_EXT_TOOLS_PMU_STATE_STOP
} hsa_ext_tools_pmu_state_t;

//---------------------------------------------------------------------------//
//  Opaque pointer to HSA performance monitor unit (PMU)                     //
//---------------------------------------------------------------------------//
typedef void *  hsa_ext_tools_pmu_t;

//---------------------------------------------------------------------------//
// Opaque pointer to HSA counter block                                       //
//---------------------------------------------------------------------------//
typedef void *  hsa_ext_tools_counter_block_t;

//---------------------------------------------------------------------------//
// Opaque pointer to HSA counter                                             //
//---------------------------------------------------------------------------//
typedef void *  hsa_ext_tools_counter_t;

//---------------------------------------------------------------------------//
// @brief Create a PMU object on a device                                    //
// @param device Pointer to hsa device                                       //
// @param pmu Pointer to a PMU object                                        //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//          HSA_STATUS_ERROR_INVALID_ARGUMENT if input device or pmu is NULL //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_create_pmu(const hsa_agent_t device,
                           hsa_ext_tools_pmu_t *pmu);

//---------------------------------------------------------------------------//
// @brief Release a PMU object on a device                                   //
// @param pmu Pmu handle                                                     //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the input pmu is NULL      //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_release_pmu(hsa_ext_tools_pmu_t pmu);

//---------------------------------------------------------------------------//
// @brief Get a counter block from the PMU object based on the counter       //
// block ID.                                                                 //
// @param pmu Pmu object handle                                              //
// @param counter_block_id ID of the counter block                           //
// @param counter_block Pointer to the returned counter block                //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the pmu, counter_id, or    //
//           counter_block is NULL.                                          //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_counter_block_by_id(
    const hsa_ext_tools_pmu_t pmu,
    uint32_t counter_block_id,
    hsa_ext_tools_counter_block_t  *counter_block);

//---------------------------------------------------------------------------//
// @brief Get all the counter blocks in the PMU                              //
// @param pmu Pmu object handle                                              //
// @param counter_blocks Pointer to the returned counter block list          //
// @param counter_block_num Number of the returned counter blocks            //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if input pmu, counter_blocks, //
//           or counter_block_num is NULL.                                   //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_all_counter_blocks(
    const hsa_ext_tools_pmu_t pmu,
    hsa_ext_tools_counter_block_t **counter_blocks,
    uint32_t *counter_block_num);

//---------------------------------------------------------------------------//
// @brief Get the PMU profiling state                                        //
// @param pmu Pmu object handle                                              //
// @param state Pointer to the state of the PMU                              //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the input pmu or state     //
//           is NULL.                                                        //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_pmu_state(
    const hsa_ext_tools_pmu_t pmu, 
    hsa_ext_tools_pmu_state_t *state);

//---------------------------------------------------------------------------//
// @brief Start profiling on the PMU                                         //
// @param pmu Pmu object handle                                              //
// @param queue Pointer to the queue that the counter is retrieved.          //
// @param aql_translation_handle Pointer to the location that the perf       //
//        counter packet is written to.                                      //
// @param queue reset_counter Whether reset the counter register before      //
//         profiling.                                                        //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the input pmu or queue     //
//           is NULL.                                                        //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_pmu_begin(
    const hsa_ext_tools_pmu_t pmu,
    const hsa_queue_t *queue,
    void *aql_translation_handle,
    bool reset_counter);

//---------------------------------------------------------------------------//
// @brief Stop profiling on the PMU. Note: This function must be called      //
//        after hsa_ext_tools_pmu_end().                                     //
// @param pmu Pmu object handle                                              //
// @param queue Pointer to the queue that the counter is retrieved.          //
// @param aql_translation_handle Pointer to the location that the perf       //
//        counter packet is written to.                                      //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the input pmu or queue     //
//           is NULL.                                                        //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_pmu_end(
    const hsa_ext_tools_pmu_t pmu, 
    const hsa_queue_t *queue,
	void *aql_translation_handle);

//---------------------------------------------------------------------------//
// @brief Wait a given number of milliseconds for the PMU to be stopped.     //
// This function must be called after hsa_ext_tools_pmu_end().               //
// @param pmu Pmu object handle                                              //
// @param milli_seconds The maximum number of milliseconds to wait for the   //
// results to become available.                                              //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if if the input pmu is NULL.  //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_pmu_wait_for_completion(
    const hsa_ext_tools_pmu_t pmu,
    uint32_t milli_seconds);

//---------------------------------------------------------------------------//
// @brief Set value for the parameter specified by param in a PMU            //
// @param pmu Pmu object handle                                              //
// @param param The enumeration of parameter to be set                       //
// @param param_size Size of the parameter                                   //
// @param p_data Pointer to the argument                                     //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if any input argument is      //
//           invalid.                                                        //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_set_pmu_parameter(
    const hsa_ext_tools_pmu_t pmu,
    uint32_t param,
    uint32_t param_size,
    void *p_data);

//---------------------------------------------------------------------------//
// @brief Query value of the parameter specified by param in a PMU           //
// @param pmu Pmu object handle                                              //
// @param param The enumeration of parameter to be retrieved                 //
// @param param_size Pointer to the size of the parameter to be returned     //
// @param p_data Pointer to the returned parameter buffer                    //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if any input argument is      //
//           invalid.                                                        //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_pmu_parameter(
    const hsa_ext_tools_pmu_t pmu,
    uint32_t param,
    uint32_t *param_size,
    void **p_data);

//---------------------------------------------------------------------------//
// @brief Query value of the information specified by info in a PMU          //
// @param pmu Pmu object handle                                              //
// @param info The enumeration of information to be queried                  //
// @param info_size Pointer to the size of the information to be returned    //
// @param p_data Pointer to the returned information buffer                  //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if any input argument is NULL.//
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_pmu_info(
    const hsa_ext_tools_pmu_t pmu,
    uint32_t info,
    uint32_t *info_size,
    void **p_data);

//---------------------------------------------------------------------------//
// @brief Create a counter in the counter block                              //
// @param counter_block Counter block handle                                 //
// @param counter Pointer to the returned counter handle                     //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if any input argument is NULL.//
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_create_counter(
    const hsa_ext_tools_counter_block_t counter_block,
    hsa_ext_tools_counter_t *counter);

//---------------------------------------------------------------------------//
// @brief Destroy a counter in a counter block                               //
// @param counter_block Counter block handle                                 //
// @param counter Counter handle                                             //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the input counter_block or //
//           counter i sNULL.                                                //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_destroy_counter(
    const hsa_ext_tools_counter_block_t counter_block,
    hsa_ext_tools_counter_t counter);

//---------------------------------------------------------------------------//
// @brief Destroy alll counter in a counter block. The counter block must    //
// be in the disable state.                                                  //
// @param counter_block Counter block handle                                 //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the input counter_block is //
//           NULL.                                                           //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_destroy_all_counters(
    const hsa_ext_tools_counter_block_t counter_block);

//---------------------------------------------------------------------------//
// @brief Get a list of pointers to the enabled counters in this counter     //
// block. The returned counters are created in the same counter block object //
// using hsa_ext_tools_create_counter().                                     //
// @param counter_block Counter block handle                                 //
// @param counters Pointers to a list of counter handles                     //
// @param counter_num Number of counters returned.                           //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if counter_bloc, couters, or  //
//           counter_num is NULL.                                            //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_enabled_counters(
    const hsa_ext_tools_counter_block_t counter_block,
    hsa_ext_tools_counter_t **counters,
    uint32_t *counter_num);

//---------------------------------------------------------------------------//
// @brief Get a list of pointers to the all counters in this counter block.  //
// The returned counters are created in the same counter block object using  //
// hsa_ext_tools_create_counter().                                           //
// @param counter_block Counter block handle                                 //
// @param counters Pointers to a list of counters                            //
// @param counter_num Number of counters returned.                           //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the input counter_block,   //
//           couters, or counter_num is NULL.                                //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_all_counters(
    const hsa_ext_tools_counter_block_t counter_block,
    hsa_ext_tools_counter_t **counters,
    uint32_t *counter_num);

//---------------------------------------------------------------------------//
// @brief Set value for the parameter specified by param in the counter      //
//        block                                                              //
// @param counter_block Counter block object handle                          //
// @param param The enumeration of parameter to be set                       //
// @param param_size Size of the parameter                                   //
// @param p_data Pointer to the argument                                     //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if any input argument is NULL.//
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_set_counter_block_parameter(
    const hsa_ext_tools_counter_block_t counter_block,
    uint32_t param,
    uint32_t param_size,
    void *p_data);

//---------------------------------------------------------------------------//
// @brief Query value of the parameter specified by param in a counter block //
// @param counter_block Counter block object handle                          //
// @param param The enumeration of parameter to be retrieved                 //
// @param param_size Pointer to the size of the parameter to be returned     //
// @param p_data Pointer to the returned parameter buffer                    //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if any input argument is NULL.//
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_counter_block_parameter(
    const hsa_ext_tools_counter_block_t counter_block,
    uint32_t param,
    uint32_t *param_size,
    void **p_data);

//---------------------------------------------------------------------------//
// @brief Query value of the information specified by param in a counter     //
//        block                                                              //
// @param counter_block Counter block object handle                          //
// @param param The enumeration of information to be queried                 //
// @param info_size Pointer to the size of the information to be returned    //
// @param p_data Pointer to the returned information buffer                  //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if any input argument is NULL.//
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_counter_block_info(
    const hsa_ext_tools_counter_block_t counter_block,
    uint32_t param,
    uint32_t *info_size,
    void **p_data);

//---------------------------------------------------------------------------//
// @brief Get the counter block corresponding to a a counter                 //
// @param counter Counter object handle                                      //
// @param counter_block Pointer to the handle of the returned counter block  //
// @retval        - HSA_STATUS_SUCCESS if success                            //
//                  HSA_INVALID_ARG if counter or counter_block is NULL.     //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_counter_block(
    const hsa_ext_tools_counter_t counter,
    hsa_ext_tools_counter_block_t *counter_block);

//---------------------------------------------------------------------------//
// @brief Enable/disable the counter to be retrieved                         //
// @param counter Counter object handle                                      //
// @param enabled indicate whether a counter is enabled.                     //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the input counter is NULL  //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_set_counter_enabled(
    const hsa_ext_tools_counter_t counter,
    bool enabled);

//---------------------------------------------------------------------------//
// @brief Check whether a counter is enabled                                 //
// @param counter Counter object handle                                      //
// @param enabled Pointer to a flag to indicate whether a counter is enabled.//
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the input couter is NULL.  //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_is_counter_enabled(
    const hsa_ext_tools_counter_t counter,
    bool *enabled);

//---------------------------------------------------------------------------//
// @brief Return the status of this counter whether the result is available. //
// @param counter Counter object handle                                      //
// @param ready Pointer to a flag to indicate whether the result is ready.   //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the input couter           //
//           or ready flag is NULL.                                          //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_is_counter_result_ready(
    const hsa_ext_tools_counter_t counter,
    bool *ready);

//---------------------------------------------------------------------------//
// @brief Return the status of this counter whether the result is available. //
// @param counter Counter object handle                                      //
// @param result Pointer to a variable where is counter result is returned.  //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if the input couter or ready  //
//           flag is NULL.                                                   //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_counter_result(
    const hsa_ext_tools_counter_t counter,
    uint64_t *result);

//---------------------------------------------------------------------------//
// @brief Set value for the parameter specified by param in a counter        //
// @param counter Counter object handle                                      //
// @param param The enumeration of parameter to be set                       //
// @param param_size Size of the parameter                                   //
// @param p_data Pointer to the argument                                     //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if any input argument is NULL.//
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_set_counter_parameter(
    const hsa_ext_tools_counter_t counter,
    uint32_t param,
    uint32_t param_size,
    void *p_data);

//---------------------------------------------------------------------------//
// @brief Query value of the parameter specified by param in a counter       //
// @param counter Counter object handle                                      //
// @param param The enumeration of parameter to be retrieved                 //
// @param param_size Pointer to the size of the parameter to be returned     //
// @param p_data Pointer to the returned parameter buffer                    //
// @retval - HSA_STATUS_SUCCESS if success                                   //
//           HSA_STATUS_ERROR_INVALID_ARGUMENT if any input argument is NULL.//
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API 
  hsa_ext_tools_get_counter_parameter(
    const hsa_ext_tools_counter_t counter,
    uint32_t param,
    uint32_t *param_size,
    void **p_data);

//---------------------------------------------------------------------------//
// @brief Support for kernel execution time profiling                        //
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// @brief Definition of kernel execution time records                        //
//---------------------------------------------------------------------------//
typedef struct hsa_profiler_kernel_time_s
{
	hsa_amd_profiling_dispatch_time_t time;
	uint64_t kernel;
	hsa_agent_t agent;
	const hsa_queue_t *queue;
} hsa_profiler_kernel_time_t;

/**
 * @brief Structure encapsulating information about the AQL packet being
 * enqueued. Users can query the "type" field to distinguish between the
 * various types of AQL packets that a user can submit.
 */
typedef struct hsa_aql_trace_s {

  // Specifies the AQL packet type
  hsa_packet_type_t type;

  // Hsa AQL packet instance
  void * packet;

  // Hsa Agent associated with AQL packet
  hsa_agent_t agent;

  // Hsa Queue associated with AQL packet
  hsa_queue_t *queue;

  // Monotonically increasing numeric identifier
  // of an Aql packet.
  uint64_t packet_id;

  // Default constructor of struct
  hsa_aql_trace_s(const hsa_agent_t agent) : agent(agent) { }

} hsa_aql_trace_t;

/**
 * @brief Definition of callback that could be used to register for AQL
 * packet tracing.
 *
 * @note: This callback will be invoked before the Aql packet is submitted
 * for execution and therefore it is possible to amend one or more parameters
 * of the dispatch as appropriate. For example, the callback could substitute user
 * given completion signal with its own signal.
 *
 * @param aql_trace Provides callback implementations with information
 * on the agent and queue on which the AQL packet was submitted.
 *
 * @param user_arg handle of user data provided during callback registration.
 *
 * @return hsa_status_t HSA_STATUS_SUCCESS if user is interested in further
 * callbacks. A value other than HSA_STATUS_SUCCESS will cause callback to be
 * deregistered.
 */
typedef hsa_status_t
  (*hsa_ext_tools_aql_trace_callback)
        (const hsa_aql_trace_t *aql_trace, void *user_arg);

/**
 * @brief Api for users to register a callback that will be invoked once per
 * each AQL packet being submitted. It is upto users as to how they wish to
 * use the AQL packet that is passed as a parameter of the callback, as long
 * as they do not make it illegal from being executed. Only one callback is
 * allowed to be registered as it can update the Aql packet that will be
 * submitted for execution. Allowing more callbacks will complicate the logic
 * as to whose update should be used for submission.
 *
 * @note: At present an Api to deregister callback is not specified. Instead
 * users can return a value that is not HSA_STATUS_SUCCESS to indicate callback
 * be deregistered
 *
 * @param queue handle of the Queue that was created using Hsa tools Api -
 * hsa_ext_tools_queue_create_profiled.
 *
 * @param callback_arg pointer to data block that will be passed as a parameter
 * when callback is invoked.
 *
 * @param aql_trac Callback function to register. Hsa Tool library will invoke
 * this callback once per AQL packet.
 *
 * @return hsa_status_t status of callback registration.
 */
hsa_status_t HSA_TOOLS_API
  hsa_ext_tools_register_aql_trace_callback(
                             hsa_queue_t *queue,
                             void *callback_arg,
                             hsa_ext_tools_aql_trace_callback aql_trace_func);

//---------------------------------------------------------------------------//
// @brief Creates a profiled queue.  All dispatches on this queue will be    //
// profiled.                                                                 //
//---------------------------------------------------------------------------//
hsa_status_t HSA_TOOLS_API hsa_ext_tools_queue_create_profiled(
    hsa_agent_t agent_handle, uint32_t size, hsa_queue_type_t type,
    void (*callback)(hsa_status_t status, hsa_queue_t* source, void* data),
    void* data, uint32_t private_segment_size, uint32_t group_segment_size,
    hsa_queue_t** queue);

//---------------------------------------------------------------------------//
// @brief Collects profiled kernel times.
// @param count Fetches at most count records.
// @param records Output buffer for kernel timings.
// @retval Returns the number of timings actually copied if records is not NULL.
// If records is NULL returns the approximate number of kernel timings available
// for retrieval.
//
// @note: If the env variable HSA_SERVICE_GET_KERNEL_TIMES is set to ZERO the
// Api will return Zero. This truyl does not capture indication of programming
// error but is left as is as this Api is deprecated and will be removed in
// near future
//---------------------------------------------------------------------------//
size_t HSA_TOOLS_API hsa_ext_tools_get_kernel_times(size_t count, hsa_profiler_kernel_time_t* records);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // HSA_EXT_TOOLS_H
