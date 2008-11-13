#include <platform.h>
#include <kernel.h>
#include <stdio.h>

void initsemaphore(struct semaphore * sem, uint32 initial)
{
	sem->value=initial;
	sem->pendingtasks=NULL;
}

void wait(struct semaphore * sem)
{
	_cli();
	if(sem->value==0)
	{
		kePendTask(&sem->pendingtasks, tasks[currentTaskId]);
		_sti();
		keDoSchedNormal();
	}else
		sem->value=0;
	_sti();
}

void release(struct semaphore * sem)
{
	struct taskblock * tb;
	_cli();
	
	if(sem->value==0)
	{
		tb=keTaskDequeue(&sem->pendingtasks);
		if(tb!=NULL)
		{
			keActiveTask(tb);
			_sti();
			keDoSchedNormal();
		}else
			sem->value=1;
	}
	_sti();
}

void waitevent(struct semaphore * sem)
{
	_cli();
	kePendTask(&sem->pendingtasks, tasks[currentTaskId]);
	_sti();
	keDoSchedNormal();
}

void setevent(struct semaphore * sem)
{
	struct taskblock * tb;
	_cli();
	
	tb=keTaskDequeue(&sem->pendingtasks);
	if(tb!=NULL)
	{
		keActiveTask(tb);
		_sti();
		keDoSchedNormal();
	}else
		_sti();
}