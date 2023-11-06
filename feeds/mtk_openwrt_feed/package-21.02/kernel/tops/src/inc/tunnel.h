/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 MediaTek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _TOPS_TUNNEL_H_
#define _TOPS_TUNNEL_H_

#include <linux/bitmap.h>
#include <linux/hashtable.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <linux/refcount.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#include <pce/cls.h>

#include "protocol/l2tp/l2tp.h"

/* tunnel info status */
#define TNL_STA_UNINIT		(BIT(TNL_STATUS_UNINIT))
#define TNL_STA_INIT		(BIT(TNL_STATUS_INIT))
#define TNL_STA_QUEUED		(BIT(TNL_STATUS_QUEUED))
#define TNL_STA_UPDATING	(BIT(TNL_STATUS_UPDATING))
#define TNL_STA_UPDATED		(BIT(TNL_STATUS_UPDATED))
#define TNL_STA_DIP_UPDATE	(BIT(TNL_STATUS_DIP_UPDATE))
#define TNL_STA_DELETING	(BIT(TNL_STATUS_DELETING))

/* tunnel params flags */
#define TNL_DECAP_ENABLE	(BIT(TNL_PARAMS_DECAP_ENABLE_BIT))
#define TNL_ENCAP_ENABLE	(BIT(TNL_PARAMS_ENCAP_ENABLE_BIT))

/* tunnel info flags */
#define TNL_INFO_DEBUG		(BIT(TNL_INFO_DEBUG_BIT))

struct tops_tnl_info;
struct tops_tnl_params;

/*
 * tops_crsn
 *   TOPS_CRSN_TNL_ID_START
 *   TOPS_CRSN_TNL_ID_END
 *     APMCU checks whether tops_crsn is in this range to know if this packet
 *     was processed by TOPS previously.
 */
enum tops_crsn {
	TOPS_CRSN_IGNORE = 0x00,
	TOPS_CRSN_TNL_ID_START = 0x10,
	TOPS_CRSN_TNL_ID_END = 0x2F,
};

enum tops_entry_type {
	TOPS_ENTRY_NONE = 0,
	TOPS_ENTRY_GRETAP,
	TOPS_ENTRY_PPTP,
	TOPS_ENTRY_IP_L2TP,
	TOPS_ENTRY_UDP_L2TP_CTRL,
	TOPS_ENTRY_UDP_L2TP_DATA = 5,
	TOPS_ENTRY_VXLAN,
	TOPS_ENTRY_NATT,
	TOPS_ENTRY_CAPWAP_CTRL,
	TOPS_ENTRY_CAPWAP_DATA,
	TOPS_ENTRY_CAPWAP_DTLS = 10,
	TOPS_ENTRY_IPSEC_ESP,
	TOPS_ENTRY_IPSEC_AH,

	__TOPS_ENTRY_MAX = CONFIG_TOPS_TNL_NUM,
};

enum tops_tunnel_mbox_cmd {
	TOPS_TNL_MBOX_CMD_RESV,
	TOPS_TNL_START_ADDR_SYNC,

	__TOPS_TNL_MBOX_CMD_MAX,
};

enum tunnel_ctrl_event {
	TUNNEL_CTRL_EVENT_NULL,
	TUNNEL_CTRL_EVENT_NEW,
	TUNNEL_CTRL_EVENT_DEL,
	TUNNEL_CTRL_EVENT_DIP_UPDATE,

	__TUNNEL_CTRL_EVENT_MAX,
};

enum tnl_status {
	TNL_STATUS_UNINIT,
	TNL_STATUS_INIT,
	TNL_STATUS_QUEUED,
	TNL_STATUS_UPDATING,
	TNL_STATUS_UPDATED,
	TNL_STATUS_DIP_UPDATE,
	TNL_STATUS_DELETING,

	__TNL_STATUS_MAX,
};

enum tops_tnl_params_flag {
	TNL_PARAMS_DECAP_ENABLE_BIT,
	TNL_PARAMS_ENCAP_ENABLE_BIT,
};

enum tops_tnl_info_flag {
	TNL_INFO_DEBUG_BIT,
};

struct tops_cls_entry {
	struct cls_entry *cls;
	struct list_head node;
	refcount_t refcnt;
	bool updated;
};

/* record outer tunnel header data for HW offloading */
struct tops_tnl_params {
	u8 daddr[ETH_ALEN];
	u8 saddr[ETH_ALEN];
	__be32 dip;
	__be32 sip;
	__be16 dport;
	__be16 sport;
	u16 protocol;
	u8 tops_entry_proto;
	u8 cls_entry;
	u8 cdrt;
	u8 flag; /* bit: enum tops_tnl_params_flag */
	union {
		struct l2tp_param l2tp; /* 4B */
	} priv;
} __packed __aligned(16);

struct tops_tnl_info {
	struct tops_tnl_params tnl_params;
	struct tops_tnl_params cache;
	struct tops_tnl_type *tnl_type;
	struct tops_cls_entry *tcls;
	struct list_head sync_node;
	struct hlist_node hlist;
	struct net_device *dev;
	spinlock_t lock;
	u32 tnl_idx;
	u32 status;
	u32 flag; /* bit: enum tops_tnl_info_flag */
} __aligned(16);

/*
 * tnl_l2_param_update:
 *	update tunnel l2 info only
 *	return 1 on l2 params have difference
 *	return 0 on l2 params are the same
 *	return negative value on error
 */
struct tops_tnl_type {
	const char *type_name;
	enum tops_entry_type tops_entry;

	int (*cls_entry_setup)(struct tops_tnl_info *tnl_info,
			       struct cls_desc *cdesc);
	struct list_head tcls_head;
	bool use_multi_cls;

	int (*tnl_decap_param_setup)(struct sk_buff *skb,
				     struct tops_tnl_params *tnl_params);
	int (*tnl_encap_param_setup)(struct sk_buff *skb,
				     struct tops_tnl_params *tnl_params);
	int (*tnl_debug_param_setup)(const char *buf, int *ofs,
				     struct tops_tnl_params *tnl_params);
	int (*tnl_l2_param_update)(struct sk_buff *skb,
				   struct tops_tnl_params *tnl_params);
	int (*tnl_dump_param)(char *buf, struct tops_tnl_params *tnl_params);
	bool (*tnl_info_match)(struct tops_tnl_params *params1,
			       struct tops_tnl_params *params2);
	bool (*tnl_decap_offloadable)(struct sk_buff *skb);
	bool has_inner_eth;
};

void mtk_tops_tnl_info_submit_no_tnl_lock(struct tops_tnl_info *tnl_info);
void mtk_tops_tnl_info_submit(struct tops_tnl_info *tnl_info);
struct tops_tnl_info *mtk_tops_tnl_info_find(struct tops_tnl_params *tnl_params);
struct tops_tnl_info *mtk_tops_tnl_info_alloc(struct tops_tnl_type *tnl_type);
void mtk_tops_tnl_info_hash(struct tops_tnl_info *tnl_info);

int mtk_tops_tnl_offload_init(struct platform_device *pdev);
void mtk_tops_tnl_offload_deinit(struct platform_device *pdev);
int mtk_tops_tnl_offload_proto_setup(struct platform_device *pdev);
void mtk_tops_tnl_offload_proto_teardown(struct platform_device *pdev);
void mtk_tops_tnl_offload_flush(void);
void mtk_tops_tnl_offload_recover(void);
void mtk_tops_tnl_offload_netdev_down(struct net_device *ndev);

struct tops_tnl_type *mtk_tops_tnl_type_get_by_name(const char *name);
int mtk_tops_tnl_type_register(struct tops_tnl_type *tnl_type);
void mtk_tops_tnl_type_unregister(struct tops_tnl_type *tnl_type);
#endif /* _TOPS_TUNNEL_H_ */
