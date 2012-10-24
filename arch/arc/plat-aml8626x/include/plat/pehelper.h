/*******************************************************************
 *
 *  Copyright C 2008 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description:
 *      Peripheral register helper defines
 *  Author: Amlogic Software
 *  Created: 10/17/2008 3:51:02 PM
 *
 *******************************************************************/
#ifndef PEHELPER_H
#define PEHELPER_H



#define PERI_BASE_ADDR               0xc1100000

#define P_REG(reg)                  (PERI_BASE_ADDR + ((reg)<<2))

#define WRITE_PERI_REG(reg, val) IO_WRITE32(val,P_REG(reg))
#define READ_PERI_REG(reg) (IO_READ32(P_REG(reg)))

#define WRITE_PERIPHS_REG(reg,val) IO_WRITE32(val,P_REG(reg))
#define READ_PERIPHS_REG(reg)      IO_READ32(P_REG(reg))

#define CLEAR_PERI_REG_MASK(reg, mask) WRITE_PERI_REG(reg, (READ_PERI_REG(reg)&(~(mask))))
#define SET_PERI_REG_MASK(reg, mask)   WRITE_PERI_REG(reg, (READ_PERI_REG(reg)|(mask)))

//======================================================
//  NDMA
//======================================================
#define ndma_in_progress() \
    (READ_PERI_REG(PREG_NDMA_CONTROL_REG) & NDMA_IN_PROGRESS)
#define ndma_start() \
    SET_PERI_REG_MASK(PREG_NDMA_CONTROL_REG, NDMA_START)
#define ndma_halt() \
    CLEAR_PERI_REG_MASK(PREG_NDMA_CONTROL_REG, NDMA_START | NDMA_PERIOD_INT_ENABLE); \
    WRITE_PERI_REG(PREG_NMDA_TABLE_DESC_ADD, 0)
#define set_ndma_address(addr) \
    WRITE_PERI_REG(PREG_NDMA_TABLE_POINTER, (unsigned) addr)
#define set_ndma_delay(delay) \
    WRITE_PERI_REG(PREG_NDMA_CONTROL_REG, \
        (READ_PERI_REG(PREG_NDMA_CONTROL_REG) & ~NDMA_DELAY_MASK) | ((delay) & NDMA_DELAY_MASK))
#define set_ndma_entry_number(num) \
    WRITE_PERI_REG(PREG_NMDA_TABLE_DESC_ADD, (num));

#define SET_PERIPHS_REG_BITS(reg,val)   WRITE_PERIPHS_REG(reg, READ_PERIPHS_REG(reg)|val)
#define CLEAR_PERIPHS_REG_BITS(reg,val) WRITE_PERIPHS_REG(reg, READ_PERIPHS_REG(reg)&(~(val)))

#endif /* PEHELPER_H */
