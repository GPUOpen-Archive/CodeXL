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

// LOCAL VARIABLES
//

// EXTERN FUNCTIONS
long CheckHwSupport(void);

// STATIC VARIABLES
// Minor build version for pcore
static unsigned int pcore_build_number = 001;

// Client id and state
static unsigned long g_clientId = 0L;
static bool g_isClientActiv = false;

// LOCAL FUNCTIONS

// delete the client from the client list
void DeleteClient(void)
{
    g_isClientActiv = false;
    g_clientId = 0;
    UnconfigureTimer(g_clientId);
}

// Mark the client for cleanup.
void MarkClientForCleanup(unsigned long id)
{
    if (id == g_clientId)
    {
        g_isClientActiv = false;
        printk("Cleaning up client %lu ", id);
    }
    else
    {
        printk("Power Profiler: Incorrect Client id. \n");
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

    PROFILER_PROPERTIES prof_props;
    PROF_CONFIGS prof_configs;
    ProfileConfig prof_config;

    FILE_HEADER file_header;
    DATA_BUFFER data_buffer;

    ACCESS_PCI pci;
    ACCESS_MSR msr;
    uint32 pci_data;
    uint64 msr_data;
    ACCESS_MMIO mmio;

    DRVPRINT("device_ioctl: ioctl_num = %x", ioctl_num);

    switch (ioctl_num)
    {
        case IOCTL_GET_VERSION:
            DRVPRINT(" In get version Ioctl");
            version  = (unsigned long)LINUX_PWR_DRV_MAJOR << 32 | (unsigned int)LINUX_PWR_DRV_MINOR;
            DRVPRINT(" Power Profiler version is %llu ", version);

            retval = CopyToUser((unsigned long*) ioctl_param, &version, sizeof(unsigned long));
            return retval;

        case IOCTL_REGISTER_CLIENT:
            DRVPRINT("In register Client Ioctl");

            if (g_isClientActiv)
            {
                DRVPRINT(KERN_INFO "Power Profiler: Max one instance of Power Profiler client run permitted. ");
                return -EACCES;
            }

            g_isClientActiv = true;
            retval = CopyToUser((unsigned long*) ioctl_param, &g_clientId, sizeof(unsigned long));

            if (retval < 0)
            {
                g_isClientActiv = false;
		g_clientId = 0;
                DRVPRINT(KERN_WARNING "Power Profiler: Unknown error  in register client retval %d", retval);
                return -1;
            }

            return retval;

        case IOCTL_UNREGISTER_CLIENT:
            retval = 0;
            DRVPRINT("In un register Client Ioctl");

            if (CopyFromUser(&tmp1, (uint32_t*)ioctl_param, sizeof(uint32_t)) == 0)
            {
                DRVPRINT("UnRegistering client %u", tmp1);
            }
            else
            {
                DRVPRINT(KERN_WARNING "Invalid parameter to Unregister Client");
                return -1;
            }

            DeleteClient();

            return retval;

        case IOCTL_ADD_PROF_CONFIGS:
            retval = 0;
            /* Extract the configuration information */
            DRVPRINT("In Add Profile Configs Ioctl");

            if (CopyFromUser(&prof_configs, (PROF_CONFIGS*)ioctl_param, sizeof(PROF_CONFIGS)) == 0)
            {
                DRVPRINT("Adding %u profiles for client %u", prof_configs.ulConfigCnt, prof_configs.ulClientId);
            }
            else
            {
                DRVPRINT(KERN_WARNING "Invalid parameter to Add profile Config");
                return -1;
            }

            /* Get the profiler config */
            clientId = prof_configs.ulClientId;

            for (i = 0; i < prof_configs.ulConfigCnt; i++)
            {
                if (CopyFromUser(&prof_config, (ProfileConfig*)(prof_configs.uliProfileConfigs) + i, sizeof(ProfileConfig)) == 0)
                {
                    retval = ConfigureTimer(&prof_config, clientId);
                }
                else
                {
                    DRVPRINT(KERN_WARNING "Failed copying parameter to Add profile Config");
                    return -1;
                }
            }

            return retval;

        case IOCTL_RESUME_PROFILER :
            retval = 0;
            DRVPRINT("In Resume Profiler Ioctl");

            if (CopyFromUser(&prof_props, (PROFILER_PROPERTIES*)ioctl_param, sizeof(PROFILER_PROPERTIES)) == 0)
            {
                DRVPRINT("Resuming Profile for client %u", prof_props.ulClientId);
            }
            else
            {
                DRVPRINT(KERN_WARNING "Power Profiler: Invalid parameter to Resume Profiler");
                return -1;
            }

            retval = ResumeTimer(prof_props.ulClientId);
            return retval;

        case IOCTL_START_PROFILER:
            retval = 0;
            DRVPRINT("In Start Profiler Ioctl");

            if (CopyFromUser(&prof_props, (PROFILER_PROPERTIES*)ioctl_param, sizeof(PROFILER_PROPERTIES)) == 0)
            {
                DRVPRINT("Starting Profile for client %u", prof_props.ulClientId);
            }
            else
            {
                DRVPRINT(KERN_WARNING "Power Profiler: Invalid parameter to Start Profiler");
                return -1;
            }

            retval = StartTimer(prof_props.ulClientId);
            return retval;

        case IOCTL_PAUSE_PROFILER:
            retval = 0;
            DRVPRINT("In Pause profiler ioctl");

            if (CopyFromUser(&stop_client_id, (uint32_t*)ioctl_param, sizeof(uint32_t)) == 0)
            {
                DRVPRINT("Pausing Profile for client %llu", stop_client_id);
                tmp_client_id = (unsigned long)stop_client_id;
            }
            else
            {
                DRVPRINT(KERN_WARNING "Power Profiler: Invalid parameter to Pause Profiler");
                return -1;
            }

            retval = PauseTimer(tmp_client_id);
            return retval;

        case IOCTL_STOP_PROFILER:
            DRVPRINT("In stop profiler ioctl");
            retval = 0;

            if (CopyFromUser(&stop_client_id, (uint32_t*)ioctl_param, sizeof(uint32_t)) == 0)
            {
                DRVPRINT("Stopping Profile for client %llu", stop_client_id);
                tmp_client_id = (unsigned long)stop_client_id;
            }
            else
            {
                DRVPRINT(KERN_WARNING "Power Profiler: Invalid parameter to Stop Profiler");
                return -1;
            }

            retval = StopTimer(tmp_client_id);
            return retval;

        case IOCTL_GET_FILE_HEADER_BUFFER:
            DRVPRINT("In get file header ioctl");

            if (CopyFromUser(&file_header, (FILE_HEADER*)ioctl_param, sizeof(FILE_HEADER)) == 0)
            {
                DRVPRINT("Get File header for client %u ", file_header.ulClientId);
            }

            retval = GetHeaderBuffer(&file_header);
            DRVPRINT("get header buffer ret %d", retval);

            return retval;

        case IOCTL_GET_DATA_BUFFER:
            DRVPRINT("In get data buffer ioctl");

            if (CopyFromUser(&data_buffer, (DATA_BUFFER*)ioctl_param, sizeof(DATA_BUFFER)) == 0)
            {
                DRVPRINT("Get  %u data buffers client %u ", data_buffer.ulavailableBuffCnt, data_buffer.ulClientId);
            }

            retval = GetDataBuffer(&data_buffer);
            DRVPRINT(" Avaliable Buffer Count %u", data_buffer.ulavailableBuffCnt);

            if (CopyToUser((DATA_BUFFER*)ioctl_param , &data_buffer , sizeof(DATA_BUFFER)))
            {
                DRVPRINT(KERN_WARNING "Power Profiler: Error in get data buffer");
                retval = -EACCES;
            }

            return retval;

        case IOCTL_ACCESS_PCI_DEVICE:
            retval = 0;
            DRVPRINT("In Access PCI ioctl");

            if (CopyFromUser(&pci, (ACCESS_PCI*)ioctl_param, sizeof(ACCESS_PCI)) == 0)
            {
                DRVPRINT(" PCI ACCESS address %u ", pci.address);
            }
            else
            {
                DRVPRINT(KERN_WARNING "Power Profiler: Unknown Error in ACCESS_PCI");
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
                DRVPRINT(KERN_WARNING "Power Profiler: unknown error");
                retval = -EACCES;
            }

            return retval;

        case IOCTL_ACCESS_MSR:
            retval = 0;
            DRVPRINT("In Access MSR ioctl");

            if (CopyFromUser(&msr, (ACCESS_MSR*)ioctl_param, sizeof(ACCESS_MSR)) == 0)
            {
                DRVPRINT(" MSR_ACCESS  register %u, data %u", msr.regId, msr.data);
            }
            else
            {
                DRVPRINT("WARNING: Unknown Error in ACCESS_MSR");
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
                DRVPRINT(KERN_WARNING "Power Profiler: unknown error");
                retval = -EACCES;
            }

            return retval;

        case IOCTL_ACCESS_MMIO:
            retval = 0;
            DRVPRINT("In Access MMIO ioctl");

            if (CopyFromUser(&mmio, (ACCESS_MMIO*)ioctl_param, sizeof(ACCESS_MMIO)) == 0)
            {
                DRVPRINT(" ACCESS_MMIO  add 0x%llX, data 0x%x ", mmio.m_addr, mmio.m_data);
            }
            else
            {
                DRVPRINT("WARNING: Unknown Error in ACCESS_MMIO ");
                retval = -EACCES;
                return retval;
            }

            if (false == AccessMMIO(&mmio))
            {
                DRVPRINT("ERROR: Error in ACCESS_MMIO ");
                retval = -EACCES;
                return retval;
            }

            if (CopyToUser((ACCESS_MMIO*)ioctl_param, &mmio , sizeof(ACCESS_MMIO)) != 0)
            {
                DRVPRINT(KERN_WARNING "Power Profiler: unknown error");
                retval = -EACCES;
            }

            return retval;

        case IOCTL_SET_AND_GET_FD:
            retval = 0;
            DRVPRINT("In IOCTL_SET_AND_GET_FD ioctl");
            anon_inode_fd = CreateAnonInodeFd();

            if (CopyToUser((int*)ioctl_param, &anon_inode_fd, sizeof(int)))
            {
                DRVPRINT(KERN_WARNING "Copy to User Failed for IOCTL_SET_AND_GET_FD ");
                retval = -EACCES;
            }

            return retval;

        default:
            DRVPRINT(KERN_WARNING "Power Profiler: Unknown IOCTL ");
            retval = -1;
            return retval;
    }
}

// Module specific cleanup
void PwrProfDrvCleanup(void)
{
    g_isClientActiv = false;
    g_clientId = 0;
    UnconfigureTimer(g_clientId);
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
