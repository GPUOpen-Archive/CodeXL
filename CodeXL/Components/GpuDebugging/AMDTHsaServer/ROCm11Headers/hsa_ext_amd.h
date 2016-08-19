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

// HSA AMD extension.

#ifndef HSA_RUNTIME_EXT_AMD_H_
#define HSA_RUNTIME_EXT_AMD_H_

#include "hsa.h"
#include "hsa_ext_image.h"

#define HSA_AMD_INTERFACE_VERSION_MAJOR 1
#define HSA_AMD_INTERFACE_VERSION_MINOR 0

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration constants added to ::hsa_status_t.
 *
 * @remark Additions to hsa_status_t
 */
enum {
  /**
   * The memory pool is invalid.
   */
  HSA_STATUS_ERROR_INVALID_MEMORY_POOL = 40
};

/**
 * @brief Agent attributes.
 */
typedef enum hsa_amd_agent_info_s {
  /**
   * Chip identifier. The type of this attribute is uint32_t.
   */
  HSA_AMD_AGENT_INFO_CHIP_ID = 0xA000,
  /**
   * Size of a cacheline in bytes. The type of this attribute is uint32_t.
   */
  HSA_AMD_AGENT_INFO_CACHELINE_SIZE = 0xA001,
  /**
   * The number of compute unit available in the agent. The type of this
   * attribute is uint32_t.
   */
  HSA_AMD_AGENT_INFO_COMPUTE_UNIT_COUNT = 0xA002,
  /**
   * The maximum clock frequency of the agent in MHz. The type of this
   * attribute is uint32_t.
   */
  HSA_AMD_AGENT_INFO_MAX_CLOCK_FREQUENCY = 0xA003,
  /**
   * Internal driver node identifier. The type of this attribute is uint32_t.
   */
  HSA_AMD_AGENT_INFO_DRIVER_NODE_ID = 0xA004,
  /**
   * Max number of watch points on memory address ranges to generate exception
   * events when the watched addresses are accessed.
   */
  HSA_AMD_AGENT_INFO_MAX_ADDRESS_WATCH_POINTS = 0xA005,
  /**
   * Agent BDF_ID, named LocationID in thunk. The type of this attribute is
   * uint16_t.
   */
  HSA_AMD_AGENT_INFO_BDFID = 0xA006,
  /**
   * Memory Interface width, the return value type is uint32_t.
   * This attribute is deprecated. Use
   */
  HSA_AMD_AGENT_INFO_MEMORY_WIDTH = 0xA007,
  /**
   * Max Memory Clock, the return value type is uint32_t.
   */
  HSA_AMD_AGENT_INFO_MEMORY_MAX_FREQUENCY = 0xA008
} hsa_amd_agent_info_t;

/**
 * @brief Region attributes.
 */
typedef enum hsa_amd_region_info_s {
  /**
   * Determine if host can access the region. The type of this attribute
   * is bool.
   */
  HSA_AMD_REGION_INFO_HOST_ACCESSIBLE = 0xA000,
  /**
   * Base address of the region in flat address space.
   */
  HSA_AMD_REGION_INFO_BASE = 0xA001,
  /**
   * Memory Interface width, the return value type is uint32_t.
   * This attribute is deprecated. Use HSA_AMD_AGENT_INFO_MEMORY_WIDTH.
   */
  HSA_AMD_REGION_INFO_BUS_WIDTH = 0xA002,
  /**
   * Max Memory Clock, the return value type is uint32_t.
   * This attribute is deprecated. Use HSA_AMD_AGENT_INFO_MEMORY_MAX_FREQUENCY.
   */
  HSA_AMD_REGION_INFO_MAX_CLOCK_FREQUENCY = 0xA003
} hsa_amd_region_info_t;

/**
 * @brief Coherency attributes of fine grain region.
 */
typedef enum hsa_amd_coherency_type_s {
  /**
   * Coherent region.
   */
  HSA_AMD_COHERENCY_TYPE_COHERENT = 0,
  /**
   * Non coherent region.
   */
  HSA_AMD_COHERENCY_TYPE_NONCOHERENT = 1
} hsa_amd_coherency_type_t;

/**
 * @brief Get the coherency type of the fine grain region of an agent.
 *
 * @param[in] agent A valid agent.
 *
 * @param[out] type Pointer to a memory location where the HSA runtime will
 * store the coherency type of the fine grain region.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p type is NULL.
 */
hsa_status_t HSA_API hsa_amd_coherency_get_type(hsa_agent_t agent,
                                                hsa_amd_coherency_type_t* type);

/**
 * @brief Set the coherency type of the fine grain region of an agent.
 * Deprecated.  This is supported on KV platforms.  For backward compatibility
 * other platforms will spuriously succeed.
 *
 * @param[in] agent A valid agent.
 *
 * @param[in] type The coherency type to be set.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p type is invalid.
 */
hsa_status_t HSA_API hsa_amd_coherency_set_type(hsa_agent_t agent,
                                                hsa_amd_coherency_type_t type);

/**
 * @brief Structure containing profiling dispatch time information.
 *
 * Times are reported as ticks in the domain of the HSA system clock.
 * The HSA system clock tick and frequency is obtained via hsa_system_get_info.
 */
typedef struct hsa_amd_profiling_dispatch_time_s {
  /**
   * Dispatch packet processing start time.
   */
  uint64_t start;
  /**
   * Dispatch packet completion time.
   */
  uint64_t end;
} hsa_amd_profiling_dispatch_time_t;

/**
 * @brief Enable or disable profiling capability of a queue.
 *
 * @param[in] queue A valid queue.
 *
 * @param[in] enable 1 to enable profiling. 0 to disable profiling.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_QUEUE The queue is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p queue is NULL.
 */
hsa_status_t HSA_API
    hsa_amd_profiling_set_profiler_enabled(hsa_queue_t* queue, int enable);

/**
 * @brief Retrieve packet processing time stamps.
 *
 * @param[in] agent The agent with which the signal was last used.  For instance,
 * if the profiled dispatch packet is dispatched on to queue Q, which was
 * created on agent A, then this parameter must be A.
 *
 * @param[in] signal A signal used as the completion signal of the dispatch
 * packet to retrieve time stamps from.  This dispatch packet must have been
 * issued to a queue with profiling enabled and have already completed.  Also
 * the signal must not have yet been used in any other packet following the
 * completion of the profiled dispatch packet.
 *
 * @param[out] time Packet processing timestamps in the HSA system clock
 * domain.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_SIGNAL The signal is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p time is NULL.
 */
hsa_status_t HSA_API hsa_amd_profiling_get_dispatch_time(
    hsa_agent_t agent, hsa_signal_t signal,
    hsa_amd_profiling_dispatch_time_t* time);

/**
 * @brief Computes the frequency ratio and offset between the agent clock and
 * HSA system clock and converts the agent’s tick to HSA system domain tick.
 *
 * @param[in] agent The agent used to retrieve the agent_tick. It is user's
 * responsibility to make sure the tick number is from this agent, otherwise,
 * the behavior is undefined.
 *
 * @param[in] agent_tick The tick count retrieved from the specified @p agent.
 *
 * @param[out] system_tick The translated HSA system domain clock counter tick.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p system_tick is NULL;
 */
hsa_status_t HSA_API
    hsa_amd_profiling_convert_tick_to_system_domain(hsa_agent_t agent,
                                                    uint64_t agent_tick,
                                                    uint64_t* system_tick);

/**
 * @brief Asyncronous signal handler function type.
 *
 * @details Type definition of callback function to be used with
 * hsa_amd_signal_async_handler. This callback is invoked if the associated
 * signal and condition are met. The callback receives the value of the signal
 * which satisfied the associated wait condition and a user provided value. If
 * the callback returns true then the callback will be called again if the
 * associated signal and condition are satisfied again. If the callback returns
 * false then it will not be called again.
 *
 * @param[in] value Contains the value of the signal observed by
 * hsa_amd_signal_async_handler which caused the signal handler to be invoked.
 *
 * @param[in] arg Contains the user provided value given when the signal handler
 * was registered with hsa_amd_signal_async_handler
 *
 * @retval true resumes monitoring the signal with this handler (as if calling
 * hsa_amd_signal_async_handler again with identical parameters)
 *
 * @retval false stops monitoring the signal with this handler (handler will
 * not be called again for this signal)
 *
 */
typedef bool (*hsa_amd_signal_handler)(hsa_signal_value_t value, void* arg);

/**
 * @brief Register asynchronous signal handler function.
 *
 * @details Allows registering a callback function and user provided value with
 * a signal and wait condition. The callback will be invoked if the associated
 * signal and wait condition are satisfied. Callbacks will be invoked serially
 * but in an arbitrary order so callbacks should be independent of each other.
 * After being invoked a callback may continue to wait for its associated signal
 * and condition and, possibly, be invoked again. Or the callback may stop
 * waiting. If the callback returns true then it will continue waiting and may
 * be called again. If false then the callback will not wait again and will not
 * be called again for the associated signal and condition. It is possible to
 * register the same callback multiple times with the same or different signals
 * and/or conditions. Each registration of the callback will be treated entirely
 * independently.
 *
 * @param[in] signal hsa signal to be asynchronously monitored
 *
 * @param[in] cond condition value to monitor for
 *
 * @param[in] value signal value used in condition expression
 *
 * @param[in] handler asynchronous signal handler invoked when signal's
 * condition is met
 *
 * @param[in] arg user provided value which is provided to handler when handler
 * is invoked
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_SIGNAL signal is not a valid hsa_signal_t
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT handler is invalid (NULL)
 *
 * @retval ::HSA_STATUS_ERROR_OUT_OF_RESOURCES The HSA runtime is out of
 * resources or blocking signals are not supported by the HSA driver component.
 *
 */
hsa_status_t HSA_API
    hsa_amd_signal_async_handler(hsa_signal_t signal,
                                 hsa_signal_condition_t cond,
                                 hsa_signal_value_t value,
                                 hsa_amd_signal_handler handler, void* arg);

/**
 * @brief Call a function asynchronously
 *
 * @details Provides access to the runtime's asynchronous event handling thread
 * for general asynchronous functions.  Functions queued this way are executed
 * in the same manner as if they were a signal handler who's signal is
 * satisfied.
 *
 * @param[in] callback asynchronous function to be invoked
 *
 * @param[in] arg user provided value which is provided to handler when handler
 * is invoked
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT handler is invalid (NULL)
 *
 * @retval ::HSA_STATUS_ERROR_OUT_OF_RESOURCES The HSA runtime is out of
 * resources or blocking signals are not supported by the HSA driver component.
 *
 */
hsa_status_t HSA_API
    hsa_amd_async_function(void (*callback)(void* arg), void* arg);

/**
 * @brief Wait for any signal-condition pair to be satisfied.
 *
 * @details Allows waiting for any of several signal and conditions pairs to be
 * satisfied. The function returns the index into the list of signals of the
 * first satisfying signal-condition pair. The value of the satisfying signal’s
 * value is returned in satisfying_value unless satisfying_value is NULL. This
 * function provides only relaxed memory semantics.
 */
uint32_t HSA_API
    hsa_amd_signal_wait_any(uint32_t signal_count, hsa_signal_t* signals,
                            hsa_signal_condition_t* conds,
                            hsa_signal_value_t* values, uint64_t timeout_hint,
                            hsa_wait_state_t wait_hint,
                            hsa_signal_value_t* satisfying_value);

/**
 * @brief Query image limits.
 *
 * @param[in] agent A valid agent.
 *
 * @param[in] attribute HSA image info attribute to query.
 *
 * @param[out] value Pointer to an application-allocated buffer where to store
 * the value of the attribute. If the buffer passed by the application is not
 * large enough to hold the value of @p attribute, the behavior is undefined.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_QUEUE @p value is NULL or @p attribute <
 * HSA_EXT_AGENT_INFO_IMAGE_1D_MAX_ELEMENTS or @p attribute >
 * HSA_EXT_AGENT_INFO_IMAGE_ARRAY_MAX_LAYERS.
 *
 */
hsa_status_t HSA_API hsa_amd_image_get_info_max_dim(hsa_agent_t agent,
                                                    hsa_agent_info_t attribute,
                                                    void* value);

/**
 * @brief Set a CU affinity to specific queues within the process, this function
 * call is "atomic".
 *
 * @param[in] queue A pointer to HSA queue.
 *
 * @param[in] num_cu_mask_count Size of CUMask bit array passed in.
 *
 * @param[in] cu_mask Bit-vector representing the CU mask.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_QUEUE @p queue is NULL or invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p num_cu_mask_count is not
 * multiple of 32 or @p cu_mask is NULL.
 *
 * @retval ::HSA_STATUS_ERROR failed to call thunk api
 *
 */
hsa_status_t HSA_API hsa_amd_queue_cu_set_mask(const hsa_queue_t* queue,
                                               uint32_t num_cu_mask_count,
                                               const uint32_t* cu_mask);

/**
 * @brief Memory segments associated with a memory pool.
 */
typedef enum {
  /**
   * Global segment. Used to hold data that is shared by all agents.
   */
  HSA_AMD_SEGMENT_GLOBAL = 0,
  /**
   * Read-only segment. Used to hold data that remains constant during the
   * execution of a kernel.
   */
  HSA_AMD_SEGMENT_READONLY = 1,
  /**
   * Private segment. Used to hold data that is local to a single work-item.
   */
  HSA_AMD_SEGMENT_PRIVATE = 2,
  /**
   * Group segment. Used to hold data that is shared by the work-items of a
   * work-group.
   */
  HSA_AMD_SEGMENT_GROUP = 3,
} hsa_amd_segment_t;

/**
 * @brief A memory pool represents physical storage on an agent.
 */
typedef struct hsa_amd_memory_pool_s {
  /**
   * Opaque handle.
   */
  uint64_t handle;
} hsa_amd_memory_pool_t;

typedef enum hsa_amd_memory_pool_global_flag_s {
  /**
   * The application can use allocations in the memory pool to store kernel
   * arguments, and provide the values for the kernarg segment of
   * a kernel dispatch.
   */
  HSA_AMD_MEMORY_POOL_GLOBAL_FLAG_KERNARG_INIT = 1,
  /**
   * Updates to memory in this pool conform to HSA memory consistency model.
   * If this flag is set, then ::HSA_AMD_MEMORY_POOL_GLOBAL_FLAG_COARSE_GRAINED
   * must not be set.
   */
  HSA_AMD_MEMORY_POOL_GLOBAL_FLAG_FINE_GRAINED = 2,
  /**
   * Writes to memory in this pool can be performed by a single agent at a time.
   */
  HSA_AMD_MEMORY_POOL_GLOBAL_FLAG_COARSE_GRAINED = 4
} hsa_amd_memory_pool_global_flag_t;

/**
 * @brief Memory pool features.
 */
typedef enum {
  /**
  * Segment where the memory pool resides. The type of this attribute is
  * ::hsa_amd_segment_t.
  */
  HSA_AMD_MEMORY_POOL_INFO_SEGMENT = 0,
  /**
  * Flag mask. The value of this attribute is undefined if the value of
  * ::HSA_AMD_MEMORY_POOL_INFO_SEGMENT is not ::HSA_AMD_SEGMENT_GLOBAL. The type
  * of
  * this attribute is uint32_t, a bit-field of
  * ::hsa_amd_memory_pool_global_flag_t
  * values.
  */
  HSA_AMD_MEMORY_POOL_INFO_GLOBAL_FLAGS = 1,
  /**
  * Size of this pool, in bytes. The type of this attribute is size_t.
  */
  HSA_AMD_MEMORY_POOL_INFO_SIZE = 2,
  /**
  * Indicates whether memory in this pool can be allocated using
  * ::hsa_amd_memory_pool_allocate. The type of this attribute is bool.
  *
  * The value of this flag is always false for memory pools in the group and
  * private segments.
  */
  HSA_AMD_MEMORY_POOL_INFO_RUNTIME_ALLOC_ALLOWED = 5,
  /**
  * Allocation granularity of buffers allocated by
  * ::hsa_amd_memory_pool_allocate
  * in this memory pool. The size of a buffer allocated in this pool is a
  * multiple of the value of this attribute. The value of this attribute is
  * only defined if ::HSA_AMD_MEMORY_POOL_INFO_RUNTIME_ALLOC_ALLOWED is true for
  * this pool. The type of this attribute is size_t.
  */
  HSA_AMD_MEMORY_POOL_INFO_RUNTIME_ALLOC_GRANULE = 6,
  /**
  * Alignment of buffers allocated by ::hsa_amd_memory_pool_allocate in this
  * pool. The value of this attribute is only defined if
  * ::HSA_AMD_MEMORY_POOL_INFO_RUNTIME_ALLOC_ALLOWED is true for this pool, and
  * must be a power of 2. The type of this attribute is size_t.
  */
  HSA_AMD_MEMORY_POOL_INFO_RUNTIME_ALLOC_ALIGNMENT = 7,
  /**
  * This memory_pool can be made directly accessible by all the agents in the
  * system (::hsa_amd_agent_memory_pool_get_info returns
  * ::HSA_AMD_MEMORY_POOL_ACCESS_ALLOWED_BY_DEFAULT for all agents). The type of
  * this attribute is bool.
  */
  HSA_AMD_MEMORY_POOL_INFO_ACCESSIBLE_BY_ALL = 15,
} hsa_amd_memory_pool_info_t;

/**
 * @brief Get the current value of an attribute of a memory pool.
 * 
 * @param[in] memory_pool A valid memory pool.
 * 
 * @param[in] attribute Attribute to query.
 * 
 * @param[out] value Pointer to a application-allocated buffer where to store
 * the value of the attribute. If the buffer passed by the application is not
 * large enough to hold the value of @p attribute, the behavior is undefined.
 * 
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 * 
 */
hsa_status_t HSA_API
    hsa_amd_memory_pool_get_info(hsa_amd_memory_pool_t memory_pool,
                                 hsa_amd_memory_pool_info_t attribute,
                                 void* value);

/**
 * @brief Iterate over the memory pools associated with a given agent, and
 * invoke an application-defined callback on every iteration.
 *
 * @details An agent can directly access buffers located in some memory pool, or
 * be enabled to access them by the application (see ::hsa_amd_agents_allow_access),
 * yet that memory pool may not be returned by this function for that given
 * agent.
 *
 * A memory pool of fine-grained type must be associated only with the host.
 *
 * @param[in] agent A valid agent.
 *
 * @param[in] callback Callback to be invoked on the same thread that called 
 * ::hsa_amd_agent_iterate_memory_pools, serially, once per memory pool that is
 * associated with the agent.  The HSA runtime passes two arguments to the
 * callback: the memory pool, and the application data.  If @p callback
 * returns a status other than ::HSA_STATUS_SUCCESS for a particular iteration,
 * the traversal stops and ::hsa_amd_agent_iterate_memory_pools returns that status
 * value.
 *
 * @param[in] data Application data that is passed to @p callback on every
 * iteration. May be NULL.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p callback is NULL.
 */
hsa_status_t HSA_API hsa_amd_agent_iterate_memory_pools(
    hsa_agent_t agent,
    hsa_status_t (*callback)(hsa_amd_memory_pool_t memory_pool, void* data),
    void* data);

/**
 * @brief Allocate a block of memory (or buffer) in the specified pool.
 *
 * @param[in] memory_pool Memory pool where to allocate memory from. The memory
 * pool must have the ::HSA_AMD_MEMORY_POOL_INFO_RUNTIME_ALLOC_ALLOWED flag set.
 *
 * @param[in] size Allocation size, in bytes. Must not be zero. This value is
 * rounded up to the nearest multiple of
 * ::HSA_AMD_MEMORY_POOL_INFO_RUNTIME_ALLOC_GRANULE in @p memory_pool.
 *
 * @param[in] flags A bit-field that is used to specify allocation
 * directives. Must be 0.
 *
 * @param[out] ptr Pointer to the location where to store the base virtual
 * address of
 * the allocated block. The returned base address is aligned to the value of
 * ::HSA_AMD_MEMORY_POOL_INFO_RUNTIME_ALLOC_ALIGNMENT in @p memory_pool. If the
 * allocation fails, the returned value is undefined.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_OUT_OF_RESOURCES No memory is available.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_MEMORY_POOL The memory pool is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ALLOCATION The host is not allowed to
 * allocate memory in @p memory_pool, or @p size is greater than the value of
 * HSA_AMD_MEMORY_POOL_INFO_ALLOC_MAX_SIZE in @p memory_pool.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p ptr is NULL, or @p size is 0.
 *
 */
hsa_status_t HSA_API
    hsa_amd_memory_pool_allocate(hsa_amd_memory_pool_t memory_pool, size_t size,
                                 uint32_t flags, void** ptr);

/**
 * @brief Deallocate a block of memory previously allocated using
 * ::hsa_amd_memory_pool_allocate.
 *
 * @param[in] ptr Pointer to a memory block. If @p ptr does not match a value
 * previously returned by ::hsa_amd_memory_pool_allocate, the behavior is undefined.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 */
hsa_status_t HSA_API hsa_amd_memory_pool_free(void* ptr);

/**
 * @brief Asynchronously copy a block of memory from the location pointed to by
 * @p src on the @p src_agent to the memory block pointed to by @p dst on the @p
 * dst_agent.
 * Because the DMA engines used may not be in the same coherency domain, the caller must ensure
 * that buffers are system-level coherent. In general this requires the sending device to have
 * released the buffer to system scope prior to executing the copy API and the receiving device
 * must execute a system scope acquire fence prior to use of the destination buffer.
 *
 * @param[out] dst Buffer where the content is to be copied.
 *
 * @param[in] dst_agent Agent associated with the @p dst. The agent must be able to directly
 * access both the source and destination buffers in their current locations.
 *
 * @param[in] src A valid pointer to the source of data to be copied. The source
 * buffer must not overlap with the destination buffer, otherwise the copy will succeed
 * but contents of @p dst is undefined.
 *
 * @param[in] src_agent Agent associated with the @p src. The agent must be able to directly
 * access both the source and destination buffers in their current locations.
 *
 * @param[in] size Number of bytes to copy. If @p size is 0, no copy is
 * performed and the function returns success. Copying a number of bytes larger
 * than the size of the buffers pointed by @p dst or @p src results in undefined
 * behavior.
 *
 * @param[in] num_dep_signals Number of dependent signals. Can be 0.
 *
 * @param[in] dep_signals List of signals that must be waited on before the copy
 * operation starts. The copy will start after every signal has been observed with
 * the value 0. The dependent signal should not include completion signal from hsa_amd_memory_async_copy
 * operation to be issued in future as that can result in a deadlock. If @p num_dep_signals is 0, this
 * argument is ignored.
 *
 * @param[in] completion_signal Signal used to indicate completion of the copy
 * operation. When the copy operation is finished, the value of the signal is
 * decremented. The runtime indicates that an error has occurred during the copy
 * operation by setting the value of the completion signal to a negative
 * number. The signal handle must not be 0.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully. The
 * application is responsible for checking for asynchronous error conditions
 * (see the description of @p completion_signal).
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_SIGNAL @p completion_signal is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT The source or destination
 * pointers are NULL, or the completion signal is 0.
 */
hsa_status_t HSA_API
    hsa_amd_memory_async_copy(void* dst, hsa_agent_t dst_agent, const void* src,
                              hsa_agent_t src_agent, size_t size,
                              uint32_t num_dep_signals,
                              const hsa_signal_t* dep_signals,
                              hsa_signal_t completion_signal);

/**
 * @brief Type of accesses to a memory pool from a given agent.
 */
typedef enum {
  /**
  * The agent cannot directly access any buffer in the memory pool.
  */
  HSA_AMD_MEMORY_POOL_ACCESS_NEVER_ALLOWED = 0,
  /**
  * The agent can directly access a buffer located in the pool; the application
  * does not need to invoke ::hsa_amd_agents_allow_access.
  */
  HSA_AMD_MEMORY_POOL_ACCESS_ALLOWED_BY_DEFAULT = 1,
  /**
  * The agent can directly access a buffer located in the pool, but only if the
  * application has previously requested access to that buffer using
  * ::hsa_amd_agents_allow_access.
  */
  HSA_AMD_MEMORY_POOL_ACCESS_DISALLOWED_BY_DEFAULT = 2
} hsa_amd_memory_pool_access_t;

/**
 * @brief Properties of the relationship between an agent a memory pool.
 */
typedef enum {
  /**
  * Hyper-transport bus type.
  */
  HSA_AMD_LINK_INFO_TYPE_HYPERTRANSPORT = 0,

  /**
  * QPI bus type.
  */
  HSA_AMD_LINK_INFO_TYPE_QPI = 1,

  /**
  * PCIe bus type.
  */
  HSA_AMD_LINK_INFO_TYPE_PCIE = 2,

  /**
  * Infiniband bus type.
  */
  HSA_AMD_LINK_INFO_TYPE_INFINBAND = 3

} hsa_amd_link_info_type_t;

/**
 * @brief Link properties when accessing the memory pool from the specified
 * agent.
 */
typedef struct hsa_amd_memory_pool_link_info_s {
  /**
  * Minimum transfer latency (rounded to ns).
  */
  uint32_t min_latency;

  /**
  * Maximum transfer latency (rounded to ns).
  */
  uint32_t max_latency;

  /**
  * Minimum link interface bandwidth in MB/s.
  */
  uint32_t min_bandwidth;

  /**
  * Maximum link interface bandwidth in MB/s.
  */
  uint32_t max_bandwidth;

  /**
  * Support for 32-bit atomic transactions.
  */
  bool atomic_support_32bit;

  /**
  * Support for 64-bit atomic transactions.
  */
  bool atomic_support_64bit;

  /**
  * Support for cache coherent transactions.
  */
  bool coherent_support;

  /**
  * The type of bus/link.
  */
  hsa_amd_link_info_type_t link_type;

} hsa_amd_memory_pool_link_info_t;

/**
 * @brief Properties of the relationship between an agent a memory pool.
 */
typedef enum {
  /**
  * Access to buffers located in the memory pool. The type of this attribute
  * is ::hsa_amd_memory_pool_access_t.
  *
  * An agent can always directly access buffers currently located in a memory
  * pool that is associated (the memory_pool is one of the values returned by
  * ::hsa_amd_agent_iterate_memory_pools on the agent) with that agent. If the
  * buffer is currently located in a memory pool that is not associated with
  * the agent, and the value returned by this function for the given
  * combination of agent and memory pool is not
  * HSA_AMD_MEMORY_POOL_ACCESS_NEVER_ALLOWED, the application still needs to invoke
  * ::hsa_amd_agents_allow_access in order to gain direct access to the buffer.
  *
  * If the given agent can directly access buffers the pool, the result is not
  * HSA_AMD_MEMORY_POOL_ACCESS_NEVER_ALLOWED. If the memory pool is associated with
  * the agent, or it is of fined-grained type, the result must not be
  * HSA_AMD_MEMORY_POOL_ACCESS_NEVER_ALLOWED. If the memory pool is not associated
  * with the agent, and does not reside in the global segment, the result must
  * be HSA_AMD_MEMORY_POOL_ACCESS_NEVER_ALLOWED.
  */
  HSA_AMD_AGENT_MEMORY_POOL_INFO_ACCESS = 0,

  /**
  * Number of links to hop when accessing the memory pool from the specified
  * agent. The type of this attribute is uint32_t.
  */
  HSA_AMD_AGENT_MEMORY_POOL_INFO_NUM_LINK_HOPS = 1,

  /**
  * Details of each link hop when accessing the memory pool starting from the
  * specified agent. The type of this attribute is an array size of
  * HSA_AMD_AGENT_MEMORY_POOL_INFO_NUM_LINK_HOPS with each element containing
  * ::hsa_amd_memory_pool_link_info_t.
  */
  HSA_AMD_AGENT_MEMORY_POOL_INFO_LINK_INFO = 2

} hsa_amd_agent_memory_pool_info_t;

/**
 * @brief Get the current value of an attribute of the relationship between an
 * agent and a memory pool.
 *
 * @param[in] agent Agent.
 *
 * @param[in] memory_pool Memory pool.
 *
 * @param[in] attribute Attribute to query.
 *
 * @param[out] value Pointer to a application-allocated buffer where to store
 * the value of the attribute. If the buffer passed by the application is not
 * large enough to hold the value of @p attribute, the behavior is undefined.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 */
hsa_status_t HSA_API hsa_amd_agent_memory_pool_get_info(
    hsa_agent_t agent, hsa_amd_memory_pool_t memory_pool,
    hsa_amd_agent_memory_pool_info_t attribute, void* value);

/**
 * @brief Enable direct access to a buffer from a given set of agents.
 *
 * @details
 *
 * Upon return, only the listed agents and the agent associated with the
 * buffer's memory pool have direct access to the @p ptr.
 *
 * Any agent that has access to the buffer before and after the call to
 * ::hsa_amd_agents_allow_access will also have access while
 * ::hsa_amd_agents_allow_access is in progress.
 *
 * The caller is responsible for ensuring that each agent in the list
 * must be able to access the memory pool containing @p ptr
 * (using ::hsa_amd_agent_memory_pool_get_info with ::HSA_AMD_AGENT_MEMORY_POOL_INFO_ACCESS attribute),
 * otherwise error code is returned.
 *
 * @param[in] num_agents Size of @p agents.
 *
 * @param[in] agents List of agents. If @p num_agents is 0, this argument is
 * ignored.
 *
 * @param[in] flags A list of bit-field that is used to specify access
 * information in a per-agent basis. The size of this list must match that of @p
 * agents. Must be NULL.
 *
 * @param[in] ptr A buffer previously allocated using ::hsa_amd_memory_pool_allocate.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p num_agents is 0, or @p agents
 * is NULL, @p flags is NULL, or attempting to enable access to agent(s) because
 * @p ptr is allocated from an inaccessible pool.
 *
 */
hsa_status_t HSA_API
    hsa_amd_agents_allow_access(uint32_t num_agents, const hsa_agent_t* agents,
                                const uint32_t* flags, const void* ptr);

/**
 * @brief Query if buffers currently located in some memory pool can be
 * relocated to a destination memory pool.
 *
 * @details If the returned value is non-zero, a migration of a buffer to @p
 * dst_memory_pool using ::hsa_amd_memory_migrate may nevertheless fail due to
 * resource limitations.
 *
 * @param[in] src_memory_pool Source memory pool.
 *
 * @param[in] dst_memory_pool Destination memory pool.
 *
 * @param[out] result Pointer to a memory location where the result of the query
 * is stored. Must not be NULL. If buffers currently located in @p
 * src_memory_pool can be relocated to @p dst_memory_pool, the result is
 * true.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_MEMORY_POOL One of the memory pools is
 * invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p result is NULL.
 */
hsa_status_t HSA_API
    hsa_amd_memory_pool_can_migrate(hsa_amd_memory_pool_t src_memory_pool,
                                    hsa_amd_memory_pool_t dst_memory_pool,
                                    bool* result);

/**
 * @brief Relocate a buffer to a new memory pool.
 *
 * @details When a buffer is migrated, its virtual address remains the same but
 * its physical contents are moved to the indicated memory pool.
 *
 * After migration, only the agent associated with the destination pool will have access.
 *
 * The caller is also responsible for ensuring that the allocation in the
 * source memory pool where the buffer is currently located can be migrated to the
 * specified destination memory pool (using ::hsa_amd_memory_pool_can_migrate returns a value of true
 * for the source and destination memory pools), otherwise behavior is undefined.
 *
 * The caller must ensure that the buffer is not accessed while it is migrated.
 *
 * @param[in] ptr Buffer to be relocated. The buffer must have been released to system
 * prior to call this API.  The buffer will be released to system upon completion.
 *
 * @param[in] memory_pool Memory pool where to place the buffer.
 *
 * @param[in] flags A bit-field that is used to specify migration
 * information. Must be zero.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_MEMORY_POOL The destination memory pool is
 * invalid.
 *
 * @retval ::HSA_STATUS_ERROR_OUT_OF_RESOURCES There is a failure in
 * allocating the necessary resources.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p flags is not 0.
 */
hsa_status_t HSA_API hsa_amd_memory_migrate(const void* ptr,
                                            hsa_amd_memory_pool_t memory_pool,
                                            uint32_t flags);

/**
 *
 * @brief Pin a host pointer allocated by C/C++ or OS allocator (i.e. ordinary system DRAM) and return a new
 * pointer accessible by the @p agents. If the @p host_ptr overlaps with previously locked
 * memory, then the overlap area is kept locked (i.e multiple mappings are permitted). In this case,
 * the same input @p host_ptr may give different locked @p agent_ptr and when it does, they
 * are not necessarily coherent (i.e. accessing either @p agent_ptr is not equivalent).
 *
 * @param[in] host_ptr A buffer allocated by C/C++ or OS allocator.
 *
 * @param[in] size The size to be locked.
 *
 * @param[in] agents Array of agent handle to gain access to the @p host_ptr.
 * If this parameter is NULL and the @p num_agent is 0, all agents
 * in the platform will gain access to the @p host_ptr.
 *
 * @param[out] agent_ptr Pointer to the location where to store the new address.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_OUT_OF_RESOURCES There is a failure in
 * allocating the necessary resources.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT One or more agent in @p agents is
 * invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p size is 0 or @p host_ptr or
 * @p agent_ptr is NULL or @p agents not NULL but @p num_agent is 0 or @p agents
 * is NULL but @p num_agent is not 0.
 */

hsa_status_t HSA_API hsa_amd_memory_lock(void* host_ptr, size_t size,
                                         hsa_agent_t* agents, int num_agent,
                                         void** agent_ptr);

/**
 *
 * @brief Unpin the host pointer previously pinned via ::hsa_amd_memory_lock.
 *
 * @details The behavior is undefined if the host pointer being unpinned does not
 * match previous pinned address or if the host pointer was already deallocated.
 *
 * @param[in] host_ptr A buffer allocated by C/C++ or OS allocator that was
 * pinned previously via ::hsa_amd_memory_lock.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 */
hsa_status_t HSA_API hsa_amd_memory_unlock(void* host_ptr);

/**
 * @brief Sets the first @p num of uint32_t of the block of memory pointed by
 * @p ptr to the specified @p value.
 *
 * @param[in] ptr Pointer to the block of memory to fill.
 *
 * @param[in] value Value to be set.
 *
 * @param[in] count Number of uint32_t element to be set to the value.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p ptr is NULL or
 * not 4 bytes aligned
 *
 */
hsa_status_t HSA_API
    hsa_amd_memory_fill(void* ptr, uint32_t value, size_t count);

/**
 * @brief Maps an interop object into the HSA flat address space and establishes
 * memory residency.  The metadata pointer is valid during the lifetime of the
 * map (until hsa_amd_interop_unmap_buffer is called).
 * Multiple calls to hsa_amd_interop_map_buffer with the same interop_handle
 * result in multiple mappings with potentially different addresses and
 * different metadata pointers.  Concurrent operations on these addresses are
 * not coherent.  Memory must be fenced to system scope to ensure consistency,
 * between mappings and with any views of this buffer in the originating
 * software stack.
 *
 * @param[in] num_agents Number of agents which require access to the memory
 *
 * @param[in] agents List of accessing agents.
 *
 * @param[in] interop_handle Handle of interop buffer (dmabuf handle in Linux)
 *
 * @param [in] flags Reserved, must be 0
 *
 * @param[out] size Size in bytes of the mapped object
 *
 * @param[out] ptr Base address of the mapped object
 *
 * @param[out] metadata_size Size of metadata in bytes, may be NULL
 *
 * @param[out] metadata Pointer to metadata, may be NULL
 * 
 * @retval HSA_STATUS_SUCCESS if successfully mapped
 *
 * @retval HSA_STATUS_ERROR_NOT_INITIALIZED if HSA is not initialized
 *
 * @retval HSA_STATUS_ERROR_OUT_OF_RESOURCES if there is a failure in allocating
 * necessary resources
 *
 * @retval HSA_STATUS_ERROR_INVALID_ARGUMENT all other errors
 */
hsa_status_t HSA_API hsa_amd_interop_map_buffer(uint32_t num_agents,   
                                        hsa_agent_t* agents,       
                                        int interop_handle,    
                                        uint32_t flags,        
                                        size_t* size,          
                                        void** ptr,            
                                        size_t* metadata_size, 
                                        const void** metadata);

/**
 * @brief Removes a previously mapped interop object from HSA's flat address space.
 * Ends lifetime for the mapping's associated metadata pointer.
 */
hsa_status_t HSA_API hsa_amd_interop_unmap_buffer(void* ptr);

/**
 * @brief Encodes an opaque vendor specific image format.  The length of data
 * depends on the underlying format.  This structure must not be copied as its
 * true length can not be determined.
 */
typedef struct hsa_amd_image_descriptor_s {
  /*
  Version number of the descriptor
  */
  uint32_t version;
  
  /*
  Vendor and device PCI IDs for the format as VENDOR_ID<<16|DEVICE_ID.
  */
  uint32_t deviceID;

  /*
  Start of vendor specific data.
  */
  uint32_t data[1];
} hsa_amd_image_descriptor_t;

/**
 * @brief Creates an image from an opaque vendor specific image format.
 * Does not modify data at image_data.  Intended initially for
 * accessing interop images.
 *
 * @param agent[in] Agent on which to create the image
 *
 * @param[in] image_descriptor[in] Vendor specific image format
 *
 * @param[in] image_data Pointer to image backing store
 *
 * @param[in] access_permission Access permissions for the image object
 *
 * @param[out] image Created image object.
 *
 * @retval HSA_STATUS_SUCCESS Image created successfully
 *
 * @retval HSA_STATUS_ERROR_NOT_INITIALIZED if HSA is not initialized
 *
 * @retval HSA_STATUS_ERROR_OUT_OF_RESOURCES if there is a failure in allocating
 * necessary resources
 *
 * @retval HSA_STATUS_ERROR_INVALID_ARGUMENT Bad or mismatched descriptor,
 * null image_data, or mismatched access_permission.
 */
hsa_status_t HSA_API hsa_amd_image_create(
    hsa_agent_t agent,
    const hsa_ext_image_descriptor_t *image_descriptor,
    const hsa_amd_image_descriptor_t *image_layout,
    const void *image_data,
    hsa_access_permission_t access_permission,
    hsa_ext_image_t *image
);

#ifdef __cplusplus
}  // end extern "C" block
#endif

#endif  // header guard
