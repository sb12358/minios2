#include "types.h"
#include "platform.h"
#include "stdio.h"

void keEntryMain(uint32 param);

struct heapmem
{
	struct header *p_base;  //头指针
	struct header *p_free;  //当前指针
	uint32 heapsize;
	char heapbuf[0];
};

void keHeapInit(struct heapmem * heap, int size);
void *keHeapAlloc(struct heapmem * heap,  uint32 nbytes);
void keHeapFree(struct heapmem * heap,  void *pointer);
void keHeapDump(struct heapmem * heap);

void* keMalloc(uint32 nbytes);
void keFree(void *pointer);
void keKernelHeapInit();
void keKernelHeapDump();

void keKeyboardIsr();
uint8 keGetChar();

struct taskblock
{
	uint32 *esp;
	char taskname[32];
	uint32 priority;
	uint32 taskid;
	uint32 status;
	struct taskblock *next;
	struct taskblock ** queue;
	char stacks[0];
};

typedef void(*TASK_ENTRY)(uint32 param);

#define TASK_ACTIVE 0
#define TASK_PEND 1

extern uint32 currentTaskId;
extern struct taskblock **tasks;

void keInitTaskSystem();
struct taskblock* keNewTask(char *name, TASK_ENTRY entry, uint32 param, uint32 priority, uint32 stacksize);
void keDoSched();
void keKillTask(uint32 taskid);
void keShowTasks();

void keTaskEnqueue(struct taskblock ** header, struct taskblock *data);
struct taskblock * keTaskDequeue(struct taskblock ** header);
struct taskblock * keTaskRemove(struct taskblock *data);
struct taskblock * keActiveTask(struct taskblock *task);
struct taskblock * kePendTask(struct taskblock ** pendlist, struct taskblock *task);

_inline keDoSchedNormal()
{
	_int17();
}

struct semaphore
{
	uint32 value;
	struct taskblock * pendingtasks;
};

void initsemaphore(struct semaphore * sem, uint32 initial);
void wait(struct semaphore * sem);
void release(struct semaphore * sem);
void waitevent(struct semaphore * sem);
void setevent(struct semaphore * sem);

void keTimerIsr();
uint32 keGetTickCount();
void keDelay(uint32 ticks);
void keDpcProc(uint32 param);
void keDpcJobInsert(TASK_ENTRY entry, uint32 param);
uint32 keTimeJobInsert(TASK_ENTRY entry, uint32 param, uint32 tick);
uint32 keTimeJobRemove(uint32 timetoken);

_inline void Assert(int exp, char* str)
{
	if(!exp)
	{
		puts(str);
		keKillTask(currentTaskId);
		while(1);
	}
}

