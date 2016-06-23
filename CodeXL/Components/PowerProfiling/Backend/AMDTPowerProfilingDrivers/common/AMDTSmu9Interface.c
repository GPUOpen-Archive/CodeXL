//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTPowerProfileApi.cpp
///
//==================================================================================

// LOCAL INCLUDES
#include <AMDTDriverTypedefs.h>
#include <AMDTHelpers.h>
#include <AMDTRawDataFileHeader.h>
#include <AMDTSmu9Interface.h>
#include <AMDTCommonConfig.h>

// PROJECT INCLUDES
#include <AMDTSmu9CounterTable.h>

// LOCAL DEFINE
#define SMU9_RETRY_COUNT_MAX    (8192)
#define SMU9_BASE_ADDRESS       0x10000000
#define MAPPED_BASE_OFFSET      0x608
#if 0
// PwrSmu9ReadIndexDataMapping: Index and data pair mapping for reading soft register values
static void PwrSmu9ReadIndexDataMapping(Smu9Interface* pSmu, uint64 reg, uint32* pValue)
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

// PwrSmu9MessageService: Smu9 message is used for reading counter table base address.
static bool PwrSmu9MessageService(Smu9Interface* pSmu, uint32 msgId, uint32* arg, uint32* pData, uint32 retry)
{
    bool ret = false;
    volatile uint32 repeatCnt = retry;
    uint64 res = 0;

    // Write 0 to resp to check if it is busy
    WRITE_DWORD(pSmu->m_msg.m_resp, 0);

    // Write argument if any
    if (NULL != arg)
    {
        WRITE_DWORD(pSmu->m_msg.m_arg, *arg);
    }

    // Write message id
    WRITE_DWORD(pSmu->m_msg.m_msg, msgId);

    // Wait for completion
    do
    {
        res = READ_DWORD(pSmu->m_msg.m_resp);

        if (res & SMC_RESULT_OK)
        {
            ret = true;
        }
    }
    while ((false == ret) && --repeatCnt);

    // Read the result
    if ((true == ret) && (NULL != pData))
    {
        *pData = (uint32)READ_DWORD(pSmu->m_msg.m_arg);
    }

    return ((true == ret) && (NULL != pData)) ? true : false;
}

// PwrSmu9RequestCounterTable: RequestCounterTable: Acquire the counter table base address which holds
// the SMU soft registers base address.
static bool PwrSmu9RequestCounterTable(Smu9Interface* pSmu, uint32* pData, uint32 retry)
{
    bool result = false;
	(void)pSmu;
	(void)pData;
	(void)retry;
    return result;
}
#endif
// PwrSmu9CollectRegisterValues: Get the Smu9 register values.
bool PwrSmu9CollectRegisterValues(void* pSmuInfo, uint8* pData, uint32* pLength)
{
    bool result = false;
    (void)pSmuInfo;
	(void)pData;
	(void)pLength;
    return result;
}

// PwrSmu9SessionInitialize: Initialize Smu9 Session
bool PwrSmu9SessionInitialize(void* pSmuInfo)
{
    bool result = false;
	(void)pSmuInfo;
    return result;
}

// PwrSmu9SessionClose: Close Smu9 Session.
bool PwrSmu9SessionClose(void* pSmuInfo)
{
    bool result = false;
    (void)pSmuInfo;
    return result;
}

// PwrGetSmu9CounterSize: Get size of each counter
uint32 PwrGetSmu9CounterSize(uint32 counterId)
{
    uint32 bufferLen = 0;
    (void) counterId;
    return bufferLen;
}
