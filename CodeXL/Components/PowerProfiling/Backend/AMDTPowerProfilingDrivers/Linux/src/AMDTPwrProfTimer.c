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

// System headers
#include <linux/hrtimer.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/atomic.h>
#include <linux/cpu.h>
#include <linux/poll.h>

// Project headers
#include <AMDTCommonConfig.h>
#include <AMDTDriverTypedefs.h>
#include <AMDTHelpers.h>
#include <AMDTPwrProfAttributes.h>
#include <AMDTPwrProfCoreUtils.h>
#include <AMDTPwrProfInternal.h>
#include <AMDTPwrProfTimer.h>
#include <AMDTPwrProfTimerHelper.h>
#include <AMDTSmu7Interface.h>
#include <AMDTSmu8Interface.h>

// Global core config list
CoreData* g_pCoreCfg = NULL;

// Define per-cpu client-data
DEFINE_PER_CPU(CoreData, g_coreClientData);

// Extern definations
extern struct task_struct init_task;
extern int moduleState;

// Extern functions
extern void MarkClientForCleanup(unsigned long);
extern void InitializeGenericCounterAccess(uint32 core);
extern wait_queue_head_t work_queue;

// Global signal handler
atomic_t g_signal;
uint8* pSharedBuffer = NULL;

// Local structure
typedef struct ClientList
{
    struct list_head    list;
    ClientData*         m_pClientData;
} ClientList;

static ClientList tlist =
{
    .list = LIST_HEAD_INIT(tlist.list),
};

// CallInitCef: Core effective frequency initialization
void CallInitCef(void* data)
{
    InitializeGenericCounterAccess(*(uint32*)data);
}

// CheckParentPid
//
// Check if the the process started timer is still.
//
bool CheckParentPid(pid_t parentPid)
{
    struct timespec tstamp1;
    struct timespec tstamp2;
    struct task_struct* pParent = NULL;
    struct task_struct* pTemp = NULL;
    struct pid* pspid = NULL;
    bool ret = false;

    // Before we re add the timer again, lets double check to see if the task that has
    // started the timer is alive. If not, dont readd the timer and trigger a delete
    getnstimeofday(&tstamp1);
    rcu_read_lock();
    pParent = &init_task;               // start at init

    do
    {
        if (pParent->pid == parentPid)
        {
            // does the pid (not tgid) match?
            pTemp = pParent;
            break;
        }

        pParent = next_task(pParent);   // this isn't the task you're looking for
    }
    while (pParent != &init_task);      // stop when we get back to init

    getnstimeofday(&tstamp2);

    if (pTemp != NULL)
    {
        DRVPRINT("time taken for finding the parent task %lu ", tstamp2.tv_nsec - tstamp1.tv_nsec);
        pspid = pTemp->pids[PIDTYPE_PID].pid;

        if (pspid == NULL)
        {
            printk(KERN_WARNING "Power Profiler: PID %d seems to be dead. Cleaning up power profiler.\n", parentPid);
            ret = false;
        }
        else
        {
            DRVPRINT("found the parent task for pid %d ", parentPid);
            ret = true;
        }
    }
    else
    {
        printk(KERN_WARNING "Power Profiler: Could not locate task for PID  %d. Cleaning up power profiler. \n", parentPid);
        ret = false;
    }

    rcu_read_unlock();
    return ret;
}

// SampleDataCallback
//
// Timer Callback function invoked when the HR timer expires
//
int SampleDataCallback(ClientData* pClientData)
{
    int errorCode = 0;
    int cpu = 0;
    struct task_struct* currentTask = NULL;
    CoreData* pCoreClientData = NULL;
    uint32 interval;
    bool flag = false;
    int counter = 0;

    ATOMIC_SET(&g_signal, 0);

    if (NULL != pClientData)
    {
        if (CheckParentPid(pClientData->m_osClientCfg.m_parentPid))
        {
            if (!(pClientData->m_osClientCfg.m_paused || pClientData->m_osClientCfg.m_stopped))
            {
                currentTask = GetCurrentTask();

                cpu = get_cpu();
                put_cpu();

                pCoreClientData = &per_cpu(g_coreClientData, cpu);

                // set context info for each core
                pCoreClientData->m_contextData.m_processId  = currentTask->tgid;
                pCoreClientData->m_contextData.m_threadId   = currentTask->pid;
                pCoreClientData->m_contextData.m_timeStamp  = ktime_to_ns(ktime_get());
                pCoreClientData->m_coreId                   = cpu;

                WriteSampleData(pCoreClientData);

                // Trigger event to client to inform the data availability
                if (1 == pCoreClientData->m_sampleId)
                {
                    if (PROFILE_TYPE_PROCESS_PROFILING == pCoreClientData->m_profileType)
                    {
                        if (pCoreClientData->m_samplingInterval < 100)
                        {
                            counter = pCoreClientData->m_pCoreBuffer->m_recCnt % (pCoreClientData->m_samplingInterval);

                            if (counter == 0)
                            {
                                flag = true;
                            }
                        }
                        else
                        {
                            counter = pCoreClientData->m_pCoreBuffer->m_recCnt % 100;

                            if (counter == 0)
                            {
                                flag = true;
                            }
                        }
                    }
                    else
                    {
                        if (pCoreClientData->m_samplingInterval < 100)
                        {
                            interval = 100 / pCoreClientData->m_samplingInterval;
                            counter = (pCoreClientData->m_pCoreBuffer->m_recCnt % interval);

                            if (0 == counter)
                            {
                                flag = true;
                            }
                        }
                        else
                        {
                            flag = true;
                        }
                    }

                    if (true == flag)
                    {
                        ATOMIC_SET(&g_signal, 1);
                        wake_up_interruptible(&work_queue);
                    }
                }

            }
            else
            {
                printk(" %s timer for client %d m_stopped or m_paused \n", __FUNCTION__, pClientData->m_clientId);
            }
        }
        else
        {
            MarkClientForCleanup(pCoreClientData->m_clientId);
            errorCode = -1;
        }
    }

    return errorCode;
}

// StartHrTimer
//
// Start/Re-start HR timer
//
void StartHrTimer(void)
{
    int cpu = 0;
    CoreData* pCoreClientData = NULL;

    // get current core id
    cpu = get_cpu();
    put_cpu();

    // get the CoreData Object
    pCoreClientData = &per_cpu(g_coreClientData, cpu);

    if (NULL != pCoreClientData)
    {
        // start hr timer
        hrtimer_start(&pCoreClientData->m_pOsData->m_hrTimer,
                      pCoreClientData->m_pOsData->m_interval,
                      HRTIMER_MODE_REL_PINNED);

    }
}

// StartTimer
//
// Invoked when profiling starts, add a timer for
// sampling time.
//
int StartTimer(uint32 clientId)
{
    struct list_head* pCurrent = NULL;
    ClientList* pClients = NULL;
    int ret = 0;
    int cpu = 0;

    DRVPRINT("Starting Timer for client id : %d ", clientId);
    list_for_each(pCurrent, &tlist.list)
    {
        pClients = list_entry(pCurrent, ClientList, list);

        DRVPRINT(" Check client %d ", pClients->m_pClientData->m_clientId);

        if ((pClients->m_pClientData->m_clientId == clientId))
        {
            pClients->m_pClientData->m_osClientCfg.m_paused    = false;
            pClients->m_pClientData->m_osClientCfg.m_stopped   = false;

            cpu = get_cpu();
            put_cpu();

            if (cpumask_test_cpu(cpu, &pClients->m_pClientData->m_osClientCfg.m_affinity))
            {
                StartHrTimer();
            }

            preempt_disable();

            smp_call_function_many(&pClients->m_pClientData->m_osClientCfg.m_affinity,
                                   (void*)StartHrTimer,
                                   (void*)NULL,
                                   true); // blocking call

            preempt_enable();

            ret = UpdateBufferHeader(pClients->m_pClientData, 0);
        }
    }

    // module is in use, set state to 1
    moduleState = 1;

    return ret;
}

// ResumeTimer
//
// Resume the timer to collect data
//
int ResumeTimer(uint32 clientId)
{
    struct list_head* pCurrent = NULL;
    ClientList* pClients = NULL;
    int ret = 0;
    int cpu = 0;

    DRVPRINT("Resuming the timers for client %d ", clientId);
    list_for_each(pCurrent, &tlist.list)
    {
        pClients = list_entry(pCurrent, ClientList, list);

        if (NULL != pClients->m_pClientData)
        {
            DRVPRINT("Check client %d ", pClients->m_pClientData->m_clientId);

            if ((pClients->m_pClientData->m_clientId == clientId))
            {
                // Fire the timer to start from
                DRVPRINT("Starting the timer to start in msec ");
                pClients->m_pClientData->m_osClientCfg.m_paused = false;

                cpu = get_cpu();
                put_cpu();

                if (cpumask_test_cpu(cpu, &pClients->m_pClientData->m_osClientCfg.m_affinity))
                {
                    StartHrTimer();
                }

                preempt_disable();

                smp_call_function_many(&pClients->m_pClientData->m_osClientCfg.m_affinity,
                                       (void*)StartHrTimer,
                                       (void*)NULL,
                                       true); // blocking call

                preempt_enable();
            }
        }
        else
        {
            ret = -1;
        }
    }

    return ret;
}

// GetHeaderBuffer
//
// Get header buffer for the kernel space
// convert it into user space
//
int GetHeaderBuffer(PFILE_HEADER fileHeader)
{
    struct list_head* pCurrent = NULL;
    ClientList* pClients = NULL;
    uint8* tempData = NULL;
    int ret = 0;

    DRVPRINT(" Get File Header for client %u ", fileHeader->ulClientId);
    list_for_each(pCurrent, &tlist.list)
    {
        pClients = list_entry(pCurrent, ClientList, list);

        DRVPRINT("Client id %d", pClients->m_pClientData->m_clientId);

        if (pClients->m_pClientData->m_clientId == fileHeader->ulClientId)
        {
            tempData = (unsigned char*)fileHeader->uliBuffer;

            if (NULL == pClients->m_pClientData->m_header.m_pBuffer)
            {
                DRVPRINT(KERN_ERR "Power Profiler Timer:get header buffer , timer buffer NULL ");
                return EFAULT;
            }

            if (NULL == tempData)
            {
                DRVPRINT(KERN_ERR "Power Profiler Timer:get header , user buffer NULL ");
                return EFAULT;
            }

            if (CopyToUser(tempData, pClients->m_pClientData->m_header.m_pBuffer, HEADER_BUFFER_SIZE))
            {
                DRVPRINT(KERN_ERR "Power Profiler Timer:Error in get file header ");
                ret = -EFAULT;
            }
        }
    }
    return ret;
}

// GetDataBuffer
//
// Get data buffer for the counter
//
int GetDataBuffer(PDATA_BUFFER data_buffer)
{
    struct list_head* pCurrent = NULL;
    ClientList* pClients = NULL;
    RawBufferInfo* pBufferList = NULL;
    RawBufferInfo* pBuff = NULL;
    uint8* pBuffer = NULL;
    ClientData* pClientData = NULL;
    CoreData* pCoreClientData = NULL;
    int ret = 0;
    int currentOffset = 0;
    int consumedOffset = 0;
    uint32_t cnt = 0;
    uint32 coreId = 0;

    DRVPRINT(" Get data buffer for client %u ", data_buffer->ulClientId);

    // Check if SMU access status is OK else return error
    if (false == GetSmuAccessState())
    {
        data_buffer->ulStatus = PROF_ERROR_SMU_ACCESS_FAILED;
    }
    else
    {
        list_for_each(pCurrent, &tlist.list)
        {
            pClients = list_entry(pCurrent, ClientList, list);

            pClientData = pClients->m_pClientData;

            if (NULL != pClientData && pClientData->m_clientId == data_buffer->ulClientId)
            {
                pBufferList = (RawBufferInfo*)(data_buffer->uliBuffer);

                for_each_cpu(coreId, &pClientData->m_osClientCfg.m_affinity)
                {
                    pCoreClientData = &per_cpu(g_coreClientData, coreId);
                    pBuff = (RawBufferInfo*)(pBufferList + cnt);

                    if (NULL == pBuff)
                    {
                        DRVPRINT(KERN_ERR "Power Profiler Timer:Null buff ");
                        return -EFAULT;
                    }

                    currentOffset = atomic_read(&pCoreClientData->m_pCoreBuffer->m_currentOffset);
                    consumedOffset = atomic_read(&pCoreClientData->m_pCoreBuffer->m_consumedOffset);

                    pBuff->ulvalidLength = (unsigned long)(currentOffset - consumedOffset);
                    pBuff->ulvalidLength = pBuff->ulvalidLength > DATA_PAGE_BUFFER_SIZE ?
                                           DATA_PAGE_BUFFER_SIZE : pBuff->ulvalidLength;

                    pBuffer = (uint8*)pBuff->uliBuffer.QuadPart;
                    DRVPRINT(" copy data buffer from %u to %u ", consumedOffset, consumedOffset + pBuff->ulvalidLength);

                    memcpy(pBuffer, &pCoreClientData->m_pCoreBuffer->m_pBuffer[consumedOffset], pBuff->ulvalidLength);
                    atomic_set(&pCoreClientData->m_pCoreBuffer->m_consumedOffset, currentOffset);

                    cnt++;
                }

                data_buffer->ulavailableBuffCnt = pClientData->m_configCount;
                DRVPRINT(" Avaliable Buffer Count %u ", data_buffer->ulavailableBuffCnt);
                data_buffer->ulStatus = PROF_SUCCESS;
            }
        }
    }

    DRVPRINT("Returning value from get buffer %d", ret);
    return ret;
}

// StopTimer
//
// Stop timer for the specified client id
// release the memory allocated for timer
//
int StopTimer(uint32 clientId)
{
    struct list_head* pCurrent = NULL;
    ClientList* pClients = NULL;
    bool m_stopped = false;

    DRVPRINT(" Stopping the timers for client %d ", clientId);

    if (!list_empty(&tlist.list))
    {
        list_for_each(pCurrent, &tlist.list)
        {
            pClients = list_entry(pCurrent, ClientList, list);

            DRVPRINT("pClients->m_pClientData->client_id %d, clientId %d ", pClients->m_pClientData->m_clientId, clientId);

            if (pClients->m_pClientData->m_clientId == clientId)
            {
                pClients->m_pClientData->m_osClientCfg.m_stopped = true;
                m_stopped = true;
            }
        }

        // Release memory pool
        ReleaseMemoryPool(&g_sessionPool);
    }
    else
    {
        printk(KERN_ERR "Power Profiler Timer: empty timerlist for client %d \n", clientId);
    }

    if (false == m_stopped)
    {
        printk(KERN_ERR "Power Profiler Timer:Could not stop profiler for client %d \n", clientId);
    }

    // m_stopped profiling, set state to 0
    moduleState = 0;

    return m_stopped ? 0 : -1;
}

// PauseTimer
//
// Set the pause flag to true to probhit collecting data.
//
int PauseTimer(uint32 clientId)
{
    struct list_head* pCurrent = NULL;
    ClientList* pClients = NULL;
    bool m_paused = false;

    DRVPRINT(" Pausing the timers for client %d ", clientId);

    if (!list_empty(&tlist.list))
    {
        list_for_each(pCurrent, &tlist.list)
        {
            pClients = list_entry(pCurrent, ClientList, list);

            if (NULL != pClients->m_pClientData)
            {
                DRVPRINT("pClients->m_pClientData->m_clientId %d, clientId %d ", pClients->m_pClientData->m_clientId, clientId);

                if (pClients->m_pClientData->m_clientId == clientId)
                {
                    pClients->m_pClientData->m_osClientCfg.m_paused = true;
                    m_paused = true;
                }
            }
        }
    }
    else
    {
        DRVPRINT(KERN_ERR "Power Profiler Timer: empty timerlist for client %d ", clientId);
    }

    if (false == m_paused)
    {
        DRVPRINT(KERN_ERR "Power Profiler Timer:Could not pause profiler for client %d ", clientId);
    }

    return m_paused ? 0 : -1;
}

// Preparem_affinityMask
//
// Set the cpu m_affinity mask for the specified core.
//
void Preparem_affinityMask(cpumask_t* pm_affinity, uint64_t mask)
{
    uint64_t coreId = 0;
    uint64_t coreMask = mask;

    while (coreMask)
    {
        if (coreMask & 0x01)
        {
            CpuMaskSetCpu(coreId, pm_affinity);
        }

        coreMask = coreMask >> 1;
        coreId++;
    }
}

// HrTimerCallback
//
// Timer functon call back, triggered each time
// when HR timer expires
//
enum hrtimer_restart HrTimerCallback(struct hrtimer* timer_for_restart)
{
    CoreData* pCoreClientData = NULL;
    struct list_head* pCurrent = NULL;
    ClientList* pClients = NULL;
    int errorCode = 0;
    ktime_t currtime;
    enum hrtimer_restart ret = HRTIMER_NORESTART;
    const unsigned long ms = 1000000;                   // for process profiling setting sampling time to 1 ms
    ktime_t processProfDur = ktime_set(0, ms);
    int cpu = 0;

    // get core id
    cpu = get_cpu();
    put_cpu();

    DRVPRINT("Callback (%d)", cpu);

    // get the CORE_CLIENT_OBJECT
    pCoreClientData = &per_cpu(g_coreClientData, cpu);

    if (!list_empty(&tlist.list))
    {
        list_for_each(pCurrent, &tlist.list)
        {
            pClients = list_entry(pCurrent, ClientList, list);

            if (NULL != pClients->m_pClientData
                && pCoreClientData->m_clientId == pClients->m_pClientData->m_clientId
                && false == pClients->m_pClientData->m_osClientCfg.m_paused)
            {
                currtime = ktime_get();
                errorCode = SampleDataCallback(pClients->m_pClientData);

                if (!errorCode)
                {
                    if (PROFILE_TYPE_TIMELINE == (ProfileType)pCoreClientData->m_profileType)
                    {
                        hrtimer_forward_now(timer_for_restart, pCoreClientData->m_pOsData->m_interval);
                    }
                    else
                    {
                        // for Process profiling sampling interval will be 1ms
                        hrtimer_forward_now(timer_for_restart, processProfDur);
                    }

                    ret = HRTIMER_RESTART;
                }
            }
        }
    }

    return ret;
}

// InitHrTimer
//
// Initiailize HR timer
//
void InitHrTimer(void)
{
    int cpu;
    CoreData* pCoreClientData = NULL;

    // get current core id
    cpu = get_cpu();
    put_cpu();

    // get the CORE_CLIENT_OBJECT
    pCoreClientData = &per_cpu(g_coreClientData, cpu);

    // initialize HR timer
    hrtimer_init(&pCoreClientData->m_pOsData->m_hrTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED);
    pCoreClientData->m_pOsData->m_hrTimer.function = &HrTimerCallback;

    return;
} // InitHrTimer


// ConfigureTimer
//
// Configure the timer with given sampling interval.
//
int ConfigureTimer(ProfileConfig* config, uint32 clientId)
{
    cpumask_t cpuAffinity;
    ClientData* pClientData     = NULL;
    ClientList* pTempNode       = NULL;
    CoreData* pCoreData         = NULL;
    CoreData* pCoreClientData   = NULL;
    int errRet                  = 0;
    int cpu                     = 0;
    uint32 configuredCoreCount  = 0;
    uint32 coreId               = 0;
    int cpuCnt                  = 0;
    bool isFirstConfig          = true;
    int configIdx               = 0;
    int ret                     = -1;
    PwrInternalAddr internalCounter;

    DRVPRINT("Configuring Timer for client %d\n", clientId);
    memset(&internalCounter, 0, sizeof(internalCounter));

    // Prepare cpu m_affinity mask structure for the configured core mask
    cpumask_clear(&cpuAffinity);
    Preparem_affinityMask(&cpuAffinity, config->m_samplingSpec.m_mask);

    // Create memory pool for this session
    if (false == CreateMemoryPool(&g_sessionPool, SESSION_POOL_SIZE))
    {
        printk(KERN_WARNING "Power Profiler: CreateMemoryPool failed\n");
        return -ENOMEM;
    }

    if (AllocateAndInitClientData(&pClientData, clientId, cpuAffinity, config) != 0)
    {
        return -ENOMEM;
    }

    configuredCoreCount = config->m_samplingSpec.m_maskCnt;
    cpuCnt = num_online_cpus();

    DRVPRINT("Configured Core Count %d and total Number of CPUs : %d ",
             configuredCoreCount, cpuCnt);

    if (NULL != g_pCoreCfg)
    {
        kfree(g_pCoreCfg);
        g_pCoreCfg = NULL;
    }

    g_pCoreCfg = kmalloc(cpuCnt * sizeof(CoreData), GFP_KERNEL);

    if (NULL == g_pCoreCfg)
    {
        printk(KERN_WARNING "Power Profiler: Memory Allocation for Global configuration failed \n");
        return -ENOMEM;
    }

    // setting pointer to global config, to be used by RawFileHeader.c
    pCoreData = g_pCoreCfg;

    for_each_cpu(coreId, &pClientData->m_osClientCfg.m_affinity)
    {
        DRVPRINT("Configuring for Core: %d ", coreId);

        if (NULL != pSharedBuffer)
        {
            memcpy(&internalCounter, &pSharedBuffer[PWR_INTERNAL_COUNTER_BASE], sizeof(PwrInternalAddr));
            DRVPRINT("CSTATE 0x%x SVI2 0x%x SVI2NB 0x%x", internalCounter.m_cstateRes, internalCounter.m_sviTelemetry, internalCounter.m_sviNBTelemetry);
        }

        // Get the core specific CORE_CLIENT_DATA for this configured coreId
        pCoreClientData = &per_cpu(g_coreClientData, coreId);

        ret = IntializeCoreData(pCoreClientData, configIdx, &isFirstConfig,
                                clientId, coreId, config);

        if (0 != ret)
        {
            // free client data
            printk("Power Profiler: Failed to Intiailise Core data for Core : %d \n", coreId);
            FreeClientData(pClientData);
            return -ENOMEM;
        }

        memcpy(&pCoreClientData->m_internalCounter, &internalCounter, sizeof(PwrInternalAddr));

        GetRequiredBufferLength(pCoreClientData, &pCoreClientData->m_recLen);
        pCoreData = pCoreClientData;
        ++pCoreData;

        configIdx++;
    }

    cpu = get_cpu();
    put_cpu();

    if (cpumask_test_cpu(cpu, &pClientData->m_osClientCfg.m_affinity))
    {
        InitHrTimer();
    }

    preempt_disable();
    smp_call_function_many(&pClientData->m_osClientCfg.m_affinity,
                           (void*)InitHrTimer,
                           (void*)NULL,
                           false); // non blocking call
    preempt_enable();

    // add to global list of timers data
    pTempNode = kmalloc(sizeof(ClientList), GFP_KERNEL);

    if (NULL != pTempNode)
    {
        pTempNode->m_pClientData = pClientData;
        AddToList(&pTempNode->list, &tlist.list);
    }

    // Write header data buffer for the file
    errRet = WriteHeader(pClientData, config);
    DRVPRINT("Return value from WriteHeader = %d\n", errRet);

    cpu = get_cpu();
    put_cpu();

    // Reset The CEF counters to 0 before we start sampling
    // For the current cpu
    CallInitCef((void*)&cpu);

    // for the remaining all cpus
    preempt_disable();
    smp_call_function_many(&pClientData->m_osClientCfg.m_affinity,
                           (void*)CallInitCef,
                           (void*)&cpu,
                           true);
    preempt_enable();

    DRVPRINT("Exiting Configure Timer with success ");
    return 0;
} // ConfigureTimer

// UnconfigureTimer
//
// Unconfigure timer, delete it from timer list
//
int UnconfigureTimer(uint32 clientId)
{
    struct list_head* pCurrent = NULL;
    struct list_head* pTemp = NULL;
    ClientList* pClients       = NULL;
    ClientData* pClientData     = NULL;
    CoreData* pCoreClientData   = NULL;
    int cpu;

    list_for_each_safe(pCurrent, pTemp, &tlist.list)
    {
        pClients = list_entry(pCurrent, ClientList , list);
        pClientData = (ClientData*)pClients->m_pClientData;

        if ((NULL != pClientData) && (pClientData->m_clientId == clientId))
        {
            DRVPRINT(" Deleting timer for client id %d ", clientId);

            for_each_cpu(cpu, &pClientData->m_osClientCfg.m_affinity)
            {
                pCoreClientData = &per_cpu(g_coreClientData, cpu);

                if (NULL != pCoreClientData)
                {
                    hrtimer_cancel(&pCoreClientData->m_pOsData->m_hrTimer);
                }

                FreeCoreData(pCoreClientData);

                memset(pCoreClientData, 0, sizeof(CoreData));
            }

            pClientData->m_osClientCfg.m_stopped = true;
            pClientData->m_osClientCfg.m_paused = true;

            // Release smu configuration
            ConfigureSmu(pCoreClientData->m_smuCfg, false);

            FreeClientData(pClientData);

            DeleteFromList(pCurrent);
            kfree(pClients);

            if (NULL != g_pCoreCfg)
            {
                kfree(g_pCoreCfg);
                g_pCoreCfg = NULL;
            }
        }
    }

    return 0;
}

