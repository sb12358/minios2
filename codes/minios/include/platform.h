#ifndef PLATFORM_H_INCLUDED
#define PLATFORM_H_INCLUDED

#include "types.h"

typedef struct _IDT
{
	unsigned short offsetlow;
	unsigned short selector;
	unsigned char zero;
	unsigned char attrib;
	unsigned short offsethigh;
}IDT;

extern uint32 _ISREXP[24];
extern uint32 _ISRINT[16];
extern uint32 *_ISRVECT;
extern IDT IDT_DATA[64];
extern char GDT_DATA[24];

void dummyIsr();
void _loadidtgdt();
uint8 _in(uint32 addr);
void _out(uint32 addr, uint32 value);
uint16 _in16(uint32 addr);
void _out16(uint32 addr, uint32 value);
uint32 _in32(uint32 addr);
void _out32(uint32 addr, uint32 value);


void _switch(uint32** newsp, uint32** oldsp);

// enable/disable interrupt, the function can re-in
_inline void _unlock()
{
	_asm popfd;
}
_inline void _lock()
{
	_asm pushfd;
	_asm cli;
}

// enable/disable interrupt, the fuction can't re-in
_inline void _sti()
{
	_asm sti;
}
_inline void _cli()
{
	_asm cli;
}

// halt the cpu, can release cpu resource in virtual machine
_inline void _hlt()
{
	_asm hlt;
}

// call keDoSched() indirectly
_inline void _int17()
{
	_asm int 17;
}

/*
	hardware interrupt from 32 to 47
	32:Clock
	33:Keyboard
	34:empty
	35:com2
	36:com1
	37:lpt1
	38:flopy disk
	39:lpt2
	40:realtime clock
	41:
	42:
	43:
	44:ps2 mouse
	45:8087
	46:hard disk
	47:
*/

#define PCICFG_POINTER ((PCICFG *)0x4000)
typedef struct _PCICFG
{
	unsigned short vender;
	unsigned short device;
	unsigned short command;
	unsigned short status;
	unsigned char revision;
	unsigned char classcode3;
	unsigned char classcode2;
	unsigned char classcode1;

	unsigned char cachelinesize;
	unsigned char latencytimer;
	unsigned char headertype;
	unsigned char bist;

	unsigned long baseaddress[6];
	unsigned long cispointer;
	unsigned short subsystemvender;
	unsigned short subsystem;
	unsigned long romaddr;
	unsigned long reserve[2];

	unsigned char interruptline;
	unsigned char interruptpin;
	unsigned char mingnt;
	unsigned char maxlat;
}PCICFG;	// PCI CFG BLOCK
void pciInit();
unsigned long pciReadConfig(int device_fn, void* buf, int size);
unsigned long pciWriteConfig(int device_fn, void* buf, int size);

uint32 ide_readdma(uint32 start, uint32 number);


#endif