#include <kernel.h>
#include <stdio.h>
#include <list.h>
void keDpcAppendItem(TASK_ENTRY entry, uint32 param);

struct dpcjob
{
	struct dpcjob* next;
	struct dpcjob* prev;
	TASK_ENTRY entry;
	uint32 param;
};

struct semaphore dpcevent;

volatile uint32 keTimerTick=0;

struct timerjob
{
	struct timerjob* next;
	struct timerjob* prev;
	TASK_ENTRY entry;
	uint32 param;
	uint32 tick;
	uint32 token;
};

LIST_HEAD(timehead);
LIST_HEAD(dpchead);

void keTimeJobSched()
{
	struct timerjob* header=(struct timerjob*)&timehead;
	_lock();

	while(header->next!=header)
	{
		struct timerjob *p = header->next;
		if((int32)(p->tick-keTimerTick)<0)
		{
			list_del(p);
			keDpcAppendItem(p->entry, p->param);
			keFree(p);
		}else
			break;
	}
	_unlock();
}

uint32 keTimeJobRemove(uint32 timetoken)
{
	struct timerjob *item;
	struct timerjob *header=(struct timerjob*)&timehead;
	_lock();

	list_for_each(item, header)
	{
		if(item->token==timetoken)
		{
			list_del(item);
			item=item->next;
			keFree(item);
			_unlock();
			return 1;
		}
	}
	_unlock();
	return 0;
}

uint32 keTimeJobInsert(TASK_ENTRY entry, uint32 param, uint32 tick)
{
	static uint32 gtimejobtoken=0;
	struct timerjob *item;
	struct timerjob *header=(struct timerjob*)&timehead;
	struct timerjob *p=(struct timerjob *)keMalloc(sizeof(struct timerjob));

	p->entry=entry;
	p->param=param;
	p->tick=tick;

	_lock();
	p->token=gtimejobtoken++;

	list_for_each(item, header)
	{
		if((item->tick-tick)>=0)
			break;
	}
	__list_add(p, item->prev, item);
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
	struct dpcjob* header=(struct dpcjob*)&dpchead;
	struct dpcjob* job=(struct dpcjob*)keMalloc(sizeof(struct dpcjob));
	job->entry=entry;
	job->param=param;

	_lock();
	list_add_tail(job, header);
	_unlock();
	release(&dpcevent);
}

void keDpcJobInsert(TASK_ENTRY entry, uint32 param)
{
	keDpcAppendItem(entry, param);
	keDoSchedNormal();
}

void keDpcProc(uint32 param)
{
	struct dpcjob* header=(struct dpcjob*)&dpchead;
	struct dpcjob *job;
	initsemaphore(&dpcevent, 0);

	while(1)
	{
		wait(&dpcevent);
		job=header->next;
		_lock();
		list_del(job);
		_unlock();
		
		(*job->entry)(job->param);
		keFree(job);
	}
}
