#ifndef _LINUX_PM_QOS_H
#define _LINUX_PM_QOS_H

#define PM_QOS_RESERVED 0
#define PM_QOS_CPU_DMA_LATENCY 1
#define PM_QOS_NETWORK_LATENCY 2
#define PM_QOS_NETWORK_THROUGHPUT 3

#define PM_QOS_NUM_CLASSES 4
#define PM_QOS_DEFAULT_VALUE -1

#include <linux/version.h>
#include <linux/plist.h>
#include <linux/notifier.h>
#include <linux/device.h>
#include <linux/workqueue.h>

enum dev_pm_qos_req_type {
	DEV_PM_QOS_RESUME_LATENCY = 1,
	DEV_PM_QOS_LATENCY_TOLERANCE,
	DEV_PM_QOS_FLAGS,
};

struct pm_qos_flags_request {
	struct list_head node;
	s32 flags;	/* Do not change to 64 bit */
};

struct dev_pm_qos_request {
	enum dev_pm_qos_req_type type;
	union {
		struct plist_node pnode;
		struct pm_qos_flags_request flr;
	} data;
	struct device *dev;
};

struct pm_qos_request {
	struct plist_node node;
	int pm_qos_class;
	struct delayed_work work; /* for pm_qos_update_request_timeout */
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19)
#include <linux/latency.h>

static inline int pm_qos_add_requirement(int qos, char *name, s32 value)
{
	set_acceptable_latency(name, value);
	return 0;
}

static inline void pm_qos_remove_requirement(int qos, char *name)
{
	remove_acceptable_latency(name);
}

#else

static inline int pm_qos_add_requirement(int qos, char *name, s32 value)
{
	return 0;
}

static inline void pm_qos_remove_requirement(int qos, char *name)
{
}

static inline int dev_pm_qos_request_active(struct dev_pm_qos_request *req)
{
	return req->dev != NULL;
}

static inline void pm_qos_add_request(struct pm_qos_request *req, int pm_qos_class,
			s32 value) {}
static inline void pm_qos_update_request(struct pm_qos_request *req,
			   s32 new_value) {}
static inline void pm_qos_update_request_timeout(struct pm_qos_request *req,
				   s32 new_value, unsigned long timeout_us) {}
static inline void pm_qos_remove_request(struct pm_qos_request *req) {}

static inline int pm_qos_request_active(struct pm_qos_request *req)
{
	return 0;
}
#endif /* >= 2.6.19 */

static inline s32 cpu_latency_qos_limit(void) { return INT_MAX; }
static inline bool cpu_latency_qos_request_active(struct pm_qos_request *req)
{
	return false;
}
static inline void cpu_latency_qos_add_request(struct pm_qos_request *req,
					       s32 value) {}
static inline void cpu_latency_qos_update_request(struct pm_qos_request *req,
						  s32 new_value) {}
static inline void cpu_latency_qos_remove_request(struct pm_qos_request *req) {}

#endif /* _LINUX_PM_QOS_H */
