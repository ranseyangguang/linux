/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

#include <plat/dw_gpio.h>

#define GPIO_REGISTER_RESOURCE_NAME  "dw_gpio_reg"
#define GPIO_IRQ_RESOURCE_NAME       "dw_gpio_irq"

#define DRV_VERSION         "1.0"
#define DRV_NAME            "dw_gpio"
#define DRIVER_VERSION      DRV_NAME " driver version:" DRV_VERSION "\n"

#ifdef CONFIG_DEBUG_KERNEL
#define DEBUG_WITH_LED_ON
#define DEBUG_PRINT(format, args...)  printk(format , ## args)
#else
#define DEBUG_PRINT(format, args...)
#endif

/*
 * With this define a led is used to indicate that gpio is initialised
 * This led is also used to indicate a edge toggle (key press/release)
 */
#ifdef DEBUG_WITH_LED_ON
#define GPIO_LED_OUT  GPIO_LED_6
#endif

struct dw_gpio_chip {
	struct gpio_chip chip;
	void __iomem     *iobase;
	int              porta_end;
	int              portb_end;
	int              portc_end;
	int              portd_end;
	int              irq_base;
	int              nr_irq;
	unsigned long    gpio_int_en;
	unsigned long    gpio_int_mask;
	unsigned long    gpio_int_type;
	unsigned long    gpio_int_pol;
	unsigned long    gpio_int_both_edge;
	unsigned long    gpio_int_deb;
	spinlock_t       lock;
};

static inline struct dw_gpio_chip *to_dw_gpio_chip(struct gpio_chip *chip)
{
	return container_of(chip, struct dw_gpio_chip, chip);
}

#define GPIO_SW_PORT_A_DR_REG_OFFSET	0x00
#define GPIO_SW_PORT_A_DDR_REG_OFFSET	0x04
#define GPIO_SW_PORT_A_CTL_REG_OFFSET	0x08
#define GPIO_SW_PORT_B_DR_REG_OFFSET	0x0C
#define GPIO_SW_PORT_B_DDR_REG_OFFSET	0x10
#define GPIO_SW_PORT_B_CTL_REG_OFFSET	0x14
#define GPIO_SW_PORT_C_DR_REG_OFFSET	0x18
#define GPIO_SW_PORT_C_DDR_REG_OFFSET	0x1C
#define GPIO_SW_PORT_C_CTL_REG_OFFSET	0x20
#define GPIO_SW_PORT_D_DR_REG_OFFSET	0x24
#define GPIO_SW_PORT_D_DDR_REG_OFFSET	0x28
#define GPIO_SW_PORT_D_CTL_REG_OFFSET	0x2C

#define GPIO_INT_EN_REG_OFFSET			0x30
#define GPIO_INT_MASK_REG_OFFSET		0x34
#define GPIO_INT_TYPE_LEVEL_REG_OFFSET	0x38
#define GPIO_INT_POLARITY_REG_OFFSET	0x3c
#define GPIO_INT_STATUS_REG_OFFSET		0x40
#define GPIO_INT_RAW_STATUS_REG_OFFSET	0x44
#define GPIO_PORT_A_EOI_REG_OFFSET		0x4c
#define GPIO_PORT_A_DEBOUNCE_REG_OFFSET	0x48

#define GPIO_SW_PORT_A_EXT_REG_OFFSET	0x50
#define GPIO_SW_PORT_B_EXT_REG_OFFSET	0x54
#define GPIO_SW_PORT_C_EXT_REG_OFFSET	0x58
#define GPIO_SW_PORT_D_EXT_REG_OFFSET	0x5C

#define GPIO_CFG1_REG_OFFSET            0x74
#define GPIO_NR_PORTS_SHIFT             2
#define GPIO_NR_PORTS_MASK              (0x3 << GPIO_NR_PORTS_SHIFT)

#define GPIO_CFG2_REG_OFFSET            0x70
#define GPIO_PORTA_WIDTH_SHIFT          0
#define GPIO_PORTA_WIDTH_MASK           (0x1F << GPIO_PORTA_WIDTH_SHIFT)
#define GPIO_PORTB_WIDTH_SHIFT          5
#define GPIO_PORTB_WIDTH_MASK           (0x1F << GPIO_PORTB_WIDTH_SHIFT)
#define GPIO_PORTC_WIDTH_SHIFT          10
#define GPIO_PORTC_WIDTH_MASK           (0x1F << GPIO_PORTC_WIDTH_SHIFT)
#define GPIO_PORTD_WIDTH_SHIFT          15
#define GPIO_PORTD_WIDTH_MASK           (0x1F << GPIO_PORTD_WIDTH_SHIFT)

/*
 * Get the port mapping for the controller.  There are 4 possible ports that
 * we can use but some platforms may reserve ports for hardware purposes that
 * cannot be used as GPIO so we allow these to be masked off.
 */
static void dw_gpio_configure_ports(struct dw_gpio_chip *dw,
	unsigned long port_mask)
{
	unsigned long cfg1, cfg2;
	int nr_ports;

	cfg1 = readl(dw->iobase + GPIO_CFG1_REG_OFFSET);
	cfg2 = readl(dw->iobase + GPIO_CFG2_REG_OFFSET);

	nr_ports = (cfg1 & GPIO_NR_PORTS_MASK) >> GPIO_NR_PORTS_SHIFT;

	DEBUG_PRINT(KERN_INFO DRV_NAME ": cfg1 0x%lx\n", cfg1);
	DEBUG_PRINT(KERN_INFO DRV_NAME ": cfg2 0x%lx\n", cfg2);

	DEBUG_PRINT(KERN_INFO DRV_NAME ": port_mask 0x%lx\n", port_mask);
	DEBUG_PRINT(KERN_INFO DRV_NAME
		": nr_ports_reg_val %d -> actual nr ports %d\n",
		nr_ports, nr_ports+1);

	if (port_mask & DW_GPIO_PORTA_MASK) {
		dw->porta_end = ((cfg2 & GPIO_PORTA_WIDTH_MASK) >>
			GPIO_PORTA_WIDTH_SHIFT);
		if (nr_ports == 0)
			return;
	} else
		dw->porta_end = 0;

	if (port_mask & DW_GPIO_PORTB_MASK) {
		dw->portb_end = ((cfg2 & GPIO_PORTB_WIDTH_MASK) >>
			GPIO_PORTB_WIDTH_SHIFT) + 1 + dw->porta_end;
		if (nr_ports == 1)
			return;
	} else
		dw->portb_end = dw->porta_end;

	if (port_mask & DW_GPIO_PORTC_MASK) {
		dw->portc_end = ((cfg2 & GPIO_PORTC_WIDTH_MASK) >>
			GPIO_PORTC_WIDTH_SHIFT) + 1 + dw->portb_end;
		if (nr_ports == 2)
			return;
	} else
		dw->portc_end = dw->portb_end;

	if (port_mask & DW_GPIO_PORTD_MASK)
		dw->portd_end = ((cfg2 & GPIO_PORTD_WIDTH_MASK) >>
			GPIO_PORTD_WIDTH_SHIFT) + 1 + dw->portc_end;
	else
		dw->portd_end = dw->portb_end;

	DEBUG_PRINT(KERN_INFO DRV_NAME ": dw->porta_end %d\n", dw->porta_end);
	DEBUG_PRINT(KERN_INFO DRV_NAME ": dw->portb_end %d\n", dw->portb_end);
	DEBUG_PRINT(KERN_INFO DRV_NAME ": dw->portc_end %d\n", dw->portc_end);
	DEBUG_PRINT(KERN_INFO DRV_NAME ": dw->portd_end %d\n", dw->portd_end);
}

#define __DWGPIO_REG(__chip, __gpio_nr, __reg)      \
	({                                              \
		void __iomem *ret = NULL;                   \
	if ((__gpio_nr) <= (__chip)->porta_end)        \
		ret = ((__chip)->iobase +                   \
			GPIO_SW_PORT_A_##__reg##_REG_OFFSET);   \
	else if ((__gpio_nr) <= (__chip)->portb_end)   \
		ret = ((__chip)->iobase +                   \
			GPIO_SW_PORT_B_##__reg##_REG_OFFSET);   \
	else if ((__gpio_nr) <= (__chip)->portc_end)   \
		ret = ((__chip)->iobase +                   \
			GPIO_SW_PORT_C_##__reg##_REG_OFFSET);   \
	else                                           \
		ret = ((__chip)->iobase +                   \
			GPIO_SW_PORT_D_##__reg##_REG_OFFSET);   \
			ret;                                    \
	})

#define DWGPIO_DR(__chip, __gpio_nr)  __DWGPIO_REG((__chip), (__gpio_nr), DR)
#define DWGPIO_DDR(__chip, __gpio_nr) __DWGPIO_REG((__chip), (__gpio_nr), DDR)
#define DWGPIO_CTL(__chip, __gpio_nr) __DWGPIO_REG((__chip), (__gpio_nr), CTL)
#define DWGPIO_EXT(__chip, __gpio_nr) __DWGPIO_REG((__chip), (__gpio_nr), EXT)

#define INT_EN_REG(__chip) ((__chip)->iobase + GPIO_INT_EN_REG_OFFSET)
#define INT_MASK_REG(__chip) ((__chip)->iobase + GPIO_INT_MASK_REG_OFFSET)
#define INT_TYPE_REG(__chip) ((__chip)->iobase + \
	GPIO_INT_TYPE_LEVEL_REG_OFFSET)
#define INT_POLARITY_REG(__chip) ((__chip)->iobase + \
	GPIO_INT_POLARITY_REG_OFFSET)
#define INT_STATUS_REG(__chip) ((__chip)->iobase + GPIO_INT_STATUS_REG_OFFSET)
#define INT_RAW_STATUS_REG(__chip) ((__chip)->iobase + \
	GPIO_INT_RAW_STATUS_REG_OFFSET)
#define EOI_REG(__chip) ((__chip)->iobase + GPIO_PORT_A_EOI_REG_OFFSET)
#define PORTA_DEBOUNCE_REG(__chip) ((__chip)->iobase + \
	GPIO_PORT_A_DEBOUNCE_REG_OFFSET)

static inline unsigned dw_gpio_bit_offs(struct dw_gpio_chip *dw,
	unsigned offset)
{
	/*
	 * The gpios are controlled via three sets of registers. The register
	 * addressing is already taken care of by the __DWGPIO_REG macro,
	 * this takes care of the bit offsets within each register.
	 */
	if (offset <= dw->porta_end)
		return offset;
	else if (offset <= dw->portb_end)
		return offset - (dw->porta_end + 1);
	else if (offset <= dw->portc_end)
		return offset - (dw->portb_end + 1);
	else
		return offset - (dw->portc_end + 1);
}

static int dwgpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	struct dw_gpio_chip *dw = to_dw_gpio_chip(chip);
	void __iomem *ddr = DWGPIO_DDR(dw, offset);
	void __iomem *cr = DWGPIO_CTL(dw, offset);
	unsigned long flags, val, bit_offset = dw_gpio_bit_offs(dw, offset);

	spin_lock_irqsave(&dw->lock, flags);
	/* mark the pin as an output */
	val = readl(ddr);
	val &= ~(1 << bit_offset);
	writel(val, ddr);

	/* set the pin as software controlled */
	val = readl(cr);
	val &= ~(1 << bit_offset);
	writel(val, cr);
	spin_unlock_irqrestore(&dw->lock, flags);

	return 0;
}

static int dwgpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct dw_gpio_chip *dw = to_dw_gpio_chip(chip);
	void __iomem *ext = DWGPIO_EXT(dw, offset);
	unsigned long bit_offset = dw_gpio_bit_offs(dw, offset);

	return !!(readl(ext) & (1 << bit_offset));
}

static void dwgpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct dw_gpio_chip *dw = to_dw_gpio_chip(chip);
	void __iomem *dr = DWGPIO_DR(dw, offset);
	unsigned long val, flags, bit_offset = dw_gpio_bit_offs(dw, offset);

	spin_lock_irqsave(&dw->lock, flags);
	val = readl(dr);
	val &= ~(1 << bit_offset);
	val |= (!!value << bit_offset);
	writel(val, dr);
	spin_unlock_irqrestore(&dw->lock, flags);
}

static int dwgpio_direction_output(struct gpio_chip *chip, unsigned offset,
	int value)
{
	struct dw_gpio_chip *dw = to_dw_gpio_chip(chip);
	void __iomem *ddr = DWGPIO_DDR(dw, offset);
	void __iomem *cr = DWGPIO_CTL(dw, offset);
	unsigned long flags, val, bit_offset = dw_gpio_bit_offs(dw, offset);

	/* set the value first so we don't glitch */
	dwgpio_set(chip, offset, value);

	spin_lock_irqsave(&dw->lock, flags);
	/* mark the pin as an output */
	val = readl(ddr);
	val |= (1 << bit_offset);
	writel(val, ddr);

	/* set the pin as software controlled */
	val = readl(cr);
	val &= ~(1 << bit_offset);
	writel(val, cr);
	spin_unlock_irqrestore(&dw->lock, flags);

	return 0;
}

static int dwgpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	struct dw_gpio_chip *dw = to_dw_gpio_chip(chip);

	/* only one interrupt supported for now */
	return dw->nr_irq;
}

static irqreturn_t dw_gpio_irq_handler(int irq, void *dev_id)
{
	unsigned long status;
	unsigned long type_int;
	unsigned long old_pol;
	struct dw_gpio_chip *dw = dev_id;
	void __iomem *port_mask = INT_MASK_REG(dw);
	void __iomem *port_int_status = INT_STATUS_REG(dw);
	void __iomem *port_eoi = EOI_REG(dw);
	void __iomem *port_polarity = INT_POLARITY_REG(dw);
	void __iomem *port_int_en = INT_EN_REG(dw);
	void __iomem *port_type = INT_TYPE_REG(dw);

	/* mask interrupt line, none can be generated by triggering IO */
	status = readl(port_int_status);
	/* mask current interrupt line */
	writel(status, port_mask);

	/* clear current interrupt */
	writel(status, port_eoi);

	/* get type for current interrupt 0=level, 1=edge */
	type_int = (readl(port_type) & status);

	/* check edge triggered & if trigger on both rising & falling edges */
	if ((type_int != 0) && (dw->gpio_int_both_edge & status) > 0) {
		/* disable interrupts, to prevent glitches */
		writel((dw->gpio_int_en ^ status), port_int_en);
		/* get polarity for current interrupt */
		old_pol = readl(port_polarity);
		/* toggle polarity */
		writel((old_pol ^ status), port_polarity);

#ifdef DEBUG_WITH_LED_ON
		/* now toggle led */
		if ((old_pol & status) == 0) {
			/* set edge detection to raising (was falling) */
			gpio_set_value(GPIO_LED_OUT, 0);
		} else {
			/* set edge detection to falling (was rising) */
			gpio_set_value(GPIO_LED_OUT, 1);
		}
#endif
		/* enable interrupts again */
		writel(dw->gpio_int_en, port_int_en);
	}

	/* enable, set mask back */
	writel(dw->gpio_int_mask, port_mask); /* all req bits unmasked again */

	return IRQ_HANDLED;
}

/*
 * There is no 1-1 mapping, only one interrupt available in the system
 * for GPIO. All 8 ports raise one (same) interrupt.
 */
static void dwgpio_irq_init(struct dw_gpio_chip *dw)
{
	int rc;
	void __iomem *port_mask = INT_MASK_REG(dw);
	void __iomem *port_int_en = INT_EN_REG(dw);
	void __iomem *port_type = INT_TYPE_REG(dw);
	void __iomem *port_polarity = INT_POLARITY_REG(dw);
	void __iomem *port_debounce = PORTA_DEBOUNCE_REG(dw);

	if (!dw->nr_irq) {
		/* an error occurred by requesting the interrupt */
		printk(KERN_ERR DRV_NAME ": No int in structure!\n");
		return;
	}

	/* disabled all GPIO interrupts, for now) */
	writel(0, port_int_en);

	/* setup DW_GPIO_ISR */
	rc = request_irq(dw->nr_irq, (void *)dw_gpio_irq_handler,
		IRQF_TRIGGER_LOW | IRQF_SHARED, DRV_NAME, dw);
	if (rc != 0) {
		/* an error occurred by requesting the interrupt */
		printk(KERN_ERR DRV_NAME
			": Enable interrupt failed, could not be registered\n");
	}

	/* port type level (0) or edge (1) triggered */
	writel(dw->gpio_int_type, port_type);
	/* polarity: active low (0) or active high (1) */
	writel(dw->gpio_int_pol, port_polarity);
	/* masks the IO lines/bits that do not trigger an interrupt */
	writel(dw->gpio_int_mask, port_mask);
	/* setup debounce */
	writel(dw->gpio_int_deb, port_debounce);
	/* enable interrupts */
	writel(dw->gpio_int_en, port_int_en);

	return;
}

static void dwgpio_irq_stop(struct dw_gpio_chip *dw)
{
	void __iomem *port_mask = INT_MASK_REG(dw);
	void __iomem *port_int_en = INT_EN_REG(dw);

	/* disabled all GPIO interrupts */
	writel(0, port_int_en);
	writel(0xff, port_mask);
	free_irq(dw->nr_irq, dw);
}

static int __devinit dwgpio_probe(struct platform_device *pdev)
{
	struct resource *mem = platform_get_resource_byname(pdev,
		IORESOURCE_MEM, GPIO_REGISTER_RESOURCE_NAME);
	struct resource *irqs;
	struct dw_gpio_chip *dw = devm_kzalloc(&pdev->dev, sizeof(*dw),
		GFP_KERNEL);
	int err, rc;
	struct dw_gpio_pdata *pdata = pdev->dev.platform_data;

	if (!dw)
		return -ENOMEM;

	if (!mem || !pdata)
		return -ENODEV;

	/* check if resources are free */
	rc = check_mem_region(mem->start, resource_size(mem));

	if (rc) {
		printk(KERN_ERR DRV_NAME ": GPIO I/O memory already in use\n");
		return -EBUSY;
	}

	/* claim resources */
	if (!request_mem_region(mem->start, resource_size(mem), DRV_NAME)) {
		printk(KERN_ERR DRV_NAME
			": Request for GPIO register I/O region failed\n");
		return -EBUSY;
	}

	DEBUG_PRINT(KERN_INFO DRV_NAME ": base memory addr 0x%8x\n",
		mem->start);

	dw->iobase = devm_ioremap_nocache(&pdev->dev, mem->start,
		resource_size(mem));
	if (!dw->iobase)
		return -ENOMEM;

	DEBUG_PRINT(KERN_INFO DRV_NAME ": Remap base memory addr 0x%8x\n",
		(uint) dw->iobase);

	dw_gpio_configure_ports(dw, pdata->port_mask);

	dw->chip.direction_input    = dwgpio_direction_input;
	dw->chip.direction_output   = dwgpio_direction_output;
	dw->chip.get                = dwgpio_get;
	dw->chip.set                = dwgpio_set;
	dw->chip.to_irq             = dwgpio_to_irq;
	dw->chip.owner              = THIS_MODULE;
	dw->chip.label              = DRV_NAME;
	dw->chip.base               = pdata->gpio_base;
	dw->chip.ngpio              = pdata->ngpio;
	dw->chip.dev                = &pdev->dev;

	/* GPIO Port A Interrupt settings */
	dw->gpio_int_en             = pdata->gpio_int_en;
	dw->gpio_int_mask           = pdata->gpio_int_mask;
	dw->gpio_int_type           = pdata->gpio_int_type;
	dw->gpio_int_pol            = pdata->gpio_int_pol;
	dw->gpio_int_both_edge      = pdata->gpio_int_both_edge;
	dw->gpio_int_deb            = pdata->gpio_int_deb;

	spin_lock_init(&dw->lock);

	err = gpiochip_add(&dw->chip);
	if (err) {
		dev_err(&pdev->dev, "failed to register dwgpio chip\n");
		return err;
	}

	/* only setup interrupts if required */
	if (dw->gpio_int_en > 0) {
		irqs = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
		if (!irqs) {
			printk(KERN_ERR DRV_NAME ": Could not get GPIO IRQ resource\n");
			rc = -ENODEV;
			return -ENOMEM;
		}

		dw->nr_irq = irqs->start;
		printk(KERN_INFO DRV_NAME ": GPIO IRQ #[%d]\n", dw->nr_irq);

		dwgpio_irq_init(dw);
	}

	platform_set_drvdata(pdev, dw);

#ifdef DEBUG_WITH_LED_ON
	/*
	 * Set LED6 on to indicate that GPIO is initialised. This led is also
	 * used for edge trigger toggles (key press/releas) indicator
	 */
	err = gpio_request_one(GPIO_LED_OUT, GPIOF_OUT_INIT_HIGH, "GPIO INIT");
	if (err) {
		printk(KERN_INFO DRV_NAME ": failed to set led: 0x%x\n",
			GPIO_LED_OUT);
	}
#endif

	printk(KERN_INFO DRV_NAME ":Designware GPIO initialised\n");

	return 0;
}

static int __devexit dwgpio_remove(struct platform_device *pdev)
{
	struct dw_gpio_chip *dw = platform_get_drvdata(pdev);
	int err;

	/* Is interrupts enabled? */
	if (dw->gpio_int_en > 0)
		dwgpio_irq_stop(dw);

	err = gpiochip_remove(&dw->chip);
	if (err) {
		dev_err(&pdev->dev, "failed to remove gpio_chip\n");
		goto out;
	}

	platform_set_drvdata(pdev, NULL);

out:
	return err;
}

static struct platform_driver dwgpio_driver = {
	.probe  = dwgpio_probe,
	.remove = __devexit_p(dwgpio_remove),
	.driver = {
		.owner = THIS_MODULE,
		.name  = "dw_gpio",
	},
};

static int __init dwgpio_init(void)
{
	return platform_driver_register(&dwgpio_driver);
}
module_init(dwgpio_init);

static void __exit dwgpio_exit(void)
{
	platform_driver_unregister(&dwgpio_driver);
}
module_exit(dwgpio_exit);

MODULE_AUTHOR("Jamie Iles, modified by Frank Dols");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Synopsys DesignWare GPIO driver");
