//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/HSAAPIDefs.cpp $
/// \version $Revision: #10 $
/// \brief  This file contains definitions for CL API Functions
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/HSAAPIDefs.cpp#10 $
// Last checkin:   $DateTime: 2014/06/02 10:13:39 $
// Last edited by: $Author: agaisins $
// Change list:    $Change: 497321 $
//=====================================================================

#include <qtIgnoreCompilerWarnings.h>

#ifdef _WIN32
    #pragma warning(push, 1)
#endif
#include <QtCore>
#include <QtWidgets>
#ifdef _WIN32
    #pragma warning(pop)
#endif

#include <AMDTBaseTools/Include/gtAssert.h>


#include "HSAAPIDefs.h"


HSAAPIDefs::HSAAPIDefs()
{
    BuildAPIFunctionNamesMap();



    BuildAPIGroupsMap();

}

const QString& HSAAPIDefs::GetHSAPIString(HSA_API_Type type)
{
    if ((type < HSA_API_Type_UNKNOWN) || !m_hsaTypeTofuncNameMap.contains(type))
    {
        type = HSA_API_Type_UNKNOWN;
        GT_ASSERT_EX(false, L"Unknown HSA API type");
    }

    return m_hsaTypeTofuncNameMap[type];
}

HSAAPIGroups HSAAPIDefs::GetHSAAPIGroup(HSA_API_Type type)
{
    HSAAPIGroups retVal = HSAAPIGroup_Unknown;

    if (m_HSAAPIGroupMap.contains(type))
    {
        retVal = m_HSAAPIGroupMap[type];
    }

    return retVal;
}

HSA_API_Type HSAAPIDefs::ToHSAAPIType(const QString& name)
{
    HSA_API_Type retVal = HSA_API_Type_UNKNOWN;

    if (m_funcNameToHSATypeMap.contains(name))
    {
        retVal = m_funcNameToHSATypeMap[name];
    }

    return retVal;
}

const QString HSAAPIDefs::GroupToString(HSAAPIGroup group)
{
    switch (group)
    {
        case HSAAPIGroup_CodeObject:
            return "CodeObject";

        case HSAAPIGroup_Agent:
            return "Agent";

        case HSAAPIGroup_Executable:
            return "Executable";

        case HSAAPIGroup_ExtensionsGeneral:
            return "Extensions General";

        case HSAAPIGroup_ExtensionsFinalizer:
            return "Extensions Finalizer";

        case HSAAPIGroup_ExtensionsImage:
            return "Extensions Image";

        case HSAAPIGroup_ExtensionsSampler:
            return "Extensions Sampler";

        case HSAAPIGroup_InitShutDown:
            return "Init / Shut down";

        case HSAAPIGroup_ISA:
            return "ISA";

        case HSAAPIGroup_Memory:
            return "Memory";

        case HSAAPIGroup_QueryInfo:
            return "Query Info";

        case HSAAPIGroup_Queue:
            return "Queue";

        case HSAAPIGroup_Signal:
            return "Signal";

        case HSAAPIGroup_ExtensionsAMD:
            return "Extensions AMD";

        default:
            return "Other";
    }
}

void HSAAPIDefs::BuildAPIGroupsMap()
{
    // HSAAPIGroup_Agent:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_iterate_agents, HSAAPIGroup_Agent);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_agent_iterate_regions, HSAAPIGroup_Agent);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_agent_get_exception_policies, HSAAPIGroup_Agent);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_agent_iterate_caches, HSAAPIGroup_Agent);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_agent_major_extension_supported, HSAAPIGroup_Agent);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_agent_iterate_isas, HSAAPIGroup_ISA);

    // HSAAPIGroup_CodeObject:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_serialize, HSAAPIGroup_CodeObject);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_deserialize, HSAAPIGroup_CodeObject);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_destroy, HSAAPIGroup_CodeObject);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_get_symbol, HSAAPIGroup_CodeObject);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_iterate_symbols, HSAAPIGroup_CodeObject);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_reader_create_from_file, HSAAPIGroup_CodeObject);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_reader_create_from_memory, HSAAPIGroup_CodeObject);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_reader_destroy, HSAAPIGroup_CodeObject);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_get_symbol_from_name, HSAAPIGroup_CodeObject);

    // HSAAPIGroup_Executable:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_create, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_destroy, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_load_code_object, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_freeze, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_global_variable_define, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_agent_global_variable_define, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_readonly_variable_define, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_validate, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_get_symbol, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_iterate_symbols, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_create_alt, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_load_program_code_object, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_load_agent_code_object, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_validate_alt, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_get_symbol_by_name, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_iterate_agent_symbols, HSAAPIGroup_Executable);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_iterate_program_symbols, HSAAPIGroup_Executable);

    // HSAAPIGroup_ExtensionsGeneral:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_system_extension_supported, HSAAPIGroup_ExtensionsGeneral);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_system_get_extension_table, HSAAPIGroup_ExtensionsGeneral);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_agent_extension_supported, HSAAPIGroup_ExtensionsGeneral);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_system_major_extension_supported, HSAAPIGroup_ExtensionsGeneral);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_system_get_major_extension_table, HSAAPIGroup_ExtensionsGeneral);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_agent_major_extension_supported, HSAAPIGroup_ExtensionsGeneral);

    // HSAAPIGroup_ExtensionsFinalizer:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_program_create, HSAAPIGroup_ExtensionsFinalizer);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_program_destroy, HSAAPIGroup_ExtensionsFinalizer);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_program_add_module, HSAAPIGroup_ExtensionsFinalizer);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_program_iterate_modules, HSAAPIGroup_ExtensionsFinalizer);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_program_get_info, HSAAPIGroup_ExtensionsFinalizer);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_program_finalize, HSAAPIGroup_ExtensionsFinalizer);

    // HSAAPIGroup_ExtensionsImage:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_image_get_capability, HSAAPIGroup_ExtensionsImage);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_image_data_get_info, HSAAPIGroup_ExtensionsImage);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_image_create, HSAAPIGroup_ExtensionsImage);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_image_import, HSAAPIGroup_ExtensionsImage);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_image_export, HSAAPIGroup_ExtensionsImage);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_image_copy, HSAAPIGroup_ExtensionsImage);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_image_clear, HSAAPIGroup_ExtensionsImage);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_image_destroy, HSAAPIGroup_ExtensionsImage);

    // HSAAPIGroup_ExtensionsAMD:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_coherency_get_type, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_coherency_set_type, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_profiling_set_profiler_enabled, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_profiling_async_copy_enable, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_profiling_get_dispatch_time, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_profiling_get_async_copy_time, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_profiling_convert_tick_to_system_domain, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_signal_create, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_signal_async_handler, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_async_function, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_signal_wait_any, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_image_get_info_max_dim, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_queue_cu_set_mask, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_memory_pool_get_info, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_agent_iterate_memory_pools, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_memory_pool_allocate, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_memory_pool_free, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_memory_async_copy, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_agent_memory_pool_get_info, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_agents_allow_access, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_memory_pool_can_migrate, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_memory_migrate, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_memory_lock, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_memory_unlock, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_memory_fill, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_interop_map_buffer, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_interop_unmap_buffer, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_image_create, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_ipc_signal_create, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_amd_ipc_signal_attach, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ven_amd_loader_query_host_address, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ven_amd_loader_query_segment_descriptors, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ven_amd_loader_query_executable, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ven_amd_loader_executable_iterate_loaded_code_objects, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ven_amd_loader_loaded_code_object_get_info, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ven_amd_aqlprofile_validate_event, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ven_amd_aqlprofile_start, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ven_amd_aqlprofile_stop, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ven_amd_aqlprofile_legacy_get_pm4, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ven_amd_aqlprofile_get_info, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ven_amd_aqlprofile_iterate_data, HSAAPIGroup_ExtensionsAMD);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ven_amd_aqlprofile_error_string, HSAAPIGroup_ExtensionsAMD);

    // HSAAPIGroup_ExtensionsSampler:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_sampler_create, HSAAPIGroup_ExtensionsSampler);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_sampler_destroy, HSAAPIGroup_ExtensionsSampler);

    // HSAAPIGroup_InitShutDown:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_init, HSAAPIGroup_InitShutDown);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_shut_down, HSAAPIGroup_InitShutDown);

    // HSAAPIGroup_ISA:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_isa_from_name, HSAAPIGroup_ISA);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_isa_compatible, HSAAPIGroup_ISA);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_isa_get_exception_policies, HSAAPIGroup_ISA);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_isa_get_round_method, HSAAPIGroup_ISA);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_isa_iterate_wavefronts, HSAAPIGroup_ISA);

    // HSAAPIGroup_Memory:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_memory_register, HSAAPIGroup_Memory);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_memory_deregister, HSAAPIGroup_Memory);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_memory_allocate, HSAAPIGroup_Memory);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_memory_free, HSAAPIGroup_Memory);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_memory_copy, HSAAPIGroup_Memory);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_memory_assign_agent, HSAAPIGroup_Memory);

    // HSAAPIGroup_QueryInfo
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_system_get_info, HSAAPIGroup_QueryInfo);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_agent_get_info, HSAAPIGroup_QueryInfo);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_region_get_info, HSAAPIGroup_QueryInfo);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_isa_get_info, HSAAPIGroup_QueryInfo);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_get_info, HSAAPIGroup_QueryInfo);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_symbol_get_info, HSAAPIGroup_QueryInfo);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_get_info, HSAAPIGroup_QueryInfo);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_executable_symbol_get_info, HSAAPIGroup_QueryInfo);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_cache_get_info, HSAAPIGroup_QueryInfo);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_isa_get_info_alt, HSAAPIGroup_QueryInfo);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_wavefront_get_info, HSAAPIGroup_QueryInfo);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_status_string, HSAAPIGroup_QueryInfo);

    // HSAAPIGroup_Queue:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_create, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_soft_queue_create, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_destroy, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_inactivate, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_load_read_index_relaxed, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_load_write_index_relaxed, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_store_write_index_relaxed, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_cas_write_index_relaxed, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_add_write_index_relaxed, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_store_read_index_relaxed, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_load_read_index_scacquire, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_load_write_index_scacquire, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_store_write_index_screlease, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_cas_write_index_scacq_screl, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_cas_write_index_scacquire, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_cas_write_index_screlease, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_add_write_index_scacq_screl, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_add_write_index_scacquire, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_add_write_index_screlease, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_store_read_index_screlease, HSAAPIGroup_Queue);

    // HSAAPIGroup_Signal:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_create, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_destroy, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_load_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_store_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_wait_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_and_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_or_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_xor_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_exchange_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_add_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_subtract_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_cas_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_group_create, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_group_destroy, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_group_wait_any_scacquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_group_wait_any_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_load_scacquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_store_screlease, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_wait_scacquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_and_scacquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_and_screlease, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_and_scacq_screl, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_or_scacquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_or_screlease, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_or_scacq_screl, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_xor_scacquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_xor_screlease, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_xor_scacq_screl, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_exchange_scacquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_exchange_screlease, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_exchange_scacq_screl, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_add_scacquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_add_screlease, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_add_scacq_screl, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_subtract_scacquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_subtract_screlease, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_subtract_scacq_screl, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_cas_scacquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_cas_screlease, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_cas_scacq_screl, HSAAPIGroup_Signal);
}

void HSAAPIDefs::BuildAPIFunctionNamesMap()
{
#define X(SYM) AddFunctionToMap(HSA_API_Type_hsa_##SYM, "hsa_" #SYM);
    HSA_RUNTIME_API_TABLE
    HSA_EXT_FINALIZE_API_TABLE
    HSA_EXT_IMAGE_API_TABLE
    HSA_EXT_AMD_API_TABLE
    HSA_VEN_AMD_LOADER_API_TABLE
    HSA_VEN_AMD_AQL_PROFILE_API_TABLE
#undef X
}

void HSAAPIDefs::AddFunctionToMap(HSA_API_Type hsaType, const QString& funcName)
{
    // Insert function to both maps:
    m_hsaTypeTofuncNameMap.insert(hsaType, funcName);
    m_funcNameToHSATypeMap.insert(funcName, hsaType);
    m_hsaAPIStringsList << funcName;
}

