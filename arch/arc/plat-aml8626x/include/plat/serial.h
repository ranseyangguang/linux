/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _PLAT_HAPS51_SERIAL_H
#define _PLAT_HAPS51_SERIAL_H

/*
 * clk for the DW UART on HAPS51 clk is 1/4th of CPU clk
 * The /16 divisor is standard format of specifying this value
 * (implies UART samples 16 times per symbol)
 */
#define BASE_BAUD ( CONFIG_ARC_PLAT_CLK / 4 / 16 )

#endif /* _PLAT_HAPS51_SERIAL_H */
