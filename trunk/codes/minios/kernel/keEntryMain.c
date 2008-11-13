#include <platform.h>
#include <device.h>
#include <kernel.h>

uint32 htol(char* hexstr)
{
	int r=0;
	while(*hexstr)
	{
		if(*hexstr>='0' && *hexstr<='9')
		{
			r<<=4;
			r += *hexstr-'0';
		}else if(*hexstr>='A' && *hexstr<='F')
		{
			r<<=4;
			r += *hexstr-'A'+10;
		}else if(*hexstr>='a' && *hexstr<='f')
		{
			r<<=4;
			r += *hexstr-'a'+10;
		}else
			return r;
		hexstr++;
	}
	return r;
}

void dumpMemory(uint32 addr)
{
	int i,j;
	uint8 * p = (uint8*)addr;
	for(j=0;j<16;j++)
	{
		printf("%08X  ", addr+j*16);
		for(i=0;i<16;i++)
		{
			printf("%02X ", p[j*16+i]);
		}

		for(i=0;i<16;i++)
		{
			uint8 c=p[j*16+i];

			if(c>=0x20 && c<=126)
				printf("%c", p[j*16+i]);
			else
				printf(".");
		}
		printf("\n");
	}
}

void kdebug(uint32 param)
{
	char buffer[80];
	while(1)
	{
		printf("> ");
		gets(buffer);
		
		switch(buffer[0])
		{
			case 'd':
			{
				uint32 addr=htol(&buffer[2]);
				printf("Dump Memory Address=%08x\n", addr);
				dumpMemory(addr);
				break;
			}
			case 'a':
			{
				uint32 len=htol(&buffer[2]);
				uint32 addr=(uint32)keMalloc(len);
				printf("Alloc Memory from kernel heap: %08x[%08x]\n", addr, len);
				break;
			}
			case 'f':
			{
				uint32 addr=htol(&buffer[2]);
				keFree((void*)addr);
				printf("Free Memory from kernel heap, pointer=%08x\n", addr);
				break;
			}
			case 'h':
				keKernelHeapDump();
				break;
			case '?':
				puts("a size: Alloc size Memory From Kernel Heap\n");
				puts("d addr: Dump Memory Around Address addr\n");
				puts("f addr: Free Memory at addr From Kernel Heap\n");
				puts("h: View Kernel Heap Assignmemt\n");
				puts("k id: Terminate The Task id\n");
				puts("t: Show Task List\n");
				break;
			case 'k':
				{
					uint32 id=htol(&buffer[2]);
					keKillTask(id);
					break;
				}
			case 't':
				keShowTasks();
				break;
			default :
				puts("Invalid command, type '?' to see commands list\n");
				break;
		}
	}
}

struct semaphore s;

void test1(uint32 param)
{
	while(1)
	{
		wait(&s);
		printf("Task test1 Still Active\n");
	}
}

void keEntryMain(uint32 param)
{
	_sti();

	keLoadDriver(&keyboard_driver_object);
	keLoadDriver(&console_driver_object);
	printf("booting...\n");
	keNewTask("dpcmain", keDpcProc, 0, 6, 0x4000);
	pciInit();

	keLoadDriver(&ide_driver_object);
	ide_readdma(0, 1);

	initsemaphore(&s, 0);
	keNewTask("kdebug", kdebug, 0, 8, 0x4000);
	keNewTask("test1", test1, 0, 7, 0x4000);

	while(1)
	{
		keDelay(200);
		//release(&s);
	}
}
