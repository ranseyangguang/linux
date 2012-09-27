/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>

#include <asm/uaccess.h>

#include <plat/memmap.h>
#include <plat/dw_pgu2.h>

/*---------------------------------------------------------------------------*/

#define DRIVER_NAME "dw_pgu2"
#define DRIVER_DESC "Synopsys DisignWare Pixel Graphic Unit driver"
#define ID "DW PGU2 FB"

#define BASIC_ALIGNMENT 8
#define DEFAULT_ALIGN_SIZE 32
#define BUF_ALIGN(x, alignment) ((x) & ~alignment)

/* use double buffer in video if there is enough memory */
#define NUMBER_OF_BUFFERS 2

#define VOLATILE_MEMSET(s, c, n) memset((void *) s, c, n)

DECLARE_MUTEX(dwpgu_sem);

struct dw_pgu2_devdata {
	struct fb_info info;            /* Framebuffer driver info */
	struct known_displays *display; /* Display connected to device */
	struct dw_pgu2 *regs;           /* PGU registers */
	unsigned long fb_virt_start;
	size_t fb_size;
	unsigned long fb_phys;
	unsigned long rgb_size;
	unsigned int displayed_buffer;
};

struct known_displays dw_displays[] = {
	{ /* ARCPGU_DISPTYPE:0 Demo display */
		640,       /* xres */
		480,       /* yres */
		16,        /* bpp  */
		"Philips 640x480 color(16bpp) LCD monitor",
		/* mode control */
		(DWPGU2_CTRL_VS_POL_MASK | DWPGU2_CTRL_HS_POL_MASK),
		(0x0100),  /* clkcontrol */
		(10),      /* left margin */
		(10),      /* upper margin*/
		(0x6f),    /* hsync */
		(0xb),     /* vsync */
		(0),       /* bl_active_high */
		(15000000) /* max_freq */
	},
	{ /* ARCPGU_DISPTYPE:1 Hitachi 640x480 TFT panel */
		640,          /* xres */
		480,          /* yres */
		16,           /* bpp  */
		"Hitachi TX14D11VM1CBA 640x480 colour(16bpp) LCD",
		(0),          /* mode control */
		(0x0102),     /* clkcontrol */
		(10),         /* left margin */
		(10),         /* upper margin*/
		(0x04220180), /* hsync */
		(0x010500fc), /* vsync */
		(0),          /* bl_active_high */
		(12000000)    /* max_freq */
	}
};

static struct fb_fix_screeninfo dw_pgu2fb_fix __devinitdata = {
	.id        = "dw_pgu2",
	.type      = FB_TYPE_PACKED_PIXELS,
	.visual    = FB_VISUAL_PSEUDOCOLOR,
	.xpanstep  = 1,
	.ypanstep  = 1,
	.ywrapstep = 1,
	.accel     = FB_ACCEL_NONE,
};

static struct fb_var_screeninfo dw_pgu2_var __initdata = {
	.xres           = 640,
	.yres           = 480,
	.xoffset		= 10,
	.yoffset		= 10,
	.xres_virtual   = 640,
	.yres_virtual   = 960,
	.bits_per_pixel = 16,
};

static int display_index;
static int nohwcursor;

static struct dw_pgu2_par __initdata current_par;

static struct dw_pgu2_devdata fb_devdata;

/*---------------------------------------------------------------------------*/

static void stop_dw_pgu2(void)
{
	int i;
	printk(KERN_INFO "dw_pgu2 is gone stop now.\n");
	fb_devdata.regs->ctrl &= ~DWPGU2_CTRL_ENABLE_MASK;

	/* Wait for the PGU to stop running */
	while (fb_devdata.regs->stat & DWPGU2_STAT_BUSY_MASK)
		/* do nothing */

	/* And a bit more for good measure */
	for (i = 0; i < 10000; i++)
		printk(KERN_INFO "dw_pgu2 stopping now.\n");
}

static int set_rgb_pointer(struct fb_info *info)
{
	struct dw_pgu2_par *par;
	par = info->par;

	if (par->rgb_bufno > CONFIG_DWPGU2_RGBBUFS)
		return -1;

	fb_devdata.regs->base0 = (unsigned int)	 fb_devdata.fb_virt_start;
	fb_devdata.regs->base1 = (unsigned int)	 fb_devdata.fb_virt_start
		+ (fb_devdata.display->xres * fb_devdata.display->yres
			* (fb_devdata.display->bpp / 8)) ;

	fb_devdata.regs->stride = 0;
	return 0;
}

static void set_color_bitfields(struct fb_var_screeninfo *var)
{
	switch (var->bits_per_pixel) {
	case 16:
	{
		if (current_par.main_is_fb) {
				var->red.offset = 11;
				var->red.length = 5;
				var->green.offset = 5;
				var->green.length = 6;
				var->blue.offset = 0;
				var->blue.length = 5;
				var->transp.offset = 0;
				var->transp.length = 0;
		} else {
			switch (current_par.overlay_mode) {
			case STATCTRL_OL_FMT_RGBA4444:
				var->red.offset = 8;
				var->red.length = 4;
				var->green.offset = 4;
				var->green.length = 4;
				var->blue.offset = 0;
				var->blue.length = 4;
				var->transp.offset = 12;
				var->transp.length = 4;
				break;
			case STATCTRL_OL_FMT_RGBA5551:
				var->red.offset = 10;
				var->red.length = 5;
				var->green.offset = 5;
				var->green.length = 5;
				var->blue.offset = 0;
				var->blue.length = 5;
				var->transp.offset = 15;
				var->transp.length = 1;
				break;
			case STATCTRL_OL_FMT_RGB555:
			default:
				var->red.offset = 10;
				var->red.length = 5;
				var->green.offset = 5;
				var->green.length = 5;
				var->blue.offset = 0;
				var->blue.length = 5;
				var->transp.offset = 0;
				var->transp.length = 0;
				break;
			}
		}
		break;
	}
	default:
		var->red.offset = 0;
		var->red.length = 0;
		var->green.offset = 0;
		var->green.length = 0;
		var->blue.offset = 0;
		var->blue.length = 0;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	}
	var->red.msb_right = 0;
	var->green.msb_right = 0;
	var->blue.msb_right = 0;
	var->transp.msb_right = 0;
}

int dw_pgu2_setmode(struct fb_info *info)
{
	int dispmode;
	int i;
	unsigned long freq;
	unsigned long clk_div;
	unsigned long high;
	unsigned long xfmt, yfmt, act, deact;

	printk(KERN_INFO "dw_pgu2_setmode, lcd index is %d\n", display_index);
	dispmode = current_par.overlay_mode & STATCTRL_OL_FMT_MASK;

	stop_dw_pgu2();

	/* Initialize controller */
	xfmt = info->var.xres + info->var.left_margin ;
	yfmt = info->var.yres + info->var.upper_margin ;

	fb_devdata.regs->fmt = ENCODE_PGU_DIMS(xfmt, yfmt);
	memset((void *)fb_devdata.fb_virt_start, 0, fb_devdata.rgb_size);

	clk_div = 1;
	freq = CONFIG_ARC_PLAT_CLK;

	while (freq/clk_div > fb_devdata.display->max_freq)
		clk_div++;

	clk_div--;
	high = clk_div/2;
	act = info->var.hsync_len;
	deact = info->var.right_margin;
	fb_devdata.regs->hsync = ENCODE_PGU_HSYNC_TIM(deact, act);

	act = info->var.vsync_len;
	deact = info->var.lower_margin;
	fb_devdata.regs->vsync = ENCODE_PGU_VSYNC_TIM(deact, act);

	fb_devdata.regs->frame = ENCODE_PGU_FRAME_TIM(
		info->var.left_margin, info->var.upper_margin);

	printk(KERN_INFO "dw_pgu2_setmode, fbbase 0x%lx, ybase 0x%lx\n",
		fb_devdata.fb_virt_start, fb_devdata.regs->base0);

	set_rgb_pointer(info);
	fb_devdata.regs->ctrl |= (DWPGU2_CTRL_ENABLE_MASK | \
		DWPGU2_CTRL_CONT_MASK);

	for (i = 0; i < 10000; i++)
		/* do nothing */

	/* enable interrupts  */
	set_color_bitfields(&dw_pgu2_var);
	fb_devdata.regs->start_set = 1;
	return 0;
}

/*
 *  Frame buffer operations
 */
static struct fb_ops dw_pgu2fb_ops = {
	.owner			= THIS_MODULE,
	.fb_open		= NULL,
	.fb_read		= NULL,
	.fb_write		= NULL,
	.fb_release		= NULL,
	.fb_check_var	= NULL,
	.fb_set_par		= NULL,
	.fb_setcolreg	= NULL,
	.fb_blank		= NULL,
	.fb_pan_display	= NULL,
	.fb_fillrect	= NULL,  /* cfb_fillrect */
	.fb_copyarea	= NULL,  /* cfb_copyarea */
	.fb_imageblit	= NULL,  /* cfb_imageblit */
	.fb_cursor		= NULL,
	.fb_rotate		= NULL,
	.fb_sync		= NULL,
	.fb_ioctl		= NULL,
	.fb_mmap		= NULL,
};

/* ------------------------------------------------------------------------- */

static int __init dw_pgu2fb_probe(struct platform_device *pdev)
{
	struct dw_pgu2_par *par;
	struct device *device = &pdev->dev;
	struct fb_info *info;
	unsigned long page;

	/*
	 * Dynamically allocate info and par
	 */
	info = framebuffer_alloc(sizeof(struct dw_pgu2_par), device);
	if (!info)
		return -EINVAL;

	if (!pdev)
		return -EINVAL;

	par = info->par;

	/* Get the active display */
	display_index = CONFIG_ARCPGU_DISPTYPE;
	fb_devdata.display = &dw_displays[display_index];
	fb_devdata.regs = (void *)DW_PGU_BASE;

	printk(KERN_INFO "dw_pgu2: controller ID# 0x%lx, using the: %s\n",
				 fb_devdata.regs->module_id ,
				 fb_devdata.display->display_name);

	/* This bit's easy */
	fb_devdata.rgb_size = CONFIG_DWPGU2_RGBBUFS
		* (fb_devdata.display->xres * fb_devdata.display->yres
		* (fb_devdata.display->bpp / 8));

	/*
	 * Allocate default framebuffers from system memory
	 */
	fb_devdata.fb_size = fb_devdata.rgb_size;

	fb_devdata.fb_size += 0x200;
	printk(KERN_INFO "dw_pgu2: display/RGB resolution %03ldx%03ld\n",
				 fb_devdata.display->xres,
				 fb_devdata.display->yres);
	printk(KERN_INFO "framebuffer size: 0x%x, number of buffers: %d\n",
				 fb_devdata.fb_size, CONFIG_DWPGU2_RGBBUFS);

	fb_devdata.fb_virt_start = (unsigned long)
		__get_free_pages(GFP_ATOMIC, get_order(fb_devdata.fb_size +
			DEFAULT_ALIGN_SIZE));
	if (!fb_devdata.fb_virt_start) {
		printk(KERN_WARNING "Unable to allocate fb memory; order = %d\n",
			get_order(fb_devdata.fb_size + DEFAULT_ALIGN_SIZE));
		return -ENOMEM;
	}
	fb_devdata.fb_phys = virt_to_phys((void *)fb_devdata.fb_virt_start);

	/*
	 * Set page reserved so that mmap will work. This is necessary
	 * since we'll be remapping normal memory.
	 */
	for (page = fb_devdata.fb_virt_start;
			page < BUF_ALIGN(fb_devdata.fb_virt_start
				+ fb_devdata.fb_size, DEFAULT_ALIGN_SIZE);
			page += PAGE_SIZE) {
		SetPageReserved(virt_to_page(page));
	}

	memset((void *)fb_devdata.fb_virt_start, 0,
		get_order(fb_devdata.fb_size));

	info->pseudo_palette = kmalloc(sizeof(u32) * 16, GFP_KERNEL);
	if (!info->pseudo_palette)
		return -ENOMEM;
	memset(info->pseudo_palette, 0, sizeof(u32) * 16);

	printk(KERN_INFO "framebuffer at: 0x%lx (logical), 0x%lx (physical)\n",
		fb_devdata.fb_virt_start, fb_devdata.fb_phys);

	dw_pgu2_var.xres = fb_devdata.display->xres;
	dw_pgu2_var.xres_virtual = fb_devdata.display->xres;
	dw_pgu2_var.yres = fb_devdata.display->yres;
	dw_pgu2_var.yres_virtual = fb_devdata.display->yres
		* CONFIG_DWPGU2_RGBBUFS;
	dw_pgu2_var.bits_per_pixel = fb_devdata.display->bpp;

	dw_pgu2_var.left_margin = fb_devdata.display->xoffset;
	dw_pgu2_var.upper_margin = fb_devdata.display->yoffset;

	dw_pgu2_var.hsync_len = fb_devdata.display->hsync;
	dw_pgu2_var.right_margin = (fb_devdata.display->xoffset
		- fb_devdata.display->hsync)>>1;
	dw_pgu2_var.vsync_len = fb_devdata.display->vsync;
	dw_pgu2_var.lower_margin = (fb_devdata.display->yoffset
		- fb_devdata.display->vsync)>>1;

	/* only works for 8/16 bpp */
	current_par.num_rgbbufs = CONFIG_DWPGU2_RGBBUFS;
	current_par.line_length = fb_devdata.display->xres
		* fb_devdata.display->bpp / 8;
	current_par.main_mode = STATCTRL_PIX_FMT_RGB555;
	current_par.overlay_mode = STATCTRL_OL_FMT_RGB555;
	current_par.rgb_bufno = 0;
	current_par.main_is_fb = 1;
	current_par.cmap_len = (fb_devdata.display->bpp == 8) ? 256 : 16;

	/* encode_fix */
	dw_pgu2fb_fix.smem_start = fb_devdata.fb_phys;
	dw_pgu2fb_fix.smem_len = fb_devdata.fb_size;
	dw_pgu2fb_fix.mmio_start = (unsigned long)fb_devdata.regs;
	dw_pgu2fb_fix.mmio_len = sizeof(struct dw_pgu2);
	dw_pgu2fb_fix.visual = (dw_pgu2_var.bits_per_pixel == 8) ? \
		FB_VISUAL_PSEUDOCOLOR : FB_VISUAL_TRUECOLOR;
	dw_pgu2fb_fix.line_length = current_par.line_length;

	info->node = -1;
	info->fbops = &dw_pgu2fb_ops;

	info->flags = FBINFO_FLAG_DEFAULT;

	info->screen_base = (char *) fb_devdata.fb_virt_start;
	info->fix = dw_pgu2fb_fix;
	fb_alloc_cmap(&info->cmap, 256, 0);

	info->fix = dw_pgu2fb_fix;
	info->var = dw_pgu2_var;
	dw_pgu2_setmode(info);

	platform_set_drvdata(pdev, &fb_devdata);
	if (register_framebuffer(info) < 0) {
			fb_dealloc_cmap(&info->cmap);
		return -EINVAL;
	}

	printk(KERN_INFO "fb%d: %s frame buffer device ready for usage.\n",
		MINOR(info->node), info->fix.id);

	return 0;
}

static int __devexit dw_pgu2fb_remove(struct platform_device *pdev)
{
	struct fb_info *info = platform_get_drvdata(pdev);

	if (info) {
		unregister_framebuffer(info);
		fb_dealloc_cmap(&info->cmap);
		/* ... */
		framebuffer_release(info);
	}
	return 0;
}

/* For platform devices */

static struct platform_driver dw_pgu2fb_driver = {
	.probe  = dw_pgu2fb_probe,
	.remove = dw_pgu2fb_remove,
	.driver = {
		.name = "dw_pgu2fb",
	},
};

static struct platform_device *dw_pgu2fb_device;

/*
 * Only necessary if your driver takes special options,
 * otherwise we fall back on the generic fb_setup().
 */

void __init dw_pgu2fb_setup(char *options)
{
	char *this_opt;
	int i;
	int num_dw_displays = sizeof(dw_displays) /
		sizeof(struct known_displays);

	if (!options || !*options)
		return;
    /* Parse user speficied options (`video=dw_pgu2fb:') */
	display_index = 1;

	printk(KERN_INFO "dw_pgu2 parse options: %s\n", options);

	while ((this_opt = strsep(&options, ",")) != NULL) {
		if (!strncmp(this_opt, "display:", 6)) {
			/* Get the display name, everything else if fixed */
			for (i = 0; i < num_dw_displays; i++) {
				if (!strncmp(this_opt + 6,\
					dw_displays[i].display_name,
							strlen(this_opt))) {
					display_index = i;
					break;
				}
			}
		} else {
			if (!strncmp(this_opt, "nohwcursor", 10)) {
				printk(KERN_INFO "nohwcursor\n");
				nohwcursor = 1;
			}
		}
	}
}

static int __init dw_pgu2fb_init(void)
{
	int ret;

	/*
	 *  For kernel boot options (in 'video=dw_pgu2fb:<options>' format)
	 */
	char *option = NULL;

	if (fb_get_options("dw_pgu2fb", &option))
		return -ENODEV;
	dw_pgu2fb_setup(option);

	ret = platform_driver_register(&dw_pgu2fb_driver);

	if (!ret) {
		dw_pgu2fb_device = platform_device_register_simple("dw_pgu2fb",
			0, NULL, 0);

		if (IS_ERR(dw_pgu2fb_device)) {
			platform_driver_unregister(&dw_pgu2fb_driver);
			ret = PTR_ERR(dw_pgu2fb_device);
		}
	}

	return ret;
}

static void __exit dw_pgu2fb_exit(void)
{
	platform_device_unregister(dw_pgu2fb_device);
	platform_driver_unregister(&dw_pgu2fb_driver);
}

/* ------------------------------------------------------------------------- */

module_init(dw_pgu2fb_init);
module_exit(dw_pgu2fb_remove);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Synopsys Frank Dols");
MODULE_DESCRIPTION("Synopsys DW PGU2 framebuffer device driver");
