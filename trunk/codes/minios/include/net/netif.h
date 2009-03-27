struct net_device_extend
{
    char *description;
	uint8 macAddr[6];
	uint8 broadcastAddr[6];
};

struct netif_inet
{
	struct netif_inet* next;
	struct netif_inet* prev;

	struct device_object *dev;
	struct net_device_extend *ext;

	uint32 ipAddr[4];
	uint32 ipBroadcast;
};

void registerNetDevice(struct device_object* dev);
uint16 htons(uint16 a);
void netinit();
void ArpSend(uint32 ip);

