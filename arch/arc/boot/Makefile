targets := \
	vmlinux.bin \
	vmlinux.bin.lzo \
	uImage

# uImage build relies on mkimage being availble on your host for ARC target
# You will need to build u-boot for ARC, rename mkimage to arc-elf32-mkimage
# and make sure it's reacable from your PATH

MKIMAGE := $(srctree)/scripts/mkuboot.sh

OBJCOPYFLAGS= -O binary -R .note -R .note.gnu.build-id -R .comment -S

LINUX_START_TEXT = $$(readelf -h vmlinux | \
			grep "Entry point address" | grep -o 0x.*)

COMPRESS = lzo
quiet_cmd_uimage = UIMAGE $@
	cmd_uimage = $(CONFIG_SHELL) $(MKIMAGE) -A arc -O linux -T kernel \
			-C $(COMPRESS) -a $(CONFIG_LINUX_LINK_BASE) \
			-e $(LINUX_START_TEXT) -d $< $@

# build targets don't need $(objtree) prefix for setting Dest folder to
# be top-level. However clean-files does

vmlinux.bin: vmlinux FORCE
	$(call if_changed,objcopy)
vmlinux.bin.lzo: vmlinux.bin FORCE
	$(call if_changed,lzo)

uImage: vmlinux.bin.lzo FORCE
	$(call if_changed,uimage)

# remove build results when requested

clean-files := \
	$(objtree)/vmlinux.* \
	$(objtree)/uImage

PHONY += FORCE
