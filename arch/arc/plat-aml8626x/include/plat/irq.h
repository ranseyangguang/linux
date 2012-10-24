/*
 *  Copyright (C) 2011 AMLOGIC, INC.
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __PLAT_IRQ_H
#define __PLAT_IRQ_H

#define IRQ_BIT(irq)            ((irq) & 0x1f)
#define IRQ_MASK_REG(irq)       (MEDIA_CPU_IRQ_IN0_INTR_MASK + (((irq) >> 5) << 2))
#define IRQ_STATUS_REG(irq)     (MEDIA_CPU_IRQ_IN0_INTR_STAT + (((irq) >> 5) << 2))
#define IRQ_CLR_REG(irq)        (MEDIA_CPU_IRQ_IN0_INTR_STAT_CLR + (((irq) >> 5) << 2))
#define IRQ_FIQSEL_REG(irq)     (MEDIA_CPU_IRQ_IN0_INTR_FIRQ_SEL + (((irq) >> 5) << 2))

/*  ARC in-core IRQ's:   0.. 15
 *  AM ISA GEN (0?)  :  16.. 47
 *  AM ISA GEN1      :  48.. 79
 *  AM ISA GEN2      :  80..111
 *  AM ISA AMRISC    : 112..143
 */ 
#define CORE_IRQ                    (16)
#define AM_ISA_GEN_IRQ(v)           (CORE_IRQ + v)
#define AM_ISA_GEN_IRQ_MAX()        AM_ISA_GEN_IRQ(32)
#define AM_ISA_GEN1_IRQ(v)          (AM_ISA_GEN_IRQ_MAX() + v)
#define AM_ISA_GEN1_IRQ_MAX()       AM_ISA_GEN1_IRQ(32)
#define AM_ISA_GEN2_IRQ(v)          (AM_ISA_GEN1_IRQ_MAX() + v)
#define AM_ISA_GEN2_IRQ_MAX()       AM_ISA_GEN2_IRQ(32)
#define AM_ISA_AMRISC_IRQ(v)        (AM_ISA_GEN2_IRQ_MAX() + v)
#define AM_ISA_AMRISC_IRQ_MAX()     AM_ISA_AMRISC_IRQ(32)
#define AM_ISA_IRQ_MAX              AM_ISA_AMRISC_IRQ(32)

#define NR_IRQS                     AM_ISA_IRQ_MAX

#define TIMER0_INT			3
#define INT_AM_INTC_IRQ			5
#define INT_AM_INTC_FIQ			6

#define INT_WATCHDOG                AM_ISA_GEN_IRQ(0)
#define INT_MAILBOX                 AM_ISA_GEN_IRQ(1)
#define INT_VIU_HSYNC               AM_ISA_GEN_IRQ(2)
#define INT_VIU_VSYNC               AM_ISA_GEN_IRQ(3)
#define INT_DEMUX_1                 AM_ISA_GEN_IRQ(5)
#define INT_TIMER_C                 AM_ISA_GEN_IRQ(6)
#define INT_AUDIO_IN                AM_ISA_GEN_IRQ(7)
#define INT_ETHERNET                AM_ISA_GEN_IRQ(8)
#define INT_A9_SLEEP_RATIO_CHANGE   AM_ISA_GEN_IRQ(9)
#define INT_TIMER_A                 AM_ISA_GEN_IRQ(10)
#define INT_TIMER_B                 AM_ISA_GEN_IRQ(11)
#define INT_REMOTE                  AM_ISA_GEN_IRQ(15)
#define INT_ABUF_WR                 AM_ISA_GEN_IRQ(16)
#define INT_ABUF_RD                 AM_ISA_GEN_IRQ(17)
#define INT_ASYNC_FIFO_FILL         AM_ISA_GEN_IRQ(18)
#define INT_ASYNC_FIFO_FLUSH        AM_ISA_GEN_IRQ(19)
#define INT_BT656                   AM_ISA_GEN_IRQ(20)
#define INT_I2C_MASTER              AM_ISA_GEN_IRQ(21)
#define INT_ENCODER                 AM_ISA_GEN_IRQ(22)
#define INT_DEMUX                   AM_ISA_GEN_IRQ(23)
#define INT_ASYNC_FIFO2_FILL        AM_ISA_GEN_IRQ(24)
#define INT_ASYNC_FIFO2_FLUSH       AM_ISA_GEN_IRQ(25)
#define INT_UART                    AM_ISA_GEN_IRQ(26)
#define INT_SDIO                    AM_ISA_GEN_IRQ(28)
#define INT_TIMER_D                 AM_ISA_GEN_IRQ(29)
#define INT_USB_A                   AM_ISA_GEN_IRQ(30)
#define INT_USB_B                   AM_ISA_GEN_IRQ(31)

#define INT_PARSER                  AM_ISA_GEN1_IRQ(0)
#define INT_VIFF_EMPTY              AM_ISA_GEN1_IRQ(1)
#define INT_NAND                    AM_ISA_GEN1_IRQ(2)
#define INT_SPDIF                   AM_ISA_GEN1_IRQ(3)
#define INT_NDMA                    AM_ISA_GEN1_IRQ(4)
#define INT_SMART_CARD              AM_ISA_GEN1_IRQ(5)
#define INT_I2C_MCLK                AM_ISA_GEN1_IRQ(6)
#define INT_I2C_SLAVE               AM_ISA_GEN1_IRQ(7)
#define INT_MAILBOX_2B              AM_ISA_GEN1_IRQ(8)
#define INT_MAILBOX_1B              AM_ISA_GEN1_IRQ(9)
#define INT_MAILBOX_0B              AM_ISA_GEN1_IRQ(10)
#define INT_MAILBOX_2A              AM_ISA_GEN1_IRQ(11)
#define INT_MAILBOX_1A              AM_ISA_GEN1_IRQ(12)
#define INT_MAILBOX_0A              AM_ISA_GEN1_IRQ(13)
#define INT_DEINTERLACE             AM_ISA_GEN1_IRQ(14)
#define INT_MMC                     AM_ISA_GEN1_IRQ(15)
#define INT_DEMUX_2                 AM_ISA_GEN1_IRQ(21)
#define INT_ARC_SLEEP_RATIO         AM_ISA_GEN1_IRQ(22)
#define INT_HDMI_CEC                AM_ISA_GEN1_IRQ(23)
#define INT_HDMI_TX                 AM_ISA_GEN1_IRQ(25)
#define INT_A9_PMU                  AM_ISA_GEN1_IRQ(26)
#define INT_A9_DEBUG_TX             AM_ISA_GEN1_IRQ(27)
#define INT_A9_DEBUG_RX             AM_ISA_GEN1_IRQ(28)
#define INT_A9_L2_CC                AM_ISA_GEN1_IRQ(29)
#define INT_SATA                    AM_ISA_GEN1_IRQ(30)
#define INT_DEMOD                   AM_ISA_GEN1_IRQ(31)

#define INT_GPIO_0                  AM_ISA_GEN2_IRQ(0)
#define INT_GPIO_1                  AM_ISA_GEN2_IRQ(1)
#define INT_GPIO_2                  AM_ISA_GEN2_IRQ(2)
#define INT_GPIO_3                  AM_ISA_GEN2_IRQ(3)
#define INT_GPIO_4                  AM_ISA_GEN2_IRQ(4)
#define INT_GPIO_5                  AM_ISA_GEN2_IRQ(5)
#define INT_GPIO_6                  AM_ISA_GEN2_IRQ(6)
#define INT_GPIO_7                  AM_ISA_GEN2_IRQ(7)
#define INT_RTC                     AM_ISA_GEN2_IRQ(8)
#define INT_SAR_ADC                 AM_ISA_GEN2_IRQ(9)
#define INT_I2C_MASTER_1            AM_ISA_GEN2_IRQ(10)
#define INT_UART_1                  AM_ISA_GEN2_IRQ(11)
#define INT_LED_PWM                 AM_ISA_GEN2_IRQ(12)
#define INT_VGHL_PWM                AM_ISA_GEN2_IRQ(13)
#define INT_WIFI_WATCHDOG           AM_ISA_GEN2_IRQ(14)
#define INT_VIDEO_WR                AM_ISA_GEN2_IRQ(15)
#define INT_SPI                     AM_ISA_GEN2_IRQ(16)
#define INT_SPI_1                   AM_ISA_GEN2_IRQ(17)
#define INT_VDIN_HSYNC              AM_ISA_GEN2_IRQ(18)
#define INT_VDIN_VSYNC              AM_ISA_GEN2_IRQ(19)

#define INT_AMRISC_DC_PCMLAST       AM_ISA_AMRISC_IRQ(0)
#define INT_AMRISC_VIU_VSYNC        AM_ISA_AMRISC_IRQ(1)
#define INT_AMRISC_H2TMR            AM_ISA_AMRISC_IRQ(3)
#define INT_AMRISC_H2CPAR           AM_ISA_AMRISC_IRQ(4)
#define INT_AMRISC_HI_ABX           AM_ISA_AMRISC_IRQ(5)
#define INT_AMRISC_H2CMD            AM_ISA_AMRISC_IRQ(6)
#define INT_AMRISC_AI_IEC958        AM_ISA_AMRISC_IRQ(7)
#define INT_AMRISC_VL_CP            AM_ISA_AMRISC_IRQ(8)
#define INT_AMRISC_DC_MBDONE        AM_ISA_AMRISC_IRQ(9)
#define INT_AMRISC_VIU_HSYNC        AM_ISA_AMRISC_IRQ(10)
#define INT_AMRISC_R2C              AM_ISA_AMRISC_IRQ(11)
#define INT_AMRISC_AIFIFO           AM_ISA_AMRISC_IRQ(13)
#define INT_AMRISC_HST_INTP         AM_ISA_AMRISC_IRQ(14)
#define INT_AMRISC_CPU1_STOP        AM_ISA_AMRISC_IRQ(16)
#define INT_AMRISC_CPU2_STOP        AM_ISA_AMRISC_IRQ(17)
#define INT_AMRISC_VENC_INT         AM_ISA_AMRISC_IRQ(19)
#define INT_AMRISC_TIMER0           AM_ISA_AMRISC_IRQ(26)
#define INT_AMRISC_TIMER1           AM_ISA_AMRISC_IRQ(27)

#endif
