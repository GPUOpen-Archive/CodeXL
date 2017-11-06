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
#include <linux/fs.h>
#include <linux/anon_inodes.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/version.h>

#include <AMDTPwrProfSharedMemOps.h>
#include <AMDTHelpers.h>

#ifndef VM_RESERVED
    #define VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif

DECLARE_WAIT_QUEUE_HEAD(work_queue);

// GLOBAL VARIABLE
uint8_t* g_pSharedBuffer        = NULL;
struct file_operations;

extern atomic_t g_signal;
// Helper functions to [de]allocate private data
static struct pp_anon_inode_ctx* pp_anon_inode_alloc_ctx(void)
{
    struct pp_anon_inode_ctx* ctx = NULL;

    ctx = kmalloc(sizeof(struct pp_anon_inode_ctx), GFP_KERNEL);

    if (NULL != ctx)
    {
        ctx->count = 0;
        atomic_set(&ctx->mmap_count, 0);
        ctx->flags = 0;
        ctx->mmaped_address = 0;
        ctx->order = 8;
        ctx->mmap_size = 0;
        //ctx->mmap_mutex = ?? // FIXME
    }

    return ctx;
}

static void pp_anon_inode_free_ctx(struct pp_anon_inode_ctx* ctx)
{
    if (NULL != ctx)
    {
        kfree(ctx);
    }
}

// File OPs on the anonymous inode
static int pp_anon_inode_release(struct inode* inode, struct file* file)
{
    struct pp_anon_inode_ctx* ctx = file->private_data;

    printk("in pp_anon_inode_release.. \n");

    // TODO.. if the reference count is zero, then free the memory
    // we may need to use the mutex ?
    if (NULL != ctx && (0 == atomic_read(&ctx->mmap_count)))
    {
        //TODO:
        // if ctx->mmaped_address is non-zero, free
        pp_anon_inode_free_ctx(ctx);
    }

    return 0;
}

// MMAP OPs APIs
static void pp_anon_inode_mmap_open(struct vm_area_struct* vma)
{
    int ret = 0;
    struct pp_anon_inode_ctx* ctx  = NULL;

    ctx = (struct pp_anon_inode_ctx*)vma->vm_private_data;

    if (NULL != ctx)
    {
        atomic_inc(&ctx->mmap_count);

        if (1 == atomic_read(&ctx->mmap_count))
        {
            // obtain memory for the shared buffer.
            g_pSharedBuffer = (uint8_t*)__get_free_pages(GFP_KERNEL, ctx->order);
        }

        if (NULL != g_pSharedBuffer)
        {
            ctx->mmaped_address = (uint64_t)g_pSharedBuffer;
            printk("ctx->mmaped_address : 0x%llx\n", ctx->mmaped_address);
        }
        else
        {
            printk("Insufficient memory\n");
            ret = ENOMEM;
        }
    }

    return ;
}

static void pp_anon_inode_mmap_close(struct vm_area_struct* vma)
{
    struct pp_anon_inode_ctx* ctx   = NULL;

    if (NULL != vma)
    {
        ctx = (struct pp_anon_inode_ctx*)vma->vm_private_data;

        if (NULL != ctx)
        {
            // decrement the mmap count
            atomic_dec(&ctx->mmap_count);

            if (0 == atomic_read(&ctx->mmap_count))
            {
                if (!ctx->mmaped_address)
                {
                    free_pages((unsigned long)ctx->mmaped_address, ctx->order);
                }

                // reset
                ctx->order = 0;
                ctx->mmaped_address = 0;
                ctx->mmap_size = 0;
                g_pSharedBuffer = NULL;
            }
        }
    }

    return;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 10, 0)
    static int pp_anon_inode_mmap_fault(struct vm_fault* vmf)
#else
    static int pp_anon_inode_mmap_fault(struct vm_area_struct* vma, struct vm_fault* vmf)
#endif
{
    struct page* page               = NULL;
    struct pp_anon_inode_ctx* ctx   = NULL;
    uint8_t* info                   = NULL;
    int ret                         = 0;

    if (NULL != vmf)
    {

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 10, 0)
        struct vm_area_struct* vma = vmf->vma;
#endif

        if (NULL != vma)
        {
            ctx = (struct pp_anon_inode_ctx*)(vma->vm_private_data);

            if (NULL != ctx)
            {
                if (NULL != vmf->page)
                {
                    DRVPRINT("vmf page is %p ..\n", vmf->page);
                }
                else
                {
                    info = (uint8_t*)(ctx->mmaped_address + (vmf->pgoff * 4096));
                }

                page = virt_to_page(info);

                get_page(page);
                vmf->page = page;
            }
            else
            {
                DRVPRINT("vm_private_data is NULL\n");
                ret = -1;
            }
        }
    }

    return ret;
}

struct vm_operations_struct sharedBuffer_mmap_vm_ops1 =
{
    .open  = pp_anon_inode_mmap_open,
    .close = pp_anon_inode_mmap_close,
    .fault = pp_anon_inode_mmap_fault,
};


static int pp_anon_inode_mmap(struct file* file, struct vm_area_struct* vma)
{
    int ret = -1;

    if (NULL != file && NULL != vma)
    {
        vma->vm_ops          = &sharedBuffer_mmap_vm_ops1;
        vma->vm_flags       |= VM_RESERVED;
        vma->vm_private_data = file->private_data;

        pp_anon_inode_mmap_open(vma);
        ret = 0;
    }

    return ret;
}

static unsigned int pp_anon_inode_poll(struct file* filp, poll_table* wait)
{
    unsigned int mask = 0;
    unsigned int signal = 0;

    poll_wait(filp, &work_queue, wait);

    ATOMIC_GET(&signal, g_signal);

    if (signal)
    {
        mask = POLLIN;
        ATOMIC_SET(&g_signal, 0);
    }

    return mask;
}

static const struct file_operations pp_anon_inode_fops =
{
    .llseek     = noop_llseek,
    .mmap       = pp_anon_inode_mmap,
    .release    = pp_anon_inode_release,
    .poll       = pp_anon_inode_poll,
};

int CreateAnonInodeFd(void)
{
    int fd                          = -1;
    int flags                       = O_RDWR | O_CLOEXEC;
    struct pp_anon_inode_ctx* ctx   = NULL;

    ctx = pp_anon_inode_alloc_ctx();

    if (NULL == ctx)
    {
        DRVPRINT("Failed to allocate private data for anon_inode.");
        return -ENOMEM;
    }

    fd = anon_inode_getfd("[amdtPwrProf]", &pp_anon_inode_fops, ctx, flags);

    if (fd < 0)
    {
        DRVPRINT("Failed to create anonymous inode.");
    }

    return fd;
}


