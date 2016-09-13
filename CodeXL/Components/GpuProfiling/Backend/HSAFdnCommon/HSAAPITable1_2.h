//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines an API table compatible with the 1.2 runtime
//==============================================================================

#ifndef _HSA_API_TABLE_1_2_H_
#define _HSA_API_TABLE_1_2_H_

#include "hsa.h"
#include "hsa_ext_image.h"
#include "hsa_ext_finalize.h"
#include "hsa_ext_amd.h"

struct ApiTableVersion1_2 {
    uint32_t major_id;
    uint32_t minor_id;
    uint32_t step_id;
    uint32_t reserved;
};

// Table to export HSA Finalizer Extension Apis 
struct FinalizerExtTable1_2 {
    ApiTableVersion1_2 version;
    decltype(hsa_ext_program_create)* hsa_ext_program_create_fn;
    decltype(hsa_ext_program_destroy)* hsa_ext_program_destroy_fn;
    decltype(hsa_ext_program_add_module)* hsa_ext_program_add_module_fn;
    decltype(hsa_ext_program_iterate_modules)* hsa_ext_program_iterate_modules_fn;
    decltype(hsa_ext_program_get_info)* hsa_ext_program_get_info_fn;
    decltype(hsa_ext_program_finalize)* hsa_ext_program_finalize_fn;
};

// Table to export HSA Image Extension Apis
struct ImageExtTable1_2 {
    ApiTableVersion1_2 version;
    decltype(hsa_ext_image_get_capability)* hsa_ext_image_get_capability_fn;
    decltype(hsa_ext_image_data_get_info)* hsa_ext_image_data_get_info_fn;
    decltype(hsa_ext_image_create)* hsa_ext_image_create_fn;
    decltype(hsa_ext_image_import)* hsa_ext_image_import_fn;
    decltype(hsa_ext_image_export)* hsa_ext_image_export_fn;
    decltype(hsa_ext_image_copy)* hsa_ext_image_copy_fn;
    decltype(hsa_ext_image_clear)* hsa_ext_image_clear_fn;
    decltype(hsa_ext_image_destroy)* hsa_ext_image_destroy_fn;
    decltype(hsa_ext_sampler_create)* hsa_ext_sampler_create_fn;
    decltype(hsa_ext_sampler_destroy)* hsa_ext_sampler_destroy_fn;
};

// Table to export AMD Extension Apis
struct AmdExtTable1_2 {
    ApiTableVersion1_2 version;
    decltype(hsa_amd_coherency_get_type)* hsa_amd_coherency_get_type_fn;
    decltype(hsa_amd_coherency_set_type)* hsa_amd_coherency_set_type_fn;
    decltype(hsa_amd_profiling_set_profiler_enabled)* hsa_amd_profiling_set_profiler_enabled_fn;
    decltype(hsa_amd_profiling_async_copy_enable) *hsa_amd_profiling_async_copy_enable_fn;
    decltype(hsa_amd_profiling_get_dispatch_time)* hsa_amd_profiling_get_dispatch_time_fn;
    decltype(hsa_amd_profiling_get_async_copy_time) *hsa_amd_profiling_get_async_copy_time_fn;
    decltype(hsa_amd_profiling_convert_tick_to_system_domain)* hsa_amd_profiling_convert_tick_to_system_domain_fn;
    decltype(hsa_amd_signal_async_handler)* hsa_amd_signal_async_handler_fn;
    decltype(hsa_amd_async_function)* hsa_amd_async_function_fn;
    decltype(hsa_amd_signal_wait_any)* hsa_amd_signal_wait_any_fn;
    decltype(hsa_amd_queue_cu_set_mask)* hsa_amd_queue_cu_set_mask_fn;
    decltype(hsa_amd_memory_pool_get_info)* hsa_amd_memory_pool_get_info_fn;
    decltype(hsa_amd_agent_iterate_memory_pools)* hsa_amd_agent_iterate_memory_pools_fn;
    decltype(hsa_amd_memory_pool_allocate)* hsa_amd_memory_pool_allocate_fn;
    decltype(hsa_amd_memory_pool_free)* hsa_amd_memory_pool_free_fn;
    decltype(hsa_amd_memory_async_copy)* hsa_amd_memory_async_copy_fn;
    decltype(hsa_amd_agent_memory_pool_get_info)* hsa_amd_agent_memory_pool_get_info_fn;
    decltype(hsa_amd_agents_allow_access)* hsa_amd_agents_allow_access_fn;
    decltype(hsa_amd_memory_pool_can_migrate)* hsa_amd_memory_pool_can_migrate_fn;
    decltype(hsa_amd_memory_migrate)* hsa_amd_memory_migrate_fn;
    decltype(hsa_amd_memory_lock)* hsa_amd_memory_lock_fn;
    decltype(hsa_amd_memory_unlock)* hsa_amd_memory_unlock_fn;
    decltype(hsa_amd_memory_fill)* hsa_amd_memory_fill_fn;
    decltype(hsa_amd_interop_map_buffer)* hsa_amd_interop_map_buffer_fn;
    decltype(hsa_amd_interop_unmap_buffer)* hsa_amd_interop_unmap_buffer_fn;
    decltype(::hsa_amd_image_create)* hsa_amd_image_create_fn;
};

// Table to export HSA Core Runtime Apis
struct CoreApiTable1_2 {
    ApiTableVersion1_2 version;
    decltype(hsa_init)* hsa_init_fn;
    decltype(hsa_shut_down)* hsa_shut_down_fn;
    decltype(hsa_system_get_info)* hsa_system_get_info_fn;
    decltype(hsa_system_extension_supported)* hsa_system_extension_supported_fn;
    decltype(hsa_system_get_extension_table)* hsa_system_get_extension_table_fn;
    decltype(hsa_iterate_agents)* hsa_iterate_agents_fn;
    decltype(hsa_agent_get_info)* hsa_agent_get_info_fn;
    decltype(hsa_queue_create)* hsa_queue_create_fn;
    decltype(hsa_soft_queue_create)* hsa_soft_queue_create_fn;
    decltype(hsa_queue_destroy)* hsa_queue_destroy_fn;
    decltype(hsa_queue_inactivate)* hsa_queue_inactivate_fn;
    decltype(hsa_queue_load_read_index_acquire)* hsa_queue_load_read_index_acquire_fn;
    decltype(hsa_queue_load_read_index_relaxed)* hsa_queue_load_read_index_relaxed_fn;
    decltype(hsa_queue_load_write_index_acquire)* hsa_queue_load_write_index_acquire_fn;
    decltype(hsa_queue_load_write_index_relaxed)* hsa_queue_load_write_index_relaxed_fn;
    decltype(hsa_queue_store_write_index_relaxed)* hsa_queue_store_write_index_relaxed_fn;
    decltype(hsa_queue_store_write_index_release)* hsa_queue_store_write_index_release_fn;
    decltype(hsa_queue_cas_write_index_acq_rel)* hsa_queue_cas_write_index_acq_rel_fn;
    decltype(hsa_queue_cas_write_index_acquire)* hsa_queue_cas_write_index_acquire_fn;
    decltype(hsa_queue_cas_write_index_relaxed)* hsa_queue_cas_write_index_relaxed_fn;
    decltype(hsa_queue_cas_write_index_release)* hsa_queue_cas_write_index_release_fn;
    decltype(hsa_queue_add_write_index_acq_rel)* hsa_queue_add_write_index_acq_rel_fn;
    decltype(hsa_queue_add_write_index_acquire)* hsa_queue_add_write_index_acquire_fn;
    decltype(hsa_queue_add_write_index_relaxed)* hsa_queue_add_write_index_relaxed_fn;
    decltype(hsa_queue_add_write_index_release)* hsa_queue_add_write_index_release_fn;
    decltype(hsa_queue_store_read_index_relaxed)* hsa_queue_store_read_index_relaxed_fn;
    decltype(hsa_queue_store_read_index_release)* hsa_queue_store_read_index_release_fn;
    decltype(hsa_agent_iterate_regions)* hsa_agent_iterate_regions_fn;
    decltype(hsa_region_get_info)* hsa_region_get_info_fn;
    decltype(hsa_agent_get_exception_policies)* hsa_agent_get_exception_policies_fn;
    decltype(hsa_agent_extension_supported)* hsa_agent_extension_supported_fn;
    decltype(hsa_memory_register)* hsa_memory_register_fn;
    decltype(hsa_memory_deregister)* hsa_memory_deregister_fn;
    decltype(hsa_memory_allocate)* hsa_memory_allocate_fn;
    decltype(hsa_memory_free)* hsa_memory_free_fn;
    decltype(hsa_memory_copy)* hsa_memory_copy_fn;
    decltype(hsa_memory_assign_agent)* hsa_memory_assign_agent_fn;
    decltype(hsa_signal_create)* hsa_signal_create_fn;
    decltype(hsa_signal_destroy)* hsa_signal_destroy_fn;
    decltype(hsa_signal_load_relaxed)* hsa_signal_load_relaxed_fn;
    decltype(hsa_signal_load_acquire)* hsa_signal_load_acquire_fn;
    decltype(hsa_signal_store_relaxed)* hsa_signal_store_relaxed_fn;
    decltype(hsa_signal_store_release)* hsa_signal_store_release_fn;
    decltype(hsa_signal_wait_relaxed)* hsa_signal_wait_relaxed_fn;
    decltype(hsa_signal_wait_acquire)* hsa_signal_wait_acquire_fn;
    decltype(hsa_signal_and_relaxed)* hsa_signal_and_relaxed_fn;
    decltype(hsa_signal_and_acquire)* hsa_signal_and_acquire_fn;
    decltype(hsa_signal_and_release)* hsa_signal_and_release_fn;
    decltype(hsa_signal_and_acq_rel)* hsa_signal_and_acq_rel_fn;
    decltype(hsa_signal_or_relaxed)* hsa_signal_or_relaxed_fn;
    decltype(hsa_signal_or_acquire)* hsa_signal_or_acquire_fn;
    decltype(hsa_signal_or_release)* hsa_signal_or_release_fn;
    decltype(hsa_signal_or_acq_rel)* hsa_signal_or_acq_rel_fn;
    decltype(hsa_signal_xor_relaxed)* hsa_signal_xor_relaxed_fn;
    decltype(hsa_signal_xor_acquire)* hsa_signal_xor_acquire_fn;
    decltype(hsa_signal_xor_release)* hsa_signal_xor_release_fn;
    decltype(hsa_signal_xor_acq_rel)* hsa_signal_xor_acq_rel_fn;
    decltype(hsa_signal_exchange_relaxed)* hsa_signal_exchange_relaxed_fn;
    decltype(hsa_signal_exchange_acquire)* hsa_signal_exchange_acquire_fn;
    decltype(hsa_signal_exchange_release)* hsa_signal_exchange_release_fn;
    decltype(hsa_signal_exchange_acq_rel)* hsa_signal_exchange_acq_rel_fn;
    decltype(hsa_signal_add_relaxed)* hsa_signal_add_relaxed_fn;
    decltype(hsa_signal_add_acquire)* hsa_signal_add_acquire_fn;
    decltype(hsa_signal_add_release)* hsa_signal_add_release_fn;
    decltype(hsa_signal_add_acq_rel)* hsa_signal_add_acq_rel_fn;
    decltype(hsa_signal_subtract_relaxed)* hsa_signal_subtract_relaxed_fn;
    decltype(hsa_signal_subtract_acquire)* hsa_signal_subtract_acquire_fn;
    decltype(hsa_signal_subtract_release)* hsa_signal_subtract_release_fn;
    decltype(hsa_signal_subtract_acq_rel)* hsa_signal_subtract_acq_rel_fn;
    decltype(hsa_signal_cas_relaxed)* hsa_signal_cas_relaxed_fn;
    decltype(hsa_signal_cas_acquire)* hsa_signal_cas_acquire_fn;
    decltype(hsa_signal_cas_release)* hsa_signal_cas_release_fn;
    decltype(hsa_signal_cas_acq_rel)* hsa_signal_cas_acq_rel_fn;
    decltype(hsa_isa_from_name)* hsa_isa_from_name_fn;
    decltype(hsa_isa_get_info)* hsa_isa_get_info_fn;
    decltype(hsa_isa_compatible)* hsa_isa_compatible_fn;
    decltype(hsa_code_object_serialize)* hsa_code_object_serialize_fn;
    decltype(hsa_code_object_deserialize)* hsa_code_object_deserialize_fn;
    decltype(hsa_code_object_destroy)* hsa_code_object_destroy_fn;
    decltype(hsa_code_object_get_info)* hsa_code_object_get_info_fn;
    decltype(hsa_code_object_get_symbol)* hsa_code_object_get_symbol_fn;
    decltype(hsa_code_symbol_get_info)* hsa_code_symbol_get_info_fn;
    decltype(hsa_code_object_iterate_symbols)* hsa_code_object_iterate_symbols_fn;
    decltype(hsa_executable_create)* hsa_executable_create_fn;
    decltype(hsa_executable_destroy)* hsa_executable_destroy_fn;
    decltype(hsa_executable_load_code_object)* hsa_executable_load_code_object_fn;
    decltype(hsa_executable_freeze)* hsa_executable_freeze_fn;
    decltype(hsa_executable_get_info)* hsa_executable_get_info_fn;
    decltype(hsa_executable_global_variable_define)* hsa_executable_global_variable_define_fn;
    decltype(hsa_executable_agent_global_variable_define)* hsa_executable_agent_global_variable_define_fn;
    decltype(hsa_executable_readonly_variable_define)* hsa_executable_readonly_variable_define_fn;
    decltype(hsa_executable_validate)* hsa_executable_validate_fn;
    decltype(hsa_executable_get_symbol)* hsa_executable_get_symbol_fn;
    decltype(hsa_executable_symbol_get_info)* hsa_executable_symbol_get_info_fn;
    decltype(hsa_executable_iterate_symbols)* hsa_executable_iterate_symbols_fn;
    decltype(hsa_status_string)* hsa_status_string_fn;
};

// Table to export HSA Apis from Core Runtime, Amd Extensions
// Finalizer and Images
struct HsaApiTable1_2 {
    // Version of Hsa Api Table
    ApiTableVersion1_2 version;
  
    // Table of function pointers to HSA Core Runtime
    CoreApiTable1_2* core_;

    // Table of function pointers to AMD extensions
    AmdExtTable1_2* amd_ext_;

    // Table of function pointers to HSA Finalizer Extension
    FinalizerExtTable1_2* finalizer_ext_;
  
    // Table of function pointers to HSA Image Extension
    ImageExtTable1_2* image_ext_;
};

#endif // _HSA_API_TABLE_1_2_H_
