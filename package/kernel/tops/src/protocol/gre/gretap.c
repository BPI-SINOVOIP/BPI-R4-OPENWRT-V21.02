// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 MediaTek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <net/gre.h>

#include <pce/cls.h>
#include <pce/netsys.h>
#include <pce/pce.h>

#include "tunnel.h"

static int gretap_cls_entry_setup(struct tops_tnl_info *tnl_info,
				  struct cls_desc *cdesc)
{
	CLS_DESC_DATA(cdesc, fport, PSE_PORT_PPE0);
	CLS_DESC_DATA(cdesc, tport_idx, 0x4);
	CLS_DESC_MASK_DATA(cdesc, tag, CLS_DESC_TAG_MASK, CLS_DESC_TAG_MATCH_L4_HDR);
	CLS_DESC_MASK_DATA(cdesc, dip_match, CLS_DESC_DIP_MATCH, CLS_DESC_DIP_MATCH);
	CLS_DESC_MASK_DATA(cdesc, l4_type, CLS_DESC_L4_TYPE_MASK, IPPROTO_GRE);
	CLS_DESC_MASK_DATA(cdesc, l4_udp_hdr_nez,
			   CLS_DESC_UDPLITE_L4_HDR_NEZ_MASK,
			   CLS_DESC_UDPLITE_L4_HDR_NEZ_MASK);
	CLS_DESC_MASK_DATA(cdesc, l4_valid,
			   CLS_DESC_L4_VALID_MASK,
			   CLS_DESC_VALID_UPPER_HALF_WORD_BIT |
			   CLS_DESC_VALID_LOWER_HALF_WORD_BIT);
	CLS_DESC_MASK_DATA(cdesc, l4_hdr_usr_data, 0x0000FFFF, 0x00006558);

	return 0;
}

static int gretap_tnl_decap_param_setup(struct sk_buff *skb,
					struct tops_tnl_params *tnl_params)
{
	struct gre_base_hdr *pgre;
	struct gre_base_hdr greh;
	struct ethhdr *eth;
	struct ethhdr ethh;
	struct iphdr *ip;
	struct iphdr iph;
	int ret = 0;

	if (!skb->dev->rtnl_link_ops
	    || strcmp(skb->dev->rtnl_link_ops->kind, "gretap"))
		return -EAGAIN;

	skb_push(skb, sizeof(struct gre_base_hdr));
	pgre = skb_header_pointer(skb, 0, sizeof(struct gre_base_hdr), &greh);
	if (unlikely(!pgre)) {
		ret = -EINVAL;
		goto restore_gre;
	}

	if (unlikely(ntohs(pgre->protocol) != ETH_P_TEB)) {
		pr_notice("gre: %p protocol unmatched, proto: 0x%x\n",
			pgre, ntohs(pgre->protocol));
		ret = -EINVAL;
		goto restore_gre;
	}

	/* TODO: store gre parameters? */

	skb_push(skb, sizeof(struct iphdr));
	ip = skb_header_pointer(skb, 0, sizeof(struct iphdr), &iph);
	if (unlikely(!ip)) {
		ret = -EINVAL;
		goto restore_ip;
	}

	if (unlikely(ip->version != IPVERSION || ip->protocol != IPPROTO_GRE)) {
		pr_notice("ip: %p version or protocol unmatched, ver: 0x%x, proto: 0x%x\n",
			ip, ip->version, ip->protocol);
		ret = -EINVAL;
		goto restore_ip;
	}

	/* TODO: check ip options is support for us? */
	/* TODO: store ip parameters? */
	tnl_params->protocol = ip->protocol;
	tnl_params->sip = ip->daddr;
	tnl_params->dip = ip->saddr;

	skb_push(skb, sizeof(struct ethhdr));
	eth = skb_header_pointer(skb, 0, sizeof(struct ethhdr), &ethh);
	if (unlikely(!eth)) {
		ret = -EINVAL;
		goto restore_eth;
	}

	if (unlikely(ntohs(eth->h_proto) != ETH_P_IP)) {
		pr_notice("eth proto not support, proto: 0x%x\n",
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

restore_gre:
	skb_pull(skb, sizeof(struct gre_base_hdr));

	return ret;
}

static int gretap_tnl_encap_param_setup(struct sk_buff *skb,
					struct tops_tnl_params *tnl_params)
{
	struct ethhdr *eth = eth_hdr(skb);
	struct iphdr *ip = ip_hdr(skb);

	/*
	 * ether type no need to check since it is even not constructed yet
	 * currently not support gre without ipv4
	 */
	if (unlikely(ip->version != IPVERSION || ip->protocol != IPPROTO_GRE)) {
		pr_notice("eth proto: 0x%x, ip ver: 0x%x, proto: 0x%x is not support\n",
			  ntohs(eth->h_proto),
			  ip->version,
			  ip->protocol);
		return -EINVAL;
	}

	memcpy(&tnl_params->saddr, eth->h_source, sizeof(u8) * ETH_ALEN);
	memcpy(&tnl_params->daddr, eth->h_dest, sizeof(u8) * ETH_ALEN);
	tnl_params->protocol = ip->protocol;
	tnl_params->sip = ip->saddr;
	tnl_params->dip = ip->daddr;

	return 0;
}

static int gretap_tnl_debug_param_setup(const char *buf, int *ofs,
					struct tops_tnl_params *tnl_params)
{
	tnl_params->protocol = IPPROTO_GRE;
	return 0;
}

static bool gretap_tnl_info_match(struct tops_tnl_params *parms1,
				  struct tops_tnl_params *parms2)
{
	if (parms1->sip == parms2->sip
	    && parms1->dip == parms2->dip
	    && !memcmp(parms1->saddr, parms2->saddr, sizeof(u8) * ETH_ALEN)
	    && !memcmp(parms1->daddr, parms2->daddr, sizeof(u8) * ETH_ALEN)) {
		return true;
	}

	return false;
}

static bool gretap_tnl_decap_offloadable(struct sk_buff *skb)
{
	struct iphdr *ip = ip_hdr(skb);

	if (ip->protocol != IPPROTO_GRE)
		return false;

	return true;
}

static struct tops_tnl_type gretap_type = {
	.type_name = "gretap",
	.cls_entry_setup = gretap_cls_entry_setup,
	.tnl_decap_param_setup = gretap_tnl_decap_param_setup,
	.tnl_encap_param_setup = gretap_tnl_encap_param_setup,
	.tnl_debug_param_setup = gretap_tnl_debug_param_setup,
	.tnl_info_match = gretap_tnl_info_match,
	.tnl_decap_offloadable = gretap_tnl_decap_offloadable,
	.tops_entry = TOPS_ENTRY_GRETAP,
	.has_inner_eth = true,
};

int mtk_tops_gretap_init(void)
{
	return mtk_tops_tnl_type_register(&gretap_type);
}

void mtk_tops_gretap_deinit(void)
{
	mtk_tops_tnl_type_unregister(&gretap_type);
}
