#include <platform.h>
#include <device.h>
#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>
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

uint32 inet_addr(char *str)
{
	uint32 ret=0;
	char *s=str;

	ret=atoi(s);

	s = strchr(s, '.');
	if(s==0)
		return 0;
	s++;
	ret+=atoi(s)<<8;

	s = strchr(s, '.');
	if(s==0)
		return 0;
	s++;
	ret+=atoi(s)<<16;

	s = strchr(s, '.');
	if(s==0)
		return 0;
	s++;
	ret+=atoi(s)<<24;
	return ret;
}

void netinit()
{

}

void sendpacket(struct net_device_extend *ext, uint8* buffer, int size);

void ArpSend(uint32 ip)
{
	struct netif_inet* header=(struct netif_inet*)&netifheader;
	uint8* pip=(uint8*)&ip;
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
		printf("Arp request: who has the ipaddr of %d.%d.%d.%d\n", pip[0], pip[1], pip[2], pip[3]);
	}
}

void handleArpPacket(struct net_device_extend* dev, uint8 *buf, uint32 size)
{
	struct arp* a=(struct arp*)(buf+14);
	if(a->opcode==htons(2))
	{
		uint8* mac=a->srcmac;
		uint8* ip=a->srcip;
		printf("Arp reply: ");
		printf("%d.%d.%d.%d is at:", ip[0], ip[1], ip[2], ip[3]);
		printf("%02X-%02X-%02X-%02X-%02X-%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
}

int handlePacket(struct net_device_extend* dev, uint8 *buf, uint32 size)
{
	struct mac* m=(struct mac*)buf;
	switch(htons(m->protocol))
	{
	case 0x0806:
		handleArpPacket(dev, buf, size);
		break;
	default:
		break;
	}
	return 1;
}


void registerNetDevice(struct device_object* dev)
{
	struct netif_inet* netif=(struct netif_inet*)keMalloc(sizeof(struct netif_inet));
	uint8*p;

	netif->dev=dev;
	netif->ext=(struct net_device_extend *)dev->extend;
	netif->ext->recv=handlePacket;

	netif->ipAddr[0]=0x9B30670A;
	netif->ipAddr[1]=0xFFFFFFFF;
	netif->ipAddr[2]=0xFFFFFFFF;
	netif->ipAddr[3]=0xFFFFFFFF;
	netif->ipBroadcast=0xFFFFFFFF;

	printf("Register a net interface: %s\n", netif->ext->description);
	p=(uint8*)netif->ext->macAddr;
	printf("  Mac Address: %02X-%02X-%02X-%02X-%02X-%02X", p[0], p[1], p[2], p[3], p[4], p[5]);
	p=(uint8*)netif->ipAddr;
	printf("  IP Address: %d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
	list_add_tail(netif, &netifheader);
}
