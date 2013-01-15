/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _ASM_ARC_SERIAL_H
#define _ASM_ARC_SERIAL_H

/*
 * stub file
 *
 * Some of the ARC customers use the standard 8250 driver for their UART
 * which requires ARCH exporting this header for BASE_BAUD for early serial.
 * Interesting different ARC SoCs have different values for BASE_BAUD, and since
 * that can't be extracted from DeviceTree we either need <plat/serial.h> per
 * platform (and include here), which will not work with multi-platform-image.
 *
 * Hence need platform specific #if defery here
 */

#include <asm/clk.h>

#define BASE_BAUD	115200

#endif /* _ASM_ARC_SERIAL_H */
