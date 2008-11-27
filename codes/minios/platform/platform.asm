.486p
.model flat, c
option casemap :none

.data
public _ISREXP 
public _ISRINT
public GDT_DATA
public IDT_DATA

_ISREXP	dd IsrExp0, IsrExp1, IsrExp2, IsrExp3 
	dd IsrExp4, IsrExp5, IsrExp6, IsrExp7
	dd IsrExp8, IsrExp9, IsrExp10, IsrExp11
	dd IsrExp12, IsrExp13, IsrExp14, IsrExp15
	dd IsrExp16, IsrSoft17, IsrSoft18, IsrSoft19
	dd IsrSoft20, IsrSoft21, IsrSoft22, IsrSoft23

_ISRINT	dd IsrInt0, IsrInt1, IsrInt2, IsrInt3 
	dd IsrInt4, IsrInt5, IsrInt6, IsrInt7
	dd IsrInt8, IsrInt9, IsrInt10, IsrInt11
	dd IsrInt12, IsrInt13, IsrInt14, IsrInt15

GDT_DATA db 0,0,0,0,0,0,0,0			;dummy
	db 0ffh, 0ffh, 0, 0, 0, 9Ah, 0CFh, 0	;code seg
	db 0ffh, 0ffh, 0, 0, 0, 92h, 0CFh, 0	;data seg
GDT_48	dw 01ffh
	dd offset GDT_DATA

IDT_DATA dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0
	dd 0,0,0,0,0,0,0,0

IDT_48	dw 0fffh
	dd offset IDT_DATA

.code

_switch proc
	mov ecx, [esp+4]
	mov edx, [esp+8]
	mov eax, esp
	mov esp, [ecx]
	mov [edx], eax
	ret
_switch endp

_loadidtgdt proc
	lgdt fword ptr[GDT_48]
	lidt fword ptr[IDT_48]
	ret
_loadidtgdt endp

_in proc
	mov edx, [esp+4]
	in al, dx
	ret
_in endp

_out proc
	mov edx, [esp+4]
	mov eax, [esp+8]
	out dx, al
	ret
_out endp

_in16 proc
	mov edx, [esp+4]
	in ax, dx
	ret
_in16 endp

_out16 proc
	mov edx, [esp+4]
	mov eax, [esp+8]
	out dx, ax
	ret
_out16 endp

_in32 proc
	mov edx, [esp+4]
	in eax, dx
	ret
_in32 endp

_out32 proc
	mov edx, [esp+4]
	mov eax, [esp+8]
	out dx, eax
	ret
_out32 endp

dummyIsr proc
	iretd
dummyIsr endp

IsrExp0:
	pushad
	mov eax, 0
	call dword ptr[eax]
	popad
	iretd

IsrExp1:
	pushad
	mov eax, 4
	call dword ptr[eax]
	popad
	iretd

IsrExp2:
	pushad
	mov eax, 8
	call dword ptr[eax]
	popad
	iretd

IsrExp3:
	pushad
	mov eax, 0ch
	call dword ptr[eax]
	popad
	iretd

IsrExp4:
	pushad
	mov eax, 10h
	call dword ptr[eax]
	popad
	iretd

IsrExp5:
	pushad
	mov eax, 14h
	call dword ptr[eax]
	popad
	iretd

IsrExp6:
	pushad
	mov eax, 18h
	call dword ptr[eax]
	popad
	iretd

IsrExp7:
	pushad
	mov eax, 1ch
	call dword ptr[eax]
	popad
	iretd

IsrExp8:
	pushad
	mov eax, 20h
	call dword ptr[eax]
	popad
	add esp, 4
	iretd

IsrExp9:
	pushad
	mov eax, 24h
	call dword ptr[eax]
	popad
	iretd

IsrExp10:
	pushad
	mov eax, 28h
	call dword ptr[eax]
	popad
	add esp, 4
	iretd

IsrExp11:
	pushad
	mov eax, 2ch
	call dword ptr[eax]
	popad
	add esp, 4
	iretd

IsrExp12:
	pushad
	mov eax, 30h
	call dword ptr[eax]
	popad
	add esp, 4
	iretd

IsrExp13:
	pushad
	mov eax, 34h
	call dword ptr[eax]
	popad
	add esp, 4
	iretd

IsrExp14:
	pushad
	mov eax, 38h
	call dword ptr[eax]
	popad
	add esp, 4
	iretd

IsrExp15:
	pushad
	mov eax, 3ch
	call dword ptr[eax]
	popad
	iretd

IsrExp16:
	pushad
	mov eax, 40h
	call dword ptr[eax]
	popad
	iretd

IsrSoft17:
	pushad
	mov eax, 44h
	call dword ptr[eax]
	popad
	iretd

IsrSoft18:
	pushad
	mov eax, 48h
	call dword ptr[eax]
	popad
	iretd

IsrSoft19:
	pushad
	mov eax, 4ch
	call dword ptr[eax]
	popad
	iretd

IsrSoft20:
	pushad
	mov eax, 50h
	call dword ptr[eax]
	popad
	iretd

IsrSoft21:
	pushad
	mov eax, 54h
	call dword ptr[eax]
	popad
	iretd

IsrSoft22:
	pushad
	mov eax, 58h
	call dword ptr[eax]
	popad
	iretd

IsrSoft23:
	pushad
	mov eax, 5ch
	call dword ptr[eax]
	popad
	iretd

IsrInt0:
	pushad
	mov al, 20h
	out 020h, al
	mov eax, 080h
	call dword ptr[eax]
	popad
	iretd

IsrInt1:
	pushad
	mov al, 20h
	out 020h, al
	mov eax, 084h
	call dword ptr[eax]
	popad
	iretd

IsrInt2:
	pushad
	mov al, 20h
	out 020h, al
	mov eax, 088h
	call dword ptr[eax]
	popad
	iretd

IsrInt3:
	pushad
	mov al, 20h
	out 020h, al
	mov eax, 08ch
	call dword ptr[eax]
	popad
	iretd

IsrInt4:
	pushad
	mov al, 20h
	out 020h, al
	mov eax, 090h
	call dword ptr[eax]
	popad
	iretd

IsrInt5:
	pushad
	mov al, 20h
	out 020h, al
	mov eax, 094h
	call dword ptr[eax]
	popad
	iretd

IsrInt6:
	pushad
	mov al, 20h
	out 020h, al
	mov eax, 098h
	call dword ptr[eax]
	popad
	iretd

IsrInt7:
	pushad
	mov al, 20h
	out 020h, al
	mov eax, 09ch
	call dword ptr[eax]
	popad
	iretd

IsrInt8:
	pushad
	mov al, 20h
	out 0A0h, al
	mov eax, 0a0h
	call dword ptr[eax]
	popad
	iretd

IsrInt9:
	pushad
	mov al, 20h
	out 0A0h, al
	mov eax, 0a4h
	call dword ptr[eax]
	popad
	iretd

IsrInt10:
	pushad
	mov al, 20h
	out 0A0h, al
	mov eax, 0a8h
	call dword ptr[eax]
	popad
	iretd

IsrInt11:
	pushad
	mov al, 20h
	out 0A0h, al
	mov eax, 0ach
	call dword ptr[eax]
	popad
	iretd

IsrInt12:
	pushad
	mov al, 20h
	out 0A0h, al
	mov eax, 0b0h
	call dword ptr[eax]
	popad
	iretd

IsrInt13:
	pushad
	mov al, 20h
	out 0A0h, al
	mov eax, 0b4h
	call dword ptr[eax]
	popad
	iretd

IsrInt14:
	pushad
	mov al, 20h
	out 0A0h, al
	mov eax, 0b8h
	call dword ptr[eax]
	popad
	iretd

IsrInt15:
	pushad
	mov al, 20h
	out 0A0h, al
	mov eax, 0bch
	call dword ptr[eax]
	popad
	iretd

end