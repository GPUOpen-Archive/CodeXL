/*******************************************************************************
 * Copyright (c) 2008-2009 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 ******************************************************************************/

/* $Revision: #155 $ on $Date: 2011/04/24 $ */

#ifndef __CL_EXT_H
#define __CL_EXT_H

#ifdef __cplusplus
extern "C"
{
#endif

// cl_khr_fp64 extension
//#define cl_khr_fp64 1

// enum can be used if cl_khr_fp64 extension is supported by the OpenCL implementation.
#define CL_DEVICE_DOUBLE_FP_CONFIG                  0x1032

// cl_khr_fp16 extension
//#define cl_khr_fp16 1

// enum can be used if cl_khr_fp16 extension is supported by the OpenCL implementation.
#define CL_DEVICE_HALF_FP_CONFIG                    0x1033

// cl_khr_icd extension
#define cl_khr_icd 1

// cl_platform_info
#define CL_PLATFORM_ICD_SUFFIX_KHR                  0x0920
#define CL_PLATFORM_ICD_SUFFIX_NV                   0x0905

// Additional Error Codes
#define CL_PLATFORM_NOT_FOUND_KHR                   -1001

CL_API_ENTRY cl_int CL_API_CALL
clIcdGetPlatformIDsKHR(cl_uint          /* num_entries */,
                       cl_platform_id* /* platforms */,
                       cl_uint*         /* num_platforms */) CL_API_SUFFIX__VERSION_1_0;

#ifdef __cplusplus
}
#endif


#endif /* __CL_EXT_H */
