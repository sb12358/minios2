#include <platform.h>
#include <device.h>
#include <kernel.h>
#include <stdio.h>
#include <string.h>

enum tulip_offsets {
	CSR0=0,    CSR1=0x08, CSR2=0x10, CSR3=0x18, CSR4=0x20, CSR5=0x28,
	CSR6=0x30, CSR7=0x38, CSR8=0x40, CSR9=0x48, CSR10=0x50, CSR11=0x58,
	CSR12=0x60, CSR13=0x68, CSR14=0x70, CSR15=0x78 };

int32 eth_init();
uint16 ETH_CTRL =0;

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
	"eth"
};

struct DEC21140_Descriptor_t
{
	uint32 status;
	uint32 control;
	uint32 address1;
	uint32 address2;
};
struct DEC21140_Descriptor_t recvdesc[8];
struct DEC21140_Descriptor_t senddesc[8];

void ethisr_handler()
{
//	uint32 status;
	
//	printf("(");
//	status =_in32(ETH_CTRL + CSR5);
//	_out32(ETH_CTRL+CSR6, 0x028C2042);
//	_out32(status | 0x0001ffff, ETH_CTRL + CSR5);
//	_out32(ETH_CTRL+CSR4, (uint32)&senddesc[1]);
//	_out32(ETH_CTRL+CSR7, 0);
//	_out32(ETH_CTRL+CSR6, 0x028C0042);

//	if(status & 0x5)
//		return;

//	keDelay(100);
//	printf(")");
}

int32 eth_init()
{
	int bus=0;
	PCIDEV *dev =PCIDEV_HEADER;
	
	while(dev!=NULL)
	{
		PCICFG *cfg = &dev->cfg;
		if(cfg->classcode1==2 && cfg->classcode2==0)
		{
			int i;
			uint8 * recvbuf;
			uint8 * sendbuf;

			ETH_CTRL = (uint16)(cfg->baseaddress[0]& ~3);
			printf("ethenet controller's config space %P io port %04X\n", cfg, ETH_CTRL);
			if(cfg->romaddr !=0 && cfg->romaddr!= 0xffffffff)
			{
				cfg->romaddr |= 0x1;
				pciWriteConfig32 (dev->bus, dev->devicefn, 0x30, cfg->romaddr);
			}

			_ISRVECT[dev->cfg.interruptline+0x20]=(uint32)ethisr_handler;

			memset(recvdesc, 0, sizeof(recvdesc));
			recvbuf=(uint8*)keMalloc(2048*8);
			for(i=0;i<8;i++)
			{
				recvdesc[i].status=0x80000000;
				recvdesc[i].control=2048;
				recvdesc[i].address1=(uint32)(recvbuf+i*2048);
				recvdesc[i].address2=0;
			}
			recvdesc[7].control = recvdesc[7].control | 0x01000000;
			recvdesc[7].address2 = recvdesc;

			memset(senddesc, 0, sizeof(senddesc));
			sendbuf=(uint8*)keMalloc(2048*8);
			for(i=0;i<8;i++)
			{
				senddesc[i].status=0;
				senddesc[i].control=0;
				senddesc[i].address1=(uint32)(sendbuf+i*2048);
				senddesc[i].address2=0;
			}
			senddesc[0].status=0x80000000;
			senddesc[0].control=0x880000C0;
			{
				int i;
				uint32 *setup_frm = (uint32*)sendbuf;

				*setup_frm++ = 0xffff;
				*setup_frm++ = 0xffff;
				*setup_frm++ = 0xffff;
				for (i = 1; i < 16; i++) {
					*setup_frm++ = 0x1e00;
					*setup_frm++ = 0xd04f;
					*setup_frm++ = 0x9d19;
				}
			}
			printf("rcv buf %08x snd buf %08x\n", recvdesc, senddesc);

			_out32(ETH_CTRL+CSR0, 0xfe000000);
			_out32(ETH_CTRL+CSR2, 0);
			_out32(ETH_CTRL+CSR3, (uint32)recvdesc);
			_out32(ETH_CTRL+CSR4, (uint32)senddesc);
			
			_out32(ETH_CTRL+CSR5, 0xFC660000);
			_out32(ETH_CTRL+CSR7, 0xffffa85b);
			_out32(ETH_CTRL+CSR6, 0x328C2040);


			return 1;
		}
		dev=dev->next;
	}
	return 0;
}