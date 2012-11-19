/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Vineetg: Jan 2009
 *  -DMA API functions
 *  -Centralised DMA-Cache sync
 *  -Detect buggy drivers which cause unaligned Cache Ops
 *
 * Consistent DMA Mappings:
 *  -Reqmt is to allocate memory such that writes by CPU or device are
 *   visible to other party without explcit Cache Flush.
 *  -On ARC, hardware doesnt provide explicit DMA Coherent Memory
 *   There is no Cache snooping for DMA transactions
 *  -CPU does provide a NO-CACHE bit in TLB allowing pages to be non-cached
 *   However kernel executes in untranslated space, so it doesnt exercise
 *   MMU. Thus to allocate Consistent memory we allocate a chunk of
 *   Kernel Virtual Memory, allocate contiguous free pages, and map them
 */

#ifdef CONFIG_HAS_DMA

#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <linux/kallsyms.h>

void *
dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_handle,
           gfp_t flag)
{
    void *kvaddr;
    unsigned long page_addr;

    size = PAGE_ALIGN(size);

    if (!size || (size >> PAGE_SHIFT) > num_physpages)
        return NULL;

    flag |= GFP_DMA | __GFP_HIGHMEM;

    // This is linear addr (0x8000_0000 based)
    page_addr = __get_free_pages(flag, get_order(size));

    // This is kernel Virtual address (0x7000_0000 based)
    kvaddr = ioremap_nocache(page_addr, size);

    if (kvaddr != NULL) {
        memset(kvaddr, 0, size);

        // This is bus address, platform dependent
        *dma_handle = plat_kernel_addr_to_dma(dev, (void *)page_addr);
    }

    return kvaddr;
}

void
dma_free_coherent(struct device *dev, size_t size, void *kvaddr,
                    dma_addr_t dma_handle)
{
    iounmap(kvaddr);
    free_pages(plat_dma_addr_to_kernel(dev, dma_handle), get_order(size));
}

EXPORT_SYMBOL(dma_alloc_coherent);
EXPORT_SYMBOL(dma_free_coherent);

/*
 * Helper which invokes the appropriate Cache routines
 */

void __dma_cache_maint(void *start, size_t sz, int dir, void *caller_for_dbg)
{
    unsigned long addr = (unsigned long)start;

    /* Check for buffer's Cache line alignment
     *  otherwise there could be ugly side effects.
     */
    if (is_not_cache_aligned(start)) {
        printk("Non-align Cache maint op for %#lx at", addr);
        __print_symbol("%s\n",(unsigned long)caller_for_dbg);
    }

	switch (dir) {
	case DMA_FROM_DEVICE:
        dma_cache_inv(addr, sz);
		break;
	case DMA_TO_DEVICE:
        dma_cache_wback(addr, sz);
		break;
	case DMA_BIDIRECTIONAL:
        dma_cache_wback_inv(addr, sz);
		break;
	default:
		BUG();
	}
}

/* For drivers to do explicit cache mgmt */
EXPORT_SYMBOL(__dma_cache_maint);
#endif
