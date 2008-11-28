#include <platform.h>
#include <kernel.h>
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

unsigned long pciReadConfig(int bus, int device_fn, void* buf, int size)
{
	int addr=CONFIG_CMD(bus, device_fn, 0);
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

unsigned long pciWriteConfig(int bus, int device_fn, void* buf, int size)
{
	int addr=CONFIG_CMD(bus, device_fn, 0);
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

unsigned long pciWriteConfig32(int bus, int device_fn, int addr, int value)
{
	int a=CONFIG_CMD(bus, device_fn, addr);
	_out32(0xCF8, a);
	_out32(0xCFC, value);
	return 4;
}

unsigned long pciReadConfig32(int bus, int device_fn, int addr)
{
	int a=CONFIG_CMD(bus, device_fn, addr);
	_out32(0xCF8, a);
	return _in32(0xCFC);
}

unsigned long pciWriteConfig16(int bus, int device_fn, int addr, int value)
{
	int a=CONFIG_CMD(bus, device_fn, addr);
	_out32(0xCF8, a);
	_out16(0xCFC + (addr & 2), value);
	return 4;
}

unsigned long pciReadConfig16(int bus, int device_fn, int addr)
{
	int a=CONFIG_CMD(bus, device_fn, addr);
	_out32(0xCF8, a);
	return _in16(0xCFC + (addr & 2));
}

unsigned long pciWriteConfig8(int bus, int device_fn, int addr, int value)
{
	int a=CONFIG_CMD(bus, device_fn, addr);
	_out32(0xCF8, a);
	_out(0xCFC + (addr & 3), value);
	return 4;
}

unsigned long pciReadConfig8(int bus, int device_fn, int addr)
{
	int a=CONFIG_CMD(bus, device_fn, addr);
	_out32(0xCF8, a);
	return _in(0xCFC + (addr & 3));
}

PCIDEV* pci_scanbus(int bus)
{
	int device;
	int fn;
	PCIDEV* root=NULL;
	PCIDEV** devp=&root;
	
	for(device=0; device<32; device++)
	{
		int clscode, subbus;
		int vender=pciReadConfig16(bus, device*8, 0);
		if(vender==0xffff)
			continue;

		for(fn=0;fn<8;fn++)
		{
			int devfn=device*8+fn;
			vender=pciReadConfig16(bus, devfn, 0);
			if(vender==0xffff)
				continue;
			clscode=pciReadConfig32(bus, devfn, 0x8);

			if((clscode>>16)==0x0604)		// PCI-PCI bridge
			{
				subbus=pciReadConfig8(bus, devfn, 0x19);
				*devp=pci_scanbus(subbus);
				while(*devp)
					devp=&((*devp)->next);
			}else if((clscode>>16)==0x0600)	// Host PCI bridge
			{
				continue;
			}else
			{
				PCIDEV *dev;
				dev=(PCIDEV*)keMalloc(sizeof(PCIDEV));
				dev->bus=bus;
				dev->devicefn=devfn;
				dev->next=NULL;
				pciReadConfig( bus, devfn, &dev->cfg, 0x40);
				*devp=dev;
				devp = &dev->next;
				printf("Found %d, %d at %P: %s\n", bus, devfn, dev, classcode[dev->cfg.classcode1]);
			}
		}
	}
	return root;
}

PCIDEV *PCIDEV_HEADER=NULL;

void pciInit()
{
	PCIDEV_HEADER=pci_scanbus(0);
}
