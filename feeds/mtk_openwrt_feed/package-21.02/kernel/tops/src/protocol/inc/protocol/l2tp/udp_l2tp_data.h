/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 MediaTek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _TOPS_UDP_L2TP_DATA_H_
#define _TOPS_UDP_L2TP_DATA_H_

#if defined(CONFIG_MTK_TOPS_UDP_L2TP_DATA)
int mtk_tops_udp_l2tp_data_init(void);
void mtk_tops_udp_l2tp_data_deinit(void);
#else /* !defined(CONFIG_MTK_TOPS_UDP_L2TP_DATA) */
static inline int mtk_tops_udp_l2tp_data_init(void)
{
	return 0;
}

static inline void mtk_tops_udp_l2tp_data_deinit(void)
{
}
#endif /* defined(CONFIG_MTK_TOPS_UDP_L2TP_DATA) */
#endif /* _TOPS_UDP_L2TP_DATA_H_ */
