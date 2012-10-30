/*
 * Amlogic Apollo
 * frame buffer driver
 *
 * Copyright (C) 2009 Amlogic, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the named License,
 * or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Author:  Tim Yao <timyao@amlogic.com>
 *
 */

#ifndef APOLLO_MAIN_H
#define APOLLO_MAIN_H
#include  <linux/list.h>

#include  <plat/vout/vout_notify.h>
#include "apollofbdev.h"

#if  CONFIG_FB_APOLLO_OSD2_ENABLE
#define   OSD_COUNT  2
#else
#define   OSD_COUNT  1
#endif 

#define HVGA_EXPERIMENT
#ifdef  HVGA_EXPERIMENT
#define _XX 640 /* HVGA portrait */
#define _YY 416
#endif

#if 1 /* cj: 480i defaults at first -- regular NTSC TV set */
static struct fb_var_screeninfo mydef_var[] = {
{
#ifndef HVGA_EXPERIMENT
	.xres            = 640,
	.yres            = 480,
	.xres_virtual    = 640,
	.yres_virtual    = 480,
#else
	.xres            = _XX,
	.yres            = _YY,
	.xres_virtual    = _XX,
	.yres_virtual    = _YY,
#endif
	.xoffset         = 0,
	.yoffset         = 0,
	.bits_per_pixel  = 16,
	.grayscale       = 0,
	.red             = {11, 5, 0},
	.green           = {5, 6, 0},
	.blue            = {0, 5, 0},
	.transp          = {0, 0, 0},
	.nonstd          = 0,
	.activate        = FB_ACTIVATE_NOW,
	.height          = -1,
	.width           = -1,
	.accel_flags     = 0,
	.pixclock        = 0,
	.left_margin     = 0,
	.right_margin    = 0,
	.upper_margin    = 0,
	.lower_margin    = 0,
	.hsync_len       = 0,
	.vsync_len       = 0,
	.sync            = 0,
	.vmode           = FB_VMODE_NONINTERLACED,
	.rotate          = 0,
	
},
#if  CONFIG_FB_APOLLO_OSD2_ENABLE

{
	.xres            = 640,
	.yres            = 480,
	.xres_virtual    = 640,
	.yres_virtual    = 480,
	.xoffset         = 0,
	.yoffset         = 0,
	.bits_per_pixel  = 16,
	.grayscale       = 0,
	.red             = {11, 5, 0},
	.green           = {5, 6, 0},
	.blue            = {0, 5, 0},
	.transp          = {0, 0, 0},
	.nonstd          = 0,
	.activate        = FB_ACTIVATE_NOW,
	.height          = -1,
	.width           = -1,
	.accel_flags     = 0,
	.pixclock        = 0,
	.left_margin     = 0,
	.right_margin    = 0,
	.upper_margin    = 0,
	.lower_margin    = 0,
	.hsync_len       = 0,
	.vsync_len       = 0,
	.sync            = 0,
	.vmode           = FB_VMODE_NONINTERLACED,
	.rotate          = 0,
}
#endif 
};
#else
static struct fb_var_screeninfo mydef_var[] = {
{
	.xres            = 1280,
	.yres            = 720,
	.xres_virtual    = 1280,
	.yres_virtual    = 720,
	.xoffset         = 0,
	.yoffset         = 0,
	.bits_per_pixel  = 32,
	.grayscale       = 0,
	.red             = {0, 8, 0},
	.green           = {0, 8, 0},
	.blue            = {0, 8, 0},
	.transp          = {0, 0, 0},
	.nonstd          = 0,
	.activate        = FB_ACTIVATE_NOW,
	.height          = -1,
	.width           = -1,
	.accel_flags     = 0,
	.pixclock        = 0,
	.left_margin     = 0,
	.right_margin    = 0,
	.upper_margin    = 0,
	.lower_margin    = 0,
	.hsync_len       = 0,
	.vsync_len       = 0,
	.sync            = 0,
	.vmode           = FB_VMODE_NONINTERLACED,
	.rotate          = 0,
	
},
#if  CONFIG_FB_APOLLO_OSD2_ENABLE

{
	.xres            = 1280,
	.yres            = 720,
	.xres_virtual    = 1280,
	.yres_virtual    = 720,
	.xoffset         = 0,
	.yoffset         = 0,
	.bits_per_pixel  = 32,
	.grayscale       = 0,
	.red             = {0, 8, 0},
	.green           = {0, 8, 0},
	.blue            = {0, 8, 0},
	.transp          = {0, 0, 0},
	.nonstd          = 0,
	.activate        = FB_ACTIVATE_NOW,
	.height          = -1,
	.width           = -1,
	.accel_flags     = 0,
	.pixclock        = 0,
	.left_margin     = 0,
	.right_margin    = 0,
	.upper_margin    = 0,
	.lower_margin    = 0,
	.hsync_len       = 0,
	.vsync_len       = 0,
	.sync            = 0,
	.vmode           = FB_VMODE_NONINTERLACED,
	.rotate          = 0,
}
#endif 
};
#endif
typedef   struct {
	vmode_t		vmode;
	u32			disp_start_x;
	u32			disp_start_y;
}disp_offset_t;
static  disp_offset_t   disp_offset[]={
{
	.vmode=VMODE_480P,
	.disp_start_x=0,
	.disp_start_y=0,
},
{
	.vmode=VMODE_480I,
#ifdef HVGA_EXPERIMENT
	.disp_start_x=40+(640-_XX)>>1, /* cj: offset in by 40 */
	.disp_start_y=(480-_YY)>>2,
#else
	.disp_start_x=40, /* cj: offset in by 40 */
	.disp_start_y=0,
#endif
},
{
	.vmode=VMODE_640_480I_PAL,
#ifdef HVGA_EXPERIMENT
	.disp_start_x=(640-_XX)>>1,
	.disp_start_y=(480-_YY)>>2,
#else
	.disp_start_x=0,
	.disp_start_y=0,
#endif
},
{
	.vmode=VMODE_576P,
	.disp_start_x=0,
	.disp_start_y=0,
},	
{
	.vmode=VMODE_576I,
	.disp_start_x=0,
	.disp_start_y=0,
},
{
	.vmode=VMODE_720P,
	.disp_start_x=0,
	.disp_start_y=0,
},
{
	.vmode=VMODE_1080P,
	.disp_start_x=0,
	.disp_start_y=0,
},
{
	.vmode=VMODE_1080I,
	.disp_start_x=0,
	.disp_start_y=0,
},
{
	.vmode=VMODE_LCD,
	.disp_start_x=0,
	.disp_start_y=0,
},
};

static struct fb_fix_screeninfo mydef_fix = {
	.id		    = "Apollo FB",
	.xpanstep 	= 1,
	.ypanstep 	= 1,
	.type		= FB_TYPE_PACKED_PIXELS,
	.visual		= FB_VISUAL_TRUECOLOR,
	.accel		= FB_ACCEL_NONE,
};

#endif /* APOLLO_MAIN_H */
