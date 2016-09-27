#include <linux/version.h>

#define LINUX_PRE_2_6 (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
#define LINUX_POST_2_6 (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))

#if LINUX_PRE_2_6
#   include <linux/wrapper.h>
#else
#   define mem_map_reserve(p)   set_bit(PG_reserved,   &((p)->flags))
#   define mem_map_unreserve(p) clear_bit(PG_reserved, &((p)->flags))
#endif
