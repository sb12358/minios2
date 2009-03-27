#include <platform.h>
#include <device.h>
#include <kernel.h>
#include <stdio.h>
#include <string.h>
#include <net/netif.h>

enum tulip_offsets {
	CSR0=0,    CSR1=0x08, CSR2=0x10, CSR3=0x18, CSR4=0x20, CSR5=0x28,
	CSR6=0x30, CSR7=0x38, CSR8=0x40, CSR9=0x48, CSR10=0x50, CSR11=0x58,
	CSR12=0x60, CSR13=0x68, CSR14=0x70, CSR15=0x78 };

int32 eth_init();

struct driver_object eth_driver_object={
	eth_init, 
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"eth"
};

struct DEC21140_Descriptor_t
{
	uint32 status;
	uint32 control;
	uint32 address1;
	uint32 address2;
};

struct DEC21140_dev_extend
{
    char *description;
	uint8 macAddr[6];
	uint8 broadcastAddr[6];

	struct DEC21140_Descriptor_t recvdesc[8];
	struct DEC21140_Descriptor_t senddesc[8];
	uint8* recvbuf;
	int rx_new;
	int tx_new;
	uint32 ROM_ADDR;
	uint32 ETH_CTRL;
	uint32 CMD;
	uint32 INIT_LINE;
};

struct DEC21140_dev_extend* firstdev;

void sendpacket(struct DEC21140_dev_extend *ext, uint8* buffer, int size)
{
	_lock();
	ext->senddesc[ext->tx_new].address1=(uint32)buffer;
	ext->senddesc[ext->tx_new].control = 0xE0000000 + size;
	ext->senddesc[ext->tx_new].status |= 0x80000000;
	ext->tx_new++;
	ext->tx_new%=8;
	_unlock();
	_out32(ext->ETH_CTRL+CSR1, 0);
}

void ethisr_handler()
{
	int i;
	uint32 status;
	struct DEC21140_dev_extend* ext=firstdev;

	status =_in32(ext->ETH_CTRL + CSR5);

	if(status & 1)
	{
		for(i=0;i<8;i++)
		{
			if(ext->senddesc[i].status & 0x80000000)
				continue;
			if(ext->senddesc[i].address1)
			{
				keFree((void*)ext->senddesc[i].address1);
				ext->senddesc[i].address1=0;
				ext->senddesc[i].control=0;
				ext->senddesc[i].status=0;
			}
		}
	}
	if(status & 0x40)
	{
		while(1)
		{
			if(ext->recvdesc[ext->rx_new].status == 0x80000000)
				break;
			ext->recvdesc[ext->rx_new].status=0x80000000;
			ext->rx_new++;
			ext->rx_new%=8;
		}
	}
	_out32(ext->ETH_CTRL + CSR5, status | 0x0001ffff);
}

void sendsetupframe(struct DEC21140_dev_extend *ext)
{
	int i;
	uint32 *setup_frm = (uint32*)keMalloc(192);

	_cli();
	ext->senddesc[ext->tx_new].address1=(uint32)setup_frm;
	ext->senddesc[ext->tx_new].control = 0x88000000;
	ext->senddesc[ext->tx_new].status |= 0x80000000;
	ext->tx_new++;
	ext->tx_new%=8;
	_sti();

	ext->macAddr[0]=0x9d;
	ext->macAddr[1]=0x19;
	ext->macAddr[2]=0xd0;
	ext->macAddr[3]=0x4f;
	ext->macAddr[4]=0x1e;
	ext->macAddr[5]=0x00;

	*setup_frm++ = 0x1e00;
	*setup_frm++ = 0xd04f;
	*setup_frm++ = 0x9d19;
	for (i = 1; i < 16; i++) {
		*setup_frm++ = 0xffff;
		*setup_frm++ = 0xffff;
		*setup_frm++ = 0xffff;
	}

	_out32(ext->ETH_CTRL+CSR1, 0);
}


int32 eth_init()
{
	int cmd=0;
	int bus=0;
	PCIDEV *dev =PCIDEV_HEADER;
	
	while(dev!=NULL)
	{
		PCICFG *cfg = &dev->cfg;
		if(cfg->classcode1==2 && cfg->classcode2==0)
		{
			int i;
			struct device_object* device = (struct device_object*)keMalloc(sizeof(struct device_object)
				+sizeof(struct DEC21140_dev_extend));
			struct DEC21140_dev_extend *ext=(struct DEC21140_dev_extend *)(device+1);
			device->driver=&eth_driver_object;
			device->extend=ext;
			firstdev=ext;

			ext->ETH_CTRL = (uint16)(cfg->baseaddress[0]& ~3);
			ext->ROM_ADDR = cfg->romaddr;
			ext->INIT_LINE= cfg->interruptline;
			ext->description="DEC21140 10/100M Ethernet Adapter";
			ext->rx_new=0;
			ext->tx_new=0;
			memset(ext->broadcastAddr, 0xff, 6);
			printf("ethenet controller's config space %P io port %04X\n", cfg, ext->ETH_CTRL);
			
			_ISRVECT[ext->INIT_LINE + 0x20]=(uint32)ethisr_handler;
			if(cfg->romaddr !=0 && cfg->romaddr!= 0xffffffff)
			{
				cfg->romaddr |= 0x1;
				pciWriteConfig32 (dev->bus, dev->devicefn, 0x30, cfg->romaddr);
			}

			memset(ext->recvdesc, 0, sizeof(ext->recvdesc));
			ext->recvbuf=(uint8*)keMalloc(1600*8);
			for(i=0;i<8;i++)
			{
				ext->recvdesc[i].status=0x80000000;
				ext->recvdesc[i].control=1600;
				ext->recvdesc[i].address1=(uint32)(ext->recvbuf+i*1600);
				ext->recvdesc[i].address2=0;
			}
			ext->recvdesc[7].control = ext->recvdesc[7].control | 0x02000000;

			memset(ext->senddesc, 0, sizeof(ext->senddesc));
			ext->senddesc[7].control = ext->senddesc[7].control | 0x02000000;

			_out32(ext->ETH_CTRL+CSR3, (uint32)ext->recvdesc);
			_out32(ext->ETH_CTRL+CSR4, (uint32)ext->senddesc);
			_out32(ext->ETH_CTRL+CSR7, 0xffffffff);
			ext->CMD=_in32(ext->ETH_CTRL+CSR6);
			_out32(ext->ETH_CTRL+CSR6, ext->CMD | 0x00002082);

			sendsetupframe(ext);
			registerNetDevice(device);
			return 1;
		}
		dev=dev->next;
	}
	return 0;
}