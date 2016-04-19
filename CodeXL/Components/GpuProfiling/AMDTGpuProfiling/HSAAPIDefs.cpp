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

    // HSAAPIGroup_CodeObject:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_serialize, HSAAPIGroup_CodeObject);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_deserialize, HSAAPIGroup_CodeObject);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_destroy, HSAAPIGroup_CodeObject);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_get_symbol, HSAAPIGroup_CodeObject);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_code_object_iterate_symbols, HSAAPIGroup_CodeObject);

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
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_status_string, HSAAPIGroup_Executable);

    // HSAAPIGroup_ExtensionsGeneral:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_system_extension_supported, HSAAPIGroup_ExtensionsGeneral);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_system_get_extension_table, HSAAPIGroup_ExtensionsGeneral);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_agent_extension_supported, HSAAPIGroup_ExtensionsGeneral);

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

    // HSAAPIGroup_ExtensionsSampler:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_sampler_create, HSAAPIGroup_ExtensionsSampler);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_ext_sampler_destroy, HSAAPIGroup_ExtensionsSampler);

    // HSAAPIGroup_InitShutDown:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_init, HSAAPIGroup_InitShutDown);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_shut_down, HSAAPIGroup_InitShutDown);

    // HSAAPIGroup_ISA:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_isa_from_name, HSAAPIGroup_ISA);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_isa_compatible, HSAAPIGroup_ISA);

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

    // HSAAPIGroup_Queue:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_create, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_soft_queue_create, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_destroy, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_inactivate, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_load_read_index_acquire, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_load_read_index_relaxed, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_load_write_index_acquire, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_load_write_index_relaxed, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_store_write_index_relaxed, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_store_write_index_release, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_cas_write_index_acq_rel, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_cas_write_index_acquire, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_cas_write_index_relaxed, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_cas_write_index_release, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_add_write_index_acq_rel, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_add_write_index_acquire, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_add_write_index_relaxed, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_add_write_index_release, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_store_read_index_relaxed, HSAAPIGroup_Queue);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_queue_store_read_index_release, HSAAPIGroup_Queue);

    // HSAAPIGroup_Signal:
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_create, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_destroy, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_load_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_load_acquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_store_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_store_release, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_wait_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_wait_acquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_and_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_and_acquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_and_release, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_and_acq_rel, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_or_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_or_acquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_or_release, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_or_acq_rel, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_xor_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_xor_acquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_xor_release, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_xor_acq_rel, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_exchange_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_exchange_acquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_exchange_release, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_exchange_acq_rel, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_add_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_add_acquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_add_release, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_add_acq_rel, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_subtract_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_subtract_acquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_subtract_release, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_subtract_acq_rel, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_cas_relaxed, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_cas_acquire, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_cas_release, HSAAPIGroup_Signal);
    m_HSAAPIGroupMap.insert(HSA_API_Type_hsa_signal_cas_acq_rel, HSAAPIGroup_Signal);
}

void HSAAPIDefs::BuildAPIFunctionNamesMap()
{
    AddFunctionToMap(HSA_API_Type_hsa_iterate_agents, "hsa_iterate_agents");
    AddFunctionToMap(HSA_API_Type_hsa_agent_iterate_regions, "hsa_agent_iterate_regions");
    AddFunctionToMap(HSA_API_Type_hsa_agent_get_exception_policies, "hsa_agent_get_exception_policies");
    AddFunctionToMap(HSA_API_Type_hsa_code_object_serialize, "hsa_code_object_serialize");
    AddFunctionToMap(HSA_API_Type_hsa_code_object_deserialize, "hsa_code_object_deserialize");
    AddFunctionToMap(HSA_API_Type_hsa_code_object_destroy, "hsa_code_object_destroy");
    AddFunctionToMap(HSA_API_Type_hsa_code_object_get_symbol, "hsa_code_object_get_symbol");
    AddFunctionToMap(HSA_API_Type_hsa_code_object_iterate_symbols, "hsa_code_object_iterate_symbols");
    AddFunctionToMap(HSA_API_Type_hsa_executable_create, "hsa_executable_create");
    AddFunctionToMap(HSA_API_Type_hsa_executable_destroy, "hsa_executable_destroy");
    AddFunctionToMap(HSA_API_Type_hsa_executable_load_code_object, "hsa_executable_load_code_object");
    AddFunctionToMap(HSA_API_Type_hsa_executable_freeze, "hsa_executable_freeze");
    AddFunctionToMap(HSA_API_Type_hsa_executable_global_variable_define, "hsa_executable_global_variable_define");
    AddFunctionToMap(HSA_API_Type_hsa_executable_agent_global_variable_define, "hsa_executable_agent_global_variable_define");
    AddFunctionToMap(HSA_API_Type_hsa_executable_readonly_variable_define, "hsa_executable_readonly_variable_define");
    AddFunctionToMap(HSA_API_Type_hsa_executable_validate, "hsa_executable_validate");
    AddFunctionToMap(HSA_API_Type_hsa_executable_get_symbol, "hsa_executable_get_symbol");
    AddFunctionToMap(HSA_API_Type_hsa_executable_iterate_symbols, "hsa_executable_iterate_symbols");
    AddFunctionToMap(HSA_API_Type_hsa_status_string, "hsa_status_string");
    AddFunctionToMap(HSA_API_Type_hsa_system_extension_supported, "hsa_system_extension_supported");
    AddFunctionToMap(HSA_API_Type_hsa_system_get_extension_table, "hsa_system_get_extension_table");
    AddFunctionToMap(HSA_API_Type_hsa_agent_extension_supported, "hsa_agent_extension_supported");
    AddFunctionToMap(HSA_API_Type_hsa_ext_program_create, "hsa_ext_program_create");
    AddFunctionToMap(HSA_API_Type_hsa_ext_program_destroy, "hsa_ext_program_destroy");
    AddFunctionToMap(HSA_API_Type_hsa_ext_program_add_module, "hsa_ext_program_add_module");
    AddFunctionToMap(HSA_API_Type_hsa_ext_program_iterate_modules, "hsa_ext_program_iterate_module");
    AddFunctionToMap(HSA_API_Type_hsa_ext_program_get_info, "hsa_ext_program_get_info");
    AddFunctionToMap(HSA_API_Type_hsa_ext_program_finalize, "hsa_ext_program_finalize ");
    AddFunctionToMap(HSA_API_Type_hsa_ext_image_get_capability, "hsa_ext_image_get_capability");
    AddFunctionToMap(HSA_API_Type_hsa_ext_image_data_get_info, "hsa_ext_image_data_get_info");
    AddFunctionToMap(HSA_API_Type_hsa_ext_image_create, "hsa_ext_image_create");
    AddFunctionToMap(HSA_API_Type_hsa_ext_image_import, "hsa_ext_image_import");
    AddFunctionToMap(HSA_API_Type_hsa_ext_image_export, "hsa_ext_image_export");
    AddFunctionToMap(HSA_API_Type_hsa_ext_image_copy, "hsa_ext_image_copy");
    AddFunctionToMap(HSA_API_Type_hsa_ext_image_clear, "hsa_ext_image_clear");
    AddFunctionToMap(HSA_API_Type_hsa_ext_image_destroy, "hsa_ext_image_destroy");
    AddFunctionToMap(HSA_API_Type_hsa_ext_sampler_create, "hsa_ext_sampler_create");
    AddFunctionToMap(HSA_API_Type_hsa_ext_sampler_destroy, "hsa_ext_sampler_destroy");
    AddFunctionToMap(HSA_API_Type_hsa_init, "hsa_init");
    AddFunctionToMap(HSA_API_Type_hsa_shut_down, "hsa_shut_down");
    AddFunctionToMap(HSA_API_Type_hsa_isa_from_name, "hsa_isa_from_name");
    AddFunctionToMap(HSA_API_Type_hsa_isa_compatible, "hsa_isa_compatible");
    AddFunctionToMap(HSA_API_Type_hsa_memory_register, "hsa_memory_register");
    AddFunctionToMap(HSA_API_Type_hsa_memory_deregister, "hsa_memory_deregister");
    AddFunctionToMap(HSA_API_Type_hsa_memory_allocate, "hsa_memory_allocate");
    AddFunctionToMap(HSA_API_Type_hsa_memory_free, "hsa_memory_free");
    AddFunctionToMap(HSA_API_Type_hsa_memory_copy, "hsa_memory_copy");
    AddFunctionToMap(HSA_API_Type_hsa_memory_assign_agent, "hsa_memory_assign_agent");
    AddFunctionToMap(HSA_API_Type_hsa_system_get_info, "hsa_system_get_info");
    AddFunctionToMap(HSA_API_Type_hsa_agent_get_info, "hsa_agent_get_info");
    AddFunctionToMap(HSA_API_Type_hsa_region_get_info, "hsa_region_get_info");
    AddFunctionToMap(HSA_API_Type_hsa_isa_get_info, "hsa_isa_get_info");
    AddFunctionToMap(HSA_API_Type_hsa_code_object_get_info, "hsa_code_object_get_info");
    AddFunctionToMap(HSA_API_Type_hsa_code_symbol_get_info, "hsa_code_symbol_get_info");
    AddFunctionToMap(HSA_API_Type_hsa_executable_get_info, "hsa_executable_get_info");
    AddFunctionToMap(HSA_API_Type_hsa_executable_symbol_get_info, "hsa_executable_symbol_get_info");
    AddFunctionToMap(HSA_API_Type_hsa_queue_create, "hsa_queue_create");
    AddFunctionToMap(HSA_API_Type_hsa_soft_queue_create, "hsa_soft_queue_create");
    AddFunctionToMap(HSA_API_Type_hsa_queue_destroy, "hsa_queue_destroy");
    AddFunctionToMap(HSA_API_Type_hsa_queue_inactivate, "hsa_queue_inactivate");
    AddFunctionToMap(HSA_API_Type_hsa_queue_load_read_index_acquire, "hsa_queue_load_read_index_acquire");
    AddFunctionToMap(HSA_API_Type_hsa_queue_load_read_index_relaxed, "hsa_queue_load_read_index_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_queue_load_write_index_acquire, "hsa_queue_load_write_index_acquire");
    AddFunctionToMap(HSA_API_Type_hsa_queue_load_write_index_relaxed, "hsa_queue_load_write_index_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_queue_store_write_index_relaxed, "hsa_queue_store_write_index_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_queue_store_write_index_release, "hsa_queue_store_write_index_release");
    AddFunctionToMap(HSA_API_Type_hsa_queue_cas_write_index_acq_rel, "hsa_queue_cas_write_index_acq_rel");
    AddFunctionToMap(HSA_API_Type_hsa_queue_cas_write_index_acquire, "hsa_queue_cas_write_index_acquire");
    AddFunctionToMap(HSA_API_Type_hsa_queue_cas_write_index_relaxed, "hsa_queue_cas_write_index_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_queue_cas_write_index_release, "hsa_queue_cas_write_index_release");
    AddFunctionToMap(HSA_API_Type_hsa_queue_add_write_index_acq_rel, "hsa_queue_add_write_index_acq_rel");
    AddFunctionToMap(HSA_API_Type_hsa_queue_add_write_index_acquire, "hsa_queue_add_write_index_acquire");
    AddFunctionToMap(HSA_API_Type_hsa_queue_add_write_index_relaxed, "hsa_queue_add_write_index_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_queue_add_write_index_release, "hsa_queue_add_write_index_release");
    AddFunctionToMap(HSA_API_Type_hsa_queue_store_read_index_relaxed, "hsa_queue_store_read_index_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_queue_store_read_index_release, "hsa_queue_store_read_index_release");
    AddFunctionToMap(HSA_API_Type_hsa_signal_create, "hsa_signal_create");
    AddFunctionToMap(HSA_API_Type_hsa_signal_destroy, "hsa_signal_destroy");
    AddFunctionToMap(HSA_API_Type_hsa_signal_load_relaxed, "hsa_signal_load_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_signal_load_acquire, "hsa_signal_load_acquire");
    AddFunctionToMap(HSA_API_Type_hsa_signal_store_relaxed, "hsa_signal_store_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_signal_store_release, "hsa_signal_store_release");
    AddFunctionToMap(HSA_API_Type_hsa_signal_wait_relaxed, "hsa_signal_wait_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_signal_wait_acquire, "hsa_signal_wait_acquire");
    AddFunctionToMap(HSA_API_Type_hsa_signal_and_relaxed, "hsa_signal_and_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_signal_and_acquire, "hsa_signal_and_acquire");
    AddFunctionToMap(HSA_API_Type_hsa_signal_and_release, "hsa_signal_and_release");
    AddFunctionToMap(HSA_API_Type_hsa_signal_and_acq_rel, "hsa_signal_and_acq_rel");
    AddFunctionToMap(HSA_API_Type_hsa_signal_or_relaxed, "hsa_signal_or_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_signal_or_acquire, "hsa_signal_or_acquire");
    AddFunctionToMap(HSA_API_Type_hsa_signal_or_release, "hsa_signal_or_release");
    AddFunctionToMap(HSA_API_Type_hsa_signal_or_acq_rel, "hsa_signal_or_acq_rel");
    AddFunctionToMap(HSA_API_Type_hsa_signal_xor_relaxed, "hsa_signal_xor_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_signal_xor_acquire, "hsa_signal_xor_acquire");
    AddFunctionToMap(HSA_API_Type_hsa_signal_xor_release, "hsa_signal_xor_release");
    AddFunctionToMap(HSA_API_Type_hsa_signal_xor_acq_rel, "hsa_signal_xor_acq_rel");
    AddFunctionToMap(HSA_API_Type_hsa_signal_exchange_relaxed, "hsa_signal_exchange_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_signal_exchange_acquire, "hsa_signal_exchange_acquire");
    AddFunctionToMap(HSA_API_Type_hsa_signal_exchange_release, "hsa_signal_exchange_release");
    AddFunctionToMap(HSA_API_Type_hsa_signal_exchange_acq_rel, "hsa_signal_exchange_acq_rel");
    AddFunctionToMap(HSA_API_Type_hsa_signal_add_relaxed, "hsa_signal_add_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_signal_add_acquire, "hsa_signal_add_acquire");
    AddFunctionToMap(HSA_API_Type_hsa_signal_add_release, "hsa_signal_add_release");
    AddFunctionToMap(HSA_API_Type_hsa_signal_add_acq_rel, "hsa_signal_add_acq_rel");
    AddFunctionToMap(HSA_API_Type_hsa_signal_subtract_relaxed, "hsa_signal_subtract_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_signal_subtract_acquire, "hsa_signal_subtract_acquire");
    AddFunctionToMap(HSA_API_Type_hsa_signal_subtract_release, "hsa_signal_subtract_release");
    AddFunctionToMap(HSA_API_Type_hsa_signal_subtract_acq_rel, "hsa_signal_subtract_acq_rel");
    AddFunctionToMap(HSA_API_Type_hsa_signal_cas_relaxed, "hsa_signal_cas_relaxed");
    AddFunctionToMap(HSA_API_Type_hsa_signal_cas_acquire, "hsa_signal_cas_acquire");
    AddFunctionToMap(HSA_API_Type_hsa_signal_cas_release, "hsa_signal_cas_release");
    AddFunctionToMap(HSA_API_Type_hsa_signal_cas_acq_rel, "hsa_signal_cas_acq_rel");

}

void HSAAPIDefs::AddFunctionToMap(HSA_API_Type hsaType, const QString& funcName)
{
    // Insert function to both maps:
    m_hsaTypeTofuncNameMap.insert(hsaType, funcName);
    m_funcNameToHSATypeMap.insert(funcName, hsaType);
    m_hsaAPIStringsList << funcName;
}

