//===============================================================================
//
// Copyright(c) 2015 Advanced Micro Devices, Inc.All Rights Reserved
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//=================================================================================
#ifndef _AMDTSHAREDBUFFERCONFIG_H
#define _AMDTSHAREDBUFFERCONFIG_H
#ifdef __linux__
//Need to test 512 in Linux
#define PWRPROF_SHARED_BUFFER_SIZE 1048576 //256 * 4096
#else
#define PWRPROF_SHARED_BUFFER_SIZE 2097152 //512 * 4096

#endif

// 1K for all core buffer parameter other than buffers
#define PWRPROF_SHARED_METADATA_SIZE 4096

// 16 pages are allocated for the master core
#define PWRPROF_MASTER_CORE_BUFFER_SIZE (65536)

// 3 pages are allocated for remaining cores
#define PWRPROF_NONMASTER_CORE_BUFFER_SIZE (12288)


#endif //_AMDTSHAREDBUFFERCONFIG_H


