/*************************************************************************
 * Copyright ARC International (www.arc.com) 2007-2009
 *
 * vineetg: Feb 2009
 *  -For AA4 board, System Memory Map for Peripherals etc
 *
 ************************************************************************/

#ifndef __PLAT_MEMMAP_H
#define __PLAT_MEMMAP_H

#define UART_BASE0              0xC0FC1000

#define VMAC_REG_BASEADDR       0xC0FC2000

#define IDE_CONTROLLER_BASE     0xC0FC9000

#define AHB_PCI_HOST_BRG_BASE   0xC0FD0000

#define PGU_BASEADDR            0xC0FC8000
#define VLCK_ADDR               0xC0FCF028

#define DCCM_COMPILE_BASE       0xA0000000
#define DCCM_COMPILE_SZ         (64 * 1024)
#define ICCM_COMPILE_SZ         (64 * 1024)

#define BVCI_LAT_UNIT_BASE      0xC0FED000
#endif
