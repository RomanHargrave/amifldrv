//-------------------------------------------------------------------------------------------------
//               AMI Firmware Update Utility(APTIO)  v5.05.04
// Copyright (C)2011 American Megatrends Inc. All Rights Reserved.
//-------------------------------------------------------------------------------------------------
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#include <linux/wrapper.h>
#endif
#include "amiwrap.h"

extern int amifldrv_ioctl(void);
extern int amifldrv_mmap(void);
unsigned long   ulArg0;
unsigned long   ulArg1;
unsigned long   ulArg2;
pgprot_t        pgArg0;
void            *pvArg0;
void            *pvArg1;

AFU_ATTRIBUTE_FUNC static int wrap_open(struct inode *inode, struct file *file);
AFU_ATTRIBUTE_FUNC static int wrap_release(struct inode *inode, struct file *file);
#if defined(HAVE_UNLOCKED_IOCTL)
AFU_ATTRIBUTE_FUNC static long wrap_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#else
AFU_ATTRIBUTE_FUNC static int wrap_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#endif
AFU_ATTRIBUTE_FUNC static int wrap_mmap(struct file *file, struct vm_area_struct *vma);

struct file_operations amifldrv_fops =
{
owner:
    THIS_MODULE,
open:
    wrap_open,
release:
    wrap_release,
#if defined(HAVE_UNLOCKED_IOCTL)
unlocked_ioctl:
    wrap_unlocked_ioctl,
#else
ioctl:
    wrap_ioctl,
#endif
mmap:
    wrap_mmap,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define mem_map_reserve(p) set_bit(PG_reserved, &((p)->flags))
#define mem_map_unreserve(p) clear_bit(PG_reserved, &((p)->flags))
MODULE_AUTHOR("American Megatrends Inc.");
MODULE_DESCRIPTION("AMI Flash Update utility driver");
MODULE_LICENSE("Proprietary");
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static int device_open_count = 0;
#endif

AFU_ATTRIBUTE_FUNC
int wrap_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
    MOD_INC_USE_COUNT;
#else
    if (device_open_count)
        return -EBUSY;
    device_open_count++;
    try_module_get(THIS_MODULE);
#endif
    return(0);
}

AFU_ATTRIBUTE_FUNC
int wrap_release(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
    MOD_DEC_USE_COUNT;
#else
    device_open_count--;
    module_put(THIS_MODULE);
#endif
    return(0);
}

AFU_ATTRIBUTE_FUNC
#if defined(HAVE_UNLOCKED_IOCTL)
long wrap_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
int wrap_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
#ifndef HAVE_UNLOCKED_IOCTL
    pvArg0 = inode;
#endif
    pvArg1 = filp;
    ulArg0 = cmd;
    ulArg1 = arg;
#if defined(HAVE_UNLOCKED_IOCTL)
    return (long)amifldrv_ioctl();
#else
    return amifldrv_ioctl();
#endif
}

AFU_ATTRIBUTE_FUNC
int wrap_mmap(struct file *file, struct vm_area_struct *vma)
{
    pvArg0 = file;
    pvArg1 = vma;
    return amifldrv_mmap();
}

AFU_ATTRIBUTE_FUNC
int wrap_register_chrdev()
{
    return register_chrdev((unsigned int)ulArg0, "amifldrv", (struct file_operations *)pvArg0);
}

AFU_ATTRIBUTE_FUNC
void wrap_unregister_chrdev()
{
    unregister_chrdev((unsigned int)ulArg0, "amifldrv");
}

AFU_ATTRIBUTE_FUNC
void wrap_mem_map_reserve()
{
    mem_map_reserve((struct page*)pvArg0);
}

AFU_ATTRIBUTE_FUNC
void wrap_mem_map_unreserve()
{
    mem_map_unreserve((struct page*)pvArg0);
}

AFU_ATTRIBUTE_FUNC
struct page *wrap_virt_to_page()
{
    return virt_to_page(ulArg0);
}

AFU_ATTRIBUTE_FUNC
int wrap_remap_page_range()
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
    ulArg1 = ulArg1>>PAGE_SHIFT;
    return remap_pfn_range((struct vm_area_struct *)pvArg0, ulArg0, ulArg1, ulArg2, pgArg0);
#else
    return remap_page_range((struct vm_area_struct *)pvArg0, ulArg0, ulArg1, ulArg2, pgArg0);
#endif
}

AFU_ATTRIBUTE_FUNC
void *wrap_kmalloc()
{
    return kmalloc((size_t)ulArg0, (int)ulArg1);
}

AFU_ATTRIBUTE_FUNC
void wrap_kfree()
{
    kfree(pvArg0);
}

AFU_ATTRIBUTE_FUNC
unsigned long wrap_copy_from_user()
{
    return copy_from_user(pvArg0, pvArg1, ulArg0);
}

AFU_ATTRIBUTE_FUNC
unsigned long wrap_copy_to_user()
{
    return copy_to_user(pvArg0, pvArg1, ulArg0);
}

