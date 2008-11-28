[ORG 0x7c00]
[BITS 16]
start:
	jmp start_16
	times 87 db 0		; skip 87 bytes
start_16:
	cli			; disable interrupt
	cld			;

	mov	ax, cs
	mov	ds, ax
	mov	ss, ax
	lgdt	[gdt_48]	;load gdt

	mov	eax,cr0
	or	eax,1
	mov	cr0,eax		;enter protect mode
	jmp	dword 8:start_32;dword MUST NOT FORGET!

[BITS 32]
start_32:
	mov	ax, 10h
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	gs, ax
	mov	fs, ax
	mov	esp, 08000h

	mov	edi, 400000h
	call	read_it32

	mov	eax, [40003Ch]
	mov	eax, [eax+400028h]
	add	eax, 400000h
	call	eax
loopforever:
	jmp	loopforever

read_it32:
	mov	dx, 3f6h
	mov	al, 04h
	out	dx, al		;SRST=1
	xor	al, al
	out	dx, al		;SRST=0

wait_ready:
	mov	dx, 1f7h
	in	al, dx
	and	al, 01000000b
	jz	wait_ready	;Wait DRDY=1

	xor	ecx, ecx
	mov	ebx, [7df8h]
	mov	cx, [7dfch]
	and	cx, cx
	jne	more_to_read
	ret
more_to_read:
	cmp	cx, 256;
	jg	large_than_256
	jmp	store_remain
large_than_256:
	mov	cx, 256
store_remain:
	sub	[7dfch], cx
	add	[7df8h], ecx	;calculate sectors to read
	
	sub	dl, 6
	xor	al, al
	out	dx, al		;out 1f1h, 0

	inc	dl
	mov	al, cl
	out	dx, al		;out 1f2h, sectorsToRead

	inc	dl
	mov	al, 40h
	shl	eax, 24
	add	eax, ebx
	out	dx, eax		;out 1f3h, ADDR | 0x40000000

	add	dl, 4
	mov	al, 20h
	out	dx, al		;out 1f7h, 20h

	shl	ecx, 8

waitHDD:
	in      al, dx
	and     al, 00001000b
	jz      waitHDD		;wait next data ready

	sub     dl, 7
	;rep	insw
	in      ax, dx		;read next 2 bytes
	stosw
	add	dl, 7
	loop    waitHDD
	jmp	wait_ready

showchar:
	push ax
	push bx
	push cx
	push edi
	mov edi, 0b8000h
	mov cx, 8
	mov bl, al
showcharloop:
	mov ax, 731h	
	shl bl, 1
	jc showone
	sub al, 1
showone:
	stosw
	loop showcharloop
	pop edi
	pop cx
	pop bx
	pop ax
	ret
gdt:
	db 0,0,0,0,0,0,0,0
	db 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00  ;code segment
	db 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00  ;data segment
gdt_48:
	dw 01FFh
	dw gdt, 0

times 510-($-$$) db 0
	dw 0AA55h
