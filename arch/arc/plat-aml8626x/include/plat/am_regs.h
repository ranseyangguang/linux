/*******************************************************************
 *
 *  Copyright C 2005 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: base  register definitions for nike.
 *
 *  Author: zhouzhi
 *  Created: 2008-8-10
 *
 *******************************************************************/

#ifndef __AMLOGIC_REGS_
#define __AMLOGIC_REGS_

#include <plat/regs.h>
#include <plat/isahelper.h>
#include <plat/pehelper.h>
#include <asm/io.h>
#include "c_stb_define.h" //yvonne added
#include "dmc.h"

#define IO_READ32(x)	ioread32((void __iomem *) (x))
#define IO_WRITE32(x,y)	iowrite32((x),(void __iomem *) (y))
#define IO_CBUS_BASE            0xc1100000
#define IO_AXI_BUS_BASE         0xc1300000
#define IO_AHB_BUS_BASE         0xc9000000
#define IO_APB_BUS_BASE         0xd0040000
#define ISABASE         (IO_CBUS_BASE)
#define IO_ETH_BASE     (0xC9410000)

#define MESON_PERIPHS1_VIRT_BASE    0xc1108400
#define MESON_PERIPHS1_PHYS_BASE    0xc1108400

#define CBUS_REG_OFFSET(reg) ((reg) << 2)
#define CBUS_REG_ADDR(reg)   (IO_CBUS_BASE + CBUS_REG_OFFSET(reg))

#define AXI_REG_OFFSET(reg)  ((reg) << 2)
#define AXI_REG_ADDR(reg)    (IO_AXI_BUS_BASE + AXI_REG_OFFSET(reg))

#define AHB_REG_OFFSET(reg)  ((reg) << 2)
#define AHB_REG_ADDR(reg)    (IO_AHB_BUS_BASE + AHB_REG_OFFSET(reg))

#define APB_REG_OFFSET(reg)  (reg)
#define APB_REG_ADDR(reg)    (IO_APB_BUS_BASE + APB_REG_OFFSET(reg))
#define APB_REG_ADDR_VALID(reg) (((unsigned long)(reg) & 3) == 0)

#define WRITE_CBUS_REG(reg, val) IO_WRITE32(val, CBUS_REG_ADDR(reg))
#define READ_CBUS_REG(reg) (IO_READ32(CBUS_REG_ADDR(reg)))
#define WRITE_CBUS_REG_BITS(reg, val, start, len) \
    WRITE_CBUS_REG(reg, (READ_CBUS_REG(reg) & ~(((1L<<(len))-1)<<(start)) )| ((unsigned)((val)&((1L<<(len))-1)) << (start)))
#define READ_CBUS_REG_BITS(reg, start, len) \
    ((READ_CBUS_REG(reg) >> (start)) & ((1L<<(len))-1))
#define CLEAR_CBUS_REG_MASK(reg, mask) WRITE_CBUS_REG(reg, (READ_CBUS_REG(reg)&(~(mask))))
#define SET_CBUS_REG_MASK(reg, mask)   WRITE_CBUS_REG(reg, (READ_CBUS_REG(reg)|(mask)))

#define WRITE_AXI_REG(reg, val) IO_WRITE32(val, AXI_REG_ADDR(reg))
#define READ_AXI_REG(reg) (IO_READ32(AXI_REG_ADDR(reg)))
#define WRITE_AXI_REG_BITS(reg, val, start, len) \
    WRITE_AXI_REG(reg,  (READ_AXI_REG(reg) & ~(((1L<<(len))-1)<<(start)) )| ((unsigned)((val)&((1L<<(len))-1)) << (start)))
#define READ_AXI_REG_BITS(reg, start, len) \
    ((READ_AXI_REG(reg) >> (start)) & ((1L<<(len))-1))
#define CLEAR_AXI_REG_MASK(reg, mask) WRITE_AXI_REG(reg, (READ_AXI_REG(reg)&(~(mask))))
#define SET_AXI_REG_MASK(reg, mask)   WRITE_AXI_REG(reg, (READ_AXI_REG(reg)|(mask)))

#define WRITE_AHB_REG(reg, val) IO_WRITE32(val, AHB_REG_ADDR(reg))
#define READ_AHB_REG(reg) (IO_READ32(AHB_REG_ADDR(reg)))
#define WRITE_AHB_REG_BITS(reg, val, start, len) \
    WRITE_AHB_REG(reg,  (READ_AHB_REG(reg) & ~(((1L<<(len))-1)<<(start)) )| ((unsigned)((val)&((1L<<(len))-1)) << (start)))
#define READ_AHB_REG_BITS(reg, start, len) \
    ((READ_AHB_REG(reg) >> (start)) & ((1L<<(len))-1))
#define CLEAR_AHB_REG_MASK(reg, mask) WRITE_AHB_REG(reg, (READ_AHB_REG(reg)&(~(mask))))
#define SET_AHB_REG_MASK(reg, mask)   WRITE_AHB_REG(reg, (READ_AHB_REG(reg)|(mask)))

#define WRITE_APB_REG(reg, val) IO_WRITE32(val, APB_REG_ADDR(reg))
#define READ_APB_REG(reg) (IO_READ32(APB_REG_ADDR(reg)))
#define WRITE_APB_REG_BITS(reg, val, start, len) \
    WRITE_APB_REG(reg,  (READ_APB_REG(reg) & ~(((1L<<(len))-1)<<(start)) )| ((unsigned)((val)&((1L<<(len))-1)) << (start)))
#define READ_APB_REG_BITS(reg, start, len) \
    ((READ_APB_REG(reg) >> (start)) & ((1L<<(len))-1))
#define CLEAR_APB_REG_MASK(reg, mask) WRITE_APB_REG(reg, (READ_APB_REG(reg)&(~(mask))))
#define SET_APB_REG_MASK(reg, mask)   WRITE_APB_REG(reg, (READ_APB_REG(reg)|(mask)))

/* for back compatible alias */
#define WRITE_MPEG_REG(reg, val) \
    WRITE_CBUS_REG(reg, val)
#define READ_MPEG_REG(reg) \
    READ_CBUS_REG(reg)
#define WRITE_MPEG_REG_BITS(reg, val, start, len) \
    WRITE_CBUS_REG_BITS(reg, val, start, len)
#define READ_MPEG_REG_BITS(reg, start, len) \
    READ_CBUS_REG_BITS(reg, start, len)
#define CLEAR_MPEG_REG_MASK(reg, mask) \
    CLEAR_CBUS_REG_MASK(reg, mask)
#define SET_MPEG_REG_MASK(reg, mask) \
    SET_CBUS_REG_MASK(reg, mask)

#define AUD_ARC_CTL                                0x2659//yvonne added
#define READ_VOLATILE_UINT32(r) IO_READ32(r)
#define AML_A1H

#define IO_USB_A_BASE			0xC9040000
#define IO_USB_B_BASE			0xC90C0000

// ----------------------------
// $usb/rtl/usb_reg.v   (8)
// ----------------------------
#define PREI_USB_PHY_REG              0x2100 //0xC1108400
#define PREI_USB_PHY_A_REG1           0x2101
#define PREI_USB_PHY_B_REG1           0x2102
#define PREI_USB_PHY_A_REG3           0x2103
#define PREI_USB_PHY_B_REG4           0x2104

#define PREI_USB_PHY_A_POR      (1 << 0)
#define PREI_USB_PHY_B_POR      (1 << 1)
#define PREI_USB_PHY_CLK_SEL    (7 << 5) 
#define PREI_USB_PHY_CLK_GATE 	(1 << 8) 
#define PREI_USB_PHY_B_AHB_RSET     (1 << 11)
#define PREI_USB_PHY_B_CLK_RSET     (1 << 12)
#define PREI_USB_PHY_B_PLL_RSET     (1 << 13)
#define PREI_USB_PHY_A_AHB_RSET     (1 << 17)
#define PREI_USB_PHY_A_CLK_RSET     (1 << 18)
#define PREI_USB_PHY_A_PLL_RSET     (1 << 19)
#define PREI_USB_PHY_A_DRV_VBUS     (1 << 20)
#define PREI_USB_PHY_B_DRV_VBUS			(1 << 21)
#define PREI_USB_PHY_B_CLK_DETECT   (1 << 22)
#define PREI_USB_PHY_CLK_DIV        (0x7f << 24)
#define PREI_USB_PHY_A_CLK_DETECT   (1 << 31)

#define PREI_USB_PHY_MODE_MASK   (3 << 22)

#define PREI_USB_PHY_MODE_HW        (0<<22)
#define PREI_USB_PHY_MODE_SW_HOST      (2<<22)
#define PREI_USB_PHY_MODE_SW_SLAVE        (3 << 22)

#define USB_PHY_TUNE_MASK_REFCLKDIV  (3 << 29)
#define USB_PHY_TUNE_MASK_REFCLKSEL  (3 << 27 )
#define USB_PHY_TUNE_MASK_SQRX          (7 << 16 )
#define USB_PHY_TUNE_MASK_TXVREF       (15 << 5)
#define USB_PHY_TUNE_MASK_OTGDISABLE    (1 << 2)
#define USB_PHY_TUNE_MASK_RISETIME  (3 << 9 )
#define USB_PHY_TUNE_MASK_VBUS_THRE (7 << 19)

#define USB_PHY_TUNE_SHIFT_REFCLKDIV  (29)
#define USB_PHY_TUNE_SHIFT_REFCLKSEL  (27)
#define USB_PHY_TUNE_SHIFT_SQRX          (16)
#define USB_PHY_TUNE_SHIFT_TXVREF       (5)
#define USB_PHY_TUNE_SHIFT_OTGDISABLE    (2)
#define USB_PHY_TUNE_SHIFT_RISETIME  (9)
#define USB_PHY_TUNE_SHIFT_VBUS_THRE (19)

#define	USB_PHY_CLOCK_SEL_XTAL (0)
#define	USB_PHY_CLOCK_SEL_XTAL_DIV2 (1)
#define	USB_PHY_CLOCK_SEL_OTHER_PLL (2)
#define	USB_PHY_CLOCK_SEL_DDR_PLL (3)
#define	USB_PHY_CLOCK_SEL_DEMOD_PLL (4)

/*==================
*yvonne added for A3 
*2011-03-16
================*/
#define VD1_IF0_GEN_REG                            0x1a50
#define VDIF_RESET_ON_GO_FIELD		 (1<<29)

/*==================
*yvonne ending added for A3 
*2011-03-16
================*/
#endif
