#include <platform.h>
#include <device.h>
#include <kernel.h>
#include <stdio.h>
#include <string.h>

#pragma pack(1)

struct netdevice
{
	unsigned char macAddr[6];
	unsigned char macAddrBroadcast[6];
	unsigned char ipAddr[4];
	unsigned char ipBroadcast[6];
}netdev;

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
	netdev.macAddr[0]=0x9d;
	netdev.macAddr[1]=0x19;
	netdev.macAddr[2]=0xd0;
	netdev.macAddr[3]=0x4f;
	netdev.macAddr[4]=0x1e;
	netdev.macAddr[5]=0x00;

	netdev.macAddrBroadcast[0]=0xff;
	netdev.macAddrBroadcast[1]=0xff;
	netdev.macAddrBroadcast[2]=0xff;
	netdev.macAddrBroadcast[3]=0xff;
	netdev.macAddrBroadcast[4]=0xff;
	netdev.macAddrBroadcast[5]=0xff;

	netdev.ipAddr[0]=10;
	netdev.ipAddr[1]=103;
	netdev.ipAddr[2]=48;
	netdev.ipAddr[3]=155;

	netdev.ipBroadcast[0]=255;
	netdev.ipBroadcast[1]=255;
	netdev.ipBroadcast[2]=255;
	netdev.ipBroadcast[3]=255;
}

uint16 htons(uint16 a)
{
	uint8 *t=(uint8*)&a;
	return (t[0]<<8)+t[1];
}

void sendpacket(uint8* buffer, int size);

void ArpSend(uint32 ip)
{
	unsigned char *buffer=(unsigned char *)keMalloc(42);
	struct mac* m=(struct mac*)buffer;
	struct arp* a=(struct arp*)(buffer+14);

	memcpy(m->dstmac, netdev.macAddrBroadcast, 6);
	memcpy(m->srcmac, netdev.macAddr, 6);
	m->protocol=htons(0x0806);

	a->hardware=htons(0x0001);
	a->protocol=htons(0x0800);
	a->hardwaresize=6;
	a->protocolsize=4;
	a->opcode=htons(0x0001);
	memcpy(a->srcmac, netdev.macAddr, 6);
	memcpy(a->srcip, netdev.ipAddr, 4);
	memset(a->dstmac, 0, 6);
	memcpy(a->dstip, &ip, 4);
	sendpacket(buffer, 42);
}