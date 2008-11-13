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
	struct driver_object *driver=((struct device_object*)device)->driver;
	if(driver->close)
		driver->close(device);
}

int32 read(pvoid device, uint8 *inbuf, uint32 size)
{
	struct driver_object *driver=((struct device_object*)device)->driver;
	if(driver->read)
		return driver->read(device, inbuf, size);
	else
		return 0;
}

int32 write(pvoid device, uint8 *outbuf, uint32 size)
{
	struct driver_object *driver=((struct device_object*)device)->driver;
	if(driver->write)
		return driver->write(device, outbuf, size);
	else
		return 0;
}

int32 seek(pvoid device, uint32 offset)
{
	struct driver_object *driver=((struct device_object*)device)->driver;
	if(driver->seek)
		return driver->seek(device, offset);
	else
		return 0;
}

int32 tell(pvoid device)
{
	struct driver_object *driver=((struct device_object*)device)->driver;
	if(driver->tell)
		return driver->tell(device);
	else
		return 0;
}

int32 ioctrl(pvoid device, uint32 command, uint8 *inbuf, uint32 insize, uint8 *outbuf, uint32 *outsize)
{
	struct driver_object *driver=((struct device_object*)device)->driver;
	if(driver->ioctrl)
		return driver->ioctrl(device, command, inbuf, insize, outbuf, outsize);
	else
		return 0;
}

int32 status(pvoid device)
{
	struct driver_object *driver=((struct device_object*)device)->driver;
	if(driver->status)
		return driver->status(device);
	else
		return 0;
}
