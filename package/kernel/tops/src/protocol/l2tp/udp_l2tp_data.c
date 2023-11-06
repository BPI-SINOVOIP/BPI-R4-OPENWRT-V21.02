// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 MediaTek Inc. All Rights Reserved.
 *
 * Author: Frank-zj Lin <rank-zj.lin@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/ppp_defs.h>
#include <linux/udp.h>

#include <pce/cls.h>
#include <pce/netsys.h>
#include <pce/pce.h>

#include "protocol/l2tp/l2tp.h"
#include "protocol/ppp/ppp.h"
#include "tunnel.h"

static int udp_l2tp_data_cls_entry_setup(struct tops_tnl_info *tnl_info,
					 struct cls_desc *cdesc)
{
	CLS_DESC_DATA(cdesc, fport, PSE_PORT_PPE0);
	CLS_DESC_DATA(cdesc, tport_idx, 0x4);
	CLS_DESC_MASK_DATA(cdesc, tag, CLS_DESC_TAG_MASK, CLS_DESC_TAG_MATCH_L4_USR);
	CLS_DESC_MASK_DATA(cdesc, dip_match, CLS_DESC_DIP_MATCH, CLS_DESC_DIP_MATCH);
	CLS_DESC_MASK_DATA(cdesc, l4_type, CLS_DESC_L4_TYPE_MASK, IPPROTO_UDP);
	CLS_DESC_MASK_DATA(cdesc, l4_valid,
			   CLS_DESC_L4_VALID_MASK,
			   CLS_DESC_VALID_UPPER_HALF_WORD_BIT |
			   CLS_DESC_VALID_LOWER_HALF_WORD_BIT |
			   CLS_DESC_VALID_DPORT_BIT);
	CLS_DESC_MASK_DATA(cdesc, l4_dport, CLS_DESC_L4_DPORT_MASK, 1701);
	CLS_DESC_MASK_DATA(cdesc, l4_hdr_usr_data, 0x80030000, 0x00020000);

	return 0;
}

static inline bool l2tpv2_offload_match(struct udp_l2tp_data_hdr *l2tp)
{
	u16 hdrflags = ntohs(l2tp->flag_ver);

	return ((hdrflags & L2TP_HDR_VER_MASK) == L2TP_HDR_VER_2 &&
		!(hdrflags & L2TP_HDRFLAG_T));
}

static inline bool ppp_offload_match(struct ppp_hdr *ppp)
{
	return (ppp->addr == PPP_ALLSTATIONS &&
		ppp->ctrl == PPP_UI && ntohs(ppp->proto) == PPP_IP);
}

static int udp_l2tp_data_tnl_decap_param_setup(struct sk_buff *skb,
					       struct tops_tnl_params *tnl_params)
{
	struct udp_l2tp_data_hdr *l2tp;
	struct udp_l2tp_data_hdr l2tph;
	struct ppp_hdr *ppp;
	struct ppp_hdr ppph;
	struct udphdr *udp;
	struct udphdr udph;
	struct ethhdr *eth;
	struct ethhdr ethh;
	struct iphdr *ip;
	struct iphdr iph;
	int ret = 0;

	/* ppp */
	skb_push(skb, sizeof(struct ppp_hdr));
	ppp = skb_header_pointer(skb, 0, sizeof(struct ppp_hdr), &ppph);

	if (unlikely(!ppp)) {
		ret = -EINVAL;
		goto restore_ppp;
	}

	if (unlikely(!ppp_offload_match(ppp))) {
		pr_notice("ppp offload unmatched\n");
		ret = -EINVAL;
		goto restore_ppp;
	}

	/* l2tp */
	skb_push(skb, sizeof(struct udp_l2tp_data_hdr));
	l2tp = skb_header_pointer(skb, 0, sizeof(struct udp_l2tp_data_hdr), &l2tph);
	if (unlikely(!l2tp)) {
		ret = -EINVAL;
		goto restore_l2tp;
	}

	if (unlikely(!l2tpv2_offload_match(l2tp))) {
		ret = -EINVAL;
		goto restore_l2tp;
	}

	tnl_params->priv.l2tp.tid = l2tp->tid;
	tnl_params->priv.l2tp.sid = l2tp->sid;

	/* udp */
	skb_push(skb, sizeof(struct udphdr));
	udp = skb_header_pointer(skb, 0, sizeof(struct udphdr), &udph);
	if (unlikely(!udp)) {
		ret = -EINVAL;
		goto restore_udp;
	}

	if (unlikely(ntohs(udp->dest) != UDP_L2TP_PORT)) {
		pr_notice("udp port 0x%x unmatched\n", ntohs(udp->dest));
		ret = -EINVAL;
		goto restore_udp;
	}

	tnl_params->sport = udp->dest;
	tnl_params->dport = udp->source;

	/* ip */
	skb_push(skb, sizeof(struct iphdr));
	ip = skb_header_pointer(skb, 0, sizeof(struct iphdr), &iph);
	if (unlikely(!ip)) {
		ret = -EINVAL;
		goto restore_ip;
	}

	if (unlikely(ip->version != IPVERSION || ip->protocol != IPPROTO_UDP)) {
		pr_notice("ip: %p version or protocol unmatched, ver: 0x%x, proto: 0x%x\n",
			ip, ip->version, ip->protocol);
		ret = -EINVAL;
		goto restore_ip;
	}

	tnl_params->protocol = ip->protocol;
	tnl_params->sip = ip->daddr;
	tnl_params->dip = ip->saddr;

	/* eth */
	skb_push(skb, sizeof(struct ethhdr));
	eth = skb_header_pointer(skb, 0, sizeof(struct ethhdr), &ethh);
	if (unlikely(!eth)) {
		ret = -EINVAL;
		goto restore_eth;
	}

	if (unlikely(ntohs(eth->h_proto) != ETH_P_IP)) {
		pr_notice("eth proto not supported, proto: 0x%x\n",
			  ntohs(eth->h_proto));
		ret = -EINVAL;
		goto restore_eth;
	}

	memcpy(&tnl_params->saddr, eth->h_dest, sizeof(u8) * ETH_ALEN);
	memcpy(&tnl_params->daddr, eth->h_source, sizeof(u8) * ETH_ALEN);

restore_eth:
	skb_pull(skb, sizeof(struct ethhdr));
restore_ip:
	skb_pull(skb, sizeof(struct iphdr));
restore_udp:
	skb_pull(skb, sizeof(struct udphdr));
restore_l2tp:
	skb_pull(skb, sizeof(struct udp_l2tp_data_hdr));
restore_ppp:
	skb_pull(skb, sizeof(struct ppp_hdr));

	return ret;
}

static int udp_l2tp_data_tnl_encap_param_setup(struct sk_buff *skb,
					       struct tops_tnl_params *tnl_params)
{
	struct ethhdr *eth = eth_hdr(skb);
	struct iphdr *ip = ip_hdr(skb);
	struct udp_l2tp_data_hdr *l2tp;
	struct udp_l2tp_data_hdr l2tph;
	struct udphdr *udp;
	struct udphdr udph;
	int ret = 0;

	if (unlikely(ip->version != IPVERSION || ip->protocol != IPPROTO_UDP)) {
		pr_notice("eth proto: 0x%x, ip ver: 0x%x, proto: 0x%x is not support\n",
			  ntohs(eth->h_proto),
			  ip->version,
			  ip->protocol);
		ret = -EINVAL;
		goto out;
	}

	skb_pull(skb, sizeof(struct iphdr));
	udp = skb_header_pointer(skb, 0, sizeof(struct udphdr), &udph);
	if (unlikely(!udp)) {
		ret = -EINVAL;
		goto restore_ip;
	}

	if (unlikely(ntohs(udp->dest) != UDP_L2TP_PORT)) {
		pr_notice("udp port 0x%x unmatched\n", ntohs(udp->dest));
		ret = -EINVAL;
		goto restore_ip;
	}

	skb_pull(skb, sizeof(struct udphdr));
	l2tp = skb_header_pointer(skb, 0, sizeof(struct udp_l2tp_data_hdr), &l2tph);
	if (unlikely(!l2tp)) {
		ret = -EINVAL;
		goto restore_udp;
	}

	if (unlikely(!l2tpv2_offload_match(l2tp))) {
		ret = -EINVAL;
		goto restore_udp;
	}

	memcpy(&tnl_params->saddr, eth->h_source, sizeof(u8) * ETH_ALEN);
	memcpy(&tnl_params->daddr, eth->h_dest, sizeof(u8) * ETH_ALEN);
	tnl_params->protocol = ip->protocol;
	tnl_params->sip = ip->saddr;
	tnl_params->dip = ip->daddr;
	tnl_params->sport = udp->source;
	tnl_params->dport = udp->dest;
	tnl_params->priv.l2tp.tid = l2tp->tid;
	tnl_params->priv.l2tp.sid = l2tp->sid;

restore_udp:
	skb_push(skb, sizeof(struct udphdr));
restore_ip:
	skb_push(skb, sizeof(struct iphdr));
out:
	return ret;
}

static int udp_l2tp_data_tnl_debug_param_setup(const char *buf, int *ofs,
					struct tops_tnl_params *tnl_params)
{
	return -EPERM; //TODO: not implemented
}

static int udp_l2tp_data_tnl_l2_param_update(struct sk_buff *skb,
					     struct tops_tnl_params *tnl_params)
{
	struct ethhdr *eth = eth_hdr(skb);

	memcpy(&tnl_params->saddr, eth->h_source, sizeof(u8) * ETH_ALEN);
	memcpy(&tnl_params->daddr, eth->h_dest, sizeof(u8) * ETH_ALEN);

	return 1;
}

static bool udp_l2tp_data_tnl_info_match(struct tops_tnl_params *params1,
					 struct tops_tnl_params *params2)
{
	if (params1->sip == params2->sip
	    && params1->dip == params2->dip
	    && params1->sport == params2->sport
	    && params1->dport == params2->dport
	    && params1->priv.l2tp.tid == params2->priv.l2tp.tid
	    && params1->priv.l2tp.sid == params2->priv.l2tp.sid)
		return true;

	return false;
}

static bool udp_l2tp_data_tnl_decap_offloadable(struct sk_buff *skb)
{
	struct udp_l2tp_data_hdr *l2tp;
	struct udp_l2tp_data_hdr l2tph;
	struct ppp_hdr *ppp;
	struct ppp_hdr ppph;
	struct udphdr *udp;
	struct iphdr *ip;

	ip = ip_hdr(skb);
	if (ip->protocol != IPPROTO_UDP)
		return false;

	udp = udp_hdr(skb);
	if (ntohs(udp->dest) != UDP_L2TP_PORT)
		return false;

	l2tp = skb_header_pointer(skb, ip_hdr(skb)->ihl * 4 + sizeof(struct udphdr),
				  sizeof(struct udp_l2tp_data_hdr), &l2tph);

	if (unlikely(!l2tp))
		return false;

	if (unlikely(!l2tpv2_offload_match(l2tp)))
		return false;

	ppp = skb_header_pointer(skb, (ip_hdr(skb)->ihl * 4 +
				       sizeof(struct udphdr) +
				       sizeof(struct udp_l2tp_data_hdr)),
				 sizeof(struct ppp_hdr), &ppph);

	if (unlikely(!ppp))
		return false;

	if (unlikely(!ppp_offload_match(ppp)))
		return false;

	return true;
}

static struct tops_tnl_type udp_l2tp_data_type = {
	.type_name = "udp-l2tp-data",
	.cls_entry_setup = udp_l2tp_data_cls_entry_setup,
	.tnl_decap_param_setup = udp_l2tp_data_tnl_decap_param_setup,
	.tnl_encap_param_setup = udp_l2tp_data_tnl_encap_param_setup,
	.tnl_debug_param_setup = udp_l2tp_data_tnl_debug_param_setup,
	.tnl_l2_param_update = udp_l2tp_data_tnl_l2_param_update,
	.tnl_info_match = udp_l2tp_data_tnl_info_match,
	.tnl_decap_offloadable = udp_l2tp_data_tnl_decap_offloadable,
	.tops_entry = TOPS_ENTRY_UDP_L2TP_DATA,
	.has_inner_eth = false,
};

int mtk_tops_udp_l2tp_data_init(void)
{
	return mtk_tops_tnl_type_register(&udp_l2tp_data_type);
}

void mtk_tops_udp_l2tp_data_deinit(void)
{
	mtk_tops_tnl_type_unregister(&udp_l2tp_data_type);
}
