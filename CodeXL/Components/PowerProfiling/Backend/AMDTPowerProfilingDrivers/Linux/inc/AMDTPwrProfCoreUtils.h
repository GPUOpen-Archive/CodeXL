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

#ifndef _PWR_PROF_DRV_COREUTILS_H_
#define _PWR_PROF_DRV_COREUTILS_H_

// SYSTEM INCLUDES
#include <asm/current.h>
#include <asm/ftrace.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/smp.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>

// Wrappers for kmalloc to allocate memory
void* AllocateMemory(size_t, gfp_t);

// Wrappers for kfree to free previously allocated memory
void FreeMemory(const void*);

// Wrappers for list_add to add a new entry in a list
void AddToList(struct list_head*, struct list_head*);

// Wrappers for list_del to deletes entry from list.
void DeleteFromList(struct list_head*);

// Wrapper for udelay to add delay in micro seconds
void PwrProfUdelay(unsigned long);

// Wrapper for cpumask_set_cpu
void CpuMaskSetCpu(unsigned int, struct cpumask*);

// Wrapper for cpumask_clear
void ClearCpuMask(struct cpumask*);

// Wrapper for add_timer, to start a timer
void AddTimer(struct timer_list*);

// Wrapper for mod_timer to modify timer time out
int ModifyTimer(struct timer_list*, unsigned long);

// Wrapper for setup_timer, to initializes the timer
// and sets the user-provided callback function and context.
void SetupTimer(struct timer_list*, void*, unsigned long);

// Wrapper for del_timer_sync deactivate a timer and wait for the handler
// to finish
int DeactivateTimer(struct timer_list*);

// Wrappers for memset
void* SetMemory(void*, int, size_t);

// Wrapper for msecs_to_jiffies, convert micro seconds to jiffies
unsigned long ConvertMsecToJiffies(const unsigned int);

// Wrapper for copy_to_user, to copy a block of data into user space.
size_t CopyToUser(void __user*, const void*, size_t);

// Wrapper for copy_from_user, to copy a block of data from user space.
size_t CopyFromUser(void __user*, const void*, size_t);

// Wrapper for get_current
struct task_struct* GetCurrentTask(void);

#endif //_PWR_PROF_DRV_COREUTILS_H_
