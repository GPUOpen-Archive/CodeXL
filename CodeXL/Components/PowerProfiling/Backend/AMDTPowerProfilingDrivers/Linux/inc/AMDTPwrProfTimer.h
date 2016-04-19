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

#ifndef _PWRPROFDRVTIMER_H
#define _PWRPROFDRVTIMER_H

#include <AMDTDriverTypedefs.h>
#include <AMDTRawDataFileHeader.h>

// Configure Timer for sampling period specified, set the callback
// function to be invoked when sampling duration expires.
int ConfigureTimer(ProfileConfig* config, uint32 clientId);

// Unconfigure timer, delete it from timer list
int UnconfigureTimer(uint32 clientId);

// Invoked when profiling starts, add a timer for sampling time.
int StartTimer(uint32 clientId);

// Stop timer for the specified client id release the memory allocated for timer.
int StopTimer(uint32 clientId);

// Pause Timer for the specified client id
int PauseTimer(uint32 clientId);


//Resume Timer for the specified client id
int ResumeTimer(uint32 clientId);

// Get header buffer for the kernel space convert it into user space.
int GetHeaderBuffer(PFILE_HEADER fileHeader);

// Get data buffer for the counter
int GetDataBuffer(PDATA_BUFFER dataBuffer);

#endif // _PWRPROFDRVTIMER_H
