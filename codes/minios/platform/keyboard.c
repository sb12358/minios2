#include <platform.h>
#include <device.h>
#include <kernel.h>

int32 keyboard_init();
pvoid keyboard_open(int8 *path);
void keyboard_close(pvoid device);
int32 keyboard_read(pvoid device, uint8 *inbuf, uint32 size);

struct driver_object keyboard_driver_object={
	keyboard_init, 
	keyboard_open,
	NULL,
	keyboard_read,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"keyboard"
};

struct device_object keyboard_device_object={
	&keyboard_driver_object,
	NULL
};

pvoid stdin = &keyboard_device_object;

uint8 keyboardQueue[256];
uint8 keyboardHead=0;
uint8 keyboardTail=0;
struct semaphore keyboardevent;

void keyboardEnqueue(uint8 key)
{
	if((keyboardTail+1)==keyboardHead)
		return;
	keyboardQueue[keyboardTail++]=key;
	setevent(&keyboardevent);
}

uint8 keyboardDequeue()
{
	if(keyboardTail==keyboardHead) {
		waitevent(&keyboardevent);
		if(keyboardTail==keyboardHead)
			return 0;
	}
	return keyboardQueue[keyboardHead++];
}

void pfKeyboardIsr()
{
	uint8 code;
	uint8 status;

	code=_in(0x60);	// get scan code
	keyboardEnqueue(code);

	status=_in(0x61);	// get status
	status |= 0x80;
	_out(0x61, status);		// set status bit 7=1 disable keyboard
	status &= 0x7f;
	_out(0x61, status);		// set status bit 7=0 enable keyboard
}

uint8 shift_pressed=0;
uint8 alt_pressed=0;
uint8 ctrl_pressed=0;
uint8 caps_lock=0;
uint8 scroll_lock=0;
uint8 num_lock=1;

char hdcodetoascii1[96]={
	0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b','\t', 
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S', 
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0,0,0,0,0,
	0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
	'2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0
};

char hdcodetoascii2[96]={
	0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b','\t', 
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
	'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,'-',0,0,0,'+',0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

void keyboardLED()
{
	uint8 flag;
	_out(0x60, 0xed);		// set status bit 7=1 disable keyboard
	flag=scroll_lock;
	flag+=num_lock<<1;
	flag+=caps_lock<<2;
	_out(0x60, flag);
}

uint8 keyboardGetChar()
{
	uint8 key;
	uint8 pressed;
	uint8 extkey=0;

	while(1)
	{
		do{
			key=keyboardDequeue();
		}while(key==0);

		if(key>=0xE0)
		{
			extkey=1;
			do{
				key=keyboardDequeue();
			}while(key==0);
		}

		if(key>=0x80)
		{
			key-=0x80;
			pressed=0;
		}else
			pressed=1;

		switch (key)
		{
		case 0x1D:		// L Ctrl
			ctrl_pressed=pressed;
			break;
		case 0x2A:		// L Shift
			shift_pressed=pressed;
			break;
		case 0x36:		// R Shift
			shift_pressed=pressed;
			break;
		case 0x38:		// L Alt
			alt_pressed=pressed;
			break;
		case 0x3A:
			if(pressed)	// caps lock
				caps_lock = !caps_lock;
			keyboardLED();
			break;
		case 0x45:
			if(pressed)	// num lock
				num_lock = !num_lock;
			keyboardLED();
			break;
		case 0x46:
			if(pressed)	// scroll lock
				scroll_lock = !scroll_lock;
			keyboardLED();
			break;
		default:
			if(pressed)
			{
				uint8 vkey=hdcodetoascii2[key];
				if(key>=0x47 && key<=0x53)
				{
					if(num_lock)
						vkey = hdcodetoascii1[key];
				}
				else if(vkey>='a' && vkey<='z')
				{
					if(shift_pressed ^ caps_lock)
						vkey = hdcodetoascii1[key];
				}else
				{
					if(shift_pressed)
						vkey = hdcodetoascii1[key];
				}
				if(vkey!=0)
					return vkey;
			}
			break;
		}
	}
}

int32 keyboard_init()
{
	_ISRVECT[33]=(uint32)pfKeyboardIsr;
	keyboardLED();
	initsemaphore(&keyboardevent, 0);
	return 1;
}

pvoid keyboard_open(int8 *path)
{
	return &keyboard_device_object;
}

int32 keyboard_read(pvoid device, uint8 *inbuf, uint32 size)
{
	if(size==0 || inbuf==0)
		return 0;
	*inbuf=keyboardGetChar();
	return 1;
}
