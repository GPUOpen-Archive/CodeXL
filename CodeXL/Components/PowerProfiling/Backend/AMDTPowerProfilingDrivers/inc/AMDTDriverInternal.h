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

#ifndef _AMDTDRIVER_INTERNAL_H
#define _AMDTDRIVER_INTERNAL_H

#include <AMDTPwrProfInternal.h>
#include <AMDTRawDataFileHeader.h>
#include <AMDTDriverTypedefs.h>

// Data/Header buffer size
#define DATA_PAGE_BUFFER_SIZE 4096

/// Header buffer size
#define HEADER_BUFFER_SIZE 4096

// Client Data
// This struct is created when a client registers with the
// driver. Only one client can be registered with driver due
// to the SMU limitaiton.
typedef struct ClientData
{
    uint32      m_clientId;
    uint32      m_validClient;
    uint32      m_isOffline;
    uint32      m_configCount;
    uint32      m_profileState;
    PageBuffer  m_header;
    OsClientCfg m_osClientCfg;
} ClientData;

// CoreData
//
// This structure holds the data for percore configurations.
// In windows this structure will be created for configured cores
// In case of Linux this structure will be created for all available
// cores in the platform
typedef struct CoreData
{
    uint32           m_clientId;
    uint32           m_sampleId;
    uint32           m_profileType;
    uint32           m_samplingInterval;
    uint32           m_recLen;
    uint32           m_coreId;
    ContextData      m_contextData;
    PageBuffer*      m_pCoreBuffer;
    uint64           m_counterMask;
    SmuList*         m_smuCfg;
    OsCoreCfgData*   m_pOsData;
    PmcCounters      m_pmc[PMC_EVENT_MAX_CNT];
    PwrInternalAddr  m_internalCounter;
} CoreData;

#endif //_AMDTDRIVER_INTERNAL_H