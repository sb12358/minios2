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
struct heapmem * netheap;

struct semaphore netevent;
uint8 netmsgheader;
uint8 netmsgtail;
struct net_message{
	uint32 msg;
	uint32 param;
}netmsg[256];

int netInQueue(uint32 msg, uint32 param)
{
	struct net_message* m;

	if(netmsgtail+1==netmsgheader)
		return 0;

	_lock();
	m=&netmsg[netmsgtail];
	m->msg=msg;
	m->param=param;
	netmsgtail++;
	_unlock();
	release(&netevent);
}

int netDeQueue(uint32 *msg, uint32 *param)
{
	struct net_message* m;

	while(1)
	{
		wait(&netevent);
		if(netmsgheader!=netmsgtail)
			break;
	}

	_lock();
	m=&netmsg[netmsgheader];
	netmsgheader++;
	*msg=m->msg;
	*param=m->param;
	_unlock();

	return 1;

}


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

void* netMalloc(uint32 nbytes)
{
	void *p;
	_lock();
	p = keHeapAlloc(netheap, nbytes);
	_unlock();
	return p;
}

void netFree(void *pointer)
{
	_lock();
	keHeapFree(netheap, pointer);
	_unlock();
}

void netHeapDump()
{
	_lock();
	keHeapDump(netheap);
	_unlock();
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
		unsigned char *buffer=(unsigned char *)netMalloc(42);
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

void handleArpPacket(struct netpacket* packet)
{
	struct arp* a=(struct arp*)(packet->macPayload);
	if(a->opcode==htons(2))
	{
		uint8* mac=a->srcmac;
		uint8* ip=a->srcip;
		printf("Arp reply: %d.%d.%d.%d is at:", ip[0], ip[1], ip[2], ip[3]);
		printf("%02X-%02X-%02X-%02X-%02X-%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}else if(a->opcode==htons(1))
	{
		uint8* mac=a->srcmac;
		uint8* ip=a->dstip;
		uint8* ip2=a->srcip;
		printf("Arp request: %d.%d.%d.%d ", ip[0], ip[1], ip[2], ip[3]);
		printf("tell %d.%d.%d.%d ", ip2[0], ip2[1], ip2[2], ip2[3]);
		printf("%02X-%02X-%02X-%02X-%02X-%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
}

int handlePacket(struct net_device_extend* dev, uint8 *buf, uint32 size)
{
	struct mac* m=(struct mac*)buf;
	struct netpacket* packet;
	
	if(m->protocol<htons(0x800))		/* It is not a eth packet */
		return 0;

	packet=(struct netpacket*)netMalloc(size+sizeof(struct netpacket));
	memcpy((void*)(packet+1), buf, size);
	packet->netif=dev->netif;
	packet->packetBuf=(uint8*)(packet+1);
	packet->macPayload=packet->packetBuf+14;
	packet->macProtocol=htons(m->protocol);
	if(netInQueue(PACKET_ARRIVED, (uint32)packet)==0)
		netFree(packet);
	return 1;
}

void registerNetDevice(struct device_object* dev)
{
	struct netif_inet* netif=(struct netif_inet*)netMalloc(sizeof(struct netif_inet));
	uint8*p;

	netif->dev=dev;
	netif->ext=(struct net_device_extend *)dev->extend;
	netif->ext->recv=handlePacket;

	netif->ipAddr[0]=0x9B30670A;	/*10.103.48.155*/
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
	netif->ext->netif=netif;
}

void netEntryMain(uint32 p)
{
	uint32 msg;
	uint32 param;

	while(1)
	{
		netDeQueue(&msg, &param);

		switch(msg)
		{
		case PACKET_ARRIVED:
		{
			struct netpacket*packet=(struct netpacket*)param;
			if(packet->macProtocol==0x806)
				handleArpPacket(packet);
			netFree(packet);
			break;
		}
		default:
			break;
		}
	}
}

void netinit()
{
	netheap=(struct heapmem *)keMalloc(0x100000);
	keHeapInit(netheap, 0x100000);
	netmsgheader=0;
	netmsgtail=0;
	initsemaphore(&netevent, 0);
	keNewTask("netEntryMain", netEntryMain, 0, 7, 0x4000);
}

