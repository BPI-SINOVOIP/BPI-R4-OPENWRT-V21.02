# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2023 Mediatek Inc. All Rights Reserved.
# Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
#

EXTRA_KCONFIG+= \
	CONFIG_MTK_TOPS_SUPPORT=m \
	CONFIG_MTK_TOPS_GRE=$(CONFIG_MTK_TOPS_GRE) \
	CONFIG_MTK_TOPS_GRETAP=$(CONFIG_MTK_TOPS_GRETAP) \
	CONFIG_MTK_TOPS_L2TP=$(CONFIG_MTK_TOPS_L2TP) \
	CONFIG_MTK_TOPS_UDP_L2TP_DATA=$(CONFIG_MTK_TOPS_UDP_L2TP_DATA) \
	CONFIG_MTK_TOPS_SECURE_FW=$(CONFIG_MTK_TOPS_SECURE_FW)

EXTRA_CFLAGS+= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG))))

EXTRA_CFLAGS+= \
	-I$(LINUX_DIR)/drivers/net/ethernet/mediatek/ \
	-I$(LINUX_DIR)/drivers/dma/ \
	-I$(KERNEL_BUILD_DIR)/pce/inc/ \
	-DCONFIG_TOPS_TNL_NUM=$(CONFIG_TOPS_TNL_NUM) \
	-DCONFIG_TOPS_TNL_MAP_BIT=$(CONFIG_TOPS_TNL_MAP_BIT) \
	-Wall -Werror
