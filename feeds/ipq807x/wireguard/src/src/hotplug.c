#include <linux/workqueue.h>
#include <linux/netlink.h>
#include <linux/kobject.h>
#include <linux/skbuff.h>
#include <linux/if.h>

#define SUBSYSTEM_NAME	"wireguard"

#define EVENT_BUF_SIZE	2048

struct wg_event {
	struct work_struct work;
	char ifname[IFNAMSIZ];
	const char *action;
};


/* -------------------------------------------------------------------------*/
static int bh_event_add_var(struct sk_buff *skb, int argv,
							const char *format, ...)
{
	static char buf[128];
	char *s;
	va_list args;
	int len;

	if (argv)
		return 0;

	va_start(args, format);
	len = vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	if (len >= sizeof(buf)) {
		//WARN(1, "buffer size too small\n");
		return -ENOMEM;
	}

	s = skb_put(skb, len + 1);
	strcpy(s, buf);

	return 0;
}

static void hotplug_work(struct work_struct *work)
{
	struct wg_event *event = container_of(work, struct wg_event, work);
	struct sk_buff *skb;

	skb = alloc_skb(EVENT_BUF_SIZE, GFP_KERNEL);
	if (!skb)
		goto out_free_event;

	bh_event_add_var(skb, 0, "SUBSYSTEM=%s", SUBSYSTEM_NAME);
	bh_event_add_var(skb, 0, "ACTION=%s", event->action);
	bh_event_add_var(skb, 0, "ifname=%s", event->ifname);

	NETLINK_CB(skb).dst_group = 1;
	broadcast_uevent(skb, 0, 1, GFP_KERNEL);

out_free_event:
	kfree(event);
}

void wireguard_hotplug(const char *ifname, const char *action)
{
	struct wg_event *event;

	if (!strcmp(ifname, "wgserver"))
		return;

	event = kzalloc(sizeof(struct wg_event), GFP_ATOMIC);
	if (!event)
		return;

	memcpy(event->ifname, ifname, IFNAMSIZ);

	event->action = action;

	INIT_WORK(&event->work, hotplug_work);
	schedule_work(&event->work);
}
