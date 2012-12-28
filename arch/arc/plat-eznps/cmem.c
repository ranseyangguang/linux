/*******************************************************************************

  EZNPS Platform support code
  Copyright(c) 2012 EZchip Technologies.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

*******************************************************************************/

#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/slab.h>

struct cmem_arg {
	void __iomem *addr;
	void *buf;
	int len;
	int write;
};

struct cmem_mmap_arg {
	struct task_struct *task;
	resource_size_t user_base;
	void __iomem *kernel_base;
};

static void cmem_access_phys_local(void *arg)
{
	struct cmem_arg *a = (struct cmem_arg *)arg;

	if (arg == NULL)
		return;

	if (a->write)
		memcpy_toio(a->addr, a->buf, a->len);
	else
		memcpy_fromio(a->buf, a->addr, a->len);
}

static int cmem_access_phys(struct vm_area_struct *vma, unsigned long addr,
		void *buf, int len, int write)
{
	struct cmem_mmap_arg *mmap_arg;
	int cpu, offset;
	struct cmem_arg arg = {
			.buf = buf,
			.len = len,
			.write = write,
	};

	if (vma == NULL)
		return -EINVAL;

	if (vma->vm_private_data == NULL)
		return -EINVAL;

	mmap_arg = vma->vm_private_data;
	offset = addr - mmap_arg->user_base;
	arg.addr = mmap_arg->kernel_base + offset;
	cpu = task_cpu(mmap_arg->task);

	preempt_disable();
	smp_call_function_single(cpu, cmem_access_phys_local, &arg, 1);
	preempt_enable();

	return len;
}

static void cmem_vma_close(struct vm_area_struct * area)
{
	struct cmem_mmap_arg *mmap_arg = area->vm_private_data;

	iounmap(mmap_arg->kernel_base);
	kfree(mmap_arg);
}

static int cmem_minor = 1;

static const struct vm_operations_struct cmem_mem_ops = {
	.access = cmem_access_phys,
	.close = cmem_vma_close,
};

static int mmap_cmem(struct file *file, struct vm_area_struct *vma)
{
	size_t size = vma->vm_end - vma->vm_start;
	resource_size_t phys_addr = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long prot = vma->vm_page_prot;
	struct cmem_mmap_arg *arg;

	vma->vm_ops = &cmem_mem_ops;

	arg = kmalloc(sizeof(*arg), GFP_KERNEL);
	if (arg == NULL)
			return -ENOMEM;

	vma->vm_private_data = arg;

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff,
			    size,
			    vma->vm_page_prot)) {
		return -EAGAIN;
	}

	arg->task = current;
	arg->user_base = vma->vm_start;

	/*
	 * map whole cmem in advanced
	 * It is not done in cmem_access_phys() since it is during IPI
	 * And we cannot get virtual memory area during interrupt context.
	 */
	arg->kernel_base = ioremap_prot(phys_addr, size, prot);

	return 0;
}

static const struct file_operations cmem_fops = {
	.mmap		= mmap_cmem,
};

static int cmem_open(struct inode *inode, struct file *filp)
{
	if (iminor(inode) != cmem_minor)
		return -ENXIO;

	filp->f_op = &cmem_fops;
	filp->f_mapping->backing_dev_info = &directly_mappable_cdev_bdi;
	filp->f_mode |= FMODE_UNSIGNED_OFFSET;

	return 0;
}

static const struct file_operations dev_fops = {
	.open = cmem_open,
};

static struct class *cmem_class;

static int __init chr_dev_init(void)
{
	int cmem_major;

	cmem_major = register_chrdev(0, "cmem", &dev_fops);
	if (cmem_major < 0) {
		printk("unable to get dynamic major for cmem dev\n");
		return cmem_major;
	}

	cmem_class = class_create(THIS_MODULE, "cmem");
	if (IS_ERR(cmem_class))
		return PTR_ERR(cmem_class);

	device_create(cmem_class, NULL, MKDEV(cmem_major, cmem_minor),
		      NULL, "cmem");

	return 0;
}

module_init(chr_dev_init);
