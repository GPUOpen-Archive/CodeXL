#ifdef DEBUG
#include <AMDTPwrProfInternal.h>
#include <AMDTHelpers.h>

// PrintPageBuffer
//
// Print PageBuffer Data structure
void PrintPageBuffer(const PageBuffer* pPageBuffer)
{
    if (NULL != pPageBuffer)
    {
        DRVPRINT("Printing Page Buffer ....");
        DRVPRINT("      Rec Count       : %llu ", pPageBuffer->m_recCnt);
        DRVPRINT("      Current Offset  : %d ", ATOMIC_GET(pPageBuffer->m_currentOffset));
        DRVPRINT("      Consumed Offset : %d ", ATOMIC_GET(pPageBuffer->m_consumedOffset));
    }
}

// PrintSmuInfo
//
// Printing Smu INfo
void PrintSmuInfo(const SmuInfo* pSmuInfo)
{
    DRVPRINT(" Printing Smu Info ...");
    DRVPRINT(" is Accessible    : %d ",  pSmuInfo->m_isAccessible);
    DRVPRINT(" Package ID       : %d ",  pSmuInfo->m_packageId);
    DRVPRINT(" Smu Version      : %d ",  pSmuInfo->m_smuIpVersion);
    DRVPRINT(" Gpu base Address : %llu ",  pSmuInfo->m_gpuBaseAddress);
    DRVPRINT(" Counter Mask     : %llu ",  pSmuInfo->m_counterMask);
}

// PrintSmuList
//
// Printing Smu List
void PrintSmuList(const SmuList* pSmuList)
{
    int itr = 0;

    if (NULL != pSmuList)
    {
        DRVPRINT("Printing SmuList....");
        DRVPRINT("    Smu Count %d: ", pSmuList->m_count);

        if (0 != pSmuList->m_count)
        {
            DRVPRINT("Printing SmuInfo ...");

            while (itr < pSmuList->m_count)
            {
                DRVPRINT("For Smu : %d ", itr);
                PrintSmuInfo(&pSmuList->m_info[itr]);
                itr++;
            }
        }
    }
}

// PrintClientData
//
// Printing Client Data
void PrintClientData(const ClientData* pClientData)
{
    if (NULL != pClientData)
    {
        DRVPRINT(" Client Id                : %d ", pClientData->m_clientId);
        DRVPRINT(" Offline m_recCnt         : %d ", pClientData->m_isOffline);
        DRVPRINT(" Configure count          : %d ", pClientData->m_configCount);
        DRVPRINT(" Profile State            : %d ", pClientData->m_profileState);
        PrintPageBuffer(&pClientData->m_header);
        DRVPRINT(" OsClientCfg ");
        DRVPRINT("          Affinity        : %d ",  1);
        DRVPRINT("          Paused          : %d ", pClientData->m_osClientCfg.m_paused);
        DRVPRINT("          Stopped         : %d ", pClientData->m_osClientCfg.m_stopped);
        DRVPRINT("          Parent Id       : %d ", pClientData->m_osClientCfg.m_parentPid);
    }
}

// PrintCoreData
//
// Printing Core Data
void PrintCoreData(const CoreData* pCoreData)
{
    if (NULL != pCoreData)
    {
        DRVPRINT("Client ID : %d ", pCoreData->m_clientId);
        DRVPRINT("Sample ID : %d ", pCoreData->m_sampleId);
        DRVPRINT("Profile Type : %d ", pCoreData->m_profileType);
        DRVPRINT("Sampling Interval : %d ", pCoreData->m_samplingInterval);
        DRVPRINT("Record Length : %d ", pCoreData->m_recLen);
        DRVPRINT(" Core ID : %d ", pCoreData->m_coreId);
        DRVPRINT(" Process ID : %d ", pCoreData->m_contextData.m_processId);
        DRVPRINT(" Thread ID : %d ", pCoreData->m_contextData.m_threadId);
        DRVPRINT("TimeStamp : %llu ", pCoreData->m_contextData.m_timeStamp);
        PrintSmuList(pCoreData->m_smuCfg);
    }
}
#endif // DEBUG
