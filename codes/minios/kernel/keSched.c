#include <platform.h>
#include <kernel.h>
#include <string.h>

struct taskblock **tasks;
uint32 lasttaskid;
uint32 currentTaskId;
struct taskblock *activequeue[16];

struct taskblock * keTaskRemove(struct taskblock *data)
{
	struct taskblock ** p=data->queue;
	
	if(p!=NULL)
	{
		if(*p!=NULL)
		{
			while((*p)!=0 && (*p)!=data)
				p=&((*p)->next);

			if((*p)==data)
				(*p)=data->next;
		}
	}

	data->next=NULL;
	data->queue=NULL;
	return data;
}

void keTaskEnqueue(struct taskblock ** header, struct taskblock *data)
{
	// A TB can only belong's to one queue
	// So first remove it from it's old queue
	keTaskRemove(data);

	// Then add to the new queue
	data->queue=header;
	data->next=NULL;
	while(*header)
		header=&((*header)->next);
	*header=data;

}

struct taskblock * keTaskDequeue(struct taskblock ** header)
{
	struct taskblock *p;

	if(*header==NULL)
		return NULL;
	

	p=*header;
	*header=(*header)->next;

	p->queue=NULL;
	p->next=NULL;
	
	return p;
}

struct taskblock * keActiveTask(struct taskblock *task)
{
	keTaskEnqueue(&activequeue[task->priority], task);
	task->status=TASK_ACTIVE;
	return task;
}

struct taskblock * kePendTask(struct taskblock ** pendlist, struct taskblock *task)
{
	keTaskEnqueue(pendlist, task);
	task->status=TASK_PEND;
	return task;
}

uint32 keNewTaskId()
{
	uint32 i;
	for(i=lasttaskid+1;i<1024;i++)
	{
		if(tasks[i]==0)
			break;
	}
	if(i==1024)
	{
		for(i=0;i<lasttaskid;i++)
		if(tasks[i]==0)
			break;
	}

	if(i==lasttaskid)
		return 0xFFFFFFFF;

	lasttaskid=i;
	return i;
}

void keInitTaskSystem()
{
	int i;
	tasks=(struct taskblock **)keMalloc(1024*sizeof(struct taskblock *));
	for(i=0;i<1024;i++)
		tasks[i]=0;
	for(i=0;i<16;i++)
		activequeue[i]=0;
	lasttaskid=0xFFFFFFFF;
	currentTaskId=0;
}

void keTaskEnd()
{
	printf("Task %d Terminated\n", currentTaskId);
	keKillTask(currentTaskId);
	while(1)
		;
}

struct taskblock* keNewTask(char *name, TASK_ENTRY entry, uint32 param, uint32 priority, uint32 stacksize)
{
	int i;
	struct taskblock* newtask;

	if(priority>15)
		return 0;

	i=keNewTaskId();
	if(i==0xFFFFFFFF)
		return 0;

	if(stacksize==0)
		stacksize=0x4000;

	tasks[i]=keMalloc(stacksize+sizeof(struct taskblock));
	newtask=tasks[i];
	newtask->next=NULL;
	newtask->taskid=i;
	newtask->priority=priority;
	newtask->queue=NULL;
	strncpy(newtask->taskname, name, 32);
	newtask->taskname[31]=0;
	newtask->esp=(uint32*)&newtask->stacks[stacksize-16];
	newtask->esp[0]=(uint32)entry;
	newtask->esp[1]=(uint32)keTaskEnd;
	newtask->esp[2]=param;

	_lock();
	keActiveTask(newtask);
	_unlock();

	return newtask;
}

uint32 keSchedNextTask()
{
	uint32 i;

	for(i=0;i<16;i++)
	{
		struct taskblock* p=keTaskDequeue(&activequeue[i]);
		if(p!=NULL)
		{
			keTaskEnqueue(&activequeue[p->priority], p);
			return p->taskid;
		}
	}

	return 0xFFFFFFFF;
}

void keKillTask(uint32 taskid)
{
	if(taskid==0)
		return;		// task 0 can never be killed

	_lock();
	if(tasks[taskid])
	{
		keTaskRemove(tasks[taskid]);
		keFree(tasks[taskid]);
		tasks[taskid]=0;
	}
	_unlock();
	keDoSchedNormal();
}

void keDoSched()
{
	uint32 i;
	uint32 j;

	_cli();
	i=keSchedNextTask();
	while(i==0xFFFFFFFF)
	{
		_sti();
		_hlt();
		i=keSchedNextTask();
	}						// not any task can be sched

	if(i==currentTaskId)
	{
		_sti();
		return;				// only the current task can be sched
	}
	j=currentTaskId;
	currentTaskId=i;
	_sti();
	_switch(&(tasks[i]->esp), &(tasks[j]->esp));
}

void keShowTasks()
{
	uint32 i;
	for(i=0;i<1024;i++)
	{
		if(tasks[i])
		{
			if(i==currentTaskId)
				printf("Task%d name=\"%s\" status=%d tb=%08x *\n",i, tasks[i]->taskname, tasks[i]->status, tasks[i], tasks[i]->esp[12]);
			else
				printf("Task%d name=\"%s\" status=%d tb=%08x ip=%08x\n",i, tasks[i]->taskname, tasks[i]->status, tasks[i], tasks[i]->esp[12]);
		}
	}
}