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

// SYSTEM INCLUDES
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>

// PROJECT INCLUDES
#include <AMDTPwrProfSharedMemOps.h>

// MODULE DEFINES
#define PROC_DIR_SIZE                   3
#define PWR_PROF_DRV_FIRST_MINOR        0
#define PWR_PROF_DRV_MINOR_CNT          1
#define PWR_PROF_DRV_PROCFS_MAX_SIZE    1024
#define PWR_PROF_DRV_DEVICE_NAME        "amdtPwrProf"

// PCORE MODULE FUNCTIONS
void GetVersions(unsigned int*, unsigned int*, unsigned int*);
long PwrProfDrvIoctlImpl(struct file*, unsigned int, unsigned long);
long CheckPwrProfHwSupport(void);
void PwrProfDrvCleanup(void);

// STATIC VARIABLE
static dev_t dev;
struct cdev* kernel_cdev;
static struct proc_dir_entry* base;

// GLOBAL VARIABLE
// module state, tell if module is in use
// 0: Not in use, 1: In use
int moduleState = 0;

#define _DGPU_ENABLED_

// Invoke IOCTL implementation for amdtPwrProf driver.
long PwrProfDrvDeviceIoctl(struct file* file, unsigned int ioctl_num, unsigned long ioctl_param)
{
    return PwrProfDrvIoctlImpl(file, ioctl_num, ioctl_param);
}

// File operations for amdtPwrProf driver.
struct file_operations Fops =
{
    .unlocked_ioctl = PwrProfDrvDeviceIoctl,
};

// File operations to show amdtPwrProf-device version.
static int amdtPwrProf_proc_show_device(struct seq_file* m, void* v)
{
    seq_printf(m, "%d\n", MAJOR(dev));
    return 0;
}

// File operation to open amdtPwrProf-device file.
static int amdtPwrProf_proc_open_device(struct inode* inode, struct  file* file)
{
    return single_open(file, amdtPwrProf_proc_show_device, NULL);
}

// File operation's for amdtPwrProf-device
static const struct file_operations amdtPwrProf_proc_fops_device =
{
    .open = amdtPwrProf_proc_open_device,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static int amdtPwrProf_proc_show_version(struct seq_file* m, void* v)
{
    unsigned int major_ver, minor_ver, build_num;
    GetVersions(&major_ver, &minor_ver, &build_num);
    seq_printf(m, "%u.%u-%u\n", major_ver, minor_ver, build_num);
    return 0;
}

static int amdtPwrProf_proc_open_version(struct inode* inode, struct  file* file)
{
    return single_open(file, amdtPwrProf_proc_show_version, NULL);
}

static const struct file_operations amdtPwrProf_proc_fops_version =
{
    .open = amdtPwrProf_proc_open_version,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static int
amdtPwrProf_proc_fops_mod_show(struct seq_file* m, void* v)
{
    seq_printf(m, "%d\n", moduleState);
    return 0;
}

static int
amdtPwrProf_proc_fops_mod_open(struct inode* inode, struct file* file)
{
    return single_open(file, amdtPwrProf_proc_fops_mod_show, NULL);
}

static const struct
    file_operations amdtPwrProf_proc_fops_mod_state =
{
    .owner      = THIS_MODULE,
    .open       = amdtPwrProf_proc_fops_mod_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static struct
{
    const char* name;
    const struct file_operations* proc_fops;
} Entries[] =
{
    {"device", &amdtPwrProf_proc_fops_device},
    {"version", &amdtPwrProf_proc_fops_version},
    {"state", &amdtPwrProf_proc_fops_mod_state},
};


// Initialize the module - Register the character device
static int __init PwrProfInitModule(void)
{
    int ret;
    int i;
    unsigned int major_ver, minor_ver, build_num;
    struct proc_dir_entry* entry;
    unsigned int entry_created = 0;

    // Check for KERNEL VERSION SUPPORT.
    // We dont support versions earlier to 2.6.32
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 32)
    printk("Power Profiler: Error, Unsupported kernel version. amdtPwrProf only supports version >= 2.6.32 \n");
    return -EPERM;
#endif

    // Check for x86_64 support
#ifndef CONFIG_X86_64
    printk("Power Profiler: Error, Unsupported kernel version. amdtPwrProf only supports 64 bit kernels \n");
    return -EPERM;
#endif
#ifndef _DGPU_ENABLED_
    ret = CheckPwrProfHwSupport();

    if ((ret < 0))
    {
        return ret;
    }

#endif
    ret = alloc_chrdev_region(&dev, PWR_PROF_DRV_FIRST_MINOR, PWR_PROF_DRV_MINOR_CNT, PWR_PROF_DRV_DEVICE_NAME);

    if (ret < 0)
    {
        printk(KERN_WARNING "Power Profiler: Char device allocation failed\n");
        return ret;
    }

    kernel_cdev = cdev_alloc();

    if (!kernel_cdev)
    {
        printk(KERN_WARNING "Power Profiler: Unable to allocate cdev\n");
        unregister_chrdev_region(dev, PWR_PROF_DRV_MINOR_CNT);
        return -EIO;
    }

    kernel_cdev->ops = &Fops;
    kernel_cdev->owner = THIS_MODULE;

    ret = cdev_add(kernel_cdev, dev, 1);

    if (ret < 0)
    {
        printk(KERN_WARNING "Power Profiler: Unable to add cdev");
        cdev_del(kernel_cdev);
        unregister_chrdev_region(dev, PWR_PROF_DRV_MINOR_CNT);
        return ret;
    }

    if (!(base = proc_mkdir(PWR_PROF_DRV_DEVICE_NAME, NULL)))
    {
        printk(KERN_WARNING "pcore: Unable to create pcore dir");
        return -EIO;
    }

    for (entry_created = 0; entry_created < PROC_DIR_SIZE ; ++entry_created)
    {
        entry = proc_create(Entries[entry_created].name, 0, base,
                            Entries[entry_created].proc_fops);

        if (NULL == entry)
        {
            ret = -ENOMEM;
            break;
        }
    }

    if (ret < 0)
    {
        printk(KERN_WARNING " Power Profiler: internal error");

        if (base)
        {
            for (i = 0; i < entry_created; ++i)
            {
                remove_proc_entry(Entries[i].name, base);
            }

            // delete directory
            remove_proc_entry(PWR_PROF_DRV_DEVICE_NAME, NULL);
        }

        cdev_del(kernel_cdev);
        unregister_chrdev_region(dev, PWR_PROF_DRV_MINOR_CNT);
        return ret;
    }

    GetVersions(&major_ver, &minor_ver, &build_num);

    printk(KERN_INFO "pore:registration was successful.\n");
    printk(KERN_INFO "Power Profiler:version is %u.%u-%u\n", major_ver, minor_ver, build_num);
    printk(KERN_INFO "Power Profiler:device name: %s, major device number: %d.\n", PWR_PROF_DRV_DEVICE_NAME, MAJOR(dev));

    return 0;
}

// Cleanup - unregister the appropriate file from /proc
static void __exit PwrProfDrvCleanupModule(void)
{
    int i;
    // Module Specific Cleanup
    PwrProfDrvCleanup();

    if (base)
    {
        for (i = 0; i < PROC_DIR_SIZE ; ++i)
        {
            remove_proc_entry(Entries[i].name, base);
        }

        remove_proc_entry(PWR_PROF_DRV_DEVICE_NAME, NULL);
    }

    cdev_del(kernel_cdev);
    unregister_chrdev_region(dev, PWR_PROF_DRV_MINOR_CNT);
    printk(KERN_INFO "Power Profiler: unregistreing amdtPwrProf ");
}

module_init(PwrProfInitModule);
module_exit(PwrProfDrvCleanupModule);
MODULE_LICENSE("Dual MIT/GPL");
