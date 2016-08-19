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

#ifndef HSA_RUNTIME_INC_HSA_API_TRACE_H
#define HSA_RUNTIME_INC_HSA_API_TRACE_H

#include "hsa.h"
#ifdef AMD_INTERNAL_BUILD
#include "hsa_ext_image.h"
#include "hsa_ext_amd.h"
#include "hsa_ext_finalize.h"
#else
#include "inc/hsa_ext_image.h"
#include "inc/hsa_ext_amd.h"
#include "inc/hsa_ext_finalize.h"
#endif

struct ExtTable {
	decltype(hsa_ext_program_create)* hsa_ext_program_create_fn;
	decltype(hsa_ext_program_destroy)* hsa_ext_program_destroy_fn;
	decltype(hsa_ext_program_add_module)* hsa_ext_program_add_module_fn;
	decltype(hsa_ext_program_iterate_modules)* hsa_ext_program_iterate_modules_fn;
	decltype(hsa_ext_program_get_info)* hsa_ext_program_get_info_fn;
	decltype(hsa_ext_program_finalize)* hsa_ext_program_finalize_fn;
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

struct ApiTable {
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

	ExtTable* std_exts_;
};

#endif
