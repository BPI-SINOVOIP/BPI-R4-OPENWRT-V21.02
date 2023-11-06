/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 MediaTek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _TOPS_PPP_H_
#define _TOPS_PPP_H_

/* Limited support: ppp header, no options */
struct ppp_hdr {
	u8 addr;
	u8 ctrl;
	u16 proto;
};

#endif /* _TOPS_PPP_H_ */
