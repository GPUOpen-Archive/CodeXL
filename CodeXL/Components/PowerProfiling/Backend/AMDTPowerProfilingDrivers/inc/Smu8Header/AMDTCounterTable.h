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

#ifndef AMDTCOUNTERTABLE_H
#define AMDTCOUNTERTABLE_H

// This table contains all supported counters for power profile
// Need to modify incase we need to add/remove supported
// counters in the list.
typedef struct _Smu8CounterTable
{
    //T_calc
    uint32 T_calc_cu[2];
    uint32 T_calc_vddgfx;

    //P_calc
    uint32 P_calc_cu[2];
    uint32 P_calc_vddgfx;
    uint32 P_calc_vddnb;
    uint32 P_calc_vddio;
    uint32 P_calc_vddp;
    uint32 P_calc_roc;

    uint32 P_calc_apu;

    uint32 P_calc_uvd;
    uint32 P_calc_vce;
    uint32 P_calc_acp;
    uint32 P_calc_unb;
    uint32 P_calc_smu;

    //I_calc
    uint32 I_calc_vdd;
    uint32 I_calc_vddgfx;
    uint32 I_calc_vddnb;

    //V_calc
    uint32 V_calc_vdd;
    uint32 V_calc_vddgfx;
    uint32 V_calc_vddnb;

    //Effective Clock Frequencies
    uint32 Sclk_frequency;
    uint32 Aclk_frequency;

    //Cstate residencies
    uint32 C0_residency[2];
    uint32 C1_residency[2];
    uint32 CC6_residency[2];

    //TDP
    uint32 Tdp;
} Smu8CounterTable;

#define SMC_RESULT_OK                           ((uint8_t)(0x1))
#define COUNTER_TABLE_VERSION_MSG_ID            ((uint8_t)0x40)

#endif

