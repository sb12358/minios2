#include <kernel.h>
#include <stdio.h>

void keDpcAppendItem(TASK_ENTRY entry, uint32 param);

struct dpcjob
{
	TASK_ENTRY entry;
	uint32 param;
	struct dpcjob* next;
};

struct semaphore dpcevent;
struct dpcjob * dpchead=NULL;

volatile uint32 keTimerTick=0;

struct timerjob
{
	TASK_ENTRY entry;
	uint32 param;
	uint32 tick;
	uint32 token;
	struct timerjob* next;
};
struct timerjob * timehead=NULL;

void keTimeJobSched()
{
	struct timerjob *p;
	_lock();
	p=timehead;
	while(p)
	{
		if((int32)(p->tick-keTimerTick)<0)
		{
			timehead=p->next;
			keDpcAppendItem(p->entry, p->param);
			keFree(p);
			p=timehead;
		}else
			break;
	}
	_unlock();
}

uint32 keTimeJobRemove(uint32 timetoken)
{
	_lock();
	if(timehead!=NULL)
	{
		struct timerjob **item= &timehead;
		struct timerjob *next=timehead;

		while(next)
		{
			if(next->token==timetoken)
			{
				*item=next->next;
				keFree(next);
				_unlock();
				return 1;
			}
			item=&(next->next);
			next=next->next;
		}
	}
	_unlock();
	return 0;
}

uint32 keTimeJobInsert(TASK_ENTRY entry, uint32 param, uint32 tick)
{
	static uint32 gtimejobtoken=0;
	struct timerjob *p=(struct timerjob *)keMalloc(sizeof(struct timerjob));
	p->entry=entry;
	p->param=param;
	p->tick=tick;
	p->next=NULL;

	_lock();
	p->token=gtimejobtoken++;
	if(timehead==NULL)
	{
		timehead=p;
	}else
	{
		struct timerjob **item= &timehead;
		struct timerjob *next=timehead;

		while(next)
		{
			if((next->tick-tick)>=0)
				break;
			item=&(next->next);
			next=next->next;
		}

		p->next=*item;
		*item=p;
	}
	_unlock();

	return p->token;
}

void keTimerIsr()
{
	++keTimerTick;
	keTimeJobSched();
	keDoSched();
}

uint32 keGetTickCount()
{
	return keTimerTick;
}

void keDelayDpc(uint32 param)
{
	release((struct semaphore *)param);
}

void keDelay(uint32 ticks)
{
	struct semaphore sem;
	uint32 timeuntil=keTimerTick+ticks;
	initsemaphore(&sem, 0);
	keTimeJobInsert(keDelayDpc, (uint32)&sem, timeuntil);
	wait(&sem);
}

void keDpcAppendItem(TASK_ENTRY entry, uint32 param)
{
	struct dpcjob ** item;
	struct dpcjob *job=(struct dpcjob*)keMalloc(sizeof(struct dpcjob));
	job->entry=entry;
	job->param=param;
	job->next=NULL;

	_lock();
	item=&dpchead;
	while(*item!=NULL)
		item=&((*item)->next);
	*item=job;
	_unlock();
	setevent(&dpcevent);
}

void keDpcJobInsert(TASK_ENTRY entry, uint32 param)
{
	keDpcAppendItem(entry, param);
	setevent(&dpcevent);
	keDoSchedNormal();
}

void keDpcProc(uint32 param)
{
	struct dpcjob *job;
	initsemaphore(&dpcevent, 0);
	dpchead=NULL;

	while(1)
	{
		while(dpchead==NULL)
			waitevent(&dpcevent);
		_lock();
		job=dpchead;
		dpchead=dpchead->next;
		_unlock();
		
		(*job->entry)(job->param);
		keFree(job);
	}
}
