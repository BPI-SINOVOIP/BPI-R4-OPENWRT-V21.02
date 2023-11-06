// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 MediaTek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <linux/device.h>

#include "firmware.h"
#include "hpdma.h"
#include "internal.h"
#include "mcu.h"
#include "netsys.h"
#include "trm.h"
#include "tunnel.h"
#include "wdt.h"

static const char *tops_role_name[__TOPS_ROLE_TYPE_MAX] = {
	[TOPS_ROLE_TYPE_MGMT] = "tops-mgmt",
	[TOPS_ROLE_TYPE_CLUSTER] = "tops-offload",
};

static ssize_t mtk_tops_fw_info_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	enum tops_role_type rtype;
	struct tm tm = {0};
	const char *value;
	const char *prop;
	int len = 0;
	u32 nattr;
	u32 i;

	for (rtype = TOPS_ROLE_TYPE_MGMT; rtype < __TOPS_ROLE_TYPE_MAX; rtype++) {
		mtk_tops_fw_get_built_date(rtype, &tm);

		len += snprintf(buf + len, PAGE_SIZE - len,
				"%s FW information:\n", tops_role_name[rtype]);
		len += snprintf(buf + len, PAGE_SIZE - len,
				"Git revision:\t%llx\n",
				mtk_tops_fw_get_git_commit_id(rtype));
		len += snprintf(buf + len, PAGE_SIZE - len,
				"Build date:\t%04ld/%02d/%02d %02d:%02d:%02d\n",
				tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
				tm.tm_hour, tm.tm_min, tm.tm_sec);

		nattr = mtk_tops_fw_attr_get_num(rtype);

		for (i = 0; i < nattr; i++) {
			prop = mtk_tops_fw_attr_get_property(rtype, i);
			if (!prop)
				continue;

			value = mtk_tops_fw_attr_get_value(rtype, prop);

			len += snprintf(buf + len, PAGE_SIZE - len,
					"%s:\t%s\n", prop, value);
		}
		len += snprintf(buf + len, PAGE_SIZE - len, "\n");
	}

	return len;
}

static int mtk_tops_ctrl_fetch_port(const char *buf, int *ofs, u16 *port)
{
	int nchar = 0;
	int ret;
	u16 p = 0;

	ret = sscanf(buf + *ofs, "%hu %n", &p, &nchar);
	if (ret != 1)
		return -EPERM;

	*port = htons(p);

	*ofs += nchar;

	return nchar;
}

static int mtk_tops_ctrl_fetch_ip(const char *buf, int *ofs, u32 *ip)
{
	int nchar = 0;
	int ret = 0;
	u8 tmp[4];

	ret = sscanf(buf + *ofs, "%hhu.%hhu.%hhu.%hhu %n",
		&tmp[3], &tmp[2], &tmp[1], &tmp[0], &nchar);
	if (ret != 4)
		return -EPERM;

	*ip = tmp[0] | tmp[1] << 8 | tmp[2] << 16 | tmp[3] << 24;

	*ofs += nchar;

	return nchar;
}

static int mtk_tops_ctrl_fetch_mac(const char *buf, int *ofs, u8 *mac)
{
	int nchar = 0;
	int ret = 0;

	ret = sscanf(buf + *ofs, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx %n",
		&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5], &nchar);
	if (ret != 6)
		return -EPERM;

	*ofs += nchar;

	return 0;
}

static int mtk_tops_ctrl_add_tnl(const char *buf)
{
	struct tops_tnl_params tnl_params = {0};
	struct tops_tnl_info *tnl_info;
	struct tops_tnl_type *tnl_type;
	char tnl_type_name[21] = {0};
	int ofs = 0;
	int ret = 0;

	ret = sscanf(buf, "%20s %n", tnl_type_name, &ofs);
	if (ret != 1)
		return -EPERM;

	tnl_type = mtk_tops_tnl_type_get_by_name(tnl_type_name);
	if (unlikely(!tnl_type || !tnl_type->tnl_debug_param_setup))
		return -ENODEV;

	ret = mtk_tops_ctrl_fetch_mac(buf, &ofs, tnl_params.daddr);
	if (ret < 0)
		return ret;

	ret = mtk_tops_ctrl_fetch_mac(buf, &ofs, tnl_params.saddr);
	if (ret < 0)
		return ret;

	ret = mtk_tops_ctrl_fetch_ip(buf, &ofs, &tnl_params.dip);
	if (ret < 0)
		return ret;

	ret = mtk_tops_ctrl_fetch_ip(buf, &ofs, &tnl_params.sip);
	if (ret < 0)
		return ret;

	ret = mtk_tops_ctrl_fetch_port(buf, &ofs, &tnl_params.dport);
	if (ret < 0)
		return ret;

	ret = mtk_tops_ctrl_fetch_port(buf, &ofs, &tnl_params.sport);
	if (ret < 0)
		return ret;

	ret = tnl_type->tnl_debug_param_setup(buf, &ofs, &tnl_params);
	if (ret < 0)
		return ret;

	tnl_params.flag |= TNL_DECAP_ENABLE;
	tnl_params.flag |= TNL_ENCAP_ENABLE;
	tnl_params.tops_entry_proto = tnl_type->tops_entry;

	tnl_info = mtk_tops_tnl_info_alloc(tnl_type);
	if (IS_ERR(tnl_info))
		return -ENOMEM;

	tnl_info->flag |= TNL_INFO_DEBUG;
	memcpy(&tnl_info->cache, &tnl_params, sizeof(struct tops_tnl_params));

	mtk_tops_tnl_info_hash(tnl_info);

	mtk_tops_tnl_info_submit(tnl_info);

	return 0;
}

static ssize_t mtk_tops_tnl_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf,
				  size_t count)
{
	char cmd[21] = {0};
	int nchar = 0;
	int ret = 0;

	ret = sscanf(buf, "%20s %n", cmd, &nchar);

	if (ret != 1)
		return -EPERM;

	if (!strcmp(cmd, "NEW_TNL")) {
		ret = mtk_tops_ctrl_add_tnl(buf + nchar);
		if (ret)
			return ret;
	}

	return count;
}

static int mtk_tops_trm_fetch_setting(const char *buf,
				      int *ofs,
				      char *name,
				      u32 *offset,
				      u32 *size,
				      u8 *enable)
{
	int nchar = 0;
	int ret = 0;

	ret = sscanf(buf + *ofs, "%31s %x %x %hhx %n",
		name, offset, size, enable, &nchar);
	if (ret != 4)
		return -EPERM;

	*ofs += nchar;

	return nchar;
}

static ssize_t mtk_tops_trm_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf,
				  size_t count)
{
	char name[TRM_CONFIG_NAME_MAX_LEN] = { 0 };
	char cmd[21] = { 0 };
	int nchar = 0;
	int ret = 0;
	u32 offset;
	u8 enable;
	u32 size;

	ret = sscanf(buf, "%20s %n", cmd, &nchar);
	if (ret != 1)
		return -EPERM;

	if (!strcmp(cmd, "trm_dump")) {
		ret = mtk_trm_dump(TRM_RSN_NULL);
		if (ret)
			return ret;
	} else if (!strcmp(cmd, "trm_cfg_setup")) {
		ret = mtk_tops_trm_fetch_setting(buf, &nchar,
			name, &offset, &size, &enable);
		if (ret < 0)
			return ret;

		ret = mtk_trm_cfg_setup(name, offset, size, enable);
		if (ret)
			return ret;
	}

	return count;
}

static ssize_t mtk_tops_wdt_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf,
				  size_t count)
{
	char cmd[21] = {0};
	u32 core = 0;
	u32 i;
	int ret;

	ret = sscanf(buf, "%20s %x", cmd, &core);
	if (ret != 2)
		return -EPERM;

	core &= CORE_TOPS_MASK;
	if (!strcmp(cmd, "WDT_TO")) {
		for (i = 0; i < CORE_TOPS_NUM; i++) {
			if (core & 0x1)
				mtk_tops_wdt_trigger_timeout(i);
			core >>= 1;
		}
	} else {
		return -EINVAL;
	}

	return count;
}

static DEVICE_ATTR_RO(mtk_tops_fw_info);
static DEVICE_ATTR_WO(mtk_tops_tnl);
static DEVICE_ATTR_WO(mtk_tops_trm);
static DEVICE_ATTR_WO(mtk_tops_wdt);

static struct attribute *mtk_tops_attributes[] = {
	&dev_attr_mtk_tops_fw_info.attr,
	&dev_attr_mtk_tops_tnl.attr,
	&dev_attr_mtk_tops_trm.attr,
	&dev_attr_mtk_tops_wdt.attr,
	NULL,
};

static const struct attribute_group mtk_tops_attr_group = {
	.name = "mtk_tops",
	.attrs = mtk_tops_attributes,
};

int mtk_tops_ctrl_init(struct platform_device *pdev)
{
	int ret = 0;

	ret = sysfs_create_group(&pdev->dev.kobj, &mtk_tops_attr_group);
	if (ret) {
		TOPS_ERR("create sysfs failed\n");
		return ret;
	}

	return ret;
}

void mtk_tops_ctrl_deinit(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &mtk_tops_attr_group);
}
