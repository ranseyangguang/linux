/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _ASM_DW_PGU2_H
#define _ASM_DW_PGU2_H

#include <linux/fb.h>

/*---------------------------------------------------------------------------*/

#define DMT_75_VSYNC 0x0009000b
#define DMT_75_HSYNC 0x0019006f

/*---------------------------------------------------------------------------*/

/* Register definitions */

#define DWPGU2_CTRL_CONT_MASK      (0x1)
#define DWPGU2_CTRL_ENABLE_MASK    (0x2)

#define DWPGU2_CTRL_FORMAT_MASK    (0x4)
#define DWPGU2_CTRL_VS_POL_MASK    (0x8)
#define DWPGU2_CTRL_HS_POL_MASK    (0x10)
#define DWPGU2_CTRL_DE_POL_MASK    (0x20)
#define DWPGU2_CTRL_CLK_POL_MASK   0x40)
#define DWPGU2_CTRL_CLK_HIGH_MASK  (0xff00)
#define DWPGU2_CTRL_CLK_DIV_MASK   (0xff0000)

#define DWPGU2_STAT_ERR_MASK	   (0x1)
#define DWPGU2_STAT_BUSY_MASK	   (0x2)
#define DWPGU2_STAT_PENDING_MASK   (0x4)
#define DWPGU2_STAT_START_MASK	   (0x18)

#define DWPGU2_FMT_X_RES_MASK      0xffff
#define DWPGU2_FMT_Y_RES_MASK      0xffff0000
#define DWPGU2_FMT_Y_SHIFT         16
#define PGU_MODULE_ID_OFFSET       0x3fc
#define PGU_DISPLAY_TYPE_MASK      0x1000000

#define CLK_CFG_REG_OFFSET         0x8
#define CLK_CFG_REG_CLK_DIV_MASK   0xff
#define CLK_CFG_REG_CLK_HIGH_MASK  0xff0000

#define GET_CLK_CFG_REG_CLK_HIGH_VAL(x) ((x & CLK_CFG_REG_CLK_HIGH_MASK) >> 16)
#define SET_CLK_CFG_REG_CLK_HIGH_VAL(x, y) ((x & !CLK_CFG_REG_CLK_HIGH_MASK) |\
		((y << 16) & CLK_CFG_REG_CLK_HIGH_MASK))

#define DISP_DIM_REG_PGU_X_RES_MASK 0x3ff0000
#define DISP_DIM_REG_PGU_Y_RES_MASK 0x3ff
#define ENCODE_PGU_DIMS(x, y) (((x * 0x10000) & DISP_DIM_REG_PGU_X_RES_MASK)\
	| (y & DISP_DIM_REG_PGU_Y_RES_MASK))

#define HSYNC_TIM_REG_PGU_ACT_MASK 0x3ff0000
#define HSYNC_TIM_REG_PGU_DEACT_MASK 0x3ff
#define ENCODE_PGU_HSYNC_TIM(x, y) (((x * 0x10000) &\
	HSYNC_TIM_REG_PGU_ACT_MASK) | (y & HSYNC_TIM_REG_PGU_DEACT_MASK))

#define VSYNC_TIM_REG_PGU_ACT_MASK 0x3ff0000
#define VSYNC_TIM_REG_PGU_DEACT_MASK 0x3ff
#define ENCODE_PGU_VSYNC_TIM(x, y) (((x * 0x10000) &\
	VSYNC_TIM_REG_PGU_ACT_MASK) | (y & VSYNC_TIM_REG_PGU_DEACT_MASK))

#define PGU_FRAME_REG_HACT_MASK 0x3ff0000
#define PGU_FRAME_REG_VACT_MASK 0x3ff
#define ENCODE_PGU_FRAME_TIM(x, y) (((x * 0x10000) & PGU_FRAME_REG_HACT_MASK)\
	| (y & PGU_FRAME_REG_VACT_MASK))

#define PGU_SYNC_REGS_CYC_MASK     0xfff
#define PGU_SYNC_REGS_PORCH_MASK   0xff0000
#define PGU_SYNC_REGS_WIDTH_MASK   0xff000000
#define PGU_SYNC_REGS_PORCH_OFFSET 16
#define PGU_SYNC_REGS_WIDTH_OFFSET 24

#define HSYNC_CFG_REG_OFFSET       0x18
#define VSYNC_CFG_REG_OFFSET       0x20
#define FRMSTART_REG_OFFSET        0x28
#define STRIDE_REG_OFFSET          0x30

#define FRM_DIM_REG_OFFSET         0x38
#define FRM_DIM_REG_FRM_WTH_MASK   0x7ff
#define FRM_DIM_REG_FRM_LNS_MASK   0x7ff

#define STATCTRL_REG_OFFSET        0x40
#define STATCTRL_REG_DISP_EN_MASK  1
#define STATCTRL_INTR_EN_MASK      2
#define STATCTRL_INTR_CLR_MASK     4
#define STATCTRL_PIX_FMT_MASK      0x30
#define STATCTRL_PIX_FMT_OFFSET    4
#define STATCTRL_PIX_FMT_RGB555    0
#define STATCTRL_PIX_FMT_YUV420    0x10
#define STATCTRL_BACKLIGHT_MASK    0x40
#define STATCTRL_X2_MASK           0x80
#define STATCTRL_REV_LN_SCAN_MASK  0x100
#define STATCTRL_OL_EN_MASK        0x1000
#define STATCTRL_OL_FMT_MASK       0x6000
#define STATCTRL_OL_FMT_OFFSET     13
#define STATCTRL_OL_FMT_RGBA4444   0
#define STATCTRL_OL_FMT_RGBA5551   0x2000
#define STATCTRL_OL_FMT_RGB555     0x4000
#define STATCTRL_DISP_BUSY_MASK    0x10000
#define STATCTRL_BU_BUSY_MASK      0x20000
#define STATCTRL_INTR_FLAG         0x40000
#define STATCTRL_IL_MODE_MASK      0x180000
#define STATCTRL_IL_NONE           0
#define STATCTRL_IL_FRAME          0x80000
#define STATCTRL_IL_EVEN           0x100000
#define STATCTRL_IL_ODD            0x180000

/*---------------------------------------------------------------------------*/

struct dw_pgu2 {
	unsigned long  ctrl;
	unsigned long  stat;
	unsigned long  padding1[2];
	unsigned long  fmt;
	unsigned long  hsync;
	unsigned long  vsync;
	unsigned long  frame;
	unsigned long  padding2[8];
	unsigned long  base0;
	unsigned long  base1;
	unsigned long  base2;
	unsigned long  padding3[1];
	unsigned long  stride;
	unsigned long  padding4[11];
	unsigned long  start_clr;
	unsigned long  start_set;
	unsigned long  padding5[206];
	unsigned long  int_en_clr;
	unsigned long  int_en_set;
	unsigned long  int_en;
	unsigned long  padding6[5];
	unsigned long  int_stat_clr;
	unsigned long  int_stat_set;
	unsigned long  int_stat;
	unsigned long  padding7[4];
	unsigned long  module_id;
};

struct known_displays {
	unsigned long  xres;
	unsigned long  yres;
	unsigned long  bpp;
	unsigned char  display_name[256];
	unsigned long  control;
	unsigned long  clkcontrol;
	unsigned long  xoffset;
	unsigned long  yoffset;
	unsigned long  hsync;
	unsigned long  vsync;
	unsigned char  bl_active_high;
	unsigned long  max_freq;
};

struct dw_pgu2_par {
	int            line_length;
	int            cmap_len;
	unsigned long  main_mode;
	unsigned long  overlay_mode;
	int            num_rgbbufs;
	int            rgb_bufno;
	int            main_is_fb;
};

#endif	/* _ASM_DW_PGU2_H */
