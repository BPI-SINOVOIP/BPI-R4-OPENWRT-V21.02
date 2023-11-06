# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2023 MediaTek Inc. All Rights Reserved.
#
# Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
#

obj-$(CONFIG_MTK_TOPS_SUPPORT) += tops.o

ccflags-y += -I$(src)/inc
ccflags-y += -I$(src)/protocol/inc

tops-y += ctrl.o
tops-y += firmware.o
tops-y += init.o
tops-y += hpdma.o
tops-y += hwspinlock.o
tops-y += mbox.o
tops-y += mcu.o
tops-y += netsys.o
tops-y += net-event.o
tops-y += tnl_offload.o
tops-y += ser.o
tops-y += tdma.o
tops-y += trm-fs.o
tops-y += trm-mcu.o
tops-y += trm-debugfs.o
tops-y += trm.o
tops-y += wdt.o

tops-$(CONFIG_MTK_TOPS_GRETAP) += protocol/gre/gretap.o
tops-$(CONFIG_MTK_TOPS_UDP_L2TP_DATA) += protocol/l2tp/udp_l2tp_data.o

include $(wildcard $(src)/*.mk)
