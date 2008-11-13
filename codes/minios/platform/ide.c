#include <platform.h>
#include <device.h>
#include <kernel.h>
#include <stdio.h>
#include <string.h>

#define PRIMARY_ATA 0x1F0
#define PRIMARY_CTRL 0x3F6
#define SECONDARY_ATA 0x170
#define SECONDARY_CTRL 0x376
uint16 MASTER_IDE_CTRL =0;
uint16 SLAVER_IDE_CTRL =0;

int32 ide_init();
pvoid ide_open(int8 *path);
void ide_close(pvoid device);
int32 ide_write(pvoid device, uint8 *inbuf, uint32 size);
int32 ide_read(pvoid device, uint8 *inbuf, uint32 size);
int32 ide_ioctrl(pvoid device, uint32 command, uint8 *inbuf, uint32 insize, uint8 *outbuf, uint32 *outsize);
int32 ide_status(pvoid device);

struct driver_object ide_driver_object={
	ide_init, 
	ide_open,
	ide_close,
	ide_read,
	ide_write,
	NULL,
	NULL,
	ide_ioctrl,
	ide_status,
	"ide"
};

struct _DMABUFDES
{
	uint32 base;
	uint16 eot;
	uint16 len;
} idedmabuf = {0x10000, 0x8000, 0};

uint32 ide_readdma(uint32 start, uint32 number)
{
	start |= 0x40000000;

	_out(MASTER_IDE_CTRL, 0);
	_out(MASTER_IDE_CTRL + 2, 0x06);
	_out32(MASTER_IDE_CTRL + 4, (uint32)&idedmabuf);

	while((_in(PRIMARY_ATA + 7) & 0x40) == 0)
		;
	_out(PRIMARY_CTRL, 0);
	_out(PRIMARY_ATA + 2, number);
	_out32(PRIMARY_ATA + 3, start);
	_out(PRIMARY_ATA + 7, 0xC8);
	
	_out(MASTER_IDE_CTRL, 9);
	return 0;
}

void pfIdeIsr()
{
	puts("*****");
}


int32 ide_init()
{
	int i;
	PCICFG * cfg=PCICFG_POINTER;
	_ISRVECT[46]=(uint32)pfIdeIsr;
	_ISRVECT[47]=(uint32)pfIdeIsr;
	memset((void*)0x10000, 0, 0x10000);
	for(i=0;i<256;i++)
	{
		if(cfg->classcode1==1 && cfg->classcode2==1)
		{
			MASTER_IDE_CTRL = (uint16)(cfg->baseaddress[4]& ~3);
			printf("Primary IDE controller's config space %P\n", cfg);
			printf("Primary IDE controller port %04X\n", MASTER_IDE_CTRL);
			return 1;
		}
		cfg++;
	}
	return 0;
}

pvoid ide_open(int8 *path)
{
	return 0;
}

void ide_close(pvoid device)
{
}

int32 ide_write(pvoid device, uint8 *inbuf, uint32 size)
{
	return 0;
}

int32 ide_read(pvoid device, uint8 *inbuf, uint32 size)
{
	return 0;
}

int32 ide_ioctrl(pvoid device, uint32 command, uint8 *inbuf, uint32 insize, uint8 *outbuf, uint32 *outsize)
{
	return 0;
}

int32 ide_status(pvoid device)
{
	return 0;
}

