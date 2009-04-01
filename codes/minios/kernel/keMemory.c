#include <kernel.h>

struct header
{
    struct header *next;
    uint32 size;
};
#define head_len (sizeof (struct header))

void keHeapInit(struct heapmem * heap, int size)
{
	heap->p_base=(struct header *)(heap->heapbuf);
	heap->p_base->size = size - head_len - sizeof(struct heapmem);
    heap->p_base->next = heap->p_base;
	heap->heapsize=size;
    heap->p_free = heap->p_base;
}

void *keHeapAlloc(struct heapmem * heap,  uint32 nbytes)
{
    struct header * p = 0;
    struct header * p_old_free;
	
	if(nbytes==0)
		return 0;
	
	nbytes=(nbytes+3) & 0xFFFFFFFC;

    p_old_free = heap->p_free;
    while (heap->p_free->size < nbytes + head_len)
    {
        heap->p_free = heap->p_free->next;
		if(heap->p_free == p_old_free)
		{
			printf("Memory exhausted! pid=%d\n", currentTaskId);
			keHeapDump(heap);
			while(1);
		}
    }
    
    heap->p_free->size -= nbytes + head_len;
    p = (struct header *)((char *)heap->p_free + heap->p_free->size + head_len);
    p->size = nbytes;
    p->next=(struct header *)0x55AA5A5A;
    p++;
    
    return (void *)p;
}

void keHeapFree(struct heapmem * heap,  void *pointer)
{
    struct header *bp = (struct header *)pointer;
    struct header *p = heap->p_base;
    
	if(pointer==NULL)
	{
		printf("Cannot free NULL Pointer pid=%d\n", currentTaskId);
		keHeapDump(heap);
		while(1);
	}
    
    bp--;
    
	if(bp->next!=(struct header *)0x55AA5A5A)
	{
		printf("Memory block damaged pid=%d\n", currentTaskId);
		keHeapDump(heap);
		while(1);
	}

    bp->next=0;
    
    for ( ; !(bp>p && bp<p->next); p=p->next)
    {
        if (p>=p->next && (bp > p || bp < p->next))
            break;
    }
    
    if ((struct header *)((char *)bp + bp->size + head_len) == p->next)
    {
        bp->size += p->next->size + head_len;
        bp->next = p->next->next;
    	if(heap->p_free==p->next)
    		heap->p_free=bp;
    }
    else
    {
        bp->next = p->next; 
    }
    if ((struct header *)((char *)p + p->size + head_len) == bp)
    {
        p->size += bp->size + head_len;
        p->next = bp->next;
     	if(heap->p_free==bp)
    		heap->p_free=p;
    }
    else
    {
        p->next = bp;
    }
}

void keHeapDump(struct heapmem * heap) {
  struct header * p_tmp_base = heap->p_base;
  
  for (;;)
  {
    if(p_tmp_base->next==(struct header *)0x55AA5A5A)
      printf("%08X[55AA5A5A][%6d] *\n", p_tmp_base, p_tmp_base->size);
    else
      printf("%08X[%08X][%6d]\n", p_tmp_base, p_tmp_base->next, p_tmp_base->size);
    
	p_tmp_base = (struct header *)((uint32)p_tmp_base + (uint32)p_tmp_base->size + (uint32)head_len);
    
	if(p_tmp_base >= (struct header *)((uint32)heap + heap->heapsize))
      return ;
  }
}

struct heapmem * keheap=0;

void keKernelHeapInit()
{
	keheap=(struct heapmem *)0x100000;
	keHeapInit(keheap, 0x300000);
}

void* keMalloc(uint32 nbytes)
{
	void *p;
	_lock();
	p = keHeapAlloc(keheap, nbytes);
	_unlock();
	return p;
}

void keFree(void *pointer)
{
	_lock();
	keHeapFree(keheap, pointer);
	_unlock();
}

void keKernelHeapDump()
{
	_lock();
	keHeapDump(keheap);
	_unlock();
}