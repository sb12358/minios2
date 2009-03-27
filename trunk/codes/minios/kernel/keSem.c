#include <platform.h>
#include <kernel.h>
#include <stdio.h>

void initsemaphore(struct semaphore * sem, uint32 initial)
{
	sem->value=initial;
	sem->pendingtasks=NULL;
}

void initmutex(struct semaphore * sem)
{
	sem->value=1;
	sem->pendingtasks=NULL;
}

void wait(struct semaphore * sem)
{
	_lock();
	if(sem->value==0)
	{
		kePendTask(&sem->pendingtasks, tasks[currentTaskId]);
		_unlock();
		keDoSchedNormal();
		_lock();
	}
	sem->value--;
	_unlock();
}

void release(struct semaphore * sem)
{
	struct taskblock * tb;
	_lock();
	if(sem->value==0)
	{
		tb=keTaskDequeue(&sem->pendingtasks);
		if(tb!=NULL)
			keActiveTask(tb);
	}
	sem->value++;
	_unlock();
}

void clearevent(struct semaphore * sem)
{
	sem->value=0;
}

void waitevent(struct semaphore * sem)
{
	_lock();
	if(sem->value==0)
	{
		kePendTask(&sem->pendingtasks, tasks[currentTaskId]);
		_unlock();
		keDoSchedNormal();
		_lock();
	}
	sem->value=0;
	_unlock();
}

void setevent(struct semaphore * sem)
{
	struct taskblock * tb;
	_lock();
	if(sem->value==0)
	{
		tb=keTaskDequeue(&sem->pendingtasks);
		if(tb!=NULL)
			keActiveTask(tb);
	}
	sem->value=1;
	_unlock();
}