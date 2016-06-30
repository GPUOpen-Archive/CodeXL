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

#include <AMDTHelpers.h>
#include <AMDTCounterAccessInterface.h>

static uint32 g_boostedPstateCnt = INVALID_UINT32_VALUE;
static bool g_cefSupported = false;
static bool g_cefROSupported = false;

#define MPERF_MSR_ADDRESS      0x000000E7
#define APERF_MSR_ADDRESS      0x000000E8
#define P0STATE_MSR_ADDRESS    0xC0010064
#define MPERF_RO_MSR_ADDRESS   0xC00000E7
#define APERF_RO_MSR_ADDRESS   0xC00000E8
#define CEF_BUFFER_SIZE        (4)

typedef struct
{
    uint64 m_aperf;
    uint64 m_mperf;
} CefInfo;

// Per core CEF data to calculate the overflow
static CefInfo g_prevROCefData[CEF_BUFFER_SIZE];

// ReadBoostedPstate:
static uint32 ReadBoostedPstate(void)
{
    // Get the number of boosted P-States from D18F4x15C Core Performance Boost Control
    ACCESS_PCI pci;
    pci.address =  HelpEncodeExtendedConfigSpaceAddress(0, 0x18, 0x4, 0x15C);
    pci.isReadAccess = true;
    HelpAccessPciAddress(&pci);

    // bits 4:2 are NumBoostStates
    return ((pci.data & 0x1C) >> 2);
}

// ResetCoreEffectiveFreqCounters:
static void ResetCoreEffectiveFreqCounters(void)
{
    ACCESS_MSR msr;

    if (true == g_cefSupported)
    {
        msr.isReadAccess = false;
        msr.regId = MPERF_MSR_ADDRESS;
        msr.data = 0;
        HelpAccessMSRAddress(&msr);
        msr.isReadAccess = false;
        msr.data = 0;
        msr.regId = APERF_MSR_ADDRESS;
        HelpAccessMSRAddress(&msr);
    }
}

// ReadCoreEffectiveFreqROCounters:
static void ReadCoreEffectiveFreqROCounters(uint32 core, uint64* pAperf, uint64* pMperf, uint64* pP0State)
{
    ACCESS_MSR msr;
    uint64 mperf = 0;
    uint64 aperf = 0;

    if (g_cefROSupported)
    {
        msr.isReadAccess = true;
        mperf = HelpReadMsr64(MPERF_RO_MSR_ADDRESS);
        aperf = HelpReadMsr64(APERF_RO_MSR_ADDRESS);

        if (g_prevROCefData[core].m_mperf < mperf)
        {
            *pMperf = mperf - g_prevROCefData[core].m_mperf;
        }
        else
        {
            *pMperf = mperf + ~g_prevROCefData[core].m_mperf;
        }


        if (g_prevROCefData[core].m_aperf < aperf)
        {
            *pAperf = aperf - g_prevROCefData[core].m_aperf;
        }
        else
        {
            *pAperf = aperf + ~g_prevROCefData[core].m_aperf;
        }

        // Store the data
        g_prevROCefData[core].m_mperf = mperf;
        g_prevROCefData[core].m_aperf = aperf;

        // Read P-State-0 Frequency
        msr.regId = P0STATE_MSR_ADDRESS + g_boostedPstateCnt;
        HelpAccessMSRAddress(&msr);
        *pP0State = msr.data;
    }
    else
    {
        *pAperf = 0;
        *pMperf = 0;
        *pP0State = 0;
    }
}


// ReadCoreEffectiveFreqCounters:
static void ReadCoreEffectiveFreqCounters(uint64* pAperf, uint64* pMperf, uint64* pP0State)
{
    ACCESS_MSR msr;

    if (true == g_cefSupported)
    {
        msr.isReadAccess = true;
        msr.regId = MPERF_MSR_ADDRESS;
        HelpAccessMSRAddress(&msr);
        *pMperf = msr.data;

        msr.regId = APERF_MSR_ADDRESS;
        HelpAccessMSRAddress(&msr);
        *pAperf = msr.data;

        // Read P-State-0 Frequency
        msr.regId = P0STATE_MSR_ADDRESS + g_boostedPstateCnt;
        HelpAccessMSRAddress(&msr);
        *pP0State = msr.data;

        // Reset the counter value adter reading the registers
        ResetCoreEffectiveFreqCounters();
    }
    else
    {
        *pAperf = 0;
        *pMperf = 0;
        *pP0State = 0;
    }
}

// CollectPerCoreCounters:
bool CollectNodeCounters(CoreData* pCoreCfg, uint32* pLength)
{
    bool result = true;
    uint32 idx = 0;
    uint32 offset = 0;
    uint32* value = NULL;
    uint64* value64 = NULL;
    uint8* pData = NULL;

    if ((NULL == pCoreCfg)
        || (NULL == pCoreCfg->m_pCoreBuffer)
        || (NULL == pCoreCfg->m_pCoreBuffer->m_pBuffer))
    {
        DRVPRINT("InvalidpCoreCfg or pCoreCfg->m_pCoreBuffer->m_pBuffer");
        result = false;
    }

    if (true == result)
    {
        pData = pCoreCfg->m_pCoreBuffer->m_pBuffer;
        offset = *pLength;

        for (idx = COUNTERID_PID; idx <= COUNTERID_NODE_MAX_CNT; idx++)
        {
            if (false == ((pCoreCfg->m_counterMask >> idx) & 0x01))
            {
                continue;
            }

            switch (idx)
            {
                case COUNTERID_PID:
                {
                    //Get PID
                    value64 = (uint64*)&pData[offset];
                    *value64 = (uint64)pCoreCfg->m_contextData.m_processId;
                    offset += sizeof(uint64);
                    break;
                }

                case COUNTERID_TID:
                {
                    //Get TID
                    value64 = (uint64*)&pData[offset];
                    *value64 = (uint64)pCoreCfg->m_contextData.m_threadId;
                    offset += sizeof(uint64);
                    break;
                }

                case COUNTERID_CEF:
                {
                    if (g_cefROSupported)
                    {
                        ReadCoreEffectiveFreqROCounters(pCoreCfg->m_coreId,
                                                        (uint64*)&pData[offset],
                                                        (uint64*)&pData[offset + sizeof(uint64)],
                                                        (uint64*)&pData[offset + (2 * sizeof(uint64))]);
                    }
                    else
                    {
                        ReadCoreEffectiveFreqCounters((uint64*)&pData[offset],
                                                      (uint64*)&pData[offset + sizeof(uint64)],
                                                      (uint64*)&pData[offset + (2 * sizeof(uint64))]);
                    }

                    offset += (3 * sizeof(uint64));
                    break;

                }

                case COUNTERID_CSTATE_RES:
                {

                    ACCESS_PCI pci;
                    uint32 core = GetCurrentCoreId();
                    pci.isReadAccess = true;
                    pci.address = pCoreCfg->m_internalCounter.m_cstateRes + ((core) * sizeof(uint32));
                    HelpAccessPciAddress(&pci);
                    value = (uint32*)&pData[offset];
                    *value = pci.data;
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_PSTATE:
                {
#define PSTATE_MSR_ADDR 0xC0010071
                    ACCESS_MSR msr;
                    msr.isReadAccess = true;
                    msr.regId = PSTATE_MSR_ADDR;
                    HelpAccessMSRAddress(&msr);
                    value = (uint32*)&pData[offset];
                    *value = (msr.data >> 16) & 0x03;
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SVI2_CORE_TELEMETRY:
                {
                    ACCESS_PCI pciData;
                    pciData.address = pCoreCfg->m_internalCounter.m_sviTelemetry;
                    pciData.isReadAccess = true;
                    HelpAccessPciAddress(&pciData);
                    value = (uint32*)&pData[offset];
                    *value = pciData.data;
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_SVI2_NB_TELEMETRY:
                {
                    ACCESS_PCI pciData;
                    pciData.address = pCoreCfg->m_internalCounter.m_sviNBTelemetry;
                    pciData.isReadAccess = true;
                    HelpAccessPciAddress(&pciData);
                    value = (uint32*)&pData[offset];
                    *value = pciData.data;
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_NODE_TCTL_TEPERATURE:
                {
                    ACCESS_PCI pciData;

                    // D18F3xA4 Tctl temperature monitoring
                    pciData.address = HelpEncodeExtendedConfigSpaceAddress(0, 0x18, 0x3, 0xA4);
                    pciData.isReadAccess = true;
                    HelpAccessPciAddress(&pciData);
                    value = (uint32*)&pData[offset];
                    *value = pciData.data;
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_FAMILY17_CORE_ENERGY:
                {
                    uint64 energy = HelpReadMsr64(pCoreCfg->m_internalCounter.m_coreEnergyMsr);
                    value = (uint32*)&pData[offset];
                    *value = (uint32)energy;
                    offset += sizeof(uint32);
                    break;
                }

                case COUNTERID_FAMILY17_PKG_ENERGY:
                {
                    uint64 energy = HelpReadMsr64(pCoreCfg->m_internalCounter.m_packageEnergyMsr);
                    value = (uint32*)&pData[offset];
                    *value = (uint32)energy;
                    offset += sizeof(uint32);
                    break;
                }


                default:
                    break;
            }
        }
    }

    *pLength = offset;

    return result;
}

// CollectBasicCounters:
bool CollectBasicCounters(CoreData* pCoreCfg, uint32* pLength)
{
    bool result = true;
    uint32 idx = 0;
    uint32 offset = 0;
    uint64* value64 = NULL;
    uint16* value16 = NULL;
    uint8* pData = NULL;

    if ((NULL == pCoreCfg)
        || (NULL == pCoreCfg->m_pCoreBuffer)
        || (NULL == pCoreCfg->m_pCoreBuffer->m_pBuffer))
    {
        DRVPRINT("InvalidpCoreCfg or pCoreCfg->m_pCoreBuffer->m_pBuffer");
        result = false;
    }

    if (true == result)
    {
        pData = pCoreCfg->m_pCoreBuffer->m_pBuffer;
        offset = *pLength;

        for (idx = 0; idx <= COUNTERID_BASIC_CNT ; idx++)
        {
            switch (idx)
            {
                case COUNTERID_SAMPLE_ID:
                {
                    value16 = (uint16*) &pData[offset];
                    *value16 = (uint16)pCoreCfg->m_sampleId;
                    offset += sizeof(uint16);
                    break;
                }

                case COUNTERID_RECORD_ID:
                {
                    value64 = (uint64*) &pData[offset];
                    *value64 = pCoreCfg->m_pCoreBuffer->m_recCnt;
                    offset += sizeof(uint64);
                    break;
                }

                case COUNTERID_SAMPLE_TIME:
                {
                    value64 = (uint64*) &pData[offset];
                    *value64 = pCoreCfg->m_contextData.m_timeStamp;
                    offset += sizeof(uint64);
                    break;
                }

                default:
                    break;
            }
        }

        *pLength = offset;
    }

    return result;
}

// InitializeGenericCounterAccess:
void InitializeGenericCounterAccess(uint32 core)
{

    // Read boosted p-state
    if (INVALID_UINT32_VALUE == g_boostedPstateCnt)
    {
        g_boostedPstateCnt = ReadBoostedPstate();
    }

    g_cefSupported = HelpIsCefSupported();
    g_cefROSupported = HelpIsROCefAvailable();

    if (g_cefROSupported)
    {
        memset(&g_prevROCefData[0], 0, CEF_BUFFER_SIZE * sizeof(CefInfo));
        g_prevROCefData[core].m_mperf = HelpReadMsr64(MPERF_RO_MSR_ADDRESS);
        g_prevROCefData[core].m_aperf = HelpReadMsr64(APERF_RO_MSR_ADDRESS);
    }
    else if (g_cefSupported)
    {
        ResetCoreEffectiveFreqCounters();
    }
}

// CloseGenericCounterAccess:
void CloseGenericCounterAccess(void)
{
    g_boostedPstateCnt = INVALID_UINT32_VALUE;
    ResetCoreEffectiveFreqCounters();
}

// GetBasicCounterSize
uint32 GetBasicCounterSize(void)
{
    uint32 bufferLen = 0;
    //COUNTERID_SAMPLE_ID:
    bufferLen += sizeof(uint16);
    //COUNTERID_RECORD_ID:
    bufferLen += sizeof(uint64);
    //COUNTERID_SAMPLE_TIME:
    bufferLen += sizeof(uint64);
    return bufferLen;
}

// GetNodeCounterSize
uint32 GetNodeCounterSize(uint32 counterId)
{
    uint32 bufferLen = 0;

    switch (counterId)
    {
        // 8 byte counters
        case COUNTERID_PID:
        case COUNTERID_TID:
        {
            bufferLen += sizeof(uint64);
            break;
        }

        // 4 byte counters
        case COUNTERID_CSTATE_RES:
        case COUNTERID_PSTATE:
        case COUNTERID_NODE_TCTL_TEPERATURE:
        case COUNTERID_SVI2_CORE_TELEMETRY:
        case COUNTERID_SVI2_NB_TELEMETRY:
        {
            bufferLen += sizeof(uint32);
            break;
        }

        // 24 bytes counter
        case COUNTERID_CEF:
        {
            bufferLen += (3 * sizeof(uint64));
            break;
        }

        default:
            break;
    }

    return bufferLen;
}

