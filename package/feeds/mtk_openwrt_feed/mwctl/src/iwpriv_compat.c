/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include <unistd.h>
#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

#define MAX_LEN_OF_CMD 128

struct advanced_set_cmd_mapping {
	char iwpriv_set_str[MAX_LEN_OF_CMD];
	char advaned_set_str[MAX_LEN_OF_CMD];
};

static unsigned char ascii2hex(char *in, unsigned int *out)
{
	unsigned int hex_val, val;
	char *p, asc_val;

	hex_val = 0;
	p = (char *)in;

	while ((*p) != 0) {
		val = 0;
		asc_val = *p;

		if ((asc_val >= 'a') && (asc_val <= 'f'))
			val = asc_val - 87;
		else if ((*p >= 'A') && (asc_val <= 'F'))
			val = asc_val - 55;
		else if ((asc_val >= '0') && (asc_val <= '9'))
			val = asc_val - 48;
		else
			return 0;

		hex_val = (hex_val << 4) + val;
		p++;
	}

	*out = hex_val;
	return 1;
}

static int handle_common_command(struct nl_msg *msg,
			   int argc, char **argv, int attr)
{
	void *data;
	size_t len = 0;
	char *cmd_str;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (argc > 0) {
		cmd_str = argv[0];

		len = strlen(cmd_str);
		if (len) {
			if (nla_put_string(msg, attr, cmd_str))
				return -EMSGSIZE;
		}
	}
	nla_nest_end(msg, data);

	return 0;
}

struct advanced_set_cmd_mapping map[] = {
	{"vow_bw_enable", "vow"},
	{"vow_airtime_fairness_en", "vow"},
	{"vow_airtime_control_en", "vow"},
	{"vow_bw_control_en", "vow"},
	{"vow_min_rate", "vow"},
	{"vow_max_rate", "vow"},
	{"vow_min_ratio", "vow"},
	{"vow_max_ratio", "vow"},
	{"BADecline", "ba_decline"},
	{"HtBaWinSize", "ba_wsize"},
	{"HtAutoBa", "ba_auto"},
	{"BASetup", "ba_setup"},
	{"BAOriTearDown", "ba_ori_teardown"},
	{"BARecTearDown", "ba_rec_teardown"},
	{"IEEE80211H", "ieee80211h"},
	{"APAifsn", "ap_aifsn"},
	{"APCwmin", "ap_cwmin"},
	{"APCwmax", "ap_cwmax"},
	{"APTxop", "ap_txop"},
	{"mnt_en", "mnt_en"},
	{"mnt_rule", "mnt_rule"},
	{"mnt_sta", "mnt_sta"},
	{"mnt_idx", "mnt_idx"},
	{"mnt_clr", "mnt_clr"},
	{"Distance", "distance"},
	{"ACK_CTS_TOUT_EN", "ackcts_tout_en"},
	{"CCK_ACK_TOUT", "cck_ack_tout"},
	{"OFDM_ACK_TOUT", "ofdm_ack_tout"},
	{"OFDMA_ACK_TOUT", "ofdma_ack_tout"},
	{"VLANEn", "vlan_en"},
	{"VLANID", "vlan_id"},
	{"VLANPriority", "vlan_priority"},
	{"VLANTag", "vlan_tag"},
	{"VLANPolicy", "vlan_policy"},
	{"HtTxStream", "ht_tx_stream"},
	{"AssocReqRssiThres", "assocreq_rssi_thres"},
	{"ApCliMWDS", "mwds"},
	{"ApMWDS", "mwds"},
	{"WirelessMode", "phymode"},
	{"TxPower", "pwr"},
	{"TxPowerInfo", "pwr"},
	{"MaxTxPwr", "pwr"},
	{"TxPowerInfo", "pwr"},
	{"SKUCtrl", "pwr"},
	{"PercentageCtrl", "pwr"},
	{"PowerDropCtrl", "pwr"},
	{"DecreasePower", "pwr"},
	{"SKUInfo", "pwr"},
	{"MUTxPower", "pwr"},
	{"mgmt_frame_pwr", "pwr"},
	{"HtMpduDensity", "mpdu_density"},
	{"HtAmsdu", "ht_amsdu"},
	{"HtBssCoexApCntThr", "bss_coex_ap_thr"},
	{"HtExtcha", "ht_ext_cha"},
	{"HtProtect", "ht_protect"},
	{"VhtDisallowNonVHT", "disallow_non_vht"},
	{"etxbf_en_cond", "etxbf_en_cond"},
	{"PMFSHA256", "pmf_sha256"},
	{"HtBw", "channel"},
	{"VhtBw", "channel"},
	{"VhtBw", "channel"},
	{"CountryRegion", "country"},
	{"CountryCode", "country"},
	{"IgmpSnEnable", "multicast_snooping"},
	{"SeamlessCSA", "csa_2g"},
	{"BssMaxIdle", "idle_timeout"},
	{"muru_dl_enable", "muru_dl_en"},
	{"muru_ul_enable", "muru_ul_en"},
	{"mu_dl_enable", "mu_dl_en"},
	{"mu_ul_enable", "mu_ul_en"},
};

#define IWPRIV_NOTICE "Your command is not suggested now, please use below advanced command:\n\t"

int handle_set_command(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	struct cmd *command;
	char cmd_str[MAX_LEN_OF_CMD] = {0}, *value;
	int i;

	if (argc > 0)
		strncpy(cmd_str, argv[0], strlen(argv[0]) >= MAX_LEN_OF_CMD ? MAX_LEN_OF_CMD - 1 : strlen(argv[0]));

	value = strchr(cmd_str, '=');
	if (value)
		*value = 0;

	for (i = 0; i < sizeof(map)/sizeof(map[0]); i++) {
		if (!strcmp(cmd_str, map[i].iwpriv_set_str)) {
			command = get_cmd_by_sect_name("set", map[i].advaned_set_str);
			if (command) {
				__usage_cmd(command, L_RED IWPRIV_NOTICE NONE, 1);
				sleep(2);
			}
			break;
		}
	}

	return handle_common_command(msg, argc, argv, MTK_NL80211_VENDOR_ATTR_VENDOR_SET_CMD_STR);
}

TOPLEVEL(set, "<param>=<value>", MTK_NL80211_VENDOR_SUBCMD_VENDOR_SET, 0, CIB_NETDEV, handle_set_command,
	"This command is used to show information of wifi driver.\n"
	"It is used to be compatible with old iwpriv set command, e.g iwpriv ra0 set channel=36");

int show_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char *show_str = NULL;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_RSP_STR]) {
			show_str = nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_RSP_STR]);
			printf("%s\n", show_str);
		}		
	} else
		printf("no any show rsp string from driver\n");

	return 0;
}

static int handle_show_command(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{	
	register_handler(show_callback, NULL);
	return handle_common_command(msg, argc, argv, MTK_NL80211_VENDOR_ATTR_VENDOR_SHOW_CMD_STR);
}

TOPLEVEL(show, "<param>", MTK_NL80211_VENDOR_SUBCMD_VENDOR_SHOW, 0, CIB_NETDEV, handle_show_command,
	 "This command is used to show information of wifi driver.\n"
	 "It is uesed to be compatible with old iwpriv show command, e.g iwpriv ra0 show stainfo");

int stat_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_STATISTICS_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char *show_str = NULL;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_STATISTICS_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_STATISTICS_STR]) {
			show_str = nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_STATISTICS_STR]);
			printf("%s\n", show_str);
		}		
	} else
		printf("no any statistic string from driver\n");

	return 0;
}

static int handle_stat_command(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{	
	register_handler(stat_callback, NULL);
	return handle_common_command(msg, 0, NULL, 0);
}

TOPLEVEL(stat, NULL, MTK_NL80211_VENDOR_SUBCMD_STATISTICS, 0, CIB_NETDEV, handle_stat_command,
	 "This command is used to show the statistic information");


int mac_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_MAC_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char *show_str = NULL;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_MAC_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_MAC_RSP_STR]) {
			show_str = nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_MAC_RSP_STR]);
			printf("%s\n", show_str);
		}		
	} else
		printf("no any statistic string from driver\n");

	return 0;
}


static int handle_mac_command(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *ptr, *seg_str, *addr_str, *val_str, *range_str;
	unsigned char is_write, is_range;
	unsigned int mac_s = 0, mac_e = 0;
	unsigned int macVal = 0;
	struct mac_param param;

	if (!argc)
		return -EINVAL;
	
	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	ptr = argv[0];

	while ((seg_str = strsep((char **)&ptr, ",")) != NULL) {
		is_write = 0;
		addr_str = seg_str;
		val_str = NULL;
		val_str = strchr(seg_str, '=');

		if (val_str != NULL) {
			*val_str++ = 0;
			is_write = 1;
		} else
			is_write = 0;

		if (addr_str) {
			range_str = strchr(addr_str, '-');

			if (range_str != NULL) {
				*range_str++ = 0;
				is_range = 1;
			} else
				is_range = 0;

			if ((ascii2hex(addr_str, &mac_s) == 0)) {
				printf("Invalid MAC CR Addr, str=%s\n", addr_str);
				break;
			}

			if (is_range) {
				if (ascii2hex(range_str, &mac_e) == 0) {
					printf("Invalid Range End MAC CR Addr[0x%x], str=%s\n",
							 mac_e, range_str);
					break;
				}

				if (mac_e < mac_s) {
					printf("Invalid Range MAC Addr[%s - %s] => [0x%x - 0x%x]\n",
							 addr_str, range_str, mac_s, mac_e);
					break;
				}
			} else
				mac_e = mac_s;
		}

		if (val_str) {
			if ((strlen(val_str) == 0) || ascii2hex(val_str, &macVal) == 0) {
				printf("Invalid MAC value[0x%s]\n", val_str);
				break;
			}
		}

		memset(&param, 0, sizeof(param));
		param.start = mac_s;
		param.value = macVal;
		param.end = mac_e;
		if (is_write) {
			if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_MAC_WRITE_PARAM, sizeof(param), &param))
				return -EMSGSIZE;
		} else {
			if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_MAC_SHOW_PARAM, sizeof(param), &param))
				return -EMSGSIZE;
		}
	}

	nla_nest_end(msg, data);
	register_handler(mac_callback, NULL);
	return 0;
}


TOPLEVEL(mac, "<addr>|<addr=value>|<addr-addr>|<addr1=hex,addr2=hex>", MTK_NL80211_VENDOR_SUBCMD_MAC, 0,
	CIB_PHY, handle_mac_command,
	"This command is used to show/set mac register, e.g iwpriv ra0 mac 2345, it means show the value of mac register 1234");


int e2p_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_E2P_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char *show_str = NULL;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_E2P_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_E2P_RSP_STR]) {
			show_str = nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_E2P_RSP_STR]);
			printf("%s\n", show_str);
		}
	} else
		printf("no any statistic string from driver\n");

	return 0;
}


static int handle_e2p_command(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *ptr, *seg_str, *addr_str, *val_str, *range_str;
	unsigned char is_write, is_range, is_dump_all = 0;
	unsigned int e2p_s = 0, e2p_e = 0;
	unsigned int e2pVal = 0;
	struct e2p_param param;

	if (!argc) {
		/* dont any argument to dump all*/
		is_dump_all = 1;
		printf("dump all %d\n", is_dump_all);
	}

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	ptr = argv[0];

	if (is_dump_all) {
		printf("%d \n", __LINE__);
		memset(&param, 0, sizeof(param));
		if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_E2P_DUMP_ALL_PARAM, sizeof(param), &param))
			return -EMSGSIZE;
	} else {
		while ((seg_str = strsep((char **)&ptr, ",")) != NULL) {
			is_write = 0;
			addr_str = seg_str;
			val_str = NULL;
			val_str = strchr(seg_str, '=');

			if (val_str != NULL) {
				*val_str++ = 0;
				is_write = 1;
			} else
				is_write = 0;

			if (addr_str) {
				range_str = strchr(addr_str, '-');

				if (range_str != NULL) {
					*range_str++ = 0;
					is_range = 1;
				} else
					is_range = 0;

				if ((ascii2hex(addr_str, &e2p_s) == 0)) {
					printf("Invalid E2P Addr, str=%s\n", addr_str);
					break;
				}

				if (is_range) {
					if (ascii2hex(range_str, &e2p_e) == 0) {
						printf("Invalid Range End E2P Addr[0x%x], str=%s\n",
								 e2p_e, range_str);
						break;
					}

					if (e2p_e < e2p_s) {
						printf("Invalid Range E2P Addr[%s : %s] => [0x%x : 0x%x]\n",
								 addr_str, range_str, e2p_s, e2p_e);
						break;
					}
				} else
					e2p_e = e2p_s;
			}

			if (val_str) {
				if ((strlen(val_str) == 0) || ascii2hex(val_str, &e2pVal) == 0) {
					printf("Invalid MAC value[0x%s]\n", val_str);
					break;
				}
			}

			memset(&param, 0, sizeof(param));
			param.start = e2p_s;
			param.value = e2pVal;
			param.end = e2p_e;
			if (is_write) {
				if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_E2P_WRITE_PARAM, sizeof(param), &param))
					return -EMSGSIZE;
			} else {
				if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_E2P_SHOW_PARAM, sizeof(param), &param))
					return -EMSGSIZE;
			}
		}
	}

	nla_nest_end(msg, data);
	register_handler(e2p_callback, NULL);
	return 0;
}

TOPLEVEL(e2p, "<addr>|<addr=value>|<addr-addr>|<addr1=hex,addr2=hex>", MTK_NL80211_VENDOR_SUBCMD_E2P, 0,
	CIB_PHY, handle_e2p_command,
	"This command is used to show/set eeprom from driver buffer, e.g mwctl phy phy0 e2p xxx, it means show the value of eeprom xxx");

int testengine_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_TESTENGINE_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char *show_str = NULL;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_TESTENGINE_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_TESTENGINE_RSP_STR]) {
			show_str = nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_TESTENGINE_RSP_STR]);
			printf("%s\n", show_str);
		}
	} else
		printf("no any statistic string from driver\n");

	return 0;
}


static int handle_testengine_command(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *ptr, *seg_str, *addr_str, *val_str;
	unsigned char is_write;
	unsigned int atIdx = 0;
	unsigned int atVal = 0;
	struct testengine_param param;

	if (!argc)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	ptr = argv[0];

	while ((seg_str = strsep((char **)&ptr, ",")) != NULL) {
		is_write = 0;
		addr_str = seg_str;
		val_str = NULL;
		val_str = strchr(seg_str, '=');

		if (val_str != NULL) {
			*val_str++ = 0;
			is_write = 1;
		} else
			is_write = 0;

		if (addr_str) {
			if ((ascii2hex(addr_str, &atIdx) == 0)) {
				printf("Invalid MAC CR Addr, str=%s\n", addr_str);
				break;
			}
		}

		if (val_str) {
			if ((strlen(val_str) == 0) || ascii2hex(val_str, &atVal) == 0) {
				printf("Invalid MAC value[0x%s]\n", val_str);
				break;
			}
		}

		memset(&param, 0, sizeof(param));
		param.atidx = atIdx;
		param.value = atVal;
		if (is_write) {
			if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_TESTENGINE_SET, sizeof(param), &param))
				return -EMSGSIZE;
		} else {
			if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_TESTENGINE_GET, sizeof(param), &param))
				return -EMSGSIZE;
		}
	}

	nla_nest_end(msg, data);
	register_handler(testengine_callback, NULL);
	return 0;
}

TOPLEVEL(teng, "<atidx>|<atidx=value>|<atidx=value,atidx=value>", MTK_NL80211_VENDOR_SUBCMD_TESTENGINE, 0,
	CIB_PHY, handle_testengine_command,
	"This command is used to show/set at index to fw, e.g mwctl phy phy0 teng <atidx>=<value>");

