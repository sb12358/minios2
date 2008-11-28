#include <platform.h>
#include <device.h>
#include <kernel.h>
#include <stdio.h>
#include <string.h>

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

int32 eth_init()
{
	int bus=0;
	PCIDEV *dev =PCIDEV_HEADER;
	
	while(dev!=NULL)
	{
		PCICFG *cfg = &dev->cfg;
		if(cfg->classcode1==2 && cfg->classcode2==0)
		{
			ETH_CTRL = (uint16)(cfg->baseaddress[0]& ~3);
			printf("ethenet controller's config space %P io port %04X\n", cfg, ETH_CTRL);
			if(cfg->romaddr !=0 && cfg->romaddr!= 0xffffffff)
			{
				cfg->romaddr |= 0x1;
				pciWriteConfig32 (dev->bus, dev->devicefn, 0x30, cfg->romaddr);
			}
			return 1;
		}
		dev=dev->next;
	}
	return 0;
}