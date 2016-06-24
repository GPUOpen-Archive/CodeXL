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

#ifndef _SMU7_INTERFACE_H_
#define _SMU7_INTERFACE_H_
#include <AMDTDriverTypedefs.h>

// Smu7IndexData: Smu7 MMIO offset for data access
typedef struct
{
    uint32 m_index;
    uint32 m_data;
} Smu7IndexData;

// Smu7Logging: Smu7 message logging ids
typedef struct
{
    uint32 m_start;
    uint32 m_dump;
} Smu7Logging;

// Smu7MessageParam: Smu7 message exchange parameters
typedef struct
{
    uint32 m_msgId;
    uint32 m_resp;
    uint32 m_arg;
    uint32 m_fill;
} Smu7MessageParam;

//Smu7CounterTable: SMU7 counter table
typedef struct
{
    uint32 m_pwrCu0;
    uint32 m_pwrCu1;
    uint32 m_tempCalcCu0;
    uint32 m_tempCalcCu1;
    uint32 m_tempMeasCu0;
    uint32 m_tempMeasCu1;
    uint32 m_pwriGpu;
    uint32 m_pwrPcie;
    uint32 m_pwrDdr;
    uint32 m_pwrDisplay;
    uint32 m_pwrPackage;
    uint32 m_tempCalcGpu;
    uint32 m_tempMeasGpu;
    uint32 m_sclk;
    uint32 m_voltVddcLoad;
    uint32 m_currVddc;
} Smu7CounterTable;

// Smu7Interface: Smu7 interfacing structure
typedef struct
{
    uint64           m_gpuBaseAddress;
    uint64           m_mappedAddress;
    uint64           m_mappedSize;
    uint64           m_indexDataPair;
    uint64           m_indexDataPairSize;
    Smu7CounterTable m_counterTable;
    Smu7MessageParam m_messageParam;
    Smu7Logging      m_logging;
    Smu7IndexData    m_indexData;
} Smu7Interface;

// CollectSMU8RegisterValues: Collect all SMU counter values
// based on the mask set and fill in the pData
bool CollectSMU7RegisterValues(void* pSmu,
                               uint8* pData,
                               uint32* pLength);

// Smu7SessionInitialize:
bool Smu7SessionInitialize(void* pSmu);

// Smu7SessionClose
bool Smu7SessionClose(void* pSmu);

// GetSmu7CounterSize
uint32 GetSmu7CounterSize(uint32 counterId);

// GetSmu7DgpuCounterSize
uint32 GetSmu7DgpuCounterSize(uint32 counterId);

#endif //_SMU7_INTERFACE_H_
