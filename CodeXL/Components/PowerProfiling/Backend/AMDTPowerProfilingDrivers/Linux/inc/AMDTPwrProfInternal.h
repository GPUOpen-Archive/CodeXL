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

#ifndef _PWR_PROF_DRV_TIMER_DATA_H_
#define _PWR_PROF_DRV_TIMER_DATA_H_

// SYSTEM INCLUDES
#include <linux/timer.h>
#include <linux/ktime.h>
#include <linux/spinlock.h>

// LOCAL INCLUDES
#include <AMDTPwrProfAsm.h>
#include <AMDTPwrProfCoreUtils.h>
#include <AMDTRawDataFileHeader.h>

typedef enum
{
    // Family is an 8-bit value and is defined as:
    // Family[7:0] = ({0000b,BaseFamily[3:0]} + ExtFamily[7:0]).
    CpuBaseFamily_MASK = 0xFU << 8,
    CpuExtFamily_MASK = 0xFFU << 20,

    // Model is an 8-bit value and is defined as:
    // Model[7:0] = {ExtModel[3:0], BaseModel[3:0]}.
    CpuBaseModel_MASK = 0xFU << 4,
    CpuExtModel_MASK = 0xFU << 16,

    CpuStepping_MASK = 0xFU << 0,
} Mask;


typedef struct _CpuSignature
{
    uint  m_value;
    bool  m_isHypervisor;
} CpuSignature;

typedef struct
{
    cpumask_t           m_affinity;
    bool                m_paused;
    bool                m_stopped;
    pid_t               m_parentPid;
} OsClientCfg;

typedef struct
{
    struct hrtimer          m_hrTimer;
    ktime_t                 m_interval;
} OsCoreCfgData;

#endif
