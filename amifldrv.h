extern struct file_operations amifldrv_fops;
#define CMD_ALLOC	0x4160
#define CMD_FREE	0x4161
typedef struct tagAMIFLDRV_ALLOC
{
    long	size;
    unsigned long	kvirtlen;
    void * kmallocptr;
    void * kvirtadd;
    void * kphysadd;
} AMIFLDRV_ALLOC;
