#include <platform.h>
#include <device.h>

#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25
#define CONSOLE_BASSADDR 0xB8000
#define CONSOLE_DEFAULTATTRIB 0x700

int32 console_init();
pvoid console_open(int8 *path);
int32 console_write(pvoid device, uint8 *inbuf, uint32 size);
int32 console_ioctrl(pvoid device, uint32 command, uint8 *inbuf, uint32 insize, uint8 *outbuf, uint32 *outsize);
int32 console_tell(pvoid device);
int32 console_seek(pvoid device, uint32 offset);

struct driver_object console_driver_object={
	console_init, 
	console_open,
	NULL,
	NULL,
	console_write,
	console_seek,
	console_tell,
	console_ioctrl,
	NULL,
	"console"
};

struct device_object console_device_object={
	&console_driver_object,
	NULL
};

pvoid stdout = &console_device_object;

int32 console_attrib = CONSOLE_DEFAULTATTRIB;
uint16* console_base = (uint16*)CONSOLE_BASSADDR;
int32 console_cursor = 0;

void console_setcursor(int cursor)
{
	_out(0x3d4, 14);
	_out(0x3d5, (uint8)(cursor>>8));
	_out(0x3d4, 15);
	_out(0x3d5, (uint8)cursor);
	console_cursor=cursor;
}

int32 console_init()
{
	console_setcursor(0);
	console_attrib=CONSOLE_DEFAULTATTRIB;
	return 1;
}

pvoid console_open(int8 *path)
{
	return &console_device_object;
}

void console_newline()
{
	int x=console_cursor % CONSOLE_WIDTH;
	int y=console_cursor / CONSOLE_WIDTH;

	if(y+1>=CONSOLE_HEIGHT)
	{
		uint32 *p1=(uint32 *)(CONSOLE_BASSADDR);
		uint32 *p2=(uint32 *)(CONSOLE_BASSADDR+160);
		while(p1<(uint32 *)(CONSOLE_BASSADDR+(CONSOLE_HEIGHT-1)*160))
			*p1++=*p2++;

		while(p1<(uint32 *)(CONSOLE_BASSADDR+CONSOLE_HEIGHT*160))
			*p1++ = (console_attrib<<16) + console_attrib;
		console_setcursor((CONSOLE_HEIGHT-1)*80);
	}else
	{
		y++;
		console_setcursor(y * CONSOLE_WIDTH);
	}
}

int32 console_write(pvoid device, uint8 *outbuf, uint32 size)
{
	char* c=(char*)outbuf;
	uint32 i;

	for(i=0;i<size;i++)
	{
		switch(*c)
		{
		case '\r':
			break;
		case '\n':
			console_newline();
			break;
		case '\t':
		{
			int t=(console_cursor+8) & 0xf8;
			if(t>=CONSOLE_WIDTH)
				console_newline();
			else
				console_setcursor(t);
			break;
		}
		case '\b':
			if(console_cursor)
			{
				console_setcursor(console_cursor-1);
				console_base[console_cursor] = console_attrib;
			}
			break;
		default:
			console_base[console_cursor] = *c + console_attrib;
			if((console_cursor+1) % CONSOLE_WIDTH == 0)
			{
				console_newline();
			}else
				console_setcursor(console_cursor+1);
			break;
		}
		c++;
	}
	return 0;
}

int32 console_ioctrl(pvoid device, uint32 command, uint8 *inbuf, uint32 insize, uint8 *outbuf, uint32 *outsize)
{
	return 0;
}

int32 console_tell(pvoid device)
{
	return console_cursor;
}

int32 console_seek(pvoid device, uint32 offset)
{
	console_setcursor(offset);
	return 1;
}