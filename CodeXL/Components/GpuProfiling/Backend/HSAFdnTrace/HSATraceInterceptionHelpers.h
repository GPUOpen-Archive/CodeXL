//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains functions called by various intercepted APIs
//==============================================================================

#ifndef _HSA_TRACE_INTERCEPTION_HELPERS_H
#define _HSA_TRACE_INTERCEPTION_HELPERS_H

#include "hsa.h"

/// interception helper function for hsa_queue_create
/// see HSA runtime spec for parameter descriptions
void HSA_APITrace_hsa_queue_create_PostCallHelper(hsa_status_t retVal, hsa_agent_t agent, uint32_t size, hsa_queue_type_t type, void(*callback)(hsa_status_t status, hsa_queue_t* source,
                                                  void* data), void* data, uint32_t private_segment_size, uint32_t group_segment_size, hsa_queue_t** queue);

/// interception helper function for hsa_executable_get_symbol
/// see HSA runtime spec for parameter descriptions
void HSA_APITrace_hsa_executable_get_symbol_PostCallHelper(hsa_status_t retVal, hsa_executable_t executable, const char* module_name, const char* symbol_name, hsa_agent_t agent, int32_t call_convention, hsa_executable_symbol_t* symbol);

#endif // _HSA_TRACE_INTERCEPTION_HELPERS_H
