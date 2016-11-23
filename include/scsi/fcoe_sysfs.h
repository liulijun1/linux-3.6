/*
 * Copyright (c) 2011-2012 Intel Corporation.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Maintained at www.Open-FCoE.org
 */

#ifndef FCOE_SYSFS
#define FCOE_SYSFS

#include <linux/if_ether.h>
#include <linux/device.h>
#include <scsi/fc/fc_fcoe.h>

struct fcoe_ctlr_device;
struct fcoe_fcf_device;

struct fcoe_sysfs_function_template {
	void (*get_fcoe_ctlr_link_fail)(struct fcoe_ctlr_device *);
	void (*get_fcoe_ctlr_vlink_fail)(struct fcoe_ctlr_device *);
	void (*get_fcoe_ctlr_miss_fka)(struct fcoe_ctlr_device *);
	void (*get_fcoe_ctlr_symb_err)(struct fcoe_ctlr_device *);
	void (*get_fcoe_ctlr_err_block)(struct fcoe_ctlr_device *);
	void (*get_fcoe_ctlr_fcs_error)(struct fcoe_ctlr_device *);
	void (*get_fcoe_ctlr_mode)(struct fcoe_ctlr_device *);
	void (*get_fcoe_fcf_selected)(struct fcoe_fcf_device *);
	void (*get_fcoe_fcf_vlan_id)(struct fcoe_fcf_device *);
};

#define dev_to_ctlr(d)					\
	container_of((d), struct fcoe_ctlr_device, dev)

enum fip_conn_type {
	FIP_CONN_TYPE_UNKNOWN,
	FIP_CONN_TYPE_FABRIC,
	FIP_CONN_TYPE_VN2VN,
};

struct fcoe_ctlr_device {
	u32				id;

	struct device			dev;
	struct fcoe_sysfs_function_template *f;

	struct list_head		fcfs;
	char				work_q_name[20];
	struct workqueue_struct		*work_q;
	char				devloss_work_q_name[20];
	struct workqueue_struct		*devloss_work_q;
	struct mutex			lock;

	int                             fcf_dev_loss_tmo;
	enum fip_conn_type              mode;

	/* expected in host order for displaying */
	struct fcoe_fc_els_lesb         lesb;
};

static inline void *fcoe_ctlr_device_priv(const struct fcoe_ctlr_device *ctlr)
{
	return (void *)(ctlr + 1);
}

/* fcf states */
enum fcf_state {
	FCOE_FCF_STATE_UNKNOWN,
	FCOE_FCF_STATE_DISCONNECTED,
	FCOE_FCF_STATE_CONNECTED,
	FCOE_FCF_STATE_DELETED,
};

struct fcoe_fcf_device {
	u32		    id;
	struct device	    dev;
	struct list_head    peers;
	struct work_struct  delete_work;
	struct delayed_work dev_loss_work;
	u32		    dev_loss_tmo;
	void                *priv;
	enum fcf_state      state;

	u64                 fabric_name;
	u64                 switch_name;
	u32                 fc_map;
	u16                 vfid;
	u8                  mac[ETH_ALEN];
	u8                  priority;
	u32                 fka_period;
	u8                  selected;
	u16                 vlan_id;
};

#define dev_to_fcf(d)					\
	container_of((d), struct fcoe_fcf_device, dev)
/* parentage should never be missing */
#define fcoe_fcf_dev_to_ctlr_dev(x)		\
	dev_to_ctlr((x)->dev.parent)
#define fcoe_fcf_device_priv(x)			\
	((x)->priv)

struct fcoe_ctlr_device *fcoe_ctlr_device_add(struct device *parent,
			    struct fcoe_sysfs_function_template *f,
			    int priv_size);
void fcoe_ctlr_device_delete(struct fcoe_ctlr_device *);
struct fcoe_fcf_device *fcoe_fcf_device_add(struct fcoe_ctlr_device *,
					    struct fcoe_fcf_device *);
void fcoe_fcf_device_delete(struct fcoe_fcf_device *);

int __init fcoe_sysfs_setup(void);
void __exit fcoe_sysfs_teardown(void);

#endif /* FCOE_SYSFS */
