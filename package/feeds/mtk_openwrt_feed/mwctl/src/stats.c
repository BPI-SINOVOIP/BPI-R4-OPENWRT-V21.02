/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

struct radio_stats {
	unsigned long BytesSent;
	unsigned long BytesReceived;
	unsigned long PacketsSent;
	unsigned long PacketsReceived;
	unsigned long ErrorsSent;
	unsigned long ErrorsReceived;
	unsigned long DiscardPacketsSent;
	unsigned long DiscardPacketsReceived;
	unsigned int PLCPErrorCount;
	unsigned int FCSErrorCount;
	unsigned int InvalidMACCount;
	unsigned int PacketsOtherReceived;
	unsigned long CtsReceived;
	unsigned long NoCtsReceived;
	unsigned long FrameHeaderError;
	unsigned long GoodPLCPReceived;
	unsigned long DPacketOtherMACReceived;
	unsigned long MPacketOtherMACReceived;
	unsigned long CPacketOtherMACReceived;
	unsigned long CtsOtherMACReceived;
	unsigned long RtsOtherMACReceived;
	unsigned int TotalChannelChangeCount;
	unsigned int ManualChannelChangeCount;
	unsigned int AutoStartupChannelChangeCount;
	unsigned int AutoUserChannelChangeCount;
	unsigned int AutoRefreshChannelChangeCount;
	unsigned int AutoDynamicChannelChangeCount;
	unsigned int AutoDFSChannelChangeCount;
	unsigned long UnicastPacketsSent;
	unsigned long UnicastPacketsReceived;
	unsigned long MulticastPacketsSent;
	unsigned long MulticastPacketsReceived;
	unsigned long BroadcastPacketsSent;
	unsigned long BroadcastPacketsReceived;
};

struct bss_stats {
	unsigned long BytesSent;
	unsigned long BytesReceived;
	unsigned long PacketsSent;
	unsigned long PacketsReceived;
	unsigned int ErrorsSent;
	unsigned int RetransCount;
	unsigned int FailedRetransCount;
	unsigned int RetryCount;
	unsigned int MultipleRetryCount;
	unsigned int ACKFailureCount;
	unsigned int AggregatedPacketCount;
	unsigned int ErrorsReceived;
	unsigned long UnicastPacketsSent;
	unsigned long UnicastPacketsReceived;
	unsigned int DiscardPacketsSent;
	unsigned int DiscardPacketsReceived;
	unsigned long MulticastPacketsSent;
	unsigned long MulticastPacketsReceived;
	unsigned long BroadcastPacketsSent;
	unsigned long BroadcastPacketsReceived;
	unsigned int UnknownProtoPacketsReceived;
	unsigned long DiscardPacketsSentBufOverflow;
	unsigned long DiscardPacketsSentNoAssoc;
	unsigned long FragSent;
	unsigned long SentNoAck;
	unsigned long DupReceived;
	unsigned long TooLongReceived;
	unsigned long TooShortReceived;
	unsigned long AckUcastReceived;
};

struct sta_txrx_stats {
	unsigned long BytesSent;
	unsigned long BytesReceived;
	unsigned long PacketsSent;
	unsigned long PacketsReceived;
	unsigned int ErrorsSent;
	unsigned int ErrorsReceived;
	unsigned int RetransCount;
	unsigned int FailedRetransCount;
	unsigned int RetryCount;
	unsigned int MultipleRetryCount;
};

struct sta_txrx_rates {
	unsigned int TxRate;
	unsigned int RxRate;
	unsigned int RxRate_rt;
	unsigned int TxRate_rt;
	unsigned int avg_tx_rate;
	unsigned int avg_rx_rate;
};

struct station {
	unsigned char MacAddr[ETH_ALEN];
	unsigned char VMacAddr[ETH_ALEN];
	unsigned short PhyMode;
	bool AuthenticationState;
	unsigned int LastConnectTime;
	signed short SignalStrength;
	signed short Noise;
	unsigned char Retransmissions;
	unsigned int UtilizationReceive;
	unsigned int UtilizationTransmit;
	struct sta_txrx_stats stats;
	struct sta_txrx_rates rates;
	bool repeater;
	bool valid;
};

DECLARE_SECTION(dump);

int radio_stat_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_RADIO_STATS_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct radio_stats *stat;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_RADIO_STATS_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_RADIO_STATS]) {
			stat = (struct radio_stats*)nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_RADIO_STATS]);
			printf("ByteSent %ld ByteReceived %ld\n",stat->BytesSent,stat->BytesReceived);
		}
	} else
		printf("No Stats from driver\n");

	return 0;
}

int radio_stats_handle(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	u8 stats[4];

	if (argc > 1)
		return -EINVAL;

	memset(stats, 0, 4);
	register_handler(radio_stat_callback, NULL);

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_RADIO_STATS, 0, stats))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(dump, radio_stats, "",
	MTK_NL80211_VENDOR_SUBCMD_GET_RADIO_STATS, 0, CIB_NETDEV, radio_stats_handle,
	"This command is used to get radio stats");

int bss_stat_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_BSS_STATS_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct bss_stats *stat;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_BSS_STATS_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_BSS_STATS]) {
			stat = (struct bss_stats*)nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_BSS_STATS]);
			printf("ByteSent %ld ByteReceived %ld\n",stat->BytesSent,stat->BytesReceived);
		}
	} else
		printf("No Stats from driver\n");

	return 0;
}

int bss_stats_handle(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	u8 stats[4];

	if (argc > 1)
		return -EINVAL;

	memset(stats, 0, 4);
	register_handler(bss_stat_callback, NULL);

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_BSS_STATS, 0, stats))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(dump, bss_stats, "",
	MTK_NL80211_VENDOR_SUBCMD_GET_BSS_STATS, 0, CIB_NETDEV, bss_stats_handle,
	"This command is used to get BSS stats");

int sta_stat_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct station *stat;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_STA]) {
			stat = (struct station*)nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_STA]);
			printf("ByteSent %ld ByteReceived %ld\n",stat->stats.BytesSent,stat->stats.BytesReceived);
		}
	} else
		printf("No Stats from driver\n");

	return 0;
}

int sta_stats_handle(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	u8 Addr[ETH_ALEN];
	int matches;

	if (!argc || argc != 1)
		return -EINVAL;

	register_handler(sta_stat_callback, NULL);

	matches = sscanf(argv[0], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			Addr, Addr+1, Addr+2, Addr+3, Addr+4, Addr+5);

	if (matches != ETH_ALEN)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_STA, ETH_ALEN, Addr))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(dump, sta_stats, "<mac_addr>(xx:xx:xx:xx:xx:xx)",
	MTK_NL80211_VENDOR_SUBCMD_GET_STA, 0, CIB_NETDEV, sta_stats_handle,
	"This command is used to get station stats");

