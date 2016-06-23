//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTPowerProfileApi.cpp
///
//==================================================================================

#ifndef _SMU9_INTERFACE_H_
#define _SMU9_INTERFACE_H_

// LOCAL INCLUDES
#include <AMDTDriverTypedefs.h>

// PROJECT INCLUDES
#include <AMDTSmu9CounterTable.h>


// CounterTableInfo: Counter table list
typedef struct PwrSmu9CounterTableInfo
{
    uint32              m_versionId;
    PwrSmu9CounterTable m_table;
} PwrSmu9CounterTableInfo;

// Smu9MessageInterface: Smu9 message interface
typedef struct Smu9MessageInterface
{
    uint64 m_msg;                  // Message id
    uint64 m_arg;                  // Argument
    uint64 m_resp;                 // Response
    uint8  m_requestCounterTable;  // Counter table request message id
    uint8  m_releaseCounterTable;  // Counter table release message id
    uint16 m_fill[3];
} Smu9MessageInterface;

// Smu9MMAccess: Channel-9 data index pair
typedef struct Smu9MMAccess
{
    uint64 m_index;
    uint64 m_data;
} Smu9MMAccess;

// Smu9Interface: Access interface for Smu9
typedef struct Smu9Interface
{
    uint64               m_gpuBaseAddr;
    uint64               m_mappedGMMMxSpace;
    uint64               m_mappedSize;
    Smu9MessageInterface m_msg;
    Smu9MMAccess         m_gmmxPair;
    uint64               m_tableBase;
    uint32               m_tableId;
    uint32               m_tableVersion;
} Smu9Interface;

bool PwrFillSmu9InterfaceInfo(Smu9Interface* pSmu);

// PwrSmu9CollectRegisterValues: Collect all SMU counterr values
// based on the mask set and fill in the pData
bool PwrSmu9CollectRegisterValues(void* pSmu,
                                          uint8* pData,
                                          uint32* pLength);

// PwrSmu9SessionInitialize:
bool PwrSmu9SessionInitialize(void* pSmu);

// PwrSmu9SessionClose
bool PwrSmu9SessionClose(void* pSmu);

// PwrGetSmu9CounterSize
uint32 PwrGetSmu9CounterSize(uint32 counterId);

#endif //_SMU9_INTERFACE_H_