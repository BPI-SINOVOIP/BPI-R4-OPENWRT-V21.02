/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(dump);

int handle_mlo_info_show(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *cmd_str;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (argc > 0) {
		cmd_str = argv[0];
		printf("Invalid argument:%s, ignore it\n", cmd_str);
		return -EINVAL;
	}

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_CMD_STR, 1))
				return -EMSGSIZE;
	nla_nest_end(msg, data);

	return 0;
}


COMMAND(dump, mlo_info, NULL,
		MTK_NL80211_VENDOR_SUBCMD_SHOW_MLO_INFO, 0, CIB_NETDEV, handle_mlo_info_show,
		"Show sta mlo link info.\n");


