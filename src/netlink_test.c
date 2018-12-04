
/*!
 * \file 	netlink_test.c
 * \brief 	Red/Write functions for netlink
 * \author 	O.D
 * \version 0.1
 * \date 	November 4th, 2018
 *
 * O.D - November 4th, 2018
 * File creation.
 * Define RW funtions for netlink ipc between kernel and user space.
 *
 */
 
#include <linux/moduleparam.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <net/genetlink.h>
#include <net/netlink.h>

#include "netlink_test.h"

unsigned short genl_dbg_level = 4;
module_param(genl_dbg_level, ushort, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ntc_genl_dbg_level, " enable debug traces (1 to 4, 4 is most verbose)");

static int handle_cmd_info_req(struct sk_buff *p_skb, struct genl_info *p_info);
static int handle_cmd_param_req(struct sk_buff *p_skb, struct genl_info *p_info);
static int send_conf_to_userspace(uint32_t portid, uint32_t seqnum, enum EN_CMD cmd);

/* attribute policy */
static struct nla_policy control_genl_policy[EN_ATTR_CTL_MAX + 1] = {
	[EN_ATTR_GET_INFO_REQ] = {.type = NLA_UNSPEC},
	[EN_ATTR_GET_INFO_CONF] = {.type = NLA_NESTED},
	[EN_ATTR_SET_PARAM_REQ] = {.type = NLA_NESTED},
	[EN_ATTR_SET_PARAM_CONF] = {.type = NLA_U16},
};

static struct nla_policy param_genl_policy[EN_ATTR_SET_PARAM_REQ_MAX + 1] = {
	[EN_ATTR_SET_PARAM_REQ_DBG_LEVEL] = {.type = NLA_U16},
};

static const struct genl_multicast_group genl_mcgrp[] = {
	[MOD_GROUP_HELLO_ID] = {.name = MOD_GROUP_HELLO_NAME},
};

/* operation definition */
static struct genl_ops mod_genl_ops[] = {
	{
		.cmd = EN_CMD_GET_INFO_REQ,
		.flags = 0,
		.policy = control_genl_policy,
		.doit = handle_cmd_info_req,
		.dumpit = NULL,
	},
	{
		.cmd = EN_CMD_SET_PARAM_REQ,
		.flags = 0,
		.policy = control_genl_policy,
		.doit = handle_cmd_param_req,
		.dumpit = NULL,
	},
};

/* family definition */
static struct genl_family control_genl_family = {
	.hdrsize = 0,
	.name = MOD_GENL_NAME,
	.version = MOD_GENL_VERSION,
	.maxattr = EN_ATTR_CTL_MAX,
	.ops = &mod_genl_ops[0],
	.n_ops = ARRAY_SIZE(mod_genl_ops),
	.mcgrps = genl_mcgrp,
	.n_mcgrps = ARRAY_SIZE(genl_mcgrp),
};

/*!
 * \fn 		netlink_test_init
 * \brief 	Module init
 * \param 	void
 * \return 	0 if no error, else -1.
 */
int netlink_test_init(void)
{
	int ret;

	ret = genl_register_family(&control_genl_family);
	if (ret != 0) {
		debugf2("error registering control genl family\n");
		return -EINVAL;
	}
	debugf2("successfully registered control genl family\n");

	return 0;
}

/*!
 * \fn 		netlink_test_exit
 * \brief 	Module exit
 * \param 	void
 * \return 	void
 */
void netlink_test_exit(void)
{
	genl_unregister_family(&control_genl_family); 
	debugf2("control genl family is now unregistered\n");
}

/*!
 * \fn 		handle_cmd_info_req
 * \brief 	This function is called by the user space in order to request the info structure
 * \param 	struct sk_buff *, struct genl_info *
 * \return 	int
 */
int handle_cmd_info_req(struct sk_buff *p_skb, struct genl_info *p_info)
{
	int ret = -EINVAL;
	struct nlmsghdr *p_nh = p_info->nlhdr; 

	debugf4("info req received\n");

	/* sanity checks */
	if (!p_skb || !p_info) {
		debugf4("invalid input parameters\n");
		return -EFAULT;
	}
	p_nh = p_info->nlhdr;
	if (p_nh == NULL) {
		debugf4("invalid header\n");
		return -EFAULT;
	}
	if (!p_info->attrs[EN_ATTR_GET_INFO_REQ]) {
		debugf4("invalid cmd id\n");
		return ret;
	}
	
	/* send the confirm back to user space */
	send_conf_to_userspace(p_nh->nlmsg_pid, p_nh->nlmsg_seq, EN_CMD_GET_INFO_REQ);

	return 0;
}

/*!
 * \fn 		handle_cmd_param_req
 * \brief 	This function is called by the user space in order to set parameters
 * \param 	struct sk_buff *, struct genl_info *
 * \return 	int
 */
int handle_cmd_param_req(struct sk_buff *p_skb, struct genl_info *p_info)
{
	int ret = -EINVAL;
	struct nlmsghdr *p_nh = p_info->nlhdr; 
	struct nlattr *p_attr, *attr_nested[EN_ATTR_SET_PARAM_REQ_MAX + 1];

	debugf4("param req received\n");

	/* sanity checks */
	if (!p_skb || !p_info) {
		debugf4("invalid input parameters\n");
		return -EFAULT;
	}
	p_nh = p_info->nlhdr;
	if (p_nh == NULL) {
		debugf4("invalid header\n");
		return -EFAULT;
	}
	if (!p_info->attrs[EN_ATTR_SET_PARAM_REQ]) {
		debugf4("invalid cmd id\n");
		return ret;
	}
	
	/* set param */
	p_attr = p_info->attrs[EN_CMD_SET_PARAM_REQ];
	ret = nla_parse_nested(attr_nested, EN_ATTR_SET_PARAM_REQ_MAX, p_attr, param_genl_policy, NULL);
	if (ret > 0) {
		debugf4("nested parsing failed\n");
		return ret;
	}
	if (!attr_nested[EN_ATTR_SET_PARAM_REQ_DBG_LEVEL])
		return -EINVAL;
	else
		genl_dbg_level = nla_get_u16(attr_nested[EN_ATTR_SET_PARAM_REQ_DBG_LEVEL]);

	/* send the confirm back to user space */
	send_conf_to_userspace(p_nh->nlmsg_pid, p_nh->nlmsg_seq, EN_CMD_SET_PARAM_REQ);

	return 0;
}

/*!
 * \fn 		send_conf_to_userspace
 * \brief 	This function builds and send the confirm back to the user space
 * \param 	uint32_t, uint32_t, enum EN_CMD
 * \return 	int
 */
int send_conf_to_userspace(uint32_t portid, uint32_t seqnum, enum EN_CMD cmd)
{
	struct sk_buff *p_skb;
	void *p_msg_head;
	int ret = 0;
	struct nlattr *p_opts;

	p_skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (p_skb == NULL) {
		debugf4("invalid sk_buff\n");
		return -ENOMEM;
	}
	/* build message */
	switch (cmd) {
	case EN_CMD_GET_INFO_REQ:
		/* create message header */
		p_msg_head = genlmsg_put(p_skb, portid, seqnum, &control_genl_family, 0, EN_CMD_GET_INFO_CONF);
		if (p_msg_head == NULL) {
			ret = -ENOMEM;
			debugf4("header error for EN_CMD_INFO_CONF\n");
			goto failure;
		}
		/* create message payload */
		p_opts = nla_nest_start(p_skb, EN_ATTR_GET_INFO_CONF);
		nla_put_u16(p_skb, EN_ATTR_GET_INFO_CONF_DBG_LEVEL, genl_dbg_level);
		nla_put_string(p_skb, EN_ATTR_GET_INFO_CONF_MOD_VER, MOD_VER);
		nla_nest_end(p_skb, p_opts);
		break;
	case EN_CMD_SET_PARAM_REQ:
		/* create message header */
		p_msg_head = genlmsg_put(p_skb, portid, seqnum, &control_genl_family, 0, EN_CMD_SET_PARAM_CONF);
		if (p_msg_head == NULL) {
			ret = -ENOMEM;
			debugf4("header error for EN_CMD_INFO_CONF\n");
			goto failure;
		}
		/* create message payload */
		p_opts = nla_nest_start(p_skb, EN_ATTR_SET_PARAM_CONF);
		nla_put_u16(p_skb, STATUS_OK, genl_dbg_level);
		nla_nest_end(p_skb, p_opts);
		break;
	default:
		ret = -EINVAL;
		debugf4("unknown command\n");
		goto failure;
		break;
	}
	
	/* end of message */
	genlmsg_end(p_skb, p_msg_head);

	/* send unicast message to userland */
	ret = genlmsg_unicast(&init_net, p_skb, portid);
	if (ret < 0) {
		/**
		 * Don't free skbuff.
		 * It is alredy deallocated by genlmsg_unicast.
		 */
		debugf4("error sending unicast\n");
		return -ESRCH;
	}

	return 0;

failure:
	if (p_skb != NULL)
		nlmsg_free(p_skb);

	return ret;
}

