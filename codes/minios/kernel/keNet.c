#include <platform.h>
#include <device.h>
#include <kernel.h>
#include <stdio.h>
#include <string.h>
#include <list.h>
#include <net/netif.h>
#pragma pack(1)

LIST_HEAD(netifheader);

struct mac
{
	uint8 dstmac[6];
	uint8 srcmac[6];
	uint16 protocol;
};

struct arp
{
	uint16 hardware;
	uint16 protocol;
	uint8 hardwaresize;
	uint8 protocolsize;
	uint16 opcode;
	uint8 srcmac[6];
	uint8 srcip[4];
	uint8 dstmac[6];
	uint8 dstip[4];
};

void netinit()
{

}

uint16 htons(uint16 a)
{
	uint8 *t=(uint8*)&a;
	return (t[0]<<8)+t[1];
}

void sendpacket(struct net_device_extend *ext, uint8* buffer, int size);

void ArpSend(uint32 ip)
{
	struct netif_inet* header=(struct netif_inet*)&netifheader;
	if(header->next!=header)
	{
		struct netif_inet* netif=header->next;
		struct net_device_extend *ext=netif->ext;
		unsigned char *buffer=(unsigned char *)keMalloc(42);
		struct mac* m=(struct mac*)buffer;
		struct arp* a=(struct arp*)(buffer+14);

		memcpy(m->dstmac, ext->broadcastAddr, 6);
		memcpy(m->srcmac, ext->macAddr, 6);
		m->protocol=htons(0x0806);

		a->hardware=htons(0x0001);
		a->protocol=htons(0x0800);
		a->hardwaresize=6;
		a->protocolsize=4;
		a->opcode=htons(0x0001);
		memcpy(a->srcmac, ext->macAddr, 6);
		memcpy(a->srcip, netif->ipAddr, 4);
		memset(a->dstmac, 0, 6);
		memcpy(a->dstip, &ip, 4);
		sendpacket(ext, buffer, 42);
}	}

void registerNetDevice(struct device_object* dev)
{
	struct netif_inet* netif=(struct netif_inet*)keMalloc(sizeof(struct netif_inet));
	netif->dev=dev;
	netif->ext=(struct net_device_extend *)dev->extend;
	netif->ipAddr[0]=0x9B30670A;
	netif->ipAddr[1]=0xFFFFFFFF;
	netif->ipAddr[2]=0xFFFFFFFF;
	netif->ipAddr[3]=0xFFFFFFFF;
	netif->ipBroadcast=0xFFFFFFFF;
	list_add_tail(netif, &netifheader);
}
