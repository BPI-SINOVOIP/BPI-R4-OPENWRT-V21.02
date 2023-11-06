/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

#define ACS_OPTIONS "[trigger=<1|2|3|5|6|7>] [checktime=<seconds>] [psc=<1|0>(valid for 6G)]\n"\
	"[partial_scan =<1|0>] [scan_dwell=<milliseconds>] [restore_dwell =<milliseconds>] [ch_num=<>]\n"

#define ACS_HELP "trigger acs operation\n"\
	"[trigger=1]:acs alg=ChannelAlgApCnt\n"\
	"[trigger=2]:acs alg=ChannelAlgCCA\n"\
	"[trigger=3]:acs alg=ChannelAlgBusyTime\n"\
	"[trigger=5]:acs alg=ChannelAlgApCnt and enable ch scores\n"\
	"[trigger=6]:acs alg=ChannelAlgCCA and enabel ch scores\n"\
	"[trigger=7]:acs alg=ChannelAlgBusyTime and enable ch scores\n"

#define SCAN_OPTIONS "[type=<full|partial|offch|overlap>] [clear] [psc=<1|0>(valid for 6G)] [ssid=<ssid>(valid for full scan)]\n"\
	"[ch=<target_ch>(valid for offch)] [active=<1|0>(valid for offch)] [scan_dwell=<milliseconds>(valid for offch)]\n"\
	"[ch_num=<num>(valid for partial)] [dump] [dump=<bss_start_idx>]\n"

#define MAX_ACS_PARAM_LEN 64
#define MAX_SCAN_PARAM_LEN 64
#define MAX_LEN_OF_SSID 32
#define MAX_SCAN_DUMP_LEN 4096


struct autoChSel_option {
	char option_name[MAX_ACS_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg, char *value);
};

struct scan_option_1 {
	char option_name[MAX_SCAN_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg);
};

struct scan_option_2 {
	char option_name[MAX_SCAN_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg, char *value);
};

int acs_trigger_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char sel;

	if (!value)
		return -EINVAL;

	switch (*value) {
	case '1':
		sel = 1;
		break;
	case '2':
		sel = 2;
		break;
	case '3':
		sel = 3;
		break;
	case '5':
		sel = 5;
		break;
	case '6':
		sel = 6;
		break;
	case '7':
		sel = 7;
		break;
	default:
		sel = 0;
	}

	if (sel == 0)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL, sel))
		return -EMSGSIZE;

	return 0;
}

int acs_checktime_attr_put(struct nl_msg *msg, char *value)
{
	u32 check_time;

	if (!value)
		return -EINVAL;

	check_time = strtoul(value, NULL, 10);

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_CHECK_TIME, check_time))
		return -EMSGSIZE;

	return 0;
}

int acs_6g_psc_attr_put(struct nl_msg *msg, char *value)
{
	u8 psc_enable;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		psc_enable = 1;
	else if (*value == '0')
		psc_enable = 0;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_6G_PSC, psc_enable))
		return -EMSGSIZE;

	return 0;
}

int acs_partial_scan_attr_put(struct nl_msg *msg, char *value)
{
	u8 partial_scan_enable;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		partial_scan_enable = 1;
	else if (*value == '0')
		partial_scan_enable = 0;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_PARTIAL, partial_scan_enable))
		return -EMSGSIZE;

	return 0;
}

int acs_scan_dwell_attr_put(struct nl_msg *msg, char *value)
{
	u16 scan_dwell;

	if (!value)
		return -EINVAL;
	scan_dwell = strtoul(value, NULL, 10);

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_SCANNING_DWELL, scan_dwell))
		return -EMSGSIZE;

	return 0;
}

int acs_restore_dwell_attr_put(struct nl_msg *msg, char *value)
{
	u16 restore_dwell;

	if (!value)
		return -EINVAL;
	restore_dwell = strtoul(value, NULL, 10);

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_RESTORE_DWELL, restore_dwell))
		return -EMSGSIZE;

	return 0;
}

int acs_ch_num_attr_put(struct nl_msg *msg, char *value)
{
	u16 ch_num;

	if (!value)
		return -EINVAL;
	ch_num = strtoul(value, NULL, 10);

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_NUM, ch_num))
		return -EMSGSIZE;

	return 0;
}



struct autoChSel_option acs_opt[] = {
	{"trigger", acs_trigger_attr_put},
	{"checktime", acs_checktime_attr_put},
	{"psc", acs_6g_psc_attr_put},
	{"partial_scan", acs_partial_scan_attr_put},
	{"scan_dwell", acs_scan_dwell_attr_put},
	{"restore_dwell", acs_restore_dwell_attr_put},
	{"ch_num", acs_ch_num_attr_put},
};

struct scan_type_option  {
	char type_name[MAX_SCAN_PARAM_LEN];
	enum mtk_vendor_attr_scantype type;
};

struct scan_type_option type_opt[] = {
	{"full", NL80211_FULL_SCAN},
	{"partial", NL80211_PARTIAL_SCAN},
	{"offch", NL80211_OFF_CH_SCAN},
	{"overlap", NL80211_2040_OVERLAP_SCAN},
};

int scan_type_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char i;

	if (!value)
		return -EINVAL;

	for (i = 0; i < (sizeof(type_opt)/sizeof(type_opt[0])); i++) {
		if (strlen(type_opt[i].type_name) == strlen(value) &&
			!strncmp(type_opt[i].type_name, value, strlen(value))) {
			if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_SCAN_TYPE, type_opt[i].type))
				return -EMSGSIZE;
			else
				return 0;
		}
	}

	if (i == sizeof(type_opt)/sizeof(type_opt[0]))
		return -EINVAL;

	return 0;
}

int scan_clear_attr_put(struct nl_msg *msg)
{
	if (nla_put_flag(msg, MTK_NL80211_VENDOR_ATTR_SCAN_CLEAR))
		return -EMSGSIZE;

	return 0;
}

int scan_ssid_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char len;

	len = strlen(value);

	if (len > MAX_LEN_OF_SSID)
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_SCAN_SSID, len, value))
		return -EMSGSIZE;

	return 0;
}

int partial_scan_ch_num_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char num_of_ch;

	if (!value)
		return -EINVAL;

	num_of_ch = strtoul(value, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_PARTIAL_SCAN_NUM_OF_CH, num_of_ch))
		return -EMSGSIZE;

	return 0;
}

int offch_scan_target_ch_attr_put(struct nl_msg *msg, char *value)
{
	u32 ch;

	if (!value)
		return -EINVAL;

	ch = strtoul(value, NULL, 10);

	if (ch == 0)
		return -EINVAL;

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_TARGET_CH, ch))
		return -EMSGSIZE;

	return 0;
}

int offch_scan_active_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char active;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		active = 1;
	else if (*value == '0')
		active = 0;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_ACTIVE, active))
		return -EMSGSIZE;

	return 0;
}

int offch_scan_duration_attr_put(struct nl_msg *msg, char *value)
{
	u32 duration;

	if (!value)
		return -EINVAL;

	duration = strtoul(value, NULL, 10);

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_DURATION, duration))
		return -EMSGSIZE;

	return 0;
}

int scan_dump_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_SCAN_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char *show_str = NULL;
	int err = 0;
	u32 scan_result_len = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_SCAN_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT]) {
			scan_result_len = nla_len(vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT]);
			show_str = nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT]);
			if (scan_result_len > MAX_SCAN_DUMP_LEN || scan_result_len <= 0) {
				printf("the scan result len is invalid !!!\n");
				return -EINVAL;
			} else if (*(show_str + scan_result_len - 1) != '\0') {
				printf("the result string is not ended with right terminator, handle it!!!\n");
				*(show_str + scan_result_len - 1) = '\0';
			}
			printf("%s\n", show_str);
		} else
			printf("no scan result attr\n");
	} else
		printf("no any scan result from driver\n");

	return 0;
}

int scan_dump_attr_put(struct nl_msg *msg)
{
	u32 bss_start_idx = 0;

	register_handler(scan_dump_callback, NULL);
	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_SCAN_DUMP_BSS_START_INDEX, bss_start_idx))
		return -EMSGSIZE;

	return 0;
}

int scan_dump_with_bss_idx_attr_put(struct nl_msg *msg, char *value)
{
	u32 bss_start_idx;

	if (!value)
		bss_start_idx = 0;
	else
		bss_start_idx = strtoul(value, NULL, 10);

	register_handler(scan_dump_callback, NULL);
	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_SCAN_DUMP_BSS_START_INDEX, bss_start_idx))
		return -EMSGSIZE;

	return 0;
}

int scan_6G_psc_attr_put(struct nl_msg *msg, char *value)
{
	u8 psc_en;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		psc_en = 1;
	else if (*value == '0')
		psc_en = 0;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_6G_PSC_SCAN_EN, psc_en))
		return -EMSGSIZE;

	return 0;
}

/*scan_opt1, defines the scan commands that trigger without parameters
**example: scan clear
*/
struct scan_option_1 scan_opt1[] = {
	{"clear", scan_clear_attr_put},
	{"dump", scan_dump_attr_put},
};

/*scan_opt2, defines the scan commands that trigger with parameters
**example: scan type=full
*/
struct scan_option_2 scan_opt2[] = {
	{"type", scan_type_attr_put},
	{"ssid", scan_ssid_attr_put},
	{"ch_num", partial_scan_ch_num_attr_put},
	{"ch", offch_scan_target_ch_attr_put},
	{"active", offch_scan_active_attr_put},
	{"scan_dwell", offch_scan_duration_attr_put},
	{"dump", scan_dump_with_bss_idx_attr_put},
	{"psc", scan_6G_psc_attr_put},
};

int handle_auto_ch_sel_set(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *ptr, *param_str, *val_str, invalide = 0;
	int i, j;

	if (!argc)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	for (i = 0; i < argc; i++) {
		ptr = argv[i];
		param_str = ptr;
		val_str = strchr(ptr, '=');

		if (!val_str)
			continue;

		*val_str++ = 0;

		for (j = 0; j < (sizeof(acs_opt) / sizeof(acs_opt[0])); j++) {
			if (strlen(acs_opt[j].option_name) == strlen(param_str) &&
				!strncmp(acs_opt[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(acs_opt) / sizeof(acs_opt[0]))) {
			if (acs_opt[j].attr_put(msg, val_str) < 0)
				printf("invalide argument %s=%s, ignore it\n", param_str, val_str);
			else
				invalide = 1;
		}
	}

	nla_nest_end(msg, data);

	if (!invalide)
		return -EINVAL;
	return 0;
}

int handle_scan_set(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *ptr, *param_str, *val_str, invalide = 0;
	int i, j;

	if (!argc)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	for (i = 0; i < argc; i++) {
		ptr = argv[i];
		param_str = ptr;

		/*scan_opt1 parse*/
		for (j = 0; j < (sizeof(scan_opt1) / sizeof(scan_opt1[0])); j++) {
			if (strlen(scan_opt1[j].option_name) == strlen(param_str) &&
				!strncmp(scan_opt1[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(scan_opt1) / sizeof(scan_opt1[0]))) {
			if (scan_opt1[j].attr_put(msg) < 0)
				printf("param_str %s att_put fail\n", param_str);
			else
				invalide = 1;
			continue;
		}

		/*scan_opt2 parse*/
		val_str = strchr(ptr, '=');

		if (!val_str)
			continue;

		*val_str++ = 0;

		for (j = 0; j < (sizeof(scan_opt2) / sizeof(scan_opt2[0])); j++) {
			if (strlen(scan_opt2[j].option_name) == strlen(param_str) &&
				!strncmp(scan_opt2[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(scan_opt2) / sizeof(scan_opt2[0]))) {
			if (scan_opt2[j].attr_put(msg, val_str) < 0)
				printf("invalide argument %s=%s, ignore it\n", param_str, val_str);
			else
				invalide = 1;
		}
	}

	nla_nest_end(msg, data);

	if (!invalide)
		return -EINVAL;
	return 0;
}

TOPLEVEL(acs, ACS_OPTIONS, MTK_NL80211_VENDOR_SUBCMD_SET_AUTO_CH_SEL, 0, CIB_NETDEV, handle_auto_ch_sel_set,
	ACS_HELP);

TOPLEVEL(scan, SCAN_OPTIONS, MTK_NL80211_VENDOR_SUBCMD_SET_SCAN, 0, CIB_NETDEV, handle_scan_set,
	"trigger scan operation\n");

