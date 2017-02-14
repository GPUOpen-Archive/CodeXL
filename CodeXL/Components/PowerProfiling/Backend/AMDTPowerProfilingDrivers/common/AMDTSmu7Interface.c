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

#include <AMDTDriverTypedefs.h>
#include <AMDTHelpers.h>
#include <AMDTCommonConfig.h>

#define SMU7_RETRY_COUNT_MAX (8 * 1024)

// Smu7MessageMMIOSpec: This message use used to start and dump logging
// following two commands are sent
bool Smu7MessageMMIOSpec(Smu7Interface* pSmudata, uint32 msgId)
{
    volatile uint32 cnt = 0;
    bool result = false;
    bool ret = true;

    if (ret && pSmudata->m_mappedAddress)
    {
        uint32* pMessage   = (uint32*)pSmudata->m_mappedAddress;
        uint32* pResponse = pMessage + ((pSmudata->m_messageParam.m_resp - pSmudata->m_messageParam.m_msgId) / sizeof(uint32));
        uint32* pMsgArg = pMessage + ((pSmudata->m_messageParam.m_arg - pSmudata->m_messageParam.m_msgId) / sizeof(uint32));

        for (cnt = 0; cnt < SMU7_RETRY_COUNT_MAX && (! result); ++cnt)
        {
            if (0 != READ_DWORD(pResponse))
            {
                result = true;
            }
        }

        if (result)
        {
            //Write the argument
            WRITE_DWORD(pMsgArg, (uint32)(0));

            // Write the message
            WRITE_DWORD(pMessage, msgId);

            // Wait for the opration to complete
            for (cnt = 0; cnt < SMU7_RETRY_COUNT_MAX; ++cnt)
            {
                if (0 != READ_DWORD(pResponse))
                {
                    break;
                }
            }

            result = (1 == READ_DWORD(pResponse)) ? true : false;
        }
    }

    return result;
}

// SMU7ReadSmuIndirectMappingRegister: Read SMU registers using Channel-2 indirect mapped
// index/Data pair
void SMU7ReadSmuIndirectMappingRegister(Smu7Interface* pSmuData, uint32 reg, uint32* pValue, bool isAccessible)
{
    if ( isAccessible && (SMU_REG_ADDRESS_UNDEFINED != reg))
    {
        // use indirect mapping
        uint32* pIdx = (uint32*)pSmuData->m_indexDataPair;
        WRITE_DWORD(pIdx, reg);
        *pValue = READ_DWORD((((uint32*)pSmuData->m_indexDataPair) + 1));
    }
    else
    {
         DRVPRINT("SMU not Accessible. Filling 0");
        *pValue = 0x00;
    }
}

// CollectSMU7RegisterValues: Collect seleted counter values from smu
bool CollectSMU7RegisterValues(void* pSmu, uint8* pData, uint32* pLength)
{
    bool result = false;
    uint32 offset = 0;
    uint32 cuCnt = GetComputeUnitCntPerNode();
    uint64 mask = 0;
    SmuInfo* pSmuInfo = (SmuInfo*)pSmu;
    Smu7Interface* pSmuData = &pSmuInfo->m_access.m_smu7;
    Smu7CounterTable* pTable = NULL;
    bool isPkgPwrSelected = false;
    uint32 cu0Pwr = 0;
    uint32 cu1Pwr = 0;
    uint32 pciePwr = 0;
    uint32 displayPwr = 0;
    uint32 ddrPwr = 0;
    uint32 igpuPwr = 0;
    uint32 pkgPwr = 0;
    uint32 idx = 0;

    // Dump logging
    result = Smu7MessageMMIOSpec(pSmuData, pSmuData->m_logging.m_dump);

    // Set the status of the smu access to restrict the data access from backend
    if (false == result)
    {
        SetSmuAccessState(result);
    }

    pTable = &pSmuData->m_counterTable;

    if (APU_SMU_ID == pSmuInfo->m_packageId)
    {
        mask = pSmuInfo->m_counterMask;

        // Package power need to be calculated from all power counters
        isPkgPwrSelected = (mask & (1 << COUNTERID_SMU7_APU_PWR_PACKAGE)) ? true : false;

        if (isPkgPwrSelected)
        {
            // Collect calculated Package power
            SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_pwrCu0, &cu0Pwr, result);

            if (2 == cuCnt)
            {
                SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_pwrCu1, &cu1Pwr, result);
            }

            SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_pwrPcie, &pciePwr, result);
            SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_pwrDisplay, &displayPwr, result);
            SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_pwrDdr, &ddrPwr, result);
            SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_pwriGpu, &igpuPwr, result);
            pkgPwr += cu0Pwr + cu1Pwr + pciePwr + ddrPwr + displayPwr + igpuPwr;
        }

        for (idx = 0; idx <= COUNTERID_SMU7_CNT; idx++)
        {
            if (false == ((mask >> idx) & 0x01))
            {
                continue;
            }

            switch (idx)
            {
                case COUNTERID_SMU7_APU_PWR_CU:
                {
                    if (isPkgPwrSelected)
                    {
                        memcpy((uint32*)&pData[offset], &cu0Pwr, sizeof(uint32));
                        offset += sizeof(uint32);

                        if (2 == cuCnt)
                        {
                            memcpy((uint32*)&pData[offset], &cu1Pwr, sizeof(uint32));
                            offset += sizeof(uint32);
                        }
                    }
                    else
                    {
                        // Collect power for CU0
                        SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_pwrCu0, (uint32*)&pData[offset], result);
                        offset += sizeof(uint32);

                        if (2 == cuCnt)
                        {
                            // Collect power for CU1
                            SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_pwrCu1, (uint32*)&pData[offset], result);
                            offset += sizeof(uint32);
                        }
                    }

                    break;
                }

                case COUNTERID_SMU7_APU_TEMP_CU:
                {
                    // Collect calculated temperature for CU0
                    SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_tempCalcCu0, (uint32*)&pData[offset], result);
                    offset += sizeof(uint32);

                    if (2 == cuCnt)
                    {
                        // Collect calculated temperature for CU1
                        SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_tempCalcCu1, (uint32*)&pData[offset], result);
                        offset += sizeof(uint32);
                    }

                    break;
                }

                case COUNTERID_SMU7_APU_TEMP_MEAS_CU:
                {
                    // Collect measured temperature for CU0
                    SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_tempMeasCu0, (uint32*)&pData[offset], result);
                    offset += sizeof(uint32);

                    if (2 == cuCnt)
                    {
                        // Collect measured temperature for CU1
                        SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_tempMeasCu1, (uint32*)&pData[offset], result);
                        offset += sizeof(uint32);
                    }

                    break;
                }

                case COUNTERID_SMU7_APU_PWR_IGPU:
                {
                    if (isPkgPwrSelected)
                    {
                        memcpy((uint32*)&pData[offset], &igpuPwr, sizeof(uint32));
                        offset += sizeof(uint32);
                    }
                    else
                    {
                        // Collect calculated GPU power
                        SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_pwriGpu, (uint32*)&pData[offset], result);
                        offset += sizeof(uint32);
                    }

                    break;
                }

                case COUNTERID_SMU7_APU_PWR_PCIE:
                {
                    if (isPkgPwrSelected)
                    {
                        memcpy((uint32*)&pData[offset], &pciePwr, sizeof(uint32));
                        offset += sizeof(uint32);
                    }
                    else
                    {
                        // Collect calculated PCIE power
                        SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_pwrPcie, (uint32*)&pData[offset], result);
                        offset += sizeof(uint32);
                    }

                    break;
                }

                case COUNTERID_SMU7_APU_PWR_DDR:
                {
                    if (isPkgPwrSelected)
                    {
                        memcpy((uint32*)&pData[offset], &ddrPwr, sizeof(uint32));
                        offset += sizeof(uint32);
                    }
                    else
                    {
                        // Collect calculated DDR power
                        SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_pwrDdr, (uint32*)&pData[offset], result);
                        offset += sizeof(uint32);
                    }

                    break;
                }

                case COUNTERID_SMU7_APU_PWR_DISPLAY:
                {
                    if (isPkgPwrSelected)
                    {
                        memcpy((uint32*)&pData[offset], &displayPwr, sizeof(uint32));
                        offset += sizeof(uint32);
                    }
                    else
                    {
                        // Collect calculated DDR power
                        SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_pwrDisplay, (uint32*)&pData[offset], result);
                        offset += sizeof(uint32);
                    }

                    break;
                }


                case COUNTERID_SMU7_APU_PWR_PACKAGE:
                {
                    memcpy((uint32*)&pData[offset], &pkgPwr, sizeof(uint32));
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU7_APU_TEMP_IGPU:
                {
                    // Collect calculated GPU temperature
                    SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_tempCalcGpu, (uint32*)&pData[offset], result);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU7_APU_TEMP_MEAS_IGPU:
                {
                    // Collect measured GPU temperature
                    SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_tempMeasGpu, (uint32*)&pData[offset], result);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SMU7_APU_FREQ_IGPU:
                {
                    // Collect GPU frequency SCLK
                    SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_sclk, (uint32*)&pData[offset], result);
                    offset += sizeof(uint32);
                    break;
                }

                default:
                    break;
            }

        }
    }
    else
    {
        mask = pSmuInfo->m_counterMask;

        for (idx = 0; idx < COUNTERID_DGPU_MAX_CNT; idx++)
        {
            if (false == ((mask >> idx) & 0x01))
            {
                continue;
            }

            switch (idx)
            {
                case COUNTERID_PKG_PWR_DGPU:
                {
                    SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_pwrPackage, (uint32*)&pData[offset], result);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_TEMP_MEAS_DGPU:
                {
                    SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_tempMeasGpu, (uint32*)&pData[offset], result);
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_FREQ_DGPU:
                {
                    uint32* val = (uint32*)&pData[offset];
                    SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_sclk, val, result);
                    offset += sizeof(uint32);
                    //DRVPRINT("COUNTERID_FREQ_DGPU reg 0x%x 0x%x", pTable->m_sclk, *val);
                    break;
                }

                case COUNTERID_VOLT_VDDC_LOAD_DGPU:
                {
                    uint32* val = (uint32*)&pData[offset];
                    SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_voltVddcLoad, val, result);
                    offset += sizeof(uint32);
                    //DRVPRINT("COUNTERID_VOLT_VDDC_LOAD_DGPU reg 0x%x 0x%x", pTable->m_voltVddcLoad, *val);
                    break;
                }

                case COUNTERID_CURR_VDDC_DGPU:
                {
                    uint32* val = (uint32*)&pData[offset];
                    SMU7ReadSmuIndirectMappingRegister(pSmuData, pTable->m_currVddc, val, result);
                    offset += sizeof(uint32);
                    //DRVPRINT("COUNTERID_CURR_VDDC_DGPU reg 0x%x 0x%x", pTable->m_currVddc, *val);
                    break;
                }

                default:
                    break;
            }
        }
    }

        *pLength = *pLength + offset;

    return result;
}

// Smu7SessionInitialize: Initialize Smu7 profile session
bool Smu7SessionInitialize(void* pSmu)
{

    bool result = false;
    SmuInfo* pSmuInfo = (SmuInfo*)pSmu;
    Smu7Interface* pSmuData = &pSmuInfo->m_access.m_smu7;

    // Get the GPU Memory Mapped registers based address
    pSmuData->m_gpuBaseAddress = pSmuInfo->m_gpuBaseAddress;

    if (0 != pSmuData->m_gpuBaseAddress)
    {
        // Memory map the GPI base address to send requests to SMU
        result = HelpMapMMIOSpace(pSmuData->m_gpuBaseAddress + pSmuData->m_messageParam.m_msgId,
                                  (pSmuData->m_messageParam.m_arg - pSmuData->m_messageParam.m_msgId) + sizeof(uint32),
                                  &pSmuData->m_mappedAddress,
                                  &pSmuData->m_mappedSize);

        if (true == result)
        {
            // Memory map for indirect access data/index pair.
            result = HelpMapMMIOSpace(pSmuData->m_gpuBaseAddress + pSmuData->m_indexData.m_index,
                                      2 * sizeof(uint32),
                                      &pSmuData->m_indexDataPair,
                                      &pSmuData->m_indexDataPairSize);
        }

        if (true == result)
        {
            // Send the SMU PM Start Log command
            result = Smu7MessageMMIOSpec(pSmuData, pSmuData->m_logging.m_start);
        }
        else
        {
            // Unmmap the mapped address
            HelpUnmapMMIOSpace(pSmuData->m_mappedAddress, pSmuData->m_mappedSize);
        }
    }

    return result;
}

// Smu7SessionClose: Close Smu7 access
bool Smu7SessionClose(void* pSmu)
{
    bool result = false;
    SmuInfo* pSmuInfo = (SmuInfo*)pSmu;
    Smu7Interface* pSmuData = &pSmuInfo->m_access.m_smu7;

    if ((0 != pSmuData->m_mappedAddress) && pSmuData->m_mappedSize)
    {
        HelpUnmapMMIOSpace(pSmuData->m_mappedAddress, pSmuData->m_mappedSize);
    }

    if ((0 != pSmuData->m_indexDataPair) && pSmuData->m_indexDataPairSize)
    {
        HelpUnmapMMIOSpace(pSmuData->m_indexDataPair, pSmuData->m_indexDataPairSize);
    }

    pSmuData->m_mappedAddress = 0;
    pSmuData->m_mappedSize = 0;

    pSmuData->m_indexDataPair = 0;
    pSmuData->m_indexDataPairSize = 0;

    return result;
}

uint32 GetSmu7CounterSize(uint32 counterId)
{
    uint32 bufferLen = 0;
    uint32 cuCnt = GetComputeUnitCntPerNode();

    switch (counterId)
    {
        // per cu counters
        case COUNTERID_SMU7_APU_PWR_CU:
        case COUNTERID_SMU7_APU_TEMP_CU:
        case COUNTERID_SMU7_APU_TEMP_MEAS_CU:
        {
            bufferLen += (cuCnt * sizeof(uint32));
            break;
        }

        // 4 byte counters
        case COUNTERID_SMU7_APU_PWR_IGPU:
        case COUNTERID_SMU7_APU_PWR_PCIE:
        case COUNTERID_SMU7_APU_PWR_DDR:
        case COUNTERID_SMU7_APU_PWR_DISPLAY:
        case COUNTERID_SMU7_APU_PWR_PACKAGE:
        case COUNTERID_SMU7_APU_TEMP_IGPU:
        case COUNTERID_SMU7_APU_TEMP_MEAS_IGPU:
        case COUNTERID_SMU7_APU_FREQ_IGPU:
        {
            bufferLen += sizeof(uint32);
            break;
        }

        default:
            break;
    }

    return bufferLen;
}

uint32 GetSmu7DgpuCounterSize(uint32 counterId)
{
    uint32 bufferLen = 0;

    switch (counterId)
    {
        case COUNTERID_PKG_PWR_DGPU:
        case COUNTERID_TEMP_MEAS_DGPU:
        case COUNTERID_FREQ_DGPU:
        case COUNTERID_VOLT_VDDC_LOAD_DGPU:
        case COUNTERID_CURR_VDDC_DGPU:
        {
            bufferLen += sizeof(uint32);
            break;
        }

        default:
            break;
    }

    return bufferLen;
}

