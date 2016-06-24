//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RawDataReader.cpp
///
//==================================================================================

#if ( defined (_WIN32) || defined (_WIN64) )
    #include <Windows.h>
    #include <AMDTDriverControl/inc/DriverControl.h>
#endif

#include <AMDTRawDataFileHeader.h>
#include "RawDataReader.h"
#include <string.h>
#include <OsFileWrapper.h>
#include <AMDTPowerProfileInternal.h>
#include <PowerProfileDriverInterface.h>
#include <AMDTPwrProfDriver.h>
#include <PowerProfileHelper.h>

static bool g_isOnline = false;
// ---------------------------------------------------------------------------
// Name:        RawDataReader::RawDataReader
// Description: Contructor
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
RawDataReader::RawDataReader()
{
    //Allocate separetly for buffer header as well
    m_pBufferHeader = (AMDTUInt8*) malloc(FILE_BUFFER_SIZE);
    m_pBufferList = NULL;
    m_bufferCnt = 0;
    m_hRawFile = NULL;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::~RawDataReader
// Description: Destructor
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
RawDataReader::~RawDataReader()
{
    RawDataClose();
}

// ResetBufferList
AMDTResult RawDataReader::ResetBufferList()
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTUInt32 cnt = 0;

    if (m_bufferCnt > 0)
    {
        for (cnt = 0; cnt < m_bufferCnt; cnt++)
        {
            RawBufferInfo* list = m_pBufferList + cnt;
            AMDTUInt8* pMem = (AMDTUInt8*)list->uliBuffer.QuadPart;
            memset(pMem, 0, sizeof(AMDTUInt8) * FILE_BUFFER_SIZE);
            list->ulvalidLength = 0;
        }
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::ReleaseBufferList
// Description: Create buffer per target core.
// Author:      Rajeeb Barman
// Date:        11/03/2014
// ---------------------------------------------------------------------------
AMDTResult RawDataReader::ReleaseBufferList()
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTUInt32 cnt = 0;

    if (m_bufferCnt > 0)
    {
        for (cnt = 0; cnt < m_bufferCnt; cnt++)
        {
            RawBufferInfo* list = m_pBufferList + cnt;
			if (nullptr != list)
            {
                list->ulvalidLength = 0;
                AMDTUInt8* pMem = (AMDTUInt8*)(list->uliBuffer.QuadPart);

                if (nullptr != pMem)
                {
                    free(pMem);
                    pMem = nullptr;
                }
            }
        }

        free(m_pBufferList);
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::PrepareRawBufferList
// Description: Create buffer per target core.
// Author:      Rajeeb Barman
// Date:        11/03/2014
// ---------------------------------------------------------------------------
AMDTResult RawDataReader::PrepareRawBufferList(AMDTUInt16 coreCnt)
{
    AMDTInt32 ret = AMDT_STATUS_OK;

    if (NULL == m_pBufferList)
    {
        if (!coreCnt)
        {
            ret = AMDT_ERROR_FAIL;
        }

        if (AMDT_STATUS_OK == ret)
        {
            m_pBufferList = (RawBufferInfo*)malloc(sizeof(RawBufferInfo) * coreCnt);

            if (!m_pBufferList)
            {
                ret = AMDT_ERROR_OUTOFMEMORY;
            }
        }

        if (AMDT_STATUS_OK == ret)
        {

            for (AMDTUInt16 cnt = 0; cnt < coreCnt; cnt++)
            {
                RawBufferInfo* pBuffer = NULL;
                AMDTUInt8* pMem = (AMDTUInt8*)malloc(sizeof(AMDTUInt8) * FILE_BUFFER_SIZE);

                if (NULL == pMem)
                {
                    ret = AMDT_ERROR_OUTOFMEMORY;
                    break;
                }

                pBuffer = m_pBufferList + cnt;
                pBuffer->uliBuffer.QuadPart = reinterpret_cast<uint64>(pMem);
                pBuffer->ulvalidLength = 0;
            }
        }

        m_bufferCnt = coreCnt;
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::Close
// Description: Close the raw file and release all handles and memory
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
AMDTResult RawDataReader::Close()
{
    AMDTInt32 ret = AMDT_STATUS_OK;

    if (!g_isOnline && (NULL != m_hRawFile))
    {
        PwrCloseFile(m_hRawFile);
        m_hRawFile = NULL;
    }

    if (m_pBufferList && m_bufferCnt)
    {
        for (AMDTUInt16 cnt = 0; cnt < m_bufferCnt; cnt++)
        {
            RawBufferInfo* pBuff = m_pBufferList + cnt;
            AMDTUInt8* pMem = (AMDTUInt8*)(pBuff->uliBuffer.QuadPart);
            free(pMem);
            pBuff->ulvalidLength = 0;
        }
    }

    ReleaseFileHandle(&m_rawDataHdl);
    return ret;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::ReleaseFileHandle
// Description: ReleaseFileHandle clear all allocated memory
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
AMDTResult RawDataReader::ReleaseFileHandle(RawDataHandle* pFileHld)
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    //SectionType type;
    AMDTUInt64 type;
    SectionInfo* pSectionInfo;
    AMDTUInt32 cnt = 0;

    if (!pFileHld)
    {
        ret = AMDT_ERROR_FAIL;
    }

    if (AMDT_STATUS_OK == ret)
    {
        pSectionInfo = pFileHld->m_pTabInfo->m_pSecInfo;

        while (pFileHld->m_pTabInfo->m_SecCnt > cnt++)
        {
            if (pSectionInfo->m_pSecHdrData)
            {
                //Single allocation for following sections
                type = pSectionInfo->m_secType;

                if (type == RAW_FILE_SECTION_RUN_INFO ||
                    type == RAW_FILE_SECTION_CPU_INFO ||
                    type == RAW_FILE_SECTION_CPU_TOPOLOGY ||
                    type == RAW_FILE_SECTION_SAMPLE_REC_INFO ||
                    type == RAW_FILE_SECTION_TI_REC_INFO)
                {
                    free(pSectionInfo->m_pSecHdrData);
                }
                else if (type == RAW_FILE_SECTION_SAMPLE_CONFIG)
                {
                    ProfileConfigList* pCfg = (ProfileConfigList*)pSectionInfo->m_pSecHdrData;

                    if (!pCfg)
                    {
                        ret = AMDT_ERROR_FAIL;
                        break;
                    }

                    if (pCfg->m_configCnt && pCfg->m_profileConfig)
                    {
                        free(pCfg->m_profileConfig);
                    }

                    free(pCfg);
                }
                else
                {
                    ret = AMDT_ERROR_FAIL;
                    break;
                }
            }

            pSectionInfo++;
        }

        free(pFileHld->m_pTabInfo->m_pSecInfo);
        free(pFileHld->m_pTabInfo);
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::RawDataInitialize
// Description: This function will open the raw file and read the configuration records.
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
AMDTResult RawDataReader::RawDataInitialize(const wchar_t* pFileName, bool isOnline)
{
    pFileName = pFileName;

    AMDTInt32 ret = 0;
    g_isOnline = isOnline;

    if (!g_isOnline)
    {
#if 0 //File support
        //Open the raw data file
        ret = OpenFile(&m_hRawFile, pFileName, L"rb");

        if (m_hRawFile == NULL)
        {
            ret = AMDT_ERROR_FAIL;
        }

        if (AMDT_STATUS_OK == ret)
        {
            //read file to a buffer
            SeekFile(m_hRawFile, 0, SEEK_SET);
            size = ReadFile(m_hRawFile, (AMDTUInt8*) m_fileBuffer, FILE_BUFFER_SIZE);
        }

#endif
    }
    else
    {
        //If online, read from buffer
        AMDTUInt32 clientIdx = 0;
        FILE_HEADER header;
        AMDTUInt32 res = 0;

        ret = GetPowerDriverClientId(&clientIdx);

        if (AMDT_STATUS_OK == ret)
        {
            header.ulClientId = clientIdx;
            header.ulBufferId = 0;
            header.uliBuffer = reinterpret_cast<uint64>(m_pBufferHeader);
            ret = CommandPowerDriver(DRV_CMD_TYPE_IN_OUT,
                                     IOCTL_GET_FILE_HEADER_BUFFER,
                                     &header,
                                     sizeof(FILE_HEADER),
                                     &header,
                                     sizeof(FILE_HEADER),
                                     &res);
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        //First section of the raw data file is file header
        ret = ReadFileHeader();
    }

    if (AMDT_STATUS_OK == ret)
    {
        //Allocate memory for a raw buffer chunk.
        m_offset = m_rawDataHdl.m_fileHdr.m_rawDataOffset;

        //Read section headers
        ret = ReadSections();
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (!g_isOnline)
        {
            //Calculate the length of the file
            PwrSeekFile(m_hRawFile, 0, SEEK_END);
            m_fileLen = ftell(m_hRawFile);

            //Raw record offset;
            m_curr = m_offset = m_rawDataHdl.m_fileHdr.m_rawDataOffset;
            m_onlineOffset = m_offset;
        }
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::RawDataClose
// Description: RawDataClose will close the raw file
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
AMDTResult RawDataReader::RawDataClose()
{
    if (!g_isOnline && (NULL != m_hRawFile))
    {
        PwrCloseFile(m_hRawFile);
        m_hRawFile = NULL;
    }

    return AMDT_STATUS_OK;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::CheckRawAttributeMask
// Description: Check for a valid attribute
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
bool RawDataReader::CheckRawAttributeMask(AMDTUInt16 attributeId, AMDTUInt64* pEventMask)
{
    if (attributeId > COUNTERID_MAX_CNT)
    {
        return false;
    }

    if (pEventMask[attributeId / 64] & (1ULL << (attributeId % 64)))
    {
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::GetSampleSectionInfo
// Description:GetSampleSectionInfo will provide the sample section information.
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
AMDTResult RawDataReader::GetSampleSectionInfo(SectionSampleInfo* pSampleInfo)
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTUInt32 cnt = 0;
    SectionInfo* pSectionInfo = m_rawDataHdl.m_pTabInfo[0].m_pSecInfo;

    if (!pSampleInfo)
    {
        ret = AMDT_ERROR_FAIL;
    }

    if (AMDT_STATUS_OK == ret)
    {
        for (cnt = 0; cnt < m_rawDataHdl.m_pTabInfo[0].m_SecCnt; cnt++)
        {
            if (pSectionInfo->m_secType == RAW_FILE_SECTION_SAMPLE_REC_INFO)
            {
                memcpy(pSampleInfo, pSectionInfo->m_pSecHdrData, sizeof(SectionSampleInfo));
                break;
            }

            pSectionInfo++;
        }

        if (cnt == m_rawDataHdl.m_pTabInfo[0].m_SecCnt)
        {
            ret = AMDT_ERROR_FAIL;
        }
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::GetProfileCfgInfo
// Description:GetProfileCfgInfo will provide the profile configuration information.
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
AMDTResult RawDataReader::GetProfileCfgInfo(ProfileConfigList* pCfgTabInfo)
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTUInt32 cnt = 0;
    ProfileConfigList* pCfg = NULL;
    SectionInfo* pSectionInfo = m_rawDataHdl.m_pTabInfo[0].m_pSecInfo;

    if (!pCfgTabInfo)
    {
        ret = AMDT_ERROR_FAIL;
    }

    if (AMDT_STATUS_OK == ret)
    {
        for (cnt = 0; cnt < m_rawDataHdl.m_pTabInfo[0].m_SecCnt; cnt++)
        {
            if (pSectionInfo->m_secType == RAW_FILE_SECTION_SAMPLE_CONFIG)
            {
                if (!pCfgTabInfo || !pCfgTabInfo->m_profileConfig)
                {
                    ret = AMDT_ERROR_FAIL;
                    break;
                }

                pCfg = (ProfileConfigList*)pSectionInfo->m_pSecHdrData;
                //Get profile config count
                pCfgTabInfo->m_configCnt = pCfg->m_configCnt;

                memcpy(pCfgTabInfo->m_profileConfig,
                       pCfg->m_profileConfig,
                       sizeof(ProfileConfig)* pCfgTabInfo->m_configCnt);
                break;
            }

            pSectionInfo++;
        }
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (cnt == m_rawDataHdl.m_pTabInfo[0].m_SecCnt)
        {
            ret = AMDT_ERROR_FAIL;
        }
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::ReadFileHeader
// Description:Read file header to retrieve the version info and file format.
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
AMDTResult RawDataReader::ReadFileHeader()
{
    AMDTInt32 ret = AMDT_STATUS_OK;

    memcpy((AMDTUInt8*) &m_rawDataHdl.m_fileHdr, m_pBufferHeader, sizeof(RawFileHeader));

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::ReadSections
// Description: Read optional section and power/cpu configuration sections.
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
AMDTResult RawDataReader::ReadSections()
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    AMDTUInt8 cnt = 0;
    AMDTUInt32 cnt1 = 0;
    SectionTabData* pSectionTab;

    SectionHdrInfo* pSectionHdr;
    AMDTUInt16 sectionCnt = m_rawDataHdl.m_fileHdr.m_sectionTabCnt;

    if (sectionCnt)
    {
        pSectionTab = (SectionTabData*)malloc(sizeof(SectionTabData) * sectionCnt);
        pSectionTab->m_SecCnt = m_rawDataHdl.m_fileHdr.m_tabInfo[0].m_sectionTabSize / sizeof(SectionHdrInfo);
        pSectionTab->m_pSecInfo = (SectionInfo*)malloc(sizeof(SectionInfo) * pSectionTab->m_SecCnt);
        m_rawDataHdl.m_pTabInfo = pSectionTab;

        //read the section headers
        pSectionHdr = (SectionHdrInfo*)(m_pBufferHeader + m_rawDataHdl.m_fileHdr.m_tabInfo[0].m_sectionTabOff);

        for (cnt = 0; (cnt < pSectionTab->m_SecCnt) && (AMDT_STATUS_OK == ret); cnt++)
        {
            switch (pSectionHdr->m_sectionType)
            {
                case RAW_FILE_SECTION_RUN_INFO:
                {
                    SectionRunInfo* pRunInfo;
                    SectionInfo* pSectionInfo = (SectionInfo*)(pSectionTab->m_pSecInfo + cnt);
                    pRunInfo = (SectionRunInfo*)(m_pBufferHeader + pSectionHdr->m_sectionOffset);
                    pSectionInfo->m_secType = RAW_FILE_SECTION_RUN_INFO;
                    pSectionInfo->m_pSecHdrData = (SectionRunInfo*) malloc(sizeof(SectionRunInfo));
                    memcpy(pSectionInfo->m_pSecHdrData, pRunInfo, sizeof(SectionRunInfo));
                }
                break;

                case RAW_FILE_SECTION_CPU_INFO:
                {
                    SectionCpuInfo* pCpuInfo;
                    SectionInfo* pSectionInfo = (SectionInfo*)(pSectionTab->m_pSecInfo + cnt);
                    pCpuInfo = (SectionCpuInfo*)(m_pBufferHeader + pSectionHdr->m_sectionOffset);
                    pSectionInfo->m_secType = RAW_FILE_SECTION_CPU_INFO;
                    pSectionInfo->m_pSecHdrData = (SectionCpuInfo*) malloc(sizeof(SectionCpuInfo));
                    memcpy(pSectionInfo->m_pSecHdrData, pCpuInfo, sizeof(SectionCpuInfo));
                }
                break;

                case RAW_FILE_SECTION_CPU_TOPOLOGY:
                {
                    SectionCpuTopologyInfo* pTopologyInfo;
                    SectionInfo* pSectionInfo = (SectionInfo*)(pSectionTab->m_pSecInfo + cnt);
                    pTopologyInfo = (SectionCpuTopologyInfo*)(m_pBufferHeader + pSectionHdr->m_sectionOffset);
                    pSectionInfo->m_secType = RAW_FILE_SECTION_CPU_TOPOLOGY;
                    pSectionInfo->m_pSecHdrData = (SectionCpuTopologyInfo*) malloc(sizeof(SectionCpuTopologyInfo));
                    memcpy(pSectionInfo->m_pSecHdrData, pTopologyInfo, sizeof(SectionCpuTopologyInfo));
                }
                break;

                case RAW_FILE_SECTION_SAMPLE_CONFIG:
                {
                    AMDTUInt64 coreMask = 0;
                    AMDTUInt64 coreSpecificMask = 0;
                    AMDTUInt32 profileCnt = 0;
                    ProfileConfigList* pCfgList = nullptr;
                    ProfileConfig* pCfg = nullptr;
                    ProfileConfig* pTempCfg = nullptr;
                    AMDTUInt64 cfgOffset = pSectionHdr->m_sectionOffset;
                    SectionInfo* pSectionInfo = (SectionInfo*)(pSectionTab->m_pSecInfo + cnt);

                    pSectionInfo->m_secType = RAW_FILE_SECTION_SAMPLE_CONFIG;

                    // Read config from raw buffer
                    pCfg = (ProfileConfig*)(m_pBufferHeader + pSectionHdr->m_sectionOffset);
                    profileCnt = pCfg->m_samplingSpec.m_maskCnt;

                    //Allocate memory for the configuration
                    pCfgList = (ProfileConfigList*) malloc(sizeof(ProfileConfigList));

                    if (nullptr == pCfgList)
                    {
                        ret = AMDT_ERROR_FAIL;
                        break;
                    }

                    memset(pCfgList, 0, sizeof(ProfileConfigList));
                    pCfgList->m_profileConfig = (ProfileConfig*)malloc(sizeof(ProfileConfig) * profileCnt);

                    if (nullptr == pCfgList->m_profileConfig)
                    {
                        ret = AMDT_ERROR_FAIL;
                        break;
                    }

                    memset(pCfgList->m_profileConfig, 0, sizeof(ProfileConfig) * profileCnt);
                    pCfgList->m_configCnt = profileCnt;
                    coreMask = pCfg->m_samplingSpec.m_mask;
                    coreSpecificMask = pCfg->m_apuCounterMask & PWR_PERCORE_COUNTER_MASK;

                    // Create per core configuration
                    for (cnt1 = 0; cnt1 < profileCnt; cnt1++)
                    {
                        pTempCfg = pCfgList->m_profileConfig + cnt1;
                        AMDTInt32 coreIdx = 0;

                        if (nullptr != pTempCfg)
                        {
                            // First copy the common config
                            memcpy(pTempCfg, pCfg, sizeof(ProfileConfig));
                            pTempCfg->m_sampleId = (AMDTUInt16)(cnt1 + 1);

                            // Per core config
                            pTempCfg->m_samplingSpec.m_maskCnt = 1;

                            // set the core mask
                            GetFirstSetBitIndex((AMDTUInt32*)&coreIdx, coreMask);
                            coreMask &= ~(1ULL << coreIdx);
                            pTempCfg->m_samplingSpec.m_mask = 1ULL << coreIdx;

                            // Set only core specific counters for other cores
                            if (0 != cnt1)
                            {
                                pTempCfg->m_apuCounterMask = coreSpecificMask;
                                pTempCfg->m_attrCnt = (AMDTUInt16)GetMaskCount(coreSpecificMask);
                            }
                        }
                    }

                    cfgOffset += (profileCnt * sizeof(ProfileConfig));
                    pSectionInfo->m_pSecHdrData = pCfgList;
                }
                break;

                case RAW_FILE_SECTION_SAMPLE_REC_INFO:
                {
                    SectionSampleInfo* sampleInfo;
                    SectionInfo* pSectionInfo = (SectionInfo*)(pSectionTab->m_pSecInfo + cnt);
                    sampleInfo = (SectionSampleInfo*)(m_pBufferHeader + pSectionHdr->m_sectionOffset);
                    pSectionInfo->m_secType = RAW_FILE_SECTION_SAMPLE_REC_INFO;
                    pSectionInfo->m_pSecHdrData = (SectionSampleInfo*) malloc(sizeof(SectionSampleInfo));
                    memcpy(pSectionInfo->m_pSecHdrData, sampleInfo, sizeof(SectionSampleInfo));
                }
                break;

                case RAW_FILE_SECTION_TI_REC_INFO:
                {
                    SectionTiInfo* pTiInfo;
                    SectionInfo* pSectionInfo = (SectionInfo*)(pSectionTab->m_pSecInfo + cnt);
                    pTiInfo = (SectionTiInfo*)(m_pBufferHeader + pSectionHdr->m_sectionOffset);
                    pSectionInfo->m_secType = RAW_FILE_SECTION_TI_REC_INFO;
                    pSectionInfo->m_pSecHdrData = (SectionTiInfo*) malloc(sizeof(SectionTiInfo));
                    memcpy(pSectionInfo->m_pSecHdrData, pTiInfo, sizeof(SectionTiInfo));
                }
                break;

                default:
                    ret = AMDT_ERROR_FAIL;
                    break;
            }

            pSectionHdr++;
        }
    }

    //Support for additional table (if any)

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::Reset
// Description:Reset all parameters, pointers and counters
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
AMDTResult RawDataReader::Reset()
{
    AMDTInt32 ret = AMDT_STATUS_OK;
    m_pBufferHeader = NULL;
    m_pBufferList = NULL;
    m_bufferCnt = 0;
    m_hRawFile = NULL;
    m_pHeaderTableInfo = NULL;
    m_offset = 0;
    m_fileLen = 0;
    m_curr = 0;
    m_onlineOffset = 0;

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::GetSessionDuration
// Description:Read the duration for which profile was run
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
AMDTResult RawDataReader::GetSessionDuration(AMDTUInt64* pDuration)
{
    AMDTInt32 ret = AMDT_STATUS_OK;

    if (NULL != pDuration)
    {
        *pDuration = m_rawDataHdl.m_fileHdr.m_sessionEnd - m_rawDataHdl.m_fileHdr.m_sessionStart;
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        RawDataReader::GetSessionTimeStamps
// Description:Read the start time and end sesssion time
// Author:      Rajeeb Barman
// Date:        26/09/2013
// ---------------------------------------------------------------------------
AMDTResult RawDataReader::GetSessionTimeStamps(AMDTUInt64* pStart, AMDTUInt64* pEnd)
{
    AMDTInt32 ret = AMDT_STATUS_OK;

    if (NULL != pStart)
    {
        *pStart = m_rawDataHdl.m_fileHdr.m_sessionStart;
    }

    if (NULL != pEnd)
    {
        *pEnd = m_rawDataHdl.m_fileHdr.m_sessionEnd;
    }

    return ret;
}

