/*
 * American Megatrends ROM Driver
 * There wasn't a copyright here, so idk
 */

#include <asm/io.h>
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/version.h>

#define LINUX_PRE_2_6   (LINUX_VERSION_CODE <  KERNEL_VERSION(2, 6, 0))
#define LINUX_POST_2_6  (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))

#if LINUX_PRE_2_6
#   include <linux/wrapper.h>
#else 
#   define mem_map_reserve(p)   set_bit  (PG_reserved, &((p)->flags))
#   define mem_map_unreserve(p) clear_bit(PG_reserved, &((p)->flags)) 
#endif 

#define CMD_ALLOC       0x4160
#define CMD_FREE        0x4161
#define CMD_LOCK_KB     0x4162
#define CMD_UNLOCK_KB   0x4163

/*
 * ioctl data packet used to communicate instructions to the driver
 */
typedef struct _struct_AMIFL_alloc_params {
    long            size;
    unsigned long   kvirtlen;
    void*           kmallocptr;
    void*           kvirtadd;
    void*           kphysadd;
} AMIFL_alloc_params;

static int *kmalloc_area         = NULL;
static int *kmalloc_ptr          = NULL;
static unsigned long kmalloc_len = 0L;
static int kcount                = 0;
static int major;
static AMIFL_alloc_params kmalloc_drv[128];

/*
 * Section: Character Device Implementation
 * ============================================================================
 */

#if LINUX_POST_2_6
static int chardev_open_count = 0;
#endif 

/**
 * Character Device open action
 */
int 
AMI_chrdrv_open(struct inode* inode, struct file* file)
{
#if LINUX_PRE_2_6
    MOD_INC_USE_COUNT;
#else
    if (chardev_open_count > 0)
    {
        return -EBUSY;
    }

    ++chardev_open_count;
    try_module_get(THIS_MODULE);
#endif

    return(0);
}

int
AMI_chrdrv_release(struct inode* inode, struct file* file)
{
#if LINUX_PRE_2_6
    MOD_DEC_USE_COUNT;
#else 
    --chardev_open_count;
    module_put(THIS_MODULE);
#endif 

    return(0);
}

#if defined(HAVE_UNLOCKED_IOCTL)
long 
AMI_chrdrv_ioctl(struct file*  _unused_file, unsigned int cmd, unsigned long arg)
#else
int
AMI_chrdrv_ioctl(struct inode* _unused_inode, unsigned int cmd, unsigned long arg)
#endif 
{
    switch(cmd)
    {
    case CMD_ALLOC:
    {
        unsigned long       virt_addr;
        AMIFL_alloc_params  arg_kernel_space;

        if (kcount >= 128) 
        {
            return -EINVAL;
        }

        kmalloc_ptr = NULL;

        if (!arg || kmalloc_ptr) 
        {
            return -EINVAL;
        }

        copy_from_user((void*) &arg_kernel_space, (void*) arg, sizeof(AMIFL_alloc_params));

        if (arg_kernel_space.size > 128 * 1024) 
        {
            return -EINVAL;
        }

        kmalloc_len  = ((arg_kernel_space.size + PAGE_SIZE - 1) & PAGE_MASK);
        kmalloc_ptr  = kmalloc((kmalloc_len + 2 * PAGE_SIZE), GFP_DMA | GFP_KERNEL);
        kmalloc_area = (int *)(((unsigned long)kmalloc_ptr + PAGE_SIZE - 1) & PAGE_MASK);

        for (virt_addr =  (unsigned long) kmalloc_area; 
             virt_addr <  (unsigned long) kmalloc_area + kmalloc_len; 
             virt_addr += PAGE_SIZE)
        {
            mem_map_reserve(virt_to_page(virt_addr));
        }

        {
            int i;
            for (i = 0; i < (kmalloc_len / sizeof(int)); ++i) {
                kmalloc_area[i] = 0xAFD00000 + i;
            }
        }

        kmalloc_drv[kcount].size       = arg_kernel_space.size;
        kmalloc_drv[kcount].kmallocptr = kmalloc_ptr;
        kmalloc_drv[kcount].kvirtlen   = kmalloc_len;
        kmalloc_drv[kcount].kvirtadd   = kmalloc_area;
        kmalloc_drv[kcount].kphysadd   = (void *)((unsigned long)virt_to_phys(kmalloc_area));
        ++kcount;

        arg_kernel_space.kvirtadd = kmalloc_area;
        arg_kernel_space.kphysadd = (void *)((unsigned long)virt_to_phys(kmalloc_area));

        copy_to_user((void*) arg, (void*) &arg_kernel_space, sizeof(AMIFL_alloc_params));

        return 0;
    }
    case CMD_FREE:
    {
        unsigned long       virt_addr;
        AMIFL_alloc_params  arg_kernel_space;
        int isearch = 0;

        copy_from_user((void*) &arg_kernel_space, (void*) arg, sizeof(AMIFL_alloc_params));

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
            for(virt_addr =  (unsigned long) kmalloc_area; 
                virt_addr <  (unsigned long) kmalloc_area + kmalloc_len; 
                virt_addr += PAGE_SIZE)
            {
                mem_map_unreserve(virt_to_page(virt_addr));
            }

            if (kmalloc_ptr) {
                kfree(kmalloc_ptr);
            }

            kmalloc_len  = 0L;
            kmalloc_ptr  = NULL;
            kmalloc_area = NULL;
            kcount--;

            if (isearch != kcount) {
                kmalloc_drv[isearch].size       = kmalloc_drv[kcount].size;
                kmalloc_drv[isearch].kmallocptr = kmalloc_drv[kcount].kmallocptr;
                kmalloc_drv[isearch].kvirtlen   = kmalloc_drv[kcount].kvirtlen;
                kmalloc_drv[isearch].kvirtadd   = kmalloc_drv[kcount].kvirtadd;
                kmalloc_drv[isearch].kphysadd   = kmalloc_drv[kcount].kphysadd;
            }

            kmalloc_drv[kcount].size       = 0;
            kmalloc_drv[kcount].kmallocptr = NULL;
            kmalloc_drv[kcount].kvirtlen   = 0;
            kmalloc_drv[kcount].kvirtadd   = NULL;
            kmalloc_drv[kcount].kphysadd   = NULL;
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

int 
AMI_chrdrv_mmap(struct file* file, struct vm_area_struct* vma)
{
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long size   = vma->vm_end - vma->vm_start;

    if (offset & ~PAGE_MASK) {
        return -ENXIO;
    }
    if (!kmalloc_ptr) {
        return(-ENXIO);
    }
    if (size > kmalloc_len) {
        return(-ENXIO);
    }
    if ((offset+size) > kmalloc_len) {
        return -ENXIO;
    }
    if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED)) {
        return(-EINVAL);
    }

    vma->vm_flags |= VM_LOCKED;

    {
        unsigned long pfn = virt_to_phys((void*)((unsigned long) kmalloc_area));

#if LINUX_POST_2_6
        int remap_result = 
            remap_pfn_range(vma, 
                            vma->vm_start,
                            pfn >> PAGE_SHIFT,
                            size,
                            PAGE_SHARED);
#else
        int remap_result = 
            remap_page_range(vma,
                             vma->vm_start,
                             pfn,
                             size,
                             PAGE_SHARED);
#endif

        if (remap_result) 
        {
            return -ENXIO;
        }
    }

    return(0);
}

#if defined(HAVE_UNLOCKED_IOCTL)
#   define $IOCTL_FIELD unlocked_ioctl
#else
#   define $IOCTL_FIELD ioctl 
#endif 

struct file_operations AMI_chrdrv_fops = {
    owner:          THIS_MODULE,
    open:           AMI_chrdrv_open,
    release:        AMI_chrdrv_release,
    mmap:           AMI_chrdrv_mmap,
    $IOCTL_FIELD:   AMI_chrdrv_ioctl,
};

/*
 * Section: Linux Kernel Module Setup 
 * ============================================================================
 */

#if LINUX_POST_2_6

MODULE_AUTHOR       ("American Megatrends Inc.");
MODULE_DESCRIPTION  ("AMI Flash Update utility driver");
MODULE_LICENSE      ("Proprietary");

#endif

static int /* module_init */
amifldrv_init_module(void)
{
    major = register_chrdev(0, "amifldrv", &AMI_chrdrv_fops);
    
    if (major < 0) 
    {
        return -EIO;
    }

    memset(kmalloc_drv, 0, sizeof(AMIFL_alloc_params) * 128);

    return(0);
}

static void /* module_exit */
amifldrv_cleanup_module(void)
{
    unsigned long virt_addr;
    if (kcount > 0) 
    {
        {
            int iloop;
            for (iloop = 0; iloop < kcount; ++iloop) 
            {
                kmalloc_ptr  = kmalloc_drv[iloop].kmallocptr;
                kmalloc_area = kmalloc_drv[iloop].kvirtadd;
                kmalloc_len  = kmalloc_drv[iloop].kvirtlen;
                if (kmalloc_ptr) 
                {
                    for(virt_addr =  (unsigned long)kmalloc_area; 
                        virt_addr <  (unsigned long)kmalloc_area + kmalloc_len; 
                        virt_addr += PAGE_SIZE) 
                    {
                        mem_map_unreserve(virt_to_page(virt_addr));
                    }

                    if (kmalloc_ptr) 
                    {
                        kfree(kmalloc_ptr);
                    }
                }
            }
        }
        kcount = 0;
    }

    unregister_chrdev(major, "amifldrv");

    return;
}

module_init(amifldrv_init_module);
module_exit(amifldrv_cleanup_module);

