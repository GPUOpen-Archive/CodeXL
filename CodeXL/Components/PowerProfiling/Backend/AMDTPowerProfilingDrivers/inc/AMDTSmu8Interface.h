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

#ifndef _SMU8_INTERFACE_H_
#define _SMU8_INTERFACE_H_

// LOCAL INCLUDES
#include <AMDTDriverTypedefs.h>

// PROJECT INCLUDES
#include <Smu8Header/AMDTCounterTable.h>


// CounterTableInfo: Counter table list
typedef struct _CounterTableInfo
{
    uint32             m_versionId;
    Smu8CounterTable   m_table;
} CounterTableInfo;

// Smu8SRBMMessage: SRBM message interface
typedef struct _Smu8SRBMMessage
{
    uint64 m_msg;                  // Message id
    uint64 m_arg;                  // Argument
    uint64 m_resp;                 // Response
    uint8  m_requestCounterTable;  // Counter table request message id
    uint8  m_releaseCounterTable;  // Counter table release message id
    uint16 m_fill[3];
} Smu8SRBMMessage;

// Smu8MMAccess: Channel-9 data index pair
typedef struct _Smu8MMAccess
{
    uint64 m_index;
    uint64 m_data;
} Smu8MMAccess;

// Smu8Interface: Access interface for Smu8
typedef struct
{
    uint64               m_gpuBaseAddr;
    uint64               m_mappedGMMMxSpace;
    uint64               m_mappedSize;
    Smu8SRBMMessage      m_srbmMsg;
    Smu8MMAccess         m_gmmxPair;
    uint64               m_tableBase;
    uint32               m_tableId;
    uint32               m_tableVersion;
} Smu8Interface;

bool FillSmu8InterfaceInfo(Smu8Interface* pSmu);

// CollectSMU8RegisterValues: Collect all SMU counterr values
// based on the mask set and fill in the pData
bool CollectSMU8RegisterValues(void* pSmu,
                               uint8* pData,
                               uint32* pLength);

// Smu8SessionInitialize:
bool Smu8SessionInitialize(void* pSmu);

// Smu8SessionClose
bool Smu8SessionClose(void* pSmu);

#endif //_SMU8_INTERFACE_H_
