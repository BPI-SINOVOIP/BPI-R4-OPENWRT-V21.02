#
# Copyright (C) 2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

PKG_NAME:=mtk_eth
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

PKG_EXTRA_KCONFIG:= \
	CONFIG_NET_MEDIATEK_SOC=m \

USE_SOURCE_DIR:=$(TOPDIR)/../mtk-openwrt-feeds/target/linux/mediatek/files-5.4/drivers/net/ethernet/mediatek

define KernelPackage/mtk_eth
	SECTION:=kernel
	SUBMENU:=Network Devices
	TITLE:=Mediatek ETH module
	DEPENDS:=@TARGET_mediatek
	FILES:=$(PKG_BUILD_DIR)/mtk_eth.ko
	AUTOLOAD:=$(call AutoLoad,30,mtk_eth,1)
endef

define KernelPackage/mtk_eth/description
  Kernel modules for MediaTek Ethernet
endef

define Build/Patch
	$(call PatchDir,$(PKG_BUILD_DIR),$(PATCH_DIR),)
endef

define Build/Prepare
	cp -rf $(USE_SOURCE_DIR)/* $(PKG_BUILD_DIR)
	$(Build/Patch)
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" V=1 \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		LINUX_DIR="$(KERNEL_BUILD_DIR)" \
		$(PKG_EXTRA_KCONFIG) \
		modules
endef

$(eval $(call KernelPackage,mtk_eth))
