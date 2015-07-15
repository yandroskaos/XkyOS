;;
;; PM.asm -- Protected Mode Related Routines
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

;;;;;;;;;;;;;;;;;;;;;;;;;SetIDT;;;;;;;;;;;;;;;;;;;;;;;;;
IDT:
	dw IDT_SIZE
	dd IDT_DIRECTION

SetIDT:
	xor esi, esi
	mov si, IDT_SEGMENT
	mov gs, si
	mov si, 0x0000
	
	mov ecx, IDT_SIZE
	shr ecx, 3
.set_idt_empty_descriptor:
	gs mov dword [si], 0
	gs mov dword [si+4], 0
	add si, 8
	loop .set_idt_empty_descriptor

	;Cargamos
	db 066h
	lidt [IDT]	
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;SetGDT;;;;;;;;;;;;;;;;;;;;;;;;;
GDT:
	dw GDT_SIZE
	dd GDT_DIRECTION

SetGDT:
	xor esi, esi
	mov si, GDT_SEGMENT
	mov gs, si
	mov si, 0x0000
	
	;Primero entrada NULL
	gs mov dword [si],	NULL_SEGMENT_DESCRIPTOR_R0_LOW
	gs mov dword [si+4],	NULL_SEGMENT_DESCRIPTOR_R0_HIGH
	
	;Segunda entrada CODE RING 0
	gs mov dword [si+8],	CODE_SEGMENT_DESCRIPTOR_R0_LOW
	gs mov dword [si+12],	CODE_SEGMENT_DESCRIPTOR_R0_HIGH
	
	;Tercera entrada DATA RING 0
	gs mov dword [si+16],	DATA_SEGMENT_DESCRIPTOR_R0_LOW
	gs mov dword [si+20],	DATA_SEGMENT_DESCRIPTOR_R0_HIGH

	;Cuarta entrada CODE RING 3
	gs mov dword [si+24],	CODE_SEGMENT_DESCRIPTOR_R3_LOW
	gs mov dword [si+28],	CODE_SEGMENT_DESCRIPTOR_R3_HIGH
	
	;Quinta entrada DATA RING 3
	gs mov dword [si+32],	DATA_SEGMENT_DESCRIPTOR_R3_LOW
	gs mov dword [si+36],	DATA_SEGMENT_DESCRIPTOR_R3_HIGH

	;Sexta entrada TSS
	;gs mov dword [si+40],	TSS_SEGMENT_DESCRIPTOR_LOW
	;gs mov dword [si+44],	TSS_SEGMENT_DESCRIPTOR_HIGH
	
	;Cargamos
	db 066h
	lgdt [GDT]
	ret
