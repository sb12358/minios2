#include <device.h>
#include <string.h>

struct driver_object *loadeddriver[32];
int32 loadeddrivernum=0;

int32 keLoadDriver(struct driver_object *driver)
{
	int r=driver->init();
	if(r!=0)
		loadeddriver[loadeddrivernum++]=driver;
	return r;
}

pvoid open(int8* driver, int8 *path)
{
	int i;
	for(i=0;i<loadeddrivernum;i++)
	{
		if(strcmp(driver, loadeddriver[i]->drivername)==NULL)
			break;
	}
	if(i==loadeddrivernum)
		return NULL;

	return loadeddriver[i]->open(path);
}

void close(pvoid device)
{
	struct device_object *dev = (struct device_object*)device;
	if(dev==0)
		return;
	if(dev->driver->close)
		dev->driver->close(dev->extend);
}

int32 read(pvoid device, uint8 *inbuf, uint32 size)
{
	struct device_object *dev = (struct device_object*)device;
	if(dev==0)
		return 0;
	if(dev->driver->read)
		return dev->driver->read(dev->extend, inbuf, size);
	else
		return 0;
}

int32 write(pvoid device, uint8 *outbuf, uint32 size)
{
	struct device_object *dev = (struct device_object*)device;
	if(dev==0)
		return 0;
	if(dev->driver->write)
		return dev->driver->write(dev->extend, outbuf, size);
	else
		return 0;
}

int32 seek(pvoid device, uint32 offset)
{
	struct device_object *dev = (struct device_object*)device;
	if(dev==0)
		return 0;
	if(dev->driver->seek)
		return dev->driver->seek(dev->extend, offset);
	else
		return 0;
}

int32 tell(pvoid device)
{
	struct device_object *dev = (struct device_object*)device;
	if(dev==0)
		return 0;
	if(dev->driver->tell)
		return dev->driver->tell(dev->extend);
	else
		return 0;
}

int32 ioctrl(pvoid device, uint32 command, uint8 *inbuf, uint32 insize, uint8 *outbuf, uint32 *outsize)
{
	struct device_object *dev = (struct device_object*)device;
	if(dev==0)
		return 0;
	if(dev->driver->ioctrl)
		return dev->driver->ioctrl(dev->extend, command, inbuf, insize, outbuf, outsize);
	else
		return 0;
}

int32 status(pvoid device)
{
	struct device_object *dev = (struct device_object*)device;
	if(dev==0)
		return 0;
	if(dev->driver->status)
		return dev->driver->status(dev->extend);
	else
		return 0;
}
