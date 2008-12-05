#include <platform.h>
#include <kernel.h>
#include <string.h>
#include <stdio.h>

struct break_point_t
{
	uint8 enable;
	uint8 state;
	uint8 oldvalue;
	uint8 taskid;
	uint32 ipaddr;
	DEBUGCALLBACK callback;
}bp[4];

void trapint3_handler(uint32 edi, uint32 esi, uint32 ebp, uint32 esp,
		   uint32 ebx, uint32 edx, uint32 ecx, uint32 eax, 
		   volatile uint32 eip, volatile uint32 ecs, volatile uint32 eflags)
{
	int i;
	eip--;

	for(i=0;i<4;i++)
	{
		struct break_point_t * p=&bp[i];
		if(p->enable==0)
			continue;

		if(currentTaskId==p->taskid)
		{
			if(p->state==BPSTATE_READY && p->ipaddr==eip)
			{
				*((uint8*)eip)=p->oldvalue;
				p->state=p->callback((struct break_regstat_t *)&edi);
				eflags |= 0x100;	// Set TF
				return;
			}
		}
	}
}

void trapstep_handler(uint32 edi, uint32 esi, uint32 ebp, uint32 esp,
		   uint32 ebx, uint32 edx, uint32 ecx, uint32 eax, 
		   uint32 eip, uint32 ecs, volatile uint32 eflags)
{
	int i;
	eflags &= ~0x100;	// Clear TF

	for(i=0;i<4;i++)
	{
		struct break_point_t * p=&bp[i];
		if(p->enable==0)
			continue;

		if(currentTaskId==p->taskid)
		{
			if(p->state==BPSTATE_RESUME)
			{
				p->state=BPSTATE_READY;
				p->oldvalue=*((uint8*)p->ipaddr);
				*((uint8*)p->ipaddr)=0xcc;
			}else if(p->state==BPSTATE_STEP)
			{
				p->oldvalue=*((uint8*)p->ipaddr);
				*((uint8*)p->ipaddr)=0xcc;
				p->state=p->callback((struct break_regstat_t *)&edi);
				eflags |= 0x100;	// Set TF
			}else if(p->state==BPSTATE_DISCARD)
			{
				removebreakpoint(i);
			}
		}
	}
}

void initKeDebug()
{
	memset(bp, 0, sizeof(bp));
	_ISRVECT[3]=(uint32)trapint3_handler;
	_ISRVECT[1]=(uint32)trapstep_handler;
}

int setbreadpoint(uint32 tid, uint32 eip, DEBUGCALLBACK fn)
{
	int i;
	for(i=0;i<4;i++)
		if(bp[i].enable && bp[i].ipaddr==eip)
			return -1;			// the break point alread exist
	for(i=0;i<4;i++)
		if(bp[i].enable==0)
			break;
	if(i==4)
		return -1;

	bp[i].state=BPSTATE_READY;
	bp[i].callback=fn;
	bp[i].oldvalue=*((uint8*)eip);
	bp[i].taskid=tid;
	bp[i].ipaddr=eip;
	bp[i].enable=1;
	*((uint8*)eip)=0xcc;
	return i;
}

void removebreakpoint(int i)
{
	if(bp[i].enable)
	{
		bp[i].enable=0;
		*((uint8*)bp[i].ipaddr) = bp[i].oldvalue;
	}
}
