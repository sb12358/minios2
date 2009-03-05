#include <platform.h>
#include <device.h>
#include <kernel.h>
#include <stdio.h>
#include <string.h>

#define PRIMARY_ATA 0x1F0
#define PRIMARY_CTRL 0x3F6
#define SECONDARY_ATA 0x170
#define SECONDARY_CTRL 0x376
uint16 PCIIDE_CTRL =0;

int32 ide_init();
pvoid ide_open(int8 *path);
void ide_close(pvoid device);
int32 ide_write(pvoid device, uint8 *inbuf, uint32 size);
int32 ide_read(pvoid device, uint8 *inbuf, uint32 size);
int32 ide_ioctrl(pvoid device, uint32 command, uint8 *inbuf, uint32 insize, uint8 *outbuf, uint32 *outsize);
int32 ide_status(pvoid device);

struct semaphore waitdma;

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
	uint32 len;
} idedmabuf = {0xffff, 0x80000000};

uint32 ide_readdma(uint32 buffer, uint32 start, uint32 number)
{
	start |= 0x40000000;
	idedmabuf.base = buffer;

	while((_in(PRIMARY_ATA + 7) & 0x80) != 0)
		;
	_out(PRIMARY_ATA + 6, 0x40);
	_out(PRIMARY_ATA + 2, number);
	_out32(PRIMARY_ATA + 3, start);
	while((_in(PRIMARY_ATA + 7) & 0x40) == 0)
		;
	_out(PRIMARY_ATA + 7, 0xc8);
	_out(PCIIDE_CTRL, 0);
	_out(PCIIDE_CTRL + 2, 0x06);
	while((_in(PRIMARY_ATA + 7) & 0x08) == 0)
		;
	_out(PCIIDE_CTRL, 9);
	wait(&waitdma);
	if(_in(PCIIDE_CTRL + 2) & 2)
		return 0;
	if(_in(PCIIDE_CTRL + 2) & 1)
		return 0x10000;
	return number<<9;
}

uint32 ide_readpio4(uint32 buffer, uint32 start, uint32 number)
{
	start |= 0x40000000;

	_out(PRIMARY_ATA + 6, 0x40);
	while((_in(PRIMARY_ATA + 7) & 0x40) == 0)
		;
	_out(PRIMARY_ATA + 2, number);
	_out32(PRIMARY_ATA + 3, start);
	_out(PRIMARY_ATA + 7, 0x20);
	while((_in(PRIMARY_ATA + 7) & 0x08) == 0)
		;

	__asm{
		mov dx, 1f0h
		mov edi, buffer
		mov ecx, number
		shl ecx, 7
		rep insd
	}

	return 0;
}

uint32 ide_readidentify(uint32 buffer)
{
	_out(PRIMARY_ATA + 6, 0x40);
	while((_in(PRIMARY_ATA + 7) & 0x40) == 0)
		;
	_out(PRIMARY_ATA + 7, 0xec);
	while((_in(PRIMARY_ATA + 7) & 0x08) == 0)
		;
	__asm{
		mov dx, 1f0h
		mov edi, buffer
		mov ecx, 1
		shl ecx, 7
		rep insd
	}
}

uint32 ide_setfeture(uint8 feture, uint8 param)
{
	_out(PRIMARY_ATA + 6, 0x40);
	while((_in(PRIMARY_ATA + 7) & 0x40) == 0)
		;
	_out(PRIMARY_ATA + 1, feture);
	_out(PRIMARY_ATA + 2, param);
	_out(PRIMARY_ATA + 7, 0xef);
	return 1;
}

void pfIdeIsr()
{
	release(&waitdma);
}


int32 ide_init()
{
	PCIDEV * dev=PCIDEV_HEADER;
	_ISRVECT[46]=(uint32)pfIdeIsr;
	_ISRVECT[47]=(uint32)pfIdeIsr;
	memset((void*)0x10000, 0, 0x10000);
	initsemaphore(&waitdma, 0);

	while(dev!=NULL)
	{
		PCICFG *cfg = &dev->cfg;
		if(cfg->classcode1==1 && cfg->classcode2==1)
		{
			PCIIDE_CTRL = (uint16)(cfg->baseaddress[4]& ~3);
			printf("PCI-IDE controller's config space %P io port %04X\n", cfg, PCIIDE_CTRL);

			_out(PRIMARY_CTRL, 2);							// enable interrupt
			//ide_setfeture(3, 0x22);
			//ide_readidentify(0x10000);
			_out32(PCIIDE_CTRL + 4, (uint32)&idedmabuf);	// Setup dma buffer
			_out(PRIMARY_CTRL, 0);							// enable interrupt
			return 1;
		}
		dev=dev->next;
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

