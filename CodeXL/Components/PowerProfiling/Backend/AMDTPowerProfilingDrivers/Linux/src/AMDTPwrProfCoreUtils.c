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

// LOCAL INCLUDES
#include <AMDTPwrProfCoreUtils.h>

// SYSTEM INCLUDES
#include <linux/uaccess.h>

//
// Wrappers for kmalloc to allocate memory
void* AllocateMemory(size_t n, gfp_t flags)
{
    return kmalloc(n, flags);
}

// Wrappers for kfree to free previously allocated memory
void FreeMemory(const void* ptr)
{
    kfree(ptr);
}

// Wrappers for memset
void* SetMemory(void* src, int c, size_t n)
{
    return memset(src, c, n);
}

// Wrappers for list_add to add a new entry in a list
void AddToList(struct list_head* new, struct list_head* head)
{
    list_add(new, head);

}

// Wrappers for list_del to deletes entry from list.
void DeleteFromList(struct list_head* entry)
{
    list_del(entry);
}

// Wrapper for udelay to add delay in micro seconds
void PwrProfUdelay(unsigned long d)
{
    udelay(d);
}

// Wrapper for cpumask_set_cpu
void CpuMaskSetCpu(unsigned int cpu, struct cpumask* mask)
{
    cpumask_set_cpu(cpu, mask);
}

// Wrapper for cpumask_clear
void ClearCpuMask(struct cpumask* mask)
{
    cpumask_clear(mask);
}

// Wrapper for get_current
struct task_struct* GetCurrentTask(void)
{
    return get_current();
}

// Wrapper for msecs_to_jiffies convert micro seconds to jiffies.
unsigned long ConvertMsecToJiffies(const unsigned int m)
{
    return msecs_to_jiffies(m);
}

// Wrapper for copy_to_user to copy a block of data into user space.
size_t CopyToUser(void __user* dest, const void* src, size_t n)
{
    return copy_to_user(dest, src, n);
}

// Wrapper for copy_from_user to copy a block of data from user space.
size_t CopyFromUser(void* dest, const void __user* src, size_t n)
{
    return copy_from_user(dest, src, n);
}
