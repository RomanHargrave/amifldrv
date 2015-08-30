extern struct file_operations amifldrv_fops;
#define CMD_ALLOC	0x4160
#define CMD_FREE	0x4161
#define CMD_LOCK_KB	0x4162
#define CMD_UNLOCK_KB	0x4163
typedef struct tagAMIFLDRV_ALLOC
{
    long	size;
    unsigned long	kvirtlen;
    void * kmallocptr;
    void * kvirtadd;
    void * kphysadd;
} AMIFLDRV_ALLOC;
