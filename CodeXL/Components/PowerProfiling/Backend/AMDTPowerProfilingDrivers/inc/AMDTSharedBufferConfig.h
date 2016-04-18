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

// Using 16 pages per core and additional 8 pages for other meta data
// TODO: Currently the buffer is allocated for 8 cores and this needs to be modified
// to be allocated dynamically depending on the number of available cores
#define PWRPROF_SHARED_BUFFER_SIZE 256*4096

// 1K for all core buffer parameter other than buffers
#define PWRPROF_SHARED_METADATA_SIZE 4096

// Maximum per core buffer size
#define PWRPROF_PERCORE_BUFFER_SIZE 16 * 4096

#endif //_AMDTSHAREDBUFFERCONFIG_H


