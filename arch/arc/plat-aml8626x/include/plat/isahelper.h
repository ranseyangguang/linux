/*******************************************************************
 *
 *  Copyright C 2008 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description:
 *      ISA register helper defines
 *  Author: Amlogic Software
 *  Created: 10/17/2008 3:31:39 PM
 *  modified  history:
 *  add asm for clear isa mask : jianfeng_wang
 *
 *******************************************************************/
#ifndef ISAHELPER_H
#define ISAHELPER_H

/* DMA Routines **********************************************/

#define ide_dma_started()                                               \
    ((READ_CBUS_REG(ATAPI_IDE_REG0)&IDE_DMA_START) == IDE_DMA_START)

#define ide_dma_is_busy()                                               \
    ((READ_CBUS_REG(ATAPI_IDE_REG0)&IDE_DMA_STATUS_MASK) == IDE_DMA_IN_PROGRESS)

#define set_ide_dma_address(addr)                       \
    WRITE_CBUS_REG(ATAPI_IDE_TABLE, (unsigned) addr)

#define read_next_ide_dma_address()                       \
    READ_CBUS_REG(ATAPI_IDE_TABLE)

#define add_ide_dma_table(num)                       \
    WRITE_CBUS_REG(ATAPI_TABLE_ADD_REG,num)

#define read_ide_dma_table_remain()                       \
    READ_CBUS_REG(ATAPI_TABLE_ADD_REG)

#define halt_ide_dma() \
    CLEAR_CBUS_REG_MASK(ATAPI_IDE_REG0, IDE_DMA_START)

#define start_ide_dma()                                 \
    SET_CBUS_REG_MASK(ATAPI_IDE_REG0, IDE_DMA_START)

/* Interrupt routines ******************************************/
#define set_irq_mask(mask)                      \
    SET_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN0_INTR_MASK, mask)
#define set_irq_mask1(mask)                      \
    SET_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN1_INTR_MASK, mask)
#define set_irq_mask2(mask)                      \
    SET_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN2_INTR_MASK, mask)

#define get_irq_mask()                          \
    READ_CBUS_REG(SYS_CPU_0_IRQ_IN0_INTR_MASK)
#define get_irq_mask1()                          \
    READ_CBUS_REG(SYS_CPU_0_IRQ_IN1_INTR_MASK)
#define get_irq_mask2()                          \
    READ_CBUS_REG(SYS_CPU_0_IRQ_IN2_INTR_MASK)

#define clear_irq_mask(mask)                    \
    CLEAR_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN0_INTR_MASK, mask)
#define clear_irq_mask1(mask)                    \
    CLEAR_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN1_INTR_MASK, mask)
#define clear_irq_mask2(mask)                    \
    CLEAR_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN2_INTR_MASK, mask)

#define set_firq_mask(mask)                     \
    SET_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN0_INTR_FIRQ_SEL, mask)
#define set_firq_mask1(mask)                     \
    SET_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN1_INTR_FIRQ_SEL, mask)
#define set_firq_mask2(mask)                     \
    SET_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN2_INTR_FIRQ_SEL, mask)

#define get_firq_mask()                     \
    READ_CBUS_REG(SYS_CPU_0_IRQ_IN0_INTR_FIRQ_SEL)
#define get_firq_mask1()                     \
    READ_CBUS_REG(SYS_CPU_0_IRQ_IN1_INTR_FIRQ_SEL)
#define get_firq_mask2()                     \
    READ_CBUS_REG(SYS_CPU_0_IRQ_IN2_INTR_FIRQ_SEL)

#define clear_firq_mask(mask)                       \
    CLEAR_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN0_INTR_FIRQ_SEL, mask)
#define clear_firq_mask1(mask)                       \
    CLEAR_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN1_INTR_FIRQ_SEL, mask)
#define clear_firq_mask2(mask)                       \
    CLEAR_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN2_INTR_FIRQ_SEL, mask)

#define read_irq_status()                       \
    READ_CBUS_REG(SYS_CPU_0_IRQ_IN0_INTR_STAT)
#define read_irq_status1()                       \
    READ_CBUS_REG(SYS_CPU_0_IRQ_IN1_INTR_STAT)
#define read_irq_status2()                       \
    READ_CBUS_REG(SYS_CPU_0_IRQ_IN2_INTR_STAT)

#define clear_irq_status(mask)                  \
    WRITE_CBUS_REG(SYS_CPU_0_IRQ_IN0_INTR_STAT_CLR, mask)
#define clear_irq_status1(mask)                  \
    WRITE_CBUS_REG(SYS_CPU_0_IRQ_IN1_INTR_STAT_CLR, mask)
#define clear_irq_status2(mask)                  \
    WRITE_CBUS_REG(SYS_CPU_0_IRQ_IN2_INTR_STAT_CLR, mask)

/* AMRISC Interrupt routines ***********************************/
#define set_amrisc_irq_mask(mask)                   \
    SET_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN3_INTR_MASK, mask)
#define get_amrisc_irq_mask()                   \
    READ_CBUS_REG(SYS_CPU_0_IRQ_IN3_INTR_MASK)
#define clear_amrisc_irq_mask(mask)                 \
    CLEAR_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN3_INTR_MASK, mask)
#define set_amrisc_firq_mask(mask)                  \
    SET_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN3_INTR_FIRQ_SEL, mask)
#define get_amrisc_firq_mask()                  \
    READ_CBUS_REG(SYS_CPU_0_IRQ_IN3_INTR_FIRQ_SEL)
#define clear_amrisc_firq_mask(mask)                    \
    CLEAR_CBUS_REG_MASK(SYS_CPU_0_IRQ_IN3_INTR_FIRQ_SEL, mask)
#define read_amrisc_irq_status()                \
    READ_CBUS_REG(SYS_CPU_0_IRQ_IN3_INTR_STAT)
#define clear_amrisc_irq_status(mask)           \
    WRITE_CBUS_REG(SYS_CPU_0_IRQ_IN3_INTR_STAT_CLR, mask)

/* GPIO Interrupt routines ****************************************/
/* --FIXME-- How to implement for A3*/
#define set_gpio_irq_mask(mask)                 \
    SET_CBUS_REG_MASK(GPIO_INTR_EDGE_POL, mask)
#define get_gpio_irq_mask()                     \
    READ_CBUS_REG(GPIO_INTR_EDGE_POL)
#define clear_gpio_irq_mask(mask)                   \
    CLEAR_CBUS_REG_MASK(GPIO_INTR_EDGE_POL, mask)
#define set_gpio_firq_mask(mask)                \
    SET_CBUS_REG_MASK(GPIO_INTR_GPIO_SEL0, mask)
#define get_gpio_firq_mask()                    \
    READ_CBUS_REG(GPIO_INTR_GPIO_SEL0)
#define clear_gpio_firq_mask(mask)                  \
    CLEAR_CBUS_REG_MASK(GPIO_INTR_GPIO_SEL0, mask)
#define read_gpio_irq_status()                  \
    READ_CBUS_REG(GPIO_INTR_GPIO_SEL0)
#define clear_gpio_irq_status(mask)             \
    WRITE_CBUS_REG(GPIO_INTR_GPIO_SEL0, mask)

/* RTC routines ***************************************************/
#define PRECISE_CUR_TIME   READ_CBUS_REG(ISA_TIMERC)
#define set_timer_base(n)  WRITE_CBUS_REG(ISA_TIMER_MUX, n)
#define read_timer_base()  READ_CBUS_REG(ISA_TIMER_MUX)

#endif /* ISAHELPER_H */
