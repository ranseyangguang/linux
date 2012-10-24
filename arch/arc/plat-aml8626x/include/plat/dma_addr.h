/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __PLAT_DMA_ADDR_H
#define __PLAT_DMA_ADDR_H

#include <linux/device.h>

static inline unsigned long plat_dma_addr_to_kernel(struct device *dev,
						    dma_addr_t dma_addr)
{
	return dma_addr;
}

static inline dma_addr_t plat_kernel_addr_to_dma(struct device *dev, void *ptr)
{
	unsigned long addr = (unsigned long)ptr;

	/*
	 * To Catch buggy drivers which can call DMA map API with kernel vaddr
	 * i.e. for buffers alloc via vmalloc or ioremap which are not
	 * guaranteed to be PHY contiguous and hence unfit for DMA anyways.
	 * On ARC kernel virtual address is 0x7000_0000 to 0x7FFF_FFFF, so
	 * ideally we want to check this range here, but our implementation is
	 * better as it checks for even worse user virtual address as well.
	 */
	BUG_ON(addr < PAGE_OFFSET);

	return addr;
}

#endif
