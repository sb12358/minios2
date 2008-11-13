#include <platform.h>
#include <device.h>
#include <kernel.h>
#include <string.h>

uint32 *_ISRVECT=0;

void pfDummyIsr()
{
	puts("*");
}

void cpuInit()
{
	int i;

	while(_in(0x64) & 2);	// wait for input buffer empty
	_out(0x64, 0xd1);		// want write to 8042 p2 port
	while(_in(0x64) & 2);	// wait for input buffer empty
	_out(0x60, 0xDF);		// enable A20 address line

	_out(0x20, 0x11);		// program 8259
	_out(0xA0, 0x11);		// initialization sequence
	_out(0x21, 0x20);		// start of hardware interrupt is 32
	_out(0xA1, 0x28);		// start of hardware interrupt is 40
	_out(0x21, 0x4);		// 8259-1 is master
	_out(0xA1, 0x2);		// 8259-2 is slave
	_out(0x21, 0x1);		// 8086 mode for both
	_out(0xA1, 0x1);
	_out(0x21, 0x0);		// enable all irq source
	_out(0xA1, 0x0);

	_out(0x43, 0x36);		// program 8253: binary, mode 3, LSB/MSB, ch 0
	_out(0x40, 0x9b);		// LSB 
	_out(0x40, 0x2e);		// MSB 1193180/100=11931=0x2e9b

	for(i=0;i<64;i++)		// init idts
	{
		IDT_DATA[i].zero=0;
		IDT_DATA[i].selector=8;
		IDT_DATA[i].attrib=0x8f;
		IDT_DATA[i].offsethigh = ((uint32)dummyIsr) >> 16;
		IDT_DATA[i].offsetlow = ((uint32)dummyIsr) & 0xFFFF;
	}

	for(i=0;i<24;i++)		// set exception gate attrib 0x8f
	{
		IDT_DATA[i].attrib=0x8f;
		IDT_DATA[i].offsethigh = (_ISREXP[i]) >> 16;
		IDT_DATA[i].offsetlow = (_ISREXP[i]) & 0xFFFF;
	}

	for(i=32;i<48;i++)		// set interrupt gate attrib 0x8e
	{
		IDT_DATA[i].attrib=0x8e;
		IDT_DATA[i].offsethigh = (_ISRINT[i-32]) >> 16;
		IDT_DATA[i].offsetlow = (_ISRINT[i-32]) & 0xFFFF;
	}
	
	for(i=0;i<256;i++)
		_ISRVECT[i]=(uint32)pfDummyIsr;
							// init interrupt table
	_loadidtgdt();
}

void _stdcall main(unsigned long p1, unsigned long p2, unsigned long p3)
{
	uint32 *oldesp;
	struct taskblock *task;

	cpuInit();

	_ISRVECT[17]=(uint32)keDoSched;
	_ISRVECT[32]=(uint32)keTimerIsr;

	keKernelHeapInit();
	keInitTaskSystem();

	task=keNewTask("main", keEntryMain, 0, 8, 0x4000);
	currentTaskId=0;

	_switch(&(task->esp), &oldesp);
}