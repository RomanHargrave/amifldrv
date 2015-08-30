//-------------------------------------------------------------------------------------------------
//               AMI Firmware Update Utility(APTIO) v5.05.04
// Copyright (C)2011 American Megatrends Inc. All Rights Reserved.
//-------------------------------------------------------------------------------------------------
#define AFU_ATTRIBUTE_FUNC
AFU_ATTRIBUTE_FUNC int wrap_register_chrdev(void);
AFU_ATTRIBUTE_FUNC void wrap_unregister_chrdev(void);
AFU_ATTRIBUTE_FUNC void wrap_mem_map_reserve(void);
AFU_ATTRIBUTE_FUNC void wrap_mem_map_unreserve(void);
AFU_ATTRIBUTE_FUNC int wrap_remap_page_range(void);
AFU_ATTRIBUTE_FUNC struct page *wrap_virt_to_page(void);
AFU_ATTRIBUTE_FUNC void *wrap_kmalloc(void);
AFU_ATTRIBUTE_FUNC void wrap_kfree(void);
AFU_ATTRIBUTE_FUNC unsigned long wrap_copy_from_user(void);
AFU_ATTRIBUTE_FUNC unsigned long wrap_copy_to_user(void);
extern unsigned long   ulArg0;
extern unsigned long   ulArg1;
extern unsigned long   ulArg2;
extern pgprot_t        pgArg0;
extern void            *pvArg0;
extern void            *pvArg1;
