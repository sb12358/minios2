#include <platform.h>
#include <stdio.h>

#define CONFIG_CMD(bus, device_fn, where)   (0x80000000 | (bus << 16) | (device_fn << 8) | (where & ~3))

char *classcode[]={
	"Old Device",
	"Mass storage controller",
	"Network controller",
	"Display controller",
	"Multimedia device",
	"Memory controller",
	"Bridge device",
	"Simple communication controller",
	"Base system peripherals",
	"Input device",
	"Docking station",
	"Processor",
	"Serial bus controller",
	"Wireless controller",
	"Intelligent I/O controller",
	"Satellite communication controller",
	"Encccryption/Decryption controller",
	"Data ac and signal proc controller"
};

unsigned long pciReadConfig(int device_fn, void* buf, int size)
{
	int addr=CONFIG_CMD(0, device_fn, 0);
	uint32* p=(uint32*)buf;
	int i;

	for(i=0;i<size;i+=4)
	{
		_out32(0xCF8, addr);
		addr+=4;
		*p++ = _in32(0xCFC);
	}
	return size & ~3;
}

unsigned long pciWriteConfig(int device_fn, void* buf, int size)
{
	int addr=CONFIG_CMD(0, device_fn, 0);
	uint32* p=(uint32*)buf;
	int i;

	for(i=0;i<size;i+=4)
	{
		_out32(0xCF8, addr);
		addr+=4;
		_out32(0xCFC, *p++);
	}
	return size & ~3;
}

unsigned long pciWriteConfig32(int device_fn, int addr, int value)
{
	int a=CONFIG_CMD(0, device_fn, addr);
	_out32(0xCF8, a);
	_out32(0xCFC, value);
	return 4;
}

void pciInit()
{
	int i;
	PCICFG *p = PCICFG_POINTER;
	for(i=0;i<256;i++)
	{
		pciReadConfig(i, p, 64);
		if(p->vender!=0xffff)
			printf("Found %d, %d at %08X: %s\n", p->classcode1, p->classcode2, p, classcode[p->classcode1]);
		p++;
	}
}
