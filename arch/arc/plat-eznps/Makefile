#
# Makefile for the linux kernel.
#

KBUILD_CFLAGS	+= -Iarch/arc/plat-eznps/include

obj-y := platform.o irq.o time.o cmem.o

obj-$(CONFIG_NPS_SMP_EXTN)		+= smp.o
