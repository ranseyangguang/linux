#
# Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

obj-y := platform.o irq.o

obj-$(CONFIG_ISS_SMP_EXTN)		+= smp.o

obj-$(CONFIG_ARC_AHB_PCI_BRIDGE) 	+= aa4_pci.o
export CFLAGS_aa4_pci.o = -mlong-calls
