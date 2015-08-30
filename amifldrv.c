#include <linux/mm.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include "amifldrv.h"
#include "amiwrap.h"
int amifldrv_ioctl(void);
int amifldrv_mmap(void);
static int *kmalloc_area = NULL;
static int *kmalloc_ptr = NULL;
static unsigned long kmalloc_len = 0L;
static int major;
static AMIFLDRV_ALLOC kmalloc_drv[128];
static int kcount = 0;
static int amifldrv_init_module(void)
{
    ulArg0 = 0;
    pvArg0 = &amifldrv_fops;
    if ((major=wrap_register_chrdev()) < 0) {
        return (-EIO);
    }
    memset(kmalloc_drv, 0, sizeof(AMIFLDRV_ALLOC) * 128);
    return(0);
}
static void amifldrv_cleanup_module(void)
{
    unsigned long virt_addr;
    int iloop = 0;
    if (kcount > 0) {
        for (iloop=0; iloop<kcount; iloop++) {
            kmalloc_ptr = kmalloc_drv[iloop].kmallocptr;
            kmalloc_area = kmalloc_drv[iloop].kvirtadd;
            kmalloc_len  = kmalloc_drv[iloop].kvirtlen;
            if (kmalloc_ptr) {
                for(virt_addr=(unsigned long)kmalloc_area; virt_addr<(unsigned long)kmalloc_area+kmalloc_len; virt_addr+=PAGE_SIZE) {
                    ulArg0 = virt_addr;
                    pvArg0 = wrap_virt_to_page();
                    wrap_mem_map_unreserve();
                }
                if (kmalloc_ptr) {
                    pvArg0 = kmalloc_ptr;
                    wrap_kfree();
                }
            }
        }
        kcount = 0;
    }
    ulArg0 = major;
    wrap_unregister_chrdev();
    return;
}
module_init(amifldrv_init_module);
module_exit(amifldrv_cleanup_module);
int amifldrv_ioctl(void)
{
    unsigned int cmd = (unsigned int)ulArg0;
    unsigned long arg = ulArg1;
    switch(cmd)
    {
    case CMD_ALLOC:
    {
        int i;
        unsigned long virt_addr;
        AMIFLDRV_ALLOC arg_kernel_space;
        if (kcount >= 128) return -EINVAL;
        kmalloc_ptr = NULL;
        if (!arg || kmalloc_ptr) {
            return -EINVAL;
        }
        pvArg0 = (void*)&arg_kernel_space;
        pvArg1 = (void*)arg;
        ulArg0 = sizeof(AMIFLDRV_ALLOC);
        wrap_copy_from_user();
        if (arg_kernel_space.size > 128*1024) return -EINVAL;
        kmalloc_len = ((arg_kernel_space.size + PAGE_SIZE -1) & PAGE_MASK);
        ulArg0 = kmalloc_len+2*PAGE_SIZE;
        ulArg1 = GFP_DMA|GFP_KERNEL;
        kmalloc_ptr = wrap_kmalloc();
        kmalloc_area=(int *)(((unsigned long)kmalloc_ptr + PAGE_SIZE -1) & PAGE_MASK);
        for (virt_addr=(unsigned long)kmalloc_area; virt_addr<(unsigned long)kmalloc_area+kmalloc_len; virt_addr+=PAGE_SIZE)
        {
            ulArg0 = virt_addr;
            pvArg0 = wrap_virt_to_page();
            wrap_mem_map_reserve();
        }
        for (i=0; i<(kmalloc_len/sizeof(int)); i++) {
            kmalloc_area[i]=(0xafd0<<16) +i;
        }
        kmalloc_drv[kcount].size = arg_kernel_space.size;
        kmalloc_drv[kcount].kmallocptr = kmalloc_ptr;
        kmalloc_drv[kcount].kvirtlen = kmalloc_len;
        kmalloc_drv[kcount].kvirtadd = kmalloc_area;
        kmalloc_drv[kcount].kphysadd = (void *)((unsigned long)virt_to_phys(kmalloc_area));
        kcount++;
        arg_kernel_space.kvirtadd = kmalloc_area;
        arg_kernel_space.kphysadd = (void *)((unsigned long)virt_to_phys(kmalloc_area));
        pvArg0 = (void*)arg;
        pvArg1 = (void*)&arg_kernel_space;
        ulArg0 = sizeof(AMIFLDRV_ALLOC);
        wrap_copy_to_user();
        return 0;
    }
    case CMD_FREE:
    {
        unsigned long virt_addr;
        AMIFLDRV_ALLOC arg_kernel_space;
        int isearch = 0;
        pvArg0 = (void*)&arg_kernel_space;
        pvArg1 = (void*)arg;
        ulArg0 = sizeof(AMIFLDRV_ALLOC);
        wrap_copy_from_user();
        if (kcount > 0) {
            for (isearch=0; isearch<kcount; isearch++) {
                if (kmalloc_drv[isearch].kphysadd == arg_kernel_space.kphysadd)	break;
            }
            if (isearch >= kcount) return 0;
            kmalloc_ptr	 = kmalloc_drv[isearch].kmallocptr;
            kmalloc_area = kmalloc_drv[isearch].kvirtadd;
            kmalloc_len  = kmalloc_drv[isearch].kvirtlen;
        } else
            return 0;
        if (kmalloc_ptr) {
            for(virt_addr=(unsigned long)kmalloc_area; virt_addr<(unsigned long)kmalloc_area+kmalloc_len; virt_addr+=PAGE_SIZE)
            {
                ulArg0 = virt_addr;
                pvArg0 = wrap_virt_to_page();
                wrap_mem_map_unreserve();
            }
            if (kmalloc_ptr) {
                pvArg0 = kmalloc_ptr;
                wrap_kfree();
            }
            kmalloc_len = 0L;
            kmalloc_ptr = NULL;
            kmalloc_area = NULL;
            kcount--;
            if (isearch != kcount) {
                kmalloc_drv[isearch].size = kmalloc_drv[kcount].size;
                kmalloc_drv[isearch].kmallocptr = kmalloc_drv[kcount].kmallocptr;
                kmalloc_drv[isearch].kvirtlen = kmalloc_drv[kcount].kvirtlen;
                kmalloc_drv[isearch].kvirtadd = kmalloc_drv[kcount].kvirtadd;
                kmalloc_drv[isearch].kphysadd = kmalloc_drv[kcount].kphysadd;
            }
            kmalloc_drv[kcount].size = 0;
            kmalloc_drv[kcount].kmallocptr = NULL;
            kmalloc_drv[kcount].kvirtlen = 0;
            kmalloc_drv[kcount].kvirtadd = NULL;
            kmalloc_drv[kcount].kphysadd = NULL;
        }
        return 0;
    }
    case CMD_LOCK_KB:
        disable_irq(1);
        return 0;
    case CMD_UNLOCK_KB:
        enable_irq(1);
        return 0;
    }
    return -ENOTTY;
}
int amifldrv_mmap(void)
{
    struct vm_area_struct *vma  = (struct vm_area_struct *)pvArg1;
    unsigned long offset = vma->vm_pgoff<<PAGE_SHIFT;
    unsigned long size = vma->vm_end - vma->vm_start;
    if (offset & ~PAGE_MASK) {
        return -ENXIO;
    }
    if (!kmalloc_ptr) {
        return(-ENXIO);
    }
    if (size>kmalloc_len) {
        return(-ENXIO);
    }
    if ((offset+size)>kmalloc_len) {
        return -ENXIO;
    }
    if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED)) {
        return(-EINVAL);
    }
    vma->vm_flags |= VM_LOCKED;
    pvArg0 = vma;
    ulArg0 = vma->vm_start;
    ulArg1 = virt_to_phys((void*)((unsigned long)kmalloc_area));
    ulArg2 = size;
    pgArg0 = PAGE_SHARED;
    if (wrap_remap_page_range()) {
        return -ENXIO;
    }
    return(0);
}
