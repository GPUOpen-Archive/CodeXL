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

// PROJECT INCLUDES
#include <AMDTDriverTypedefs.h>
#include <AMDTPwrProfDriver.h>

// LOCAL INCLUDES
#include <AMDTHelpers.h>
#include <AMDTHwAccessInterface.h>
#include <AMDTPwrProfAttributes.h>
#include <AMDTPwrProfCoreUtils.h>
#include <AMDTPwrProfHwaccess.h>
#include <AMDTPwrProfSharedMemOps.h>
#include <AMDTPwrProfTimer.h>
#include <AMDTRawDataFileHeader.h>

// LOCAL DEFINES

// LOCAL VARIABLES
//
struct ClientList
{
    struct list_head list;
    unsigned long id;
    pid_t parent_pid;
    bool is_active;
};

// EXTERN FUNCTIONS
extern bool CheckParentPid(pid_t);
long CheckHwSupport(void);

// STATIC VARIABLES
// Minor build version for pcore
static unsigned int pcore_build_number = 003;
// Client id and count
static unsigned long client_id = 0L;
static unsigned int client_count = 0;
// Inialialise list
static struct ClientList clist =
{
    .list = LIST_HEAD_INIT(clist.list),
};

// LOCAL FUNCTIONS

// Verify if client is active
bool CheckClientActive(unsigned long id)
{
    struct ClientList* tmp;

    bool client_found = false;

    list_for_each_entry(tmp, &clist.list, list)
    {
        if (tmp->id == id)
        {
            DRVPRINT(" Client found and is %u \n ", tmp->is_active);
            client_found = true;

            if (!CheckParentPid(tmp->parent_pid))
            {
                tmp->is_active = false;
            }

            break;
        }
    }
    return client_found ? tmp->is_active : true;
}

// delete the client from the client list
void DeleteClient(unsigned long id)
{
    struct list_head* pos, *q;
    struct ClientList* tmp;
    list_for_each_safe(pos, q, &clist.list)
    {
        tmp = list_entry(pos, struct ClientList, list);

        if (tmp->id == id)
        {
            UnconfigureTimer(id);
            DeleteFromList(pos);
            FreeMemory(tmp);
            client_id--;
        }
    }
}

// Mark the client for cleanup.
void MarkClientForCleanup(unsigned long id)
{
    struct list_head* pos, *q;
    struct ClientList* tmp;
    DRVPRINT("Clean up called for client %lu \n", id);
    list_for_each_safe(pos, q, &clist.list)
    {
        tmp = list_entry(pos, struct ClientList, list);

        if (tmp->id == id)
        {
            tmp->is_active = false;
            DRVPRINT("Cleaning up client %lu parent pid is %d \n", id, tmp->parent_pid);
        }
    }
}

// IOCTL implementation for driver.
// mapping File opertaions for the pcore driver module.
long PwrProfDrvIoctlImpl(struct file* file, unsigned int ioctl_num, unsigned long ioctl_param)
{
    int retval                  = 0;
    unsigned long i             = 0;
    uint32 clientId             = 0;
    unsigned int tmp1           = 0;
    uint64_t version            = 0;
    uint64_t stop_client_id     = 0;
    int anon_inode_fd           = -1;
    unsigned long tmp_client_id = 0;
    struct ClientList* tmp      = NULL;

    PROFILER_PROPERTIES prof_props;
    PROF_CONFIGS prof_configs;
    ProfileConfig* prof_config;

    FILE_HEADER file_header;
    DATA_BUFFER data_buffer;

    ACCESS_PCI pci;
    ACCESS_MSR msr;
    uint32 pci_data;
    uint64 msr_data;
    ACCESS_MMIO mmio;

    DRVPRINT("device_ioctl: ioctl_num = %x\n", ioctl_num);

    switch (ioctl_num)
    {
        case IOCTL_GET_VERSION:
            DRVPRINT(" In get version Ioctl \n");
            version  = (unsigned long)LINUX_PWR_DRV_MAJOR << 32 | (unsigned int)LINUX_PWR_DRV_MINOR;
            DRVPRINT(" Power Profiler version is %llu \n ", version);

            retval = CopyToUser((unsigned long*) ioctl_param, &version, sizeof(unsigned long));
            return retval;

        case IOCTL_REGISTER_CLIENT:
            DRVPRINT("In register Client Ioctl \n");
            DRVPRINT("client_id is  %lu \n", client_id);

            if (client_id == MAX_CLIENT_COUNT)
            {
                // Check if the current client in active. If not delete the client and allow the register call
                tmp_client_id = client_id - 1;

                if (CheckClientActive(tmp_client_id))
                {
                    printk(KERN_INFO "Power Profiler: Max client id reached \n");
                    return -EACCES;
                }
                else
                {
                    printk(KERN_INFO "Power Profiler: Delete the inactive client %lu \n", tmp_client_id);
                    DeleteClient(tmp_client_id);
                }
            }

            tmp = AllocateMemory(sizeof(struct ClientList), GFP_KERNEL);

            if (!tmp)
            {
                printk(KERN_WARNING "Power Profiler: Memory Allocation failure for client %lu", client_id);
                return -ENOMEM;
            }

            tmp->id = client_id;
            tmp->parent_pid = current->tgid;
            tmp->is_active = true;

            DRVPRINT("Registering client %lu \n ", client_id);
            INIT_LIST_HEAD(&tmp->list);
            AddToList(&tmp->list, &(clist.list));

            retval = CopyToUser((unsigned long*) ioctl_param, &client_id, sizeof(unsigned long));
            client_id++;

            if (retval < 0)
            {
                client_count--;
                client_id--;
                printk(KERN_WARNING "Power Profiler: Unknown error  in register client retval %d\n", retval);
                return -1;

            }

            return retval;

        case IOCTL_UNREGISTER_CLIENT:
            retval = 0;
            DRVPRINT("In un register Client Ioctl \n");

            if (CopyFromUser(&tmp1, (uint32_t*)ioctl_param, sizeof(uint32_t)) == 0)
            {
                DRVPRINT("UnRegistering client %u \n ", tmp1);
            }
            else
            {
                printk(KERN_WARNING "Invalid parameter to Unregister Client \n");
                return -1;
            }

            DeleteClient(tmp1);

            return retval;

        case IOCTL_ADD_PROF_CONFIGS:
            retval = 0;
            /* Extract the configuration information */
            DRVPRINT("In Add Profile Configs Ioctl \n");

            if (CopyFromUser(&prof_configs, (PROF_CONFIGS*)ioctl_param, sizeof(PROF_CONFIGS)) == 0)
            {
                DRVPRINT("Adding %u profiles for client %u \n ", prof_configs.ulConfigCnt, prof_configs.ulClientId);
            }
            else
            {
                printk(KERN_WARNING "Invalid parameter to Add profile Config \n ");
                return -1;
            }

            /* Get the profiler config */
            clientId = prof_configs.ulClientId;
            prof_config = (ProfileConfig*)(prof_configs.uliProfileConfigs);

            for (i = 0; i < prof_configs.ulConfigCnt; i++, prof_config++)
            {
                retval = ConfigureTimer(prof_config, clientId);
            }

            return retval;

        case IOCTL_RESUME_PROFILER :
            retval = 0;
            DRVPRINT("In Resume Profiler Ioctl \n");

            if (CopyFromUser(&prof_props, (PROFILER_PROPERTIES*)ioctl_param, sizeof(PROFILER_PROPERTIES)) == 0)
            {
                DRVPRINT("Resuming Profile for client %u \n ", prof_props.ulClientId);
            }
            else
            {
                DRVPRINT(KERN_WARNING "Power Profiler: Invalid parameter to Resume Profiler\n ");
                return -1;
            }

            retval = ResumeTimer(prof_props.ulClientId);
            return retval;

        case IOCTL_START_PROFILER:
            retval = 0;
            DRVPRINT("In Start Profiler Ioctl \n");

            if (CopyFromUser(&prof_props, (PROFILER_PROPERTIES*)ioctl_param, sizeof(PROFILER_PROPERTIES)) == 0)
            {
                DRVPRINT("Starting Profile for client %u \n ", prof_props.ulClientId);
            }
            else
            {
                printk(KERN_WARNING "Power Profiler: Invalid parameter to Start Profiler\n ");
                return -1;
            }

            retval = StartTimer(prof_props.ulClientId);
            return retval;

        case IOCTL_PAUSE_PROFILER:
            retval = 0;
            printk("In Pause profiler ioctl \n");

            if (CopyFromUser(&stop_client_id, (uint32_t*)ioctl_param, sizeof(uint32_t)) == 0)
            {
                DRVPRINT("Pausing Profile for client %llu \n ", stop_client_id);
                tmp_client_id = (unsigned long)stop_client_id;
            }
            else
            {
                DRVPRINT(KERN_WARNING "Power Profiler: Invalid parameter to Pause Profiler\n");
                return -1;
            }

            retval = PauseTimer(tmp_client_id);
            return retval;

        case IOCTL_STOP_PROFILER:
            DRVPRINT("In stop profiler ioctl \n");
            DRVPRINT("STOP PROF client_id is  %lu \n", client_id);
            retval = 0;

            if (CopyFromUser(&stop_client_id, (uint32_t*)ioctl_param, sizeof(uint32_t)) == 0)
            {
                DRVPRINT("Stopping Profile for client %llu \n ", stop_client_id);
                tmp_client_id = (unsigned long)stop_client_id;
            }
            else
            {
                printk(KERN_WARNING "Power Profiler: Invalid parameter to Stop Profiler\n");
                return -1;
            }

            retval = StopTimer(tmp_client_id);
            return retval;

        case IOCTL_GET_FILE_HEADER_BUFFER:
            DRVPRINT("In get file header ioctl \n");

            if (CopyFromUser(&file_header, (FILE_HEADER*)ioctl_param, sizeof(FILE_HEADER)) == 0)
            {
                DRVPRINT("Get File header for client %u \n ", file_header.ulClientId);
            }

            retval = GetHeaderBuffer(&file_header);
            DRVPRINT("get header buffer ret %d \n", retval);

            return retval;

        case IOCTL_GET_DATA_BUFFER:
            DRVPRINT("In get data buffer ioctl \n");

            if (CopyFromUser(&data_buffer, (DATA_BUFFER*)ioctl_param, sizeof(DATA_BUFFER)) == 0)
            {
                DRVPRINT("Get  %u data buffers client %u \n ", data_buffer.ulavailableBuffCnt, data_buffer.ulClientId);
            }

            retval = GetDataBuffer(&data_buffer);
            DRVPRINT(" Avaliable Buffer Count %u \n", data_buffer.ulavailableBuffCnt);

            if (CopyToUser((DATA_BUFFER*)ioctl_param , &data_buffer , sizeof(DATA_BUFFER)))
            {
                DRVPRINT(KERN_WARNING "Power Profiler: Error in get data buffer \n");
                retval = -EACCES;
            }

            return retval;

        case IOCTL_ACCESS_PCI_DEVICE:
            retval = 0;
            DRVPRINT("In Access PCI ioctl\n");

            if (CopyFromUser(&pci, (ACCESS_PCI*)ioctl_param, sizeof(ACCESS_PCI)) == 0)
            {
                DRVPRINT(" PCI ACCESS address %u \n ", pci.address);
            }
            else
            {
                printk(KERN_WARNING "Power Profiler: Unknown Error in ACCESS_PCI \n");
                retval = -EACCES;
                return retval;
            }

            if (pci.isReadAccess)
            {
                ReadPCI(pci.address, &pci_data);
                pci.data = (uint32)pci_data;
            }
            else
            {
                WritePCI(pci.address, pci.data);
            }

            if (CopyToUser((ACCESS_PCI*)ioctl_param, &pci , sizeof(ACCESS_PCI)) != 0)
            {
                printk(KERN_WARNING "Power Profiler: unknown error \n");
                retval = -EACCES;
            }

            return retval;

        case IOCTL_ACCESS_MSR:
            retval = 0;
            DRVPRINT("In Access MSR ioctl\n");

            if (CopyFromUser(&msr, (ACCESS_MSR*)ioctl_param, sizeof(ACCESS_MSR)) == 0)
            {
                DRVPRINT(" MSR_ACCESS  register %u, data %u \n ", msr.regId, msr.data);
            }
            else
            {
                printk("WARNING: Unknown Error in ACCESS_MSR \n");
                retval = -EACCES;
                return retval;
            }

            if (msr.isReadAccess)
            {
                ReadMSR((uint32)msr.regId, &msr_data);
                msr.data = (uint32)msr_data;
            }
            else
            {
                WriteMSR((uint32)msr.regId, msr.data);
            }

            if (CopyToUser((ACCESS_MSR*)ioctl_param, &msr , sizeof(ACCESS_MSR)) != 0)
            {
                printk(KERN_WARNING "Power Profiler: unknown error \n");
                retval = -EACCES;
            }

            return retval;

        case IOCTL_ACCESS_MMIO:
            retval = 0;
            DRVPRINT("In Access MMIO ioctl\n");

            if (CopyFromUser(&mmio, (ACCESS_MMIO*)ioctl_param, sizeof(ACCESS_MMIO)) == 0)
            {
                DRVPRINT(" ACCESS_MMIO  add 0x%llX, data 0x%x \n ", mmio.m_addr, mmio.m_data);
            }
            else
            {
                printk("WARNING: Unknown Error in ACCESS_MMIO \n");
                retval = -EACCES;
                return retval;
            }

            if (false == AccessMMIO(&mmio))
            {
                printk("ERROR: Error in ACCESS_MMIO \n");
                retval = -EACCES;
                return retval;
            }

            if (CopyToUser((ACCESS_MMIO*)ioctl_param, &mmio , sizeof(ACCESS_MMIO)) != 0)
            {
                printk(KERN_WARNING "Power Profiler: unknown error \n");
                retval = -EACCES;
            }

            return retval;

        case IOCTL_SET_AND_GET_FD:
            retval = 0;
            DRVPRINT("In IOCTL_SET_AND_GET_FD ioctl\n");
            anon_inode_fd = CreateAnonInodeFd();

            if (CopyToUser((int*)ioctl_param, &anon_inode_fd, sizeof(int)))
            {
                printk(KERN_WARNING "Copy to User Failed for IOCTL_SET_AND_GET_FD \n");
                retval = -EACCES;
            }

            return retval;

        default:
            printk(KERN_WARNING "Power Profiler: Unknown IOCTL \n");
            retval = -1;
            return retval;
    }
}

// Module specific cleanup
void PwrProfDrvCleanup(void)
{
    // Lets ensure that we delete all the timers and configs
    struct ClientList* tmp, *t;
    list_for_each_entry_safe(tmp, t, &clist.list, list)
    {
        DRVPRINT("Unregistering %lu-->", tmp->id);
        UnconfigureTimer(tmp->id);
        DeleteFromList(&tmp->list);
        kfree(tmp);
    }
}

// check if hardware supported.
long CheckPwrProfHwSupport(void)
{
    return CheckHwSupport();
}

// get the driver version.
void GetVersions(unsigned int* major, unsigned int* minor, unsigned int* build)
{
    *major = LINUX_PWR_DRV_MAJOR;
    *minor = LINUX_PWR_DRV_MINOR;
    *build = pcore_build_number;
}
