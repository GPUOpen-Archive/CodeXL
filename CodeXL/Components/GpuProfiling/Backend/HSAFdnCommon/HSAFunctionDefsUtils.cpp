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
#define X(SYM) m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_" #SYM), HSA_API_Type_hsa_##SYM));
    HSA_RUNTIME_API_TABLE
    HSA_EXT_FINALIZE_API_TABLE
    HSA_EXT_IMAGE_API_TABLE
    HSA_EXT_AMD_API_TABLE
#undef X

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
