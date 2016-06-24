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

// LOCAL INCLUDES
#include <AMDTDriverTypedefs.h>
#include <AMDTHelpers.h>
#include <AMDTRawDataFileHeader.h>
#include <AMDTSmu8Interface.h>
#include <AMDTCommonConfig.h>

// PROJECT INCLUDES
#include <Smu8Header/AMDTCounterTable.h>
#include <Smu8Header/AMDTCounterOffset.h>

// LOCAL DEFINE
#define SMU8_RETRY_COUNT_MAX    (8192)
#define CARRIZO_BASE_ADDRESS    0x10000000
#define MAPPED_BASE_OFFSET  0x608

// Smu8ReadIndexDataMapping: Index and data pair mapping for reading soft register values
static void Smu8ReadIndexDataMapping(Smu8Interface* pSmu, uint64 reg, uint32* pValue)
{
    if (SMU_REG_ADDRESS_UNDEFINED != reg)
    {
        // use indirect mapping
        uint32* Reg = (uint32*)pSmu->m_gmmxPair.m_index;
        WRITE_DWORD(Reg, (uint32)reg);
        *pValue = READ_DWORD((uint32*)pSmu->m_gmmxPair.m_data);
    }
    else
    {
        *pValue = 0xDEADBEEF;
    }
}

// Smu8SRBMTestMessageService: SRBM test message is used for reading counter table base address.
// Refer SMU8_Programming_Giude_CZ.docx for details
static bool Smu8SRBMTestMessageService(Smu8Interface* pSmu, uint32 msgId, uint32* arg, uint32* pData, uint32 retry)
{
    bool ret = false;
    volatile uint32 repeatCnt = retry;
    uint64 res = 0;

    // Write 0 to resp to check if it is busy
    WRITE_DWORD(pSmu->m_srbmMsg.m_resp, 0);

    // Write argument if any
    if (NULL != arg)
    {
        WRITE_DWORD(pSmu->m_srbmMsg.m_arg, *arg);
    }

    // Write message id
    WRITE_DWORD(pSmu->m_srbmMsg.m_msg, msgId);

    // Wait for completion
    do
    {
        res = READ_DWORD(pSmu->m_srbmMsg.m_resp);

        if (res & SMC_RESULT_OK)
        {
            ret = true;
        }
    }
    while ((false == ret) && --repeatCnt);

    // Read the result
    if ((true == ret) && (NULL != pData))
    {
        *pData = (uint32)READ_DWORD(pSmu->m_srbmMsg.m_arg);
    }

    return ((true == ret) && (NULL != pData)) ? true : false;
}

// Configure Counter Table.
bool ConfigureCounterTable(Smu8Interface* pSmu)
{
    bool result = false;
    uint32 cnt = 0;
    uint32 listCnt = 0;
    uint32 version = 0;

    listCnt = sizeof(CounterTableOffsetList) / sizeof(CounterTableInfo);

    if (NULL != pSmu)
    {
        //Set the default table;
        pSmu->m_tableBase = (uint64)&CounterTableOffsetList[0].m_table;
        version = pSmu->m_tableVersion;

        for (cnt = 0; cnt < listCnt; cnt++)
        {
            if (version == CounterTableOffsetList[cnt].m_versionId)
            {
                pSmu->m_tableBase = (uint64)&CounterTableOffsetList[cnt].m_table;
                result = true;
                break;
            }
        }
    }

    return result;
}

// ReleaseCounterTable: Release the counter table base address which
// holds the SMU soft registers base address.
static bool ReleaseCounterTable(Smu8Interface* pSmu, uint32 retry)
{
    bool result = false;
    Smu8SRBMTestMessageService(pSmu, pSmu->m_srbmMsg.m_releaseCounterTable, NULL, NULL, retry);
    return result;
}

// RequestCounterTable: Acquire the counter table base address which holds
// the SMU soft registers base address.
static bool RequestCounterTable(Smu8Interface* pSmu, uint32* pData, uint32 retry)
{
    bool result = false;
    result = Smu8SRBMTestMessageService(pSmu, pSmu->m_srbmMsg.m_requestCounterTable, &pSmu->m_tableId, pData, retry);

    if (false == result)
    {
        ReleaseCounterTable(pSmu, retry);
    }

    return result;
}

// Get the Smu8 register values.
bool CollectSMU8RegisterValues(void* pSmuInfo, uint8* pData, uint32* pLength)
{
    bool result = false;
    uint32 repeat = SMU8_RETRY_COUNT_MAX;
    uint32 tableBaseAddr = 0;
    uint32 offset = 0;
    uint32* value = NULL;
    uint32 baseOffset = 0;
    Smu8CounterTable* pCounterTab = NULL;
    uint32 cuCnt = GetComputeUnitCntPerNode();
    uint8 cuIdx = 0;

    // TODO: need to change this
    uint64 mask = ((SmuInfo*)pSmuInfo)->m_counterMask;
    Smu8Interface* pSmu = &((SmuInfo*)pSmuInfo)->m_access.m_smu8;

    do
    {
        // Get the offset int the counter table
        result = RequestCounterTable(pSmu, &tableBaseAddr, SMU8_RETRY_COUNT_MAX);
    }
    while ((false == result) && --repeat);

    // Set the status of the smu access to restrict the data access from backend
    if (false == result)
    {
        SetSmuAccessState(result);
    }

    if (true == result)
    {
        uint32 idx = 0;
        baseOffset = tableBaseAddr + CARRIZO_BASE_ADDRESS;

        for (idx = 0; idx <= COUNTERID_SMU8_CNT; idx++)
        {
            if (false == ((mask >> idx) & 0x01))
            {
                continue;
            }

            // Get the counter table
            pCounterTab = (Smu8CounterTable*)pSmu->m_tableBase;

            switch (idx)
            {
                case COUNTERID_SMU8_APU_PWR_CU:
                {
                    // Collect power for CUs
                    for (cuIdx = 0; cuIdx < cuCnt; ++cuIdx)
                    {
                        value = (uint32*) &pData[offset];
                        Smu8ReadIndexDataMapping(pSmu, pCounterTab->P_calc_cu[cuIdx] + baseOffset, value);
                        offset += sizeof(uint32);
                    }

                    break;
                }

                case COUNTERID_SMU8_APU_TEMP_CU:
                {
                    // Collect calculated temperature for CUs
                    for (cuIdx = 0; cuIdx < cuCnt; ++cuIdx)
                    {
                        value = (uint32*) &pData[offset];
                        Smu8ReadIndexDataMapping(pSmu, pCounterTab->T_calc_cu[cuIdx] + baseOffset, value);
                        offset += sizeof(uint32);
                    }

                    break;
                }

                case COUNTERID_SMU8_APU_C0STATE_RES:
                {
                    // Collect CUs state residency
                    for (cuIdx = 0; cuIdx < cuCnt; ++cuIdx)
                    {
                        value = (uint32*) &pData[offset];
                        Smu8ReadIndexDataMapping(pSmu, pCounterTab->C0_residency[cuIdx] + baseOffset, value);
                        offset += sizeof(uint32);

                    }

                    break;
                }

                case COUNTERID_SMU8_APU_C1STATE_RES:
                {
                    // Collect CUs state residency
                    for (cuIdx = 0; cuIdx < cuCnt; ++cuIdx)
                    {
                        value = (uint32*) &pData[offset];
                        Smu8ReadIndexDataMapping(pSmu, pCounterTab->C1_residency[cuIdx] + baseOffset, value);
                        offset += sizeof(uint32);
                    }

                    break;
                }

                case COUNTERID_SMU8_APU_CC6_RES:
                {
                    // Collect CU0 CC6 state residency
                    for (cuIdx = 0; cuIdx < cuCnt; ++cuIdx)
                    {
                        value = (uint32*) &pData[offset];
                        Smu8ReadIndexDataMapping(pSmu, pCounterTab->CC6_residency[cuIdx] + baseOffset, value);
                        offset += sizeof(uint32);
                    }

                    break;
                }

                case COUNTERID_SMU8_APU_PWR_VDDIO:
                {
                    // Collect VDDIO power
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->P_calc_vddio + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU8_APU_PWR_VDDNB:
                {
                    // Collect VDDNB power
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->P_calc_vddnb + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU8_APU_PWR_VDDP:
                {
                    // Collect VDDP power
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->P_calc_vddp + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU8_APU_PWR_VDDGFX:
                {
                    // Collect power for internal GPU
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->P_calc_vddgfx + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU8_APU_PWR_APU:
                {
                    // Collect power for internal GPU
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->P_calc_apu + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU8_APU_TEMP_VDDGFX:
                {
                    // Collect calculated temperature for Vddgfx
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->T_calc_vddgfx + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU8_APU_FREQ_IGPU:
                {
                    // Collect iGPU frequency
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->Sclk_frequency + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU8_APU_PWR_UVD:
                {
                    // Collect UVD power
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->P_calc_uvd + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU8_APU_PWR_VCE:
                {
                    // Collect VCE power
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->P_calc_vce + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU8_APU_PWR_ACP:
                {
                    // Collect ACP power
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->P_calc_acp + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU8_APU_PWR_UNB:
                {
                    // Collect UNB power
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->P_calc_unb + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU8_APU_PWR_SMU:
                {
                    // Collect SMU power
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->P_calc_smu + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU8_APU_PWR_ROC:
                {
                    // Collect ROC power
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->P_calc_roc + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU8_APU_FREQ_ACLK:
                {
                    // Collect Audio Co-processor clock frquency
                    value = (uint32*) &pData[offset];
                    Smu8ReadIndexDataMapping(pSmu, pCounterTab->Aclk_frequency + baseOffset, value);
                    offset += sizeof(uint32);
                    break;
                }

                default:
                    break;
            }
        }

        // Data has been collected, release the table now
        ReleaseCounterTable(pSmu, SMU8_RETRY_COUNT_MAX);
    }

    if (true == result)
    {
        *pLength = *pLength + offset;
    }

    return result;
}

// Initialize Smu8 Session
bool Smu8SessionInitialize(void* pSmuInfo)
{
    bool result = false;
    uint32 repeatCnt = SMU8_RETRY_COUNT_MAX;
    Smu8Interface* pSmu = &((SmuInfo*)pSmuInfo)->m_access.m_smu8;


    // DRVPRINT("PWRPROF: Smu8SessionInitialize!\n");

    if (NULL != pSmu)
    {
        result = ConfigureCounterTable(pSmu);

        // Get the GPU Memory Mapped registers based address
        pSmu->m_gpuBaseAddr = ((SmuInfo*)pSmuInfo)->m_gpuBaseAddress;

        if (0 != pSmu->m_gpuBaseAddr)
        {
            uint64 length = (pSmu->m_srbmMsg.m_resp - pSmu->m_gmmxPair.m_index)
                            + sizeof(uint32);
            // Memory map for indirect access data/index pair.
            result = HelpMapMMIOSpace((uint32)(pSmu->m_gpuBaseAddr + MAPPED_BASE_OFFSET),
                                      (size_t)length,
                                      &pSmu->m_mappedGMMMxSpace,
                                      &pSmu->m_mappedSize);

            if ((true == result) && pSmu->m_mappedGMMMxSpace)
            {
                // Adjust offset with  SRBM message parameters
                pSmu->m_srbmMsg.m_msg = pSmu->m_srbmMsg.m_msg + pSmu->m_mappedGMMMxSpace;
                pSmu->m_srbmMsg.m_arg = pSmu->m_srbmMsg.m_arg + pSmu->m_mappedGMMMxSpace;
                pSmu->m_srbmMsg.m_resp = pSmu->m_srbmMsg.m_resp + pSmu->m_mappedGMMMxSpace;

                // Adjust offset with index data pair
                pSmu->m_gmmxPair.m_index = pSmu->m_gmmxPair.m_index + pSmu->m_mappedGMMMxSpace;
                pSmu->m_gmmxPair.m_data = pSmu->m_gmmxPair.m_data + pSmu->m_mappedGMMMxSpace;
            }

            // SMU8 start monitoring start when counter data table is requested. When ever
            // RequestCounterTable is called it will dump the caluculated values so far from the prev call
            // this call is made to avoid the juck data ffor the first time
            do
            {
                // Get the offset int the counter table
                uint32 addr = 0;
                result = RequestCounterTable(pSmu, &addr, SMU8_RETRY_COUNT_MAX);
            }
            while ((false == result) && --repeatCnt);

            if (true == result)
            {
                ReleaseCounterTable(pSmu, SMU8_RETRY_COUNT_MAX);
            }
        }
    }

    return result;
}

// Close Smu8 Session.
bool Smu8SessionClose(void* pSmuInfo)
{
    bool result = false;
    Smu8Interface* pSmu = &((SmuInfo*)pSmuInfo)->m_access.m_smu8;

    if ((0 != pSmu->m_mappedGMMMxSpace) && pSmu->m_mappedSize)
    {
        HelpUnmapMMIOSpace(pSmu->m_mappedGMMMxSpace, pSmu->m_mappedSize);
        pSmu->m_mappedGMMMxSpace = 0;
        pSmu->m_mappedSize = 0;
    }

    return result;
}

uint32 GetSmu8CounterSize(uint32 counterId)
{
    uint32 bufferLen = 0;
    uint32 cuCnt = GetComputeUnitCntPerNode();

    switch (counterId)
    {
        case COUNTERID_SMU8_APU_PWR_CU:
        case COUNTERID_SMU8_APU_TEMP_CU:
        case COUNTERID_SMU8_APU_C0STATE_RES:
        case COUNTERID_SMU8_APU_C1STATE_RES:
        case COUNTERID_SMU8_APU_CC6_RES:
        {
            bufferLen += (cuCnt * sizeof(uint32));
            break;
        }

        // 4 byte counters
        case COUNTERID_SMU8_APU_PWR_VDDGFX:
        case COUNTERID_SMU8_APU_PWR_APU:
        case COUNTERID_SMU8_APU_TEMP_VDDGFX:
        case COUNTERID_SMU8_APU_FREQ_IGPU:
        case COUNTERID_SMU8_APU_PWR_VDDIO:
        case COUNTERID_SMU8_APU_PWR_VDDNB:
        case COUNTERID_SMU8_APU_PWR_VDDP:
        case COUNTERID_SMU8_APU_PWR_UVD:
        case COUNTERID_SMU8_APU_PWR_VCE:
        case COUNTERID_SMU8_APU_PWR_ACP:
        case COUNTERID_SMU8_APU_PWR_UNB:
        case COUNTERID_SMU8_APU_PWR_SMU:
        case COUNTERID_SMU8_APU_PWR_ROC:
        case COUNTERID_SMU8_APU_FREQ_ACLK:
        {
            bufferLen += sizeof(uint32);
            break;
        }

        default:
            break;
    }

    return bufferLen;
}
