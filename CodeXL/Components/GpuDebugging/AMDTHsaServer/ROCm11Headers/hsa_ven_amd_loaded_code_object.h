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

// HSA AMD extension for loaded code objects.

#ifndef HSA_VEN_AMD_LOADED_CODE_OBJECT_H
#define HSA_VEN_AMD_LOADED_CODE_OBJECT_H

#include "hsa.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Records loaded code object's host address in @p host_address given
 * loaded code object's device address. Recorded host address points to host
 * accessible memory, which is identical to memory pointed to by device address.
 * If device address already points to host accessible memory, then device
 * address is recorded in @p host_address.
 *
 * @param[in] device_address Device address.
 *
 * @param[out] host_address Pointer to application-allocated buffer, where to
 * record host address.
 *
 * @retval HSA_STATUS_SUCCESS Function has been executed successfully.
 *
 * @retval HSA_STATUS_ERROR_NOT_INITIALIZED Runtime has not been initialized.
 *
 * @retval HSA_STATUS_ERROR_INVALID_ARGUMENT @p device address is invalid/null,
 * or @p host address is null.
 */
hsa_status_t HSA_API hsa_ven_amd_loaded_code_object_query_host_address(
  const void *device_address,
  const void **host_address);

/**
 * @brief Extension's version.
 */
#define hsa_ven_amd_loaded_code_object 001000

/**
 * @brief Extension's function table.
 */
typedef struct hsa_ven_amd_loaded_code_object_1_00_pfn_s {
  hsa_status_t (*hsa_ven_amd_loaded_code_object_query_host_address)(
    const void *device_address,
    const void **host_address);
} hsa_ven_amd_loaded_code_object_1_00_pfn_t;

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // HSA_VEN_AMD_LOADED_CODE_OBJECT_H
