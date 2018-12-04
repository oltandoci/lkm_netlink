#ifndef NETLINK_TEST_H_
#define NETLINK_TEST_H_

#include <linux/printk.h>

/*!
 * \file 	netlink_test.h
 * \brief 	Header file for the module
 * \author 	O.D
 * \version 0.1
 * \date 	November 14th, 2018
 */

#define MOD_VER "0.0.0"
#define MOD_LIC "GPL"
#define MOD_DESCRIPT "test module using libnl for netlink ipc between kernel and user space"
#define MOD_ALIAS "perso_netlink"

#define MOD_GENL_NAME "mod"
#define MOD_GENL_VERSION 0
#define MOD_GROUP_HELLO_ID 0
#define MOD_GROUP_HELLO_NAME "hello_grp" /* max 16 char */

#define STATUS_OK 0

extern unsigned short genl_dbg_level;

#define genl_dbg_printk(level, fmt, arg...)	\
	do {	\
		if (level <= genl_dbg_level)	\
			pr_debug("mod: [mod] %s: " fmt, __func__, ##arg);	\
	} while (0);

#define debugf0( ... ) genl_dbg_printk(0, __VA_ARGS__ )
#define debugf1( ... ) genl_dbg_printk(1, __VA_ARGS__ )
#define debugf2( ... ) genl_dbg_printk(2, __VA_ARGS__ )
#define debugf3( ... ) genl_dbg_printk(3, __VA_ARGS__ )
#define debugf4( ... ) genl_dbg_printk(4, __VA_ARGS__ )

/***** attributes *****/
/* high level control "messages" (get/set) */
enum {
		EN_ATTR_INVALID,
		EN_ATTR_GET_INFO_REQ, /* from user space */
		EN_ATTR_GET_INFO_CONF, /* from kernel */
		EN_ATTR_SET_PARAM_REQ, /* from user space */
		EN_ATTR_SET_PARAM_CONF, /* from kernel */
		__EN_ATTR_CTL_MAX,
};

#define EN_ATTR_CTL_MAX (__EN_ATTR_CTL_MAX - 1)
/* "info" confirm structure */
enum {
		EN_ATTR_GET_INFO_CONF_INVALID,
		EN_ATTR_GET_INFO_CONF_DBG_LEVEL,
		EN_ATTR_GET_INFO_CONF_MOD_VER,
		__EN_ATTR_GET_INFO_CONF_MAX
};

#define EN_ATTR_GET_INFO_CONF_MAX (__EN_ATTR_GET_INFO_CONF_MAX - 1)
/* "param" request structure */
enum {
		EN_ATTR_SET_PARAM_REQ_INVALID,
		EN_ATTR_SET_PARAM_REQ_DBG_LEVEL,
		__EN_ATTR_SET_PARAM_REQ_MAX
};

#define EN_ATTR_SET_PARAM_REQ_MAX (__EN_ATTR_SET_PARAM_REQ_MAX - 1)

/* "param" confirm structure */
enum {
		EN_ATTR_SET_PARAM_CONF_INVALID,
		EN_ATTR_SET_PARAM_CONF_STATUS,
		__EN_ATTR_SET_PARAM_CONF_MAX
};

#define EN_ATTR_SET_PARAM_CONF_MAX (__EN_ATTR_SET_PARAM_CONF_MAX - 1)
/* commands */
enum EN_CMD {
		EN_CMD_INVALID,
		EN_CMD_GET_INFO_REQ, /* from user space */
		EN_CMD_GET_INFO_CONF, /* from kernel */
		EN_CMD_SET_PARAM_REQ, /* from user space */
		EN_CMD_SET_PARAM_CONF, /* from kernel */
		__EN_CMD_CTL_MAX,
};

#define EN_CMD_CTL_MAX (__EN_CMD_CTL_MAX - 1)

int netlink_test_init(void);
void netlink_test_exit(void);

#endif
