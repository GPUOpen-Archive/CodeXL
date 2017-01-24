//===============================================================================
//
// Copyright(c) 2016 Advanced Micro Devices, Inc.All Rights Reserved
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

#include <AMDTDriverTypedefs.h>
#include <AMDTHelpers.h>
#include <AMDTPwrProfAttributes.h>
#define HWCR_REG_ADDR 0xC0010015
#define HWCR_ENABLE_MASK 0x40000000
#define HWCR_DISABLE_MASK 0xBFFFFFFF
static bool g_perfEnable = false;

static uint32 g_platformId = PLATFORM_INVALID;
static uint32 g_physicalCores = 0;

// HelpGetTargetPlatformId: Get platform id
uint32 HelpPwrGetTargetPlatformId(void)
{
    uint32 family = 0;
    uint32 model = 0;

    if (PLATFORM_INVALID == g_platformId)
    {
        GetCpuModelFamily(&family, &model);

        if ((0x15 == family) && (model >= 0x30 && model <= 0x3F))
        {
            // Kaveri : 0x15 30 to 3F
            g_platformId = PLATFORM_KAVERI;
        }
        else if ((0x15 == family) && (model >= 0x60 && model <= 0x6F))
        {
            // Carrizo: 0x15 60 to 6F
            g_platformId  = PLATFORM_CARRIZO;
        }
        else if ((0x16 == family) && (model >= 0x30 && model <= 0x3F))
        {
            // Mullins : 0x16 30 to 3F
            g_platformId = PLATFORM_MULLINS;
        }
        else if (0x17 == family)
        {
            if (model <= 0x0F)
            {
                // Res : 0x17 00 to 0F
                g_platformId = PLATFORM_ZEPPELIN;
            }
        }
        else
        {
            g_platformId = PLATFORM_INVALID;
        }
    }

    return g_platformId;
}

// HelpPwrEnablePerf: Enable perf bit
void HelpPwrEnablePerf(bool enable)
{
    if (PLATFORM_ZEPPELIN == HelpPwrGetTargetPlatformId())
    {
        ACCESS_MSR msr;
        msr.isReadAccess = true;
        msr.regId = HWCR_REG_ADDR;
        HelpAccessMSRAddress(&msr);

        if (true == enable)
        {
            if (!(msr.data & HWCR_ENABLE_MASK))
            {
                msr.data |= HWCR_ENABLE_MASK;
                msr.isReadAccess = false;
                HelpAccessMSRAddress(&msr);
                g_perfEnable = true;
            }
        }
        else
        {
            if (g_perfEnable)
            {
                msr.data = msr.data & HWCR_DISABLE_MASK;
                msr.isReadAccess = false;
                HelpAccessMSRAddress(&msr);
                g_perfEnable = false;
            }
        }
    }
}

uint32 HelpPwrGetPhysicalCoreCount(void)
{
    if(0 == g_physicalCores)
    {
        g_physicalCores = HelpPwrIsSmtEnabled() ? (GetTargetCoreCount() / 2) : GetTargetCoreCount();
    }

    return g_physicalCores;
}

