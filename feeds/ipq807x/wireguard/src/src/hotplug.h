#ifndef __HOTPLUG_H__
#define __HOTPLUG_H__

#define  REKEY_GIVEUP_EVENT "REKEY-GIVEUP"
#define  REKEY_TIMEOUT_EVENT "REKEY-TIMEOUT"
#define  KEYPAIR_CREATED_EVENT "KEYPAIR-CREATED"

extern void wireguard_hotplug(const char *ifname, const char *action);
extern void wg_hotplug_init(void);
extern void wg_hotplug_free(void);

#endif

