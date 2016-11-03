//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines an enum for interceptable HSA APIs
//==============================================================================

#ifndef _HSA_FUNCTION_DEFS_H_
#define _HSA_FUNCTION_DEFS_H_

/// enum containing an item for each interceptable HSA API
enum HSA_API_Type
{
    HSA_API_Type_UNKNOWN = -1,

    HSA_API_Type_hsa_status_string,
    HSA_API_Type_hsa_init,
    HSA_API_Type_hsa_shut_down,
    HSA_API_Type_hsa_system_get_info,
    HSA_API_Type_hsa_system_extension_supported,
    HSA_API_Type_hsa_system_get_extension_table,
    HSA_API_Type_hsa_agent_get_info,
    HSA_API_Type_hsa_iterate_agents,
    HSA_API_Type_hsa_agent_get_exception_policies,
    HSA_API_Type_hsa_agent_extension_supported,
    HSA_API_Type_hsa_signal_create,
    HSA_API_Type_hsa_signal_destroy,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_load_scacquire,
#else
    HSA_API_Type_hsa_signal_load_acquire,
#endif
    HSA_API_Type_hsa_signal_load_relaxed,
    HSA_API_Type_hsa_signal_store_relaxed,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_store_screlease,
    HSA_API_Type_hsa_signal_exchange_scacq_screl,
    HSA_API_Type_hsa_signal_exchange_scacquire,
#else
    HSA_API_Type_hsa_signal_store_release,
    HSA_API_Type_hsa_signal_exchange_acq_rel,
    HSA_API_Type_hsa_signal_exchange_acquire,
#endif
    HSA_API_Type_hsa_signal_exchange_relaxed,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_exchange_screlease,
#else
    HSA_API_Type_hsa_signal_exchange_release,
#endif
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_cas_scacq_screl,
    HSA_API_Type_hsa_signal_cas_scacquire,
#else
    HSA_API_Type_hsa_signal_cas_acq_rel,
    HSA_API_Type_hsa_signal_cas_acquire,
#endif
    HSA_API_Type_hsa_signal_cas_relaxed,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_cas_screlease,
#else
    HSA_API_Type_hsa_signal_cas_release,
#endif
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_add_scacq_screl,
    HSA_API_Type_hsa_signal_add_scacquire,
#else
    HSA_API_Type_hsa_signal_add_acq_rel,
    HSA_API_Type_hsa_signal_add_acquire,
#endif
    HSA_API_Type_hsa_signal_add_relaxed,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_add_screlease,
#else
    HSA_API_Type_hsa_signal_add_release,
#endif
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_subtract_scacq_screl,
    HSA_API_Type_hsa_signal_subtract_scacquire,
#else
    HSA_API_Type_hsa_signal_subtract_acq_rel,
    HSA_API_Type_hsa_signal_subtract_acquire,
#endif
    HSA_API_Type_hsa_signal_subtract_relaxed,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_subtract_screlease,
#else
    HSA_API_Type_hsa_signal_subtract_release,
#endif
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_and_scacq_screl,
    HSA_API_Type_hsa_signal_and_scacquire,
#else
    HSA_API_Type_hsa_signal_and_acq_rel,
    HSA_API_Type_hsa_signal_and_acquire,
#endif
    HSA_API_Type_hsa_signal_and_relaxed,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_and_screlease,
#else
    HSA_API_Type_hsa_signal_and_release,
#endif
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_or_scacq_screl,
    HSA_API_Type_hsa_signal_or_scacquire,
#else
    HSA_API_Type_hsa_signal_or_acq_rel,
    HSA_API_Type_hsa_signal_or_acquire,
#endif
    HSA_API_Type_hsa_signal_or_relaxed,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_or_screlease,
#else
    HSA_API_Type_hsa_signal_or_release,
#endif
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_xor_scacq_screl,
    HSA_API_Type_hsa_signal_xor_scacquire,
#else
    HSA_API_Type_hsa_signal_xor_acq_rel,
    HSA_API_Type_hsa_signal_xor_acquire,
#endif
    HSA_API_Type_hsa_signal_xor_relaxed,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_xor_screlease,
#else
    HSA_API_Type_hsa_signal_xor_release,
#endif
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_signal_wait_scacquire,
#else
    HSA_API_Type_hsa_signal_wait_acquire,
#endif
    HSA_API_Type_hsa_signal_wait_relaxed,
    HSA_API_Type_hsa_queue_create,
    HSA_API_Type_hsa_soft_queue_create,
    HSA_API_Type_hsa_queue_destroy,
    HSA_API_Type_hsa_queue_inactivate,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_queue_load_read_index_scacquire,
#else
    HSA_API_Type_hsa_queue_load_read_index_acquire,
#endif
    HSA_API_Type_hsa_queue_load_read_index_relaxed,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_queue_load_write_index_scacquire,
#else
    HSA_API_Type_hsa_queue_load_write_index_acquire,
#endif
    HSA_API_Type_hsa_queue_load_write_index_relaxed,
    HSA_API_Type_hsa_queue_store_write_index_relaxed,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_queue_store_write_index_screlease,
#else
    HSA_API_Type_hsa_queue_store_write_index_release,
#endif
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_queue_cas_write_index_scacq_screl,
    HSA_API_Type_hsa_queue_cas_write_index_scacquire,
#else
    HSA_API_Type_hsa_queue_cas_write_index_acq_rel,
    HSA_API_Type_hsa_queue_cas_write_index_acquire,
#endif
    HSA_API_Type_hsa_queue_cas_write_index_relaxed,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_queue_cas_write_index_screlease,
#else
    HSA_API_Type_hsa_queue_cas_write_index_release,
#endif
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_queue_add_write_index_scacq_screl,
    HSA_API_Type_hsa_queue_add_write_index_scacquire,
#else
    HSA_API_Type_hsa_queue_add_write_index_acq_rel,
    HSA_API_Type_hsa_queue_add_write_index_acquire,
#endif
    HSA_API_Type_hsa_queue_add_write_index_relaxed,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_queue_add_write_index_screlease,
#else
    HSA_API_Type_hsa_queue_add_write_index_release,
#endif
    HSA_API_Type_hsa_queue_store_read_index_relaxed,
#ifdef FUTURE_ROCR_VERSION
    HSA_API_Type_hsa_queue_store_read_index_screlease,
#else
    HSA_API_Type_hsa_queue_store_read_index_release,
#endif
    HSA_API_Type_hsa_region_get_info,
    HSA_API_Type_hsa_agent_iterate_regions,
    HSA_API_Type_hsa_memory_allocate,
    HSA_API_Type_hsa_memory_free,
    HSA_API_Type_hsa_memory_copy,
    HSA_API_Type_hsa_memory_assign_agent,
    HSA_API_Type_hsa_memory_register,
    HSA_API_Type_hsa_memory_deregister,
    HSA_API_Type_hsa_isa_from_name,
    HSA_API_Type_hsa_isa_get_info,
    HSA_API_Type_hsa_isa_compatible,
    HSA_API_Type_hsa_code_object_serialize,
    HSA_API_Type_hsa_code_object_deserialize,
    HSA_API_Type_hsa_code_object_destroy,
    HSA_API_Type_hsa_code_object_get_info,
    HSA_API_Type_hsa_code_object_get_symbol,
    HSA_API_Type_hsa_code_symbol_get_info,
    HSA_API_Type_hsa_code_object_iterate_symbols,
    HSA_API_Type_hsa_executable_create,
    HSA_API_Type_hsa_executable_destroy,
    HSA_API_Type_hsa_executable_load_code_object,
    HSA_API_Type_hsa_executable_freeze,
    HSA_API_Type_hsa_executable_get_info,
    HSA_API_Type_hsa_executable_global_variable_define,
    HSA_API_Type_hsa_executable_agent_global_variable_define,
    HSA_API_Type_hsa_executable_readonly_variable_define,
    HSA_API_Type_hsa_executable_validate,
    HSA_API_Type_hsa_executable_get_symbol,
    HSA_API_Type_hsa_executable_symbol_get_info,
    HSA_API_Type_hsa_executable_iterate_symbols,
    HSA_API_Type_hsa_ext_program_create,
    HSA_API_Type_hsa_ext_program_destroy,
    HSA_API_Type_hsa_ext_program_add_module,
    HSA_API_Type_hsa_ext_program_iterate_modules,
    HSA_API_Type_hsa_ext_program_get_info,
    HSA_API_Type_hsa_ext_program_finalize,
    HSA_API_Type_hsa_ext_image_get_capability,
    HSA_API_Type_hsa_ext_image_data_get_info,
    HSA_API_Type_hsa_ext_image_create,
    HSA_API_Type_hsa_ext_image_destroy,
    HSA_API_Type_hsa_ext_image_copy,
    HSA_API_Type_hsa_ext_image_import,
    HSA_API_Type_hsa_ext_image_export,
    HSA_API_Type_hsa_ext_image_clear,
    HSA_API_Type_hsa_ext_sampler_create,
    HSA_API_Type_hsa_ext_sampler_destroy,
    HSA_API_Type_hsa_amd_coherency_get_type,
    HSA_API_Type_hsa_amd_coherency_set_type,
    HSA_API_Type_hsa_amd_profiling_set_profiler_enabled,
    HSA_API_Type_hsa_amd_profiling_async_copy_enable,
    HSA_API_Type_hsa_amd_profiling_get_dispatch_time,
    HSA_API_Type_hsa_amd_profiling_get_async_copy_time,
    HSA_API_Type_hsa_amd_profiling_convert_tick_to_system_domain,
    HSA_API_Type_hsa_amd_signal_async_handler,
    HSA_API_Type_hsa_amd_async_function,
    HSA_API_Type_hsa_amd_signal_wait_any,
    HSA_API_Type_hsa_amd_image_get_info_max_dim,
    HSA_API_Type_hsa_amd_queue_cu_set_mask,
    HSA_API_Type_hsa_amd_memory_pool_get_info,
    HSA_API_Type_hsa_amd_agent_iterate_memory_pools,
    HSA_API_Type_hsa_amd_memory_pool_allocate,
    HSA_API_Type_hsa_amd_memory_pool_free,
    HSA_API_Type_hsa_amd_memory_async_copy,
    HSA_API_Type_hsa_amd_agent_memory_pool_get_info,
    HSA_API_Type_hsa_amd_agents_allow_access,
    HSA_API_Type_hsa_amd_memory_pool_can_migrate,
    HSA_API_Type_hsa_amd_memory_migrate,
    HSA_API_Type_hsa_amd_memory_lock,
    HSA_API_Type_hsa_amd_memory_unlock,
    HSA_API_Type_hsa_amd_memory_fill,
    HSA_API_Type_hsa_amd_interop_map_buffer,
    HSA_API_Type_hsa_amd_interop_unmap_buffer,
    HSA_API_Type_hsa_amd_image_create,
    HSA_API_Type_hsa_ven_amd_loader_query_host_address,
    HSA_API_Type_hsa_ven_amd_loader_query_segment_descriptors,

    // new for ROCm 1.3
    HSA_API_Type_hsa_extension_get_name,
    HSA_API_Type_hsa_system_major_extension_supported,
    HSA_API_Type_hsa_system_get_major_extension_table,
    HSA_API_Type_hsa_cache_get_info,
    HSA_API_Type_hsa_agent_iterate_caches,
    HSA_API_Type_hsa_agent_major_extension_supported,
    HSA_API_Type_hsa_signal_silent_store_relaxed,
    HSA_API_Type_hsa_signal_silent_store_screlease,
    HSA_API_Type_hsa_signal_group_create,
    HSA_API_Type_hsa_signal_group_destroy,
    HSA_API_Type_hsa_signal_group_wait_any_scacquire,
    HSA_API_Type_hsa_signal_group_wait_any_relaxed,
    HSA_API_Type_hsa_agent_iterate_isas,
    HSA_API_Type_hsa_isa_get_info_alt,
    HSA_API_Type_hsa_isa_get_exception_policies,
    HSA_API_Type_hsa_isa_get_round_method,
    HSA_API_Type_hsa_wavefront_get_info,
    HSA_API_Type_hsa_isa_iterate_wavefronts,
    HSA_API_Type_hsa_code_object_reader_create_from_file,
    HSA_API_Type_hsa_code_object_reader_create_from_memory,
    HSA_API_Type_hsa_code_object_reader_destroy,
    HSA_API_Type_hsa_executable_create_alt,
    HSA_API_Type_hsa_executable_load_program_code_object,
    HSA_API_Type_hsa_executable_load_agent_code_object,
    HSA_API_Type_hsa_executable_validate_alt,
    HSA_API_Type_hsa_executable_get_symbol_by_name,
    HSA_API_Type_hsa_executable_iterate_agent_symbols,
    HSA_API_Type_hsa_executable_iterate_program_symbols,
    HSA_API_Type_hsa_code_object_get_symbol_from_name,

    // HSA tools library entrypoints
    HSA_API_Type_Init,
    HSA_API_Type_Unload,

    // Non-API items (used to mark dispatches in the ATP file)
    HSA_API_Type_Non_API_First,
    HSA_API_Type_Non_API_Dispatch = HSA_API_Type_Non_API_First,

    HSA_API_Type_COUNT
};

#endif // _HSA_FUNCTION_DEFS_H_
