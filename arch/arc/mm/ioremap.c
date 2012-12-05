/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <asm/cacheflush.h>
#include <asm/cache.h>
#include <asm/tlbflush.h>
#include <asm/pgtable.h>

/*
 * ioremap with access flags
 * Cache semantics wise it is same as ioremap - "forced" uncached.
 * However unline vanilla ioremap which bypasses ARC MMU for addresses in
 * ARC hardware uncached region, this one still goes thru the MMU as caller
 * might need finer access control (R/W/X)
 */
void __iomem *ioremap_prot(phys_addr_t phys_addr, unsigned long size,
			   unsigned long flags)
{
	void __iomem *addr;
	struct vm_struct *area;
	unsigned long offset, last_addr;
	pgprot_t prot = __pgprot(flags);

	/* Don't allow wraparound, zero size */
	last_addr = phys_addr + size - 1;
	if ((!size) || (last_addr < phys_addr))
		return NULL;

	/* force uncached */
	prot = pgprot_noncached(prot);

	/* Mappings have to be page-aligned */
	offset = phys_addr & ~PAGE_MASK;
	phys_addr &= PAGE_MASK;
	size = PAGE_ALIGN(last_addr + 1) - phys_addr;

	/*
	 * Ok, go for it..
	 */
	area = get_vm_area(size, VM_IOREMAP);
	if (!area)
		return NULL;
	area->phys_addr = phys_addr;
	addr = (void __iomem *)area->addr;
	if (ioremap_page_range((unsigned long)addr,
			       (unsigned long)addr + size, phys_addr, prot)) {
		vunmap((void __force *)addr);
		return NULL;
	}
	return (void __iomem *)(offset + (char __iomem *)addr);
}
EXPORT_SYMBOL(ioremap_prot);

void __iomem *__ioremap(unsigned long phys_addr, unsigned long size,
			unsigned long uncached)
{
	unsigned long last_addr;
	pgprot_t prot;

	/* Don't allow wraparound or zero size */
	last_addr = phys_addr + size - 1;
	if (!size || last_addr < phys_addr)
		return NULL;

	/* If the region is h/w uncached, nothing special needed */
	if (phys_addr >= ARC_UNCACHED_ADDR_SPACE) {
		if (!uncached) {
			pr_err("cached ioremap req for uncached addr [%lx]\n",
				phys_addr);
			return NULL;
		} else {
			return (void __iomem *)phys_addr;
		}
	}

	if (uncached)
		prot = PAGE_KERNEL_NO_CACHE;
	else
		prot = PAGE_KERNEL;

	return ioremap_prot(phys_addr, size, prot);
}

EXPORT_SYMBOL(__ioremap);

void iounmap(const void __iomem *addr)
{
	struct vm_struct *p;

	if (addr >= (void __iomem *)ARC_UNCACHED_ADDR_SPACE)
		return;

	p = remove_vm_area((void *)(PAGE_MASK & (unsigned long __force)addr));
	if (!p)
		pr_warn("iounmap: bad address %p\n", addr);

	kfree(p);
}
EXPORT_SYMBOL(iounmap);
