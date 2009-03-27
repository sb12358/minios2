struct net_device_extend
{
    char *description;
	uint8 macAddr[6];
	uint8 broadcastAddr[6];
	int32 (*send)(struct net_device_extend*, uint8*, uint32);
	int32 (*recv)(struct net_device_extend*, uint8*, uint32);
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

static _inline uint16 htons(uint16 a)
{
	uint8 *t=(uint8*)&a;
	return (t[0]<<8)+t[1];
}

static _inline uint32 htonl(uint32 a)
{
	uint8 *t=(uint8*)&a;
	return (t[0]<<24)+(t[0]<<16)+(t[0]<<8)+t[0];
}

static _inline uint16 ntohs(uint16 a)
{
	uint8 *t=(uint8*)&a;
	return (t[0]<<8)+t[1];
}

static _inline uint32 ntohl(uint32 a)
{
	uint8 *t=(uint8*)&a;
	return (t[0]<<24)+(t[0]<<16)+(t[0]<<8)+t[0];
}

uint32 inet_addr(char *str);
void registerNetDevice(struct device_object* dev);
uint16 htons(uint16 a);
void netinit();
void ArpSend(uint32 ip);

