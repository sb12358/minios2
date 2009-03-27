#ifndef KERNAL_DEVICE_INCLUDED
#define KERNAL_DEVICE_INCLUDED

#include "types.h"

struct driver_object
{
	int32 (*init)();
	pvoid (*open)(int8*);
	void  (*close)(pvoid);
	int32 (*read)(pvoid, uint8*, uint32);
	int32 (*write)(pvoid, uint8*, uint32);
	int32 (*invoke)(pvoid, uint8*, uint32);
	int32 (*seek)(pvoid, uint32 offset);
	int32 (*tell)(pvoid);
	int32 (*ioctrl)(pvoid, uint32, uint8 *, uint32, uint8 *, uint32 *);
	int32 (*status)(pvoid);
	int8* drivername;
};

struct device_object
{
	struct driver_object* driver;
	pvoid extend;
};

int32 keLoadDriver(struct driver_object *driver);
pvoid open(int8* driver, int8 *path);
void close(pvoid device);
int32 read(pvoid device, uint8 *inbuf, uint32 size);
int32 write(pvoid device, uint8 *outbuf, uint32 size);
int32 seek(pvoid device, uint32 offset);
int32 tell(pvoid device);
int32 ioctrl(pvoid device, uint32 command, uint8 *inbuf, uint32 insize, uint8 *outbuf, uint32 *outsize);
int32 status(pvoid device);


extern struct driver_object console_driver_object;
extern struct driver_object keyboard_driver_object;
extern struct driver_object ide_driver_object;
extern struct driver_object eth_driver_object;
#endif