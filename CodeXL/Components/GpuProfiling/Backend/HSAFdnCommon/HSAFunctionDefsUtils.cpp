//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines Utilities that work with HSA_API_Type defined in HSAFunctionDefs.h
//==============================================================================

#include <unordered_map>

#include "Logger.h"
#include "HSAFunctionDefsUtils.h"

using namespace GPULogger;

HSAFunctionDefsUtils::HSAFunctionDefsUtils()
{
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_status_string"), HSA_API_Type_hsa_status_string));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_init"), HSA_API_Type_hsa_init));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_shut_down"), HSA_API_Type_hsa_shut_down));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_system_get_info"), HSA_API_Type_hsa_system_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_system_extension_supported"), HSA_API_Type_hsa_system_extension_supported));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_system_get_extension_table"), HSA_API_Type_hsa_system_get_extension_table));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_agent_get_info"), HSA_API_Type_hsa_agent_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_iterate_agents"), HSA_API_Type_hsa_iterate_agents));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_agent_get_exception_policies"), HSA_API_Type_hsa_agent_get_exception_policies));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_agent_extension_supported"), HSA_API_Type_hsa_agent_extension_supported));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_create"), HSA_API_Type_hsa_signal_create));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_destroy"), HSA_API_Type_hsa_signal_destroy));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_load_relaxed"), HSA_API_Type_hsa_signal_load_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_store_relaxed"), HSA_API_Type_hsa_signal_store_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_exchange_relaxed"), HSA_API_Type_hsa_signal_exchange_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_cas_relaxed"), HSA_API_Type_hsa_signal_cas_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_add_relaxed"), HSA_API_Type_hsa_signal_add_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_subtract_relaxed"), HSA_API_Type_hsa_signal_subtract_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_and_relaxed"), HSA_API_Type_hsa_signal_and_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_or_relaxed"), HSA_API_Type_hsa_signal_or_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_xor_relaxed"), HSA_API_Type_hsa_signal_xor_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_wait_relaxed"), HSA_API_Type_hsa_signal_wait_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_create"), HSA_API_Type_hsa_queue_create));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_soft_queue_create"), HSA_API_Type_hsa_soft_queue_create));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_destroy"), HSA_API_Type_hsa_queue_destroy));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_inactivate"), HSA_API_Type_hsa_queue_inactivate));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_load_read_index_relaxed"), HSA_API_Type_hsa_queue_load_read_index_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_load_write_index_relaxed"), HSA_API_Type_hsa_queue_load_write_index_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_store_write_index_relaxed"), HSA_API_Type_hsa_queue_store_write_index_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_cas_write_index_relaxed"), HSA_API_Type_hsa_queue_cas_write_index_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_add_write_index_relaxed"), HSA_API_Type_hsa_queue_add_write_index_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_store_read_index_relaxed"), HSA_API_Type_hsa_queue_store_read_index_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_region_get_info"), HSA_API_Type_hsa_region_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_agent_iterate_regions"), HSA_API_Type_hsa_agent_iterate_regions));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_memory_allocate"), HSA_API_Type_hsa_memory_allocate));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_memory_free"), HSA_API_Type_hsa_memory_free));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_memory_copy"), HSA_API_Type_hsa_memory_copy));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_memory_assign_agent"), HSA_API_Type_hsa_memory_assign_agent));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_memory_register"), HSA_API_Type_hsa_memory_register));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_memory_deregister"), HSA_API_Type_hsa_memory_deregister));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_isa_from_name"), HSA_API_Type_hsa_isa_from_name));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_isa_get_info"), HSA_API_Type_hsa_isa_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_isa_compatible"), HSA_API_Type_hsa_isa_compatible));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_create"), HSA_API_Type_hsa_executable_create));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_destroy"), HSA_API_Type_hsa_executable_destroy));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_freeze"), HSA_API_Type_hsa_executable_freeze));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_get_info"), HSA_API_Type_hsa_executable_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_global_variable_define"), HSA_API_Type_hsa_executable_global_variable_define));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_agent_global_variable_define"), HSA_API_Type_hsa_executable_agent_global_variable_define));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_readonly_variable_define"), HSA_API_Type_hsa_executable_readonly_variable_define));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_validate"), HSA_API_Type_hsa_executable_validate));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_get_symbol"), HSA_API_Type_hsa_executable_get_symbol));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_symbol_get_info"), HSA_API_Type_hsa_executable_symbol_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_iterate_symbols"), HSA_API_Type_hsa_executable_iterate_symbols));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_extension_get_name"), HSA_API_Type_hsa_extension_get_name));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_system_major_extension_supported"), HSA_API_Type_hsa_system_major_extension_supported));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_system_get_major_extension_table"), HSA_API_Type_hsa_system_get_major_extension_table));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_cache_get_info"), HSA_API_Type_hsa_cache_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_agent_iterate_caches"), HSA_API_Type_hsa_agent_iterate_caches));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_agent_major_extension_supported"), HSA_API_Type_hsa_agent_major_extension_supported));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_group_create"), HSA_API_Type_hsa_signal_group_create));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_group_destroy"), HSA_API_Type_hsa_signal_group_destroy));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_group_wait_any_scacquire"), HSA_API_Type_hsa_signal_group_wait_any_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_group_wait_any_relaxed"), HSA_API_Type_hsa_signal_group_wait_any_relaxed));
#ifdef FUTURE_ROCR_VERSION
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_load_scacquire"), HSA_API_Type_hsa_signal_load_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_store_screlease"), HSA_API_Type_hsa_signal_store_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_silent_store_relaxed"), HSA_API_Type_hsa_signal_silent_store_relaxed));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_silent_store_screlease"), HSA_API_Type_hsa_signal_silent_store_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_exchange_scacq_screl"), HSA_API_Type_hsa_signal_exchange_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_exchange_scacquire"), HSA_API_Type_hsa_signal_exchange_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_exchange_screlease"), HSA_API_Type_hsa_signal_exchange_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_cas_scacq_screl"), HSA_API_Type_hsa_signal_cas_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_cas_scacquire"), HSA_API_Type_hsa_signal_cas_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_cas_screlease"), HSA_API_Type_hsa_signal_cas_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_add_scacq_screl"), HSA_API_Type_hsa_signal_add_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_add_scacquire"), HSA_API_Type_hsa_signal_add_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_add_screlease"), HSA_API_Type_hsa_signal_add_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_subtract_scacq_screl"), HSA_API_Type_hsa_signal_subtract_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_subtract_scacquire"), HSA_API_Type_hsa_signal_subtract_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_subtract_screlease"), HSA_API_Type_hsa_signal_subtract_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_and_scacq_screl"), HSA_API_Type_hsa_signal_and_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_and_scacquire"), HSA_API_Type_hsa_signal_and_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_and_screlease"), HSA_API_Type_hsa_signal_and_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_or_scacq_screl"), HSA_API_Type_hsa_signal_or_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_or_scacquire"), HSA_API_Type_hsa_signal_or_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_or_screlease"), HSA_API_Type_hsa_signal_or_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_xor_scacq_screl"), HSA_API_Type_hsa_signal_xor_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_xor_scacquire"), HSA_API_Type_hsa_signal_xor_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_xor_screlease"), HSA_API_Type_hsa_signal_xor_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_wait_scacquire"), HSA_API_Type_hsa_signal_wait_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_load_read_index_scacquire"), HSA_API_Type_hsa_queue_load_read_index_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_load_write_index_scacquire"), HSA_API_Type_hsa_queue_load_write_index_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_store_write_index_screlease"), HSA_API_Type_hsa_queue_store_write_index_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_cas_write_index_scacq_screl"), HSA_API_Type_hsa_queue_cas_write_index_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_cas_write_index_scacquire"), HSA_API_Type_hsa_queue_cas_write_index_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_cas_write_index_screlease"), HSA_API_Type_hsa_queue_cas_write_index_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_add_write_index_scacq_screl"), HSA_API_Type_hsa_queue_add_write_index_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_add_write_index_scacquire"), HSA_API_Type_hsa_queue_add_write_index_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_add_write_index_screlease"), HSA_API_Type_hsa_queue_add_write_index_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_store_read_index_screlease"), HSA_API_Type_hsa_queue_store_read_index_screlease));
#else
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_load_acquire"), HSA_API_Type_hsa_signal_load_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_store_release"), HSA_API_Type_hsa_signal_store_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_exchange_acquire"), HSA_API_Type_hsa_signal_exchange_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_exchange_release"), HSA_API_Type_hsa_signal_exchange_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_cas_acq_rel"), HSA_API_Type_hsa_signal_cas_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_cas_acquire"), HSA_API_Type_hsa_signal_cas_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_cas_release"), HSA_API_Type_hsa_signal_cas_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_add_acq_rel"), HSA_API_Type_hsa_signal_add_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_add_acquire"), HSA_API_Type_hsa_signal_add_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_add_release"), HSA_API_Type_hsa_signal_add_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_subtract_acq_rel"), HSA_API_Type_hsa_signal_subtract_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_subtract_acquire"), HSA_API_Type_hsa_signal_subtract_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_subtract_release"), HSA_API_Type_hsa_signal_subtract_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_and_acq_rel"), HSA_API_Type_hsa_signal_and_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_and_acquire"), HSA_API_Type_hsa_signal_and_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_and_release"), HSA_API_Type_hsa_signal_and_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_or_acq_rel"), HSA_API_Type_hsa_signal_or_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_or_acquire"), HSA_API_Type_hsa_signal_or_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_or_release"), HSA_API_Type_hsa_signal_or_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_xor_acq_rel"), HSA_API_Type_hsa_signal_xor_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_xor_acquire"), HSA_API_Type_hsa_signal_xor_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_xor_release"), HSA_API_Type_hsa_signal_xor_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_wait_acquire"), HSA_API_Type_hsa_signal_wait_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_load_read_index_acquire"), HSA_API_Type_hsa_queue_load_read_index_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_load_write_index_acquire"), HSA_API_Type_hsa_queue_load_write_index_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_store_write_index_release"), HSA_API_Type_hsa_queue_store_write_index_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_cas_write_index_acq_rel"), HSA_API_Type_hsa_queue_cas_write_index_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_cas_write_index_acquire"), HSA_API_Type_hsa_queue_cas_write_index_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_cas_write_index_release"), HSA_API_Type_hsa_queue_cas_write_index_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_add_write_index_acq_rel"), HSA_API_Type_hsa_queue_add_write_index_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_add_write_index_acquire"), HSA_API_Type_hsa_queue_add_write_index_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_add_write_index_release"), HSA_API_Type_hsa_queue_add_write_index_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_store_read_index_release"), HSA_API_Type_hsa_queue_store_read_index_release));

    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_load_scacquire"), HSA_API_Type_hsa_signal_load_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_store_screlease"), HSA_API_Type_hsa_signal_store_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_exchange_scacq_screl"), HSA_API_Type_hsa_signal_exchange_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_exchange_scacquire"), HSA_API_Type_hsa_signal_exchange_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_exchange_screlease"), HSA_API_Type_hsa_signal_exchange_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_cas_scacq_screl"), HSA_API_Type_hsa_signal_cas_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_cas_scacquire"), HSA_API_Type_hsa_signal_cas_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_cas_screlease"), HSA_API_Type_hsa_signal_cas_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_add_scacq_screl"), HSA_API_Type_hsa_signal_add_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_add_scacquire"), HSA_API_Type_hsa_signal_add_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_add_screlease"), HSA_API_Type_hsa_signal_add_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_subtract_scacq_screl"), HSA_API_Type_hsa_signal_subtract_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_subtract_scacquire"), HSA_API_Type_hsa_signal_subtract_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_subtract_screlease"), HSA_API_Type_hsa_signal_subtract_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_and_scacq_screl"), HSA_API_Type_hsa_signal_and_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_and_scacquire"), HSA_API_Type_hsa_signal_and_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_and_screlease"), HSA_API_Type_hsa_signal_and_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_or_scacq_screl"), HSA_API_Type_hsa_signal_or_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_or_scacquire"), HSA_API_Type_hsa_signal_or_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_or_screlease"), HSA_API_Type_hsa_signal_or_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_xor_scacq_screl"), HSA_API_Type_hsa_signal_xor_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_xor_scacquire"), HSA_API_Type_hsa_signal_xor_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_xor_screlease"), HSA_API_Type_hsa_signal_xor_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_wait_scacquire"), HSA_API_Type_hsa_signal_wait_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_load_read_index_scacquire"), HSA_API_Type_hsa_queue_load_read_index_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_load_write_index_scacquire"), HSA_API_Type_hsa_queue_load_write_index_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_store_write_index_screlease"), HSA_API_Type_hsa_queue_store_write_index_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_cas_write_index_scacq_screl"), HSA_API_Type_hsa_queue_cas_write_index_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_cas_write_index_scacquire"), HSA_API_Type_hsa_queue_cas_write_index_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_cas_write_index_screlease"), HSA_API_Type_hsa_queue_cas_write_index_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_add_write_index_scacq_screl"), HSA_API_Type_hsa_queue_add_write_index_acq_rel));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_add_write_index_scacquire"), HSA_API_Type_hsa_queue_add_write_index_acquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_add_write_index_screlease"), HSA_API_Type_hsa_queue_add_write_index_release));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_store_read_index_screlease"), HSA_API_Type_hsa_queue_store_read_index_release));
#endif
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_agent_iterate_isas"), HSA_API_Type_hsa_agent_iterate_isas));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_isa_get_info_alt"), HSA_API_Type_hsa_isa_get_info_alt));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_isa_get_exception_policies"), HSA_API_Type_hsa_isa_get_exception_policies));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_isa_get_round_method"), HSA_API_Type_hsa_isa_get_round_method));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_wavefront_get_info"), HSA_API_Type_hsa_wavefront_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_isa_iterate_wavefronts"), HSA_API_Type_hsa_isa_iterate_wavefronts));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_code_object_reader_create_from_file"), HSA_API_Type_hsa_code_object_reader_create_from_file));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_code_object_reader_create_from_memory"), HSA_API_Type_hsa_code_object_reader_create_from_memory));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_code_object_reader_destroy"), HSA_API_Type_hsa_code_object_reader_destroy));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_create_alt"), HSA_API_Type_hsa_executable_create_alt));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_load_program_code_object"), HSA_API_Type_hsa_executable_load_program_code_object));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_load_agent_code_object"), HSA_API_Type_hsa_executable_load_agent_code_object));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_validate_alt"), HSA_API_Type_hsa_executable_validate_alt));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_get_symbol_by_name"), HSA_API_Type_hsa_executable_get_symbol_by_name));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_iterate_agent_symbols"), HSA_API_Type_hsa_executable_iterate_agent_symbols));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_iterate_program_symbols"), HSA_API_Type_hsa_executable_iterate_program_symbols));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_code_object_serialize"), HSA_API_Type_hsa_code_object_serialize));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_code_object_deserialize"), HSA_API_Type_hsa_code_object_deserialize));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_code_object_destroy"), HSA_API_Type_hsa_code_object_destroy));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_code_object_get_info"), HSA_API_Type_hsa_code_object_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_executable_load_code_object"), HSA_API_Type_hsa_executable_load_code_object));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_code_object_get_symbol"), HSA_API_Type_hsa_code_object_get_symbol));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_code_object_get_symbol_from_name"), HSA_API_Type_hsa_code_object_get_symbol_from_name));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_code_symbol_get_info"), HSA_API_Type_hsa_code_symbol_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_code_object_iterate_symbols"), HSA_API_Type_hsa_code_object_iterate_symbols));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_program_create"), HSA_API_Type_hsa_ext_program_create));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_program_destroy"), HSA_API_Type_hsa_ext_program_destroy));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_program_add_module"), HSA_API_Type_hsa_ext_program_add_module));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_program_iterate_modules"), HSA_API_Type_hsa_ext_program_iterate_modules));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_program_get_info"), HSA_API_Type_hsa_ext_program_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_program_finalize"), HSA_API_Type_hsa_ext_program_finalize));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_image_get_capability"), HSA_API_Type_hsa_ext_image_get_capability));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_image_data_get_info"), HSA_API_Type_hsa_ext_image_data_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_image_create"), HSA_API_Type_hsa_ext_image_create));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_image_destroy"), HSA_API_Type_hsa_ext_image_destroy));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_image_copy"), HSA_API_Type_hsa_ext_image_copy));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_image_import"), HSA_API_Type_hsa_ext_image_import));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_image_export"), HSA_API_Type_hsa_ext_image_export));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_image_clear"), HSA_API_Type_hsa_ext_image_clear));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_sampler_create"), HSA_API_Type_hsa_ext_sampler_create));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ext_sampler_destroy"), HSA_API_Type_hsa_ext_sampler_destroy));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_coherency_get_type"), HSA_API_Type_hsa_amd_coherency_get_type));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_coherency_set_type"), HSA_API_Type_hsa_amd_coherency_set_type));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_profiling_set_profiler_enabled"), HSA_API_Type_hsa_amd_profiling_set_profiler_enabled));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_profiling_async_copy_enable"), HSA_API_Type_hsa_amd_profiling_async_copy_enable));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_profiling_get_dispatch_time"), HSA_API_Type_hsa_amd_profiling_get_dispatch_time));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_profiling_get_async_copy_time"), HSA_API_Type_hsa_amd_profiling_get_async_copy_time));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_profiling_convert_tick_to_system_domain"), HSA_API_Type_hsa_amd_profiling_convert_tick_to_system_domain));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_signal_async_handler"), HSA_API_Type_hsa_amd_signal_async_handler));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_async_function"), HSA_API_Type_hsa_amd_async_function));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_signal_wait_any"), HSA_API_Type_hsa_amd_signal_wait_any));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_image_get_info_max_dim"), HSA_API_Type_hsa_amd_image_get_info_max_dim));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_queue_cu_set_mask"), HSA_API_Type_hsa_amd_queue_cu_set_mask));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_memory_pool_get_info"), HSA_API_Type_hsa_amd_memory_pool_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_agent_iterate_memory_pools"), HSA_API_Type_hsa_amd_agent_iterate_memory_pools));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_memory_pool_allocate"), HSA_API_Type_hsa_amd_memory_pool_allocate));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_memory_pool_free"), HSA_API_Type_hsa_amd_memory_pool_free));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_memory_async_copy"), HSA_API_Type_hsa_amd_memory_async_copy));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_agent_memory_pool_get_info"), HSA_API_Type_hsa_amd_agent_memory_pool_get_info));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_agents_allow_access"), HSA_API_Type_hsa_amd_agents_allow_access));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_memory_pool_can_migrate"), HSA_API_Type_hsa_amd_memory_pool_can_migrate));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_memory_migrate"), HSA_API_Type_hsa_amd_memory_migrate));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_memory_lock"), HSA_API_Type_hsa_amd_memory_lock));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_memory_unlock"), HSA_API_Type_hsa_amd_memory_unlock));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_memory_fill"), HSA_API_Type_hsa_amd_memory_fill));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_interop_map_buffer"), HSA_API_Type_hsa_amd_interop_map_buffer));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_interop_unmap_buffer"), HSA_API_Type_hsa_amd_interop_unmap_buffer));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_amd_image_create"), HSA_API_Type_hsa_amd_image_create));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ven_amd_loader_query_host_address"), HSA_API_Type_hsa_ven_amd_loader_query_host_address));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_ven_amd_loader_query_segment_descriptors"), HSA_API_Type_hsa_ven_amd_loader_query_segment_descriptors));


#ifdef FUTURE_ROCR_VERSION
    // add mapping for ROCm 1.2 functions to the ROCm 1.3 enum values
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_load_read_index_acquire"), HSA_API_Type_hsa_queue_load_read_index_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_load_write_index_acquire"), HSA_API_Type_hsa_queue_load_write_index_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_store_write_index_release"), HSA_API_Type_hsa_queue_store_write_index_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_cas_write_index_acq_rel"), HSA_API_Type_hsa_queue_cas_write_index_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_cas_write_index_acquire"), HSA_API_Type_hsa_queue_cas_write_index_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_cas_write_index_release"), HSA_API_Type_hsa_queue_cas_write_index_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_add_write_index_acq_rel"), HSA_API_Type_hsa_queue_add_write_index_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_add_write_index_acquire"), HSA_API_Type_hsa_queue_add_write_index_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_add_write_index_release"), HSA_API_Type_hsa_queue_add_write_index_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_queue_store_read_index_release"), HSA_API_Type_hsa_queue_store_read_index_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_load_acquire"), HSA_API_Type_hsa_signal_load_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_store_release"), HSA_API_Type_hsa_signal_store_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_wait_acquire"), HSA_API_Type_hsa_signal_wait_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_and_acquire"), HSA_API_Type_hsa_signal_and_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_and_release"), HSA_API_Type_hsa_signal_and_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_and_acq_rel"), HSA_API_Type_hsa_signal_and_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_or_acquire"), HSA_API_Type_hsa_signal_or_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_or_release"), HSA_API_Type_hsa_signal_or_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_or_acq_rel"), HSA_API_Type_hsa_signal_or_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_xor_acquire"), HSA_API_Type_hsa_signal_xor_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_xor_release"), HSA_API_Type_hsa_signal_xor_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_xor_acq_rel"), HSA_API_Type_hsa_signal_xor_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_exchange_acquire"), HSA_API_Type_hsa_signal_exchange_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_exchange_release"), HSA_API_Type_hsa_signal_exchange_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_exchange_acq_rel"), HSA_API_Type_hsa_signal_exchange_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_add_acquire"), HSA_API_Type_hsa_signal_add_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_add_release"), HSA_API_Type_hsa_signal_add_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_add_acq_rel"), HSA_API_Type_hsa_signal_add_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_subtract_acquire"), HSA_API_Type_hsa_signal_subtract_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_subtract_release"), HSA_API_Type_hsa_signal_subtract_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_subtract_acq_rel"), HSA_API_Type_hsa_signal_subtract_scacq_screl));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_cas_acquire"), HSA_API_Type_hsa_signal_cas_scacquire));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_cas_release"), HSA_API_Type_hsa_signal_cas_screlease));
    m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_signal_cas_acq_rel"), HSA_API_Type_hsa_signal_cas_scacq_screl));
#endif
}

HSA_API_Type HSAFunctionDefsUtils::ToHSAAPIType(const std::string& strName)
{
    std::unordered_map<std::string, HSA_API_Type>::iterator it = m_hsaAPIMap.find(strName);
    SpAssert(it != m_hsaAPIMap.end());

    if (it != m_hsaAPIMap.end())
    {
        return it->second;
    }
    else
    {
        return HSA_API_Type_UNKNOWN;
    }
}
