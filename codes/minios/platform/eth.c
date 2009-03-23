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

int rx_new=0;
int tx_new=0;

void sendpacket(uint8* buffer, int size)
{
	_cli();
	senddesc[tx_new].address1=(uint32)buffer;
	senddesc[tx_new].control = 0xE0000000 + size;
	senddesc[tx_new].status |= 0x80000000;
	tx_new++;
	tx_new%=8;
	_sti();
	_out32(ETH_CTRL+CSR1, 0);
}


void ethisr_handler()
{
	int i;
	uint32 status;
	status =_in32(ETH_CTRL + CSR5);
	_out32(ETH_CTRL + CSR5, status | 0x0001ffff);

	printf("%08x ", status);
	if(status & 1)
	{
		printf("TI ");
		for(i=0;i<8;i++)
		{
			if(senddesc[i].status & 0x80000000)
				continue;
			if(senddesc[i].address1)
			{
				printf("Free %08x", senddesc[i].address1);
				keFree((void*)senddesc[i].address1);
				senddesc[i].address1=0;
				senddesc[i].control=0;
				senddesc[i].status=0;
			}
		}
	}
	if(status & 0x40)
	{
		printf("RI ");

		while(1)
		{
			if(recvdesc[rx_new].status == 0x80000000)
				break;
			printf("index=%d size=%d", rx_new, recvdesc[rx_new].status >> 16);
			recvdesc[rx_new].status=0x80000000;
			rx_new++;
			rx_new%=8;
		}
	}
	printf("\n");
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
			uint8 * recvbuf;

			ETH_CTRL = (uint16)(cfg->baseaddress[0]& ~3);
			printf("ethenet controller's config space %P io port %04X\n", cfg, ETH_CTRL);
			if(cfg->romaddr !=0 && cfg->romaddr!= 0xffffffff)
			{
				cfg->romaddr |= 0x1;
				pciWriteConfig32 (dev->bus, dev->devicefn, 0x30, cfg->romaddr);
			}
			_ISRVECT[dev->cfg.interruptline+0x20]=(uint32)ethisr_handler;

			memset(recvdesc, 0, sizeof(recvdesc));
			recvbuf=(uint8*)keMalloc(1600*8);
			for(i=0;i<8;i++)
			{
				recvdesc[i].status=0x80000000;
				recvdesc[i].control=1600;
				recvdesc[i].address1=(uint32)(recvbuf+i*1600);
				recvdesc[i].address2=0;
			}
			recvdesc[7].control = recvdesc[7].control | 0x02000000;

			memset(senddesc, 0, sizeof(senddesc));
			senddesc[7].control = senddesc[7].control | 0x02000000;

			_out32(ETH_CTRL+CSR3, (uint32)recvdesc);
			_out32(ETH_CTRL+CSR4, (uint32)senddesc);
			_out32(ETH_CTRL+CSR7, 0xffffffff);
			_out32(ETH_CTRL+CSR6, cmd | 0x00002082);

			{
				int i;
				uint32 *setup_frm = (uint32*)keMalloc(192);
				
				_cli();
				senddesc[tx_new].address1=(uint32)setup_frm;
				senddesc[tx_new].control = 0x88000000;
				senddesc[tx_new].status |= 0x80000000;
				tx_new++;
				tx_new%=8;
				_sti();

				*setup_frm++ = 0x1e00;
				*setup_frm++ = 0xd04f;
				*setup_frm++ = 0x9d19;
				for (i = 1; i < 16; i++) {
					*setup_frm++ = 0xffff;
					*setup_frm++ = 0xffff;
					*setup_frm++ = 0xffff;
				}

				_out32(ETH_CTRL+CSR1, 0);
			}

			cmd=_in32(ETH_CTRL+CSR6);
			return 1;
		}
		dev=dev->next;
	}
	return 0;
}