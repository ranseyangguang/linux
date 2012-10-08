/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/serio.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#define ARC_PS2_PORTS                   (2)

#define ARC_ARC_PS2_ID                  (0x0001f609)

#define STAT_TIMEOUT                    (128)

#define PS2_STAT_RX_FRM_ERR             (1)
#define PS2_STAT_RX_BUF_OVER            (1 << 1)
#define PS2_STAT_RX_INT_EN              (1 << 2)
#define PS2_STAT_RX_VAL                 (1 << 3)
#define PS2_STAT_TX_ISNOT_FUL           (1 << 4)
#define PS2_STAT_TX_INT_EN              (1 << 5)

struct arc_ps2_port {
	unsigned data, status;
	struct serio *io;
};

struct arc_ps2_data {
#ifdef CONFIG_ARC_PS2_DEBUG
	unsigned frame_error;
	unsigned buf_overflow;
	unsigned total_int;
#endif
	struct arc_ps2_port port[ARC_PS2_PORTS];
	struct resource *iomem_res;
	int irq;
	unsigned long addr;
};

static void arc_ps2_check_rx(struct arc_ps2_data *arc_ps2,
			     struct arc_ps2_port *port)
{
	unsigned flag, status;
	unsigned char data;

	do {
		status = inl(port->status);
		if (!(status & PS2_STAT_RX_VAL))
			break;
		flag = 0;
#ifdef CONFIG_ARC_PS2_DEBUG
		arc_ps2->total_int++;
		if (status & PS2_STAT_RX_FRM_ERR) {
			arc_ps2->frame_error++;
			flag |= SERIO_PARITY;
		} else if (status & PS2_STAT_RX_BUF_OVER) {
			arc_ps2->buf_overflow++;
			flag |= SERIO_FRAME;
		}
#else
		if (status & PS2_STAT_RX_FRM_ERR)
			flag |= SERIO_PARITY;
		else if (status & PS2_STAT_RX_BUF_OVER)
			flag |= SERIO_FRAME;
#endif
		data = inl(port->data) & 0xff;
		serio_interrupt(port->io, data, flag);
	} while (1);
}

static irqreturn_t arc_ps2_interrupt(int irq, void *dev)
{
	struct arc_ps2_data *arc_ps2 = dev;
	int i;

	for (i = 0; i < ARC_PS2_PORTS; i++)
		arc_ps2_check_rx(arc_ps2, &arc_ps2->port[i]);

	return IRQ_HANDLED;
}

static int arc_ps2_write(struct serio *io, unsigned char val)
{
	unsigned status;
	struct arc_ps2_port *port = io->port_data;
	int timeout = STAT_TIMEOUT;

	do {
		status = inl(port->status);
		cpu_relax();
		timeout--;
	} while (!(status & PS2_STAT_TX_ISNOT_FUL) && timeout);

	if (timeout)
		outl(val & 0xff, port->data);
	else {
#ifdef CONFIG_ARC_PS2_DEBUG
		pr_err("%s: write timeout\n", __func__);
#endif
		return -1;
	}

	return 0;
}

static int arc_ps2_open(struct serio *io)
{
	struct arc_ps2_port *port = io->port_data;
	outl(PS2_STAT_RX_INT_EN, port->status);
	return 0;
}

static void arc_ps2_close(struct serio *io)
{
	struct arc_ps2_port *port = io->port_data;

	outl(inl(port->status) & ~PS2_STAT_RX_INT_EN, port->status);
}

int __devinit arc_ps2_allocate_port(struct arc_ps2_port *port, int index,
				    unsigned base)
{

	port->io = kzalloc(sizeof(struct serio), GFP_KERNEL);
	if (!port->io)
		return -1;

	port->io->id.type = SERIO_8042;
	port->io->write = arc_ps2_write;
	port->io->open = arc_ps2_open;
	port->io->close = arc_ps2_close;
	snprintf(port->io->name, sizeof(port->io->name), "ARC PS/2 port%d",
		 index);
	snprintf(port->io->phys, sizeof(port->io->phys), "arc/serio%d", index);
	port->io->port_data = port;

	port->data = base + 4 + index * 4;
	port->status = base + 4 + ARC_PS2_PORTS * 4 + index * 4;

	pr_debug("%s: port%d is allocated (data = 0x%x, status = 0x%x)\n",
		 __func__, index, port->data, port->status);

	return 0;
}

static int __devinit arc_ps2_probe(struct platform_device *pdev)
{
	struct arc_ps2_data *arc_ps2;
	int ret, id, i;

	arc_ps2 = kzalloc(sizeof(struct arc_ps2_data), GFP_KERNEL);
	if (!arc_ps2) {
		pr_err("%s: ERROR: out of memory\n", __func__);
		ret = -ENOMEM;
		goto out;
	}

	arc_ps2->iomem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!arc_ps2->iomem_res) {
		pr_err("%s: ERROR: no IO memory defined\n", __func__);
		ret = -ENODEV;
		goto out_free;
	}

	arc_ps2->irq = platform_get_irq_byname(pdev, "arc_ps2_irq");
	if (arc_ps2->irq < 0) {
		pr_err("%s: ERROR: no IRQ defined\n", __func__);
		ret = -ENODEV;
		goto out_free;
	}

	if (!request_mem_region(arc_ps2->iomem_res->start,
	    resource_size(arc_ps2->iomem_res), pdev->name)) {
		pr_err("%s: ERROR: memory allocation failed"
		       "cannot get the I/O addr 0x%x\n",
		       __func__, (unsigned int)arc_ps2->iomem_res->start);

		ret = -EBUSY;
		goto out_free;
	}

	arc_ps2->addr = (unsigned long) ioremap_nocache(arc_ps2->iomem_res->
			start, resource_size(arc_ps2->iomem_res));
	if (!arc_ps2->addr) {
		pr_err("%s: ERROR: memory mapping failed\n", __func__);
		ret = -ENOMEM;
		goto out_release_region;
	}

	pr_info("%s: irq = %d, address = 0x%lx, ports = %i\n", __func__,
	       arc_ps2->irq, arc_ps2->addr, ARC_PS2_PORTS);

	id = inl(arc_ps2->addr);
	if (id != ARC_ARC_PS2_ID) {
		pr_err("%s: device id does not match\n", __func__);
		ret = ENXIO;
		goto out_unmap;
	}

	platform_set_drvdata(pdev, arc_ps2);

	for (i = 0; i < ARC_PS2_PORTS; i++) {
		if (arc_ps2_allocate_port(&arc_ps2->port[i], i,
					  arc_ps2->addr)) {
			ret = -ENOMEM;
			goto release;
		}
		serio_register_port(arc_ps2->port[i].io);
	}

	/* we have got only one shared interrupt so we can place it
	 * here insted of arc_ps2_open */
			  "ARC PS2 interrupt", arc_ps2);
	ret = request_irq(arc_ps2->irq, arc_ps2_interrupt, 0,
	if (ret) {
		pr_err("%s: Could not allocate IRQ\n", __func__);
		goto release;
	}

	return 0;

release:
	for (i = 0; i < ARC_PS2_PORTS; i++) {
		if (arc_ps2->port[i].io)
			serio_unregister_port(arc_ps2->port[i].io);
	}
out_unmap:
	iounmap((void __iomem *) arc_ps2->addr);
out_release_region:
	release_mem_region(arc_ps2->iomem_res->start,
			   resource_size(arc_ps2->iomem_res));
out_free:
	if (arc_ps2)
		kfree(arc_ps2);
out:
	return ret;
}

static int __devexit arc_ps2_remove(struct platform_device *dev)
{
	struct arc_ps2_data *arc_ps2;
	int i;

	arc_ps2 = platform_get_drvdata(dev);
	for (i = 0; i < ARC_PS2_PORTS; i++) {
		/* turn off any interrupts */
		outl(0, arc_ps2->port[i].status);
		serio_unregister_port(arc_ps2->port[i].io);
	}

	free_irq(arc_ps2->irq, arc_ps2);
	iounmap((void __iomem *) arc_ps2->addr);
	release_mem_region(arc_ps2->iomem_res->start,
			   resource_size(arc_ps2->iomem_res));

#ifdef CONFIG_ARC_PS2_DEBUG
	pr_debug("%s: interrupt count = %i\n", __func__, arc_ps2->total_int);
	pr_debug("%s: frame error count = %i\n", __func__,
	       arc_ps2->frame_error);
	pr_debug("%s: buffer overflow count = %i\n", __func__,
	       arc_ps2->buf_overflow);
#endif
	kfree(arc_ps2);

	return 0;
}

static struct platform_driver arc_ps2_driver = {
	.driver = {
		   .name = "arc_ps2",
		   .owner = THIS_MODULE,
		   },
	.probe = arc_ps2_probe,
	.remove = __devexit_p(arc_ps2_remove),
};

static int __init arc_ps2_init(void)
{
	return platform_driver_register(&arc_ps2_driver);
}

static void __exit arc_ps2_exit(void)
{
	platform_driver_unregister(&arc_ps2_driver);
}

/* --------------------------------------------------------------------- */
module_init(arc_ps2_init);
module_exit(arc_ps2_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pavel Sokolov <pavel.sokolov@viragelogic.com>");
MODULE_DESCRIPTION("ARC PS/2 Driver");
