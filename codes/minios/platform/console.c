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
	NULL,
	console_seek,
	console_tell,
	console_ioctrl,
	NULL,
	"console"
};

struct device_extend
{
	int32	attrib;
	uint16*	base;
	int32	cursor;
	int32	active;
	int32	index;
}console_device_extend[4];
struct device_object console_device_object[4];

void console_setcursor(struct device_extend* ext)
{
	if(ext->active)
	{
		int cursor = ext->index * 0x800 + ext->cursor;
		_out(0x3d4, 14);
		_out(0x3d5, (uint8)(cursor>>8));
		_out(0x3d4, 15);
		_out(0x3d5, (uint8)cursor);
	}
}

void console_setscreen(int index)
{
	int i;
	uint32 base = 0x800 * index;
	for(i=0;i<4;i++)
		console_device_extend[i].active=0;
	console_device_extend[index].active=1;

	_out(0x3d4, 12);
	_out(0x3d5, base>>8);
	_out(0x3d4, 13);
	_out(0x3d5, base);
	console_setcursor(&console_device_extend[index]);
}

int32 console_init()
{
	int i;
	for(i=0;i<4;i++)
	{
		console_device_extend[i].attrib=CONSOLE_DEFAULTATTRIB;
		console_device_extend[i].base = (unsigned short *)(0x1000 * i + CONSOLE_BASSADDR);
		console_device_extend[i].cursor = 0;
		console_device_extend[i].active = 0;
		console_device_extend[i].index = i;
		console_device_object[i].driver = &console_driver_object;
		console_device_object[i].extend = &console_device_extend[i];
	}
	console_device_extend[0].active=1;
	console_setcursor(&console_device_extend[0]);
	return 1;
}

pvoid console_open(int8 *path)
{
	int c = *path - '0';
	if(c>=0 && c<4)
		return &console_device_object[c];
	else
		return 0;
}

void console_newline(struct device_extend* ext)
{
	int x=ext->cursor % CONSOLE_WIDTH;
	int y=ext->cursor / CONSOLE_WIDTH;

	if(y+1>=CONSOLE_HEIGHT)
	{
		uint32 *p1=(uint32 *)(ext->base);
		uint32 *p2=(uint32 *)((uint32)(ext->base)+160);
		while(p1<(uint32 *)((uint32)(ext->base)+(CONSOLE_HEIGHT-1)*160))
			*p1++=*p2++;

		while(p1<(uint32 *)((uint32)(ext->base)+CONSOLE_HEIGHT*160))
			*p1++ = (ext->attrib<<16) + ext->attrib;
		ext->cursor=(CONSOLE_HEIGHT-1)*80;
	}else
	{
		y++;
		ext->cursor = y * CONSOLE_WIDTH;
	}
}

int32 console_write(struct device_extend* ext, uint8 *outbuf, uint32 size)
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
			console_newline(ext);
			break;
		case '\t':
		{
			int t = (ext->cursor % CONSOLE_WIDTH + 8) & 0xf8;
			if(t>=CONSOLE_WIDTH)
				console_newline(ext);
			else
				ext->cursor = (ext->cursor + 8) & 0xf8;
			break;
		}
		case '\b':
			if(ext->cursor)
			{
				ext->cursor--;
				ext->base[ext->cursor] = ext->attrib;
			}
			break;
		default:
			ext->base[ext->cursor] = *c + ext->attrib;
			if((ext->cursor+1) % CONSOLE_WIDTH == 0)
				console_newline(ext);
			else
				ext->cursor++;
			break;
		}
		c++;
	}
	console_setcursor(ext);
	return 0;
}

int32 console_ioctrl(struct device_extend* ext, uint32 command, uint8 *inbuf, uint32 insize, uint8 *outbuf, uint32 *outsize)
{
	return 0;
}

int32 console_tell(struct device_extend* ext)
{
	return ext->cursor;
}

int32 console_seek(struct device_extend* ext, uint32 offset)
{
	ext->cursor=offset;
	console_setcursor(ext);
	return 1;
}
