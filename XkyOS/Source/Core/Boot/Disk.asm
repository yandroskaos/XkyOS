;;
;; Disk.asm -- Real Mode Disk Routines
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

TO_SECTORS: ;dword TO_SECTORS(dword size)
	push bp
	mov bp, sp
	mov eax, dword [bp+4]
	shr eax, 9			;Depende del valor de SECTOR_SIZE
	and dword [bp+4], 0x000001FF	;Depende del valor de SECTOR_SIZE
	jz .end
	inc eax
.end:
	pop bp
	ret 4

TO_OFFSET: ;dword TO_OFFSET(dword lba)
	push bp
	mov bp, sp
	mov eax, dword [bp+4]
	shl eax, 9			;Depende del valor de SECTOR_SIZE
	pop bp
	ret 4

TO_LBA: ;dword TO_LBA(dword offset)
	push bp
	mov bp, sp
	mov eax, dword [bp+4]
	shr eax, 9			;Depende del valor de SECTOR_SIZE
	pop bp
	ret 4

;;;;;;;;;;;;;;;;;;;;;;;;LBA2CHS4FLP;;;;;;;;;;;;;;;;;;;;;
;;Input  --> STACK = LBA direction (dword)
;;Output --> EAX   = 00 CC HH SS
;;Comments:
;;    cylinder = LBA / (heads_per_cylinder * sectors_per_track)
;;        temp = LBA % (heads_per_cylinder * sectors_per_track)
;;        head = temp / sectors_per_track
;;      sector = temp % sectors_per_track + 1
%define FLOPPY_SECTORS_PER_TRACK	0x00000012
%define FLOPPY_HEADS_PER_CYLINDER	0x00000002
LBA2CHS4FLP: ;dword LBA2CHS4FLP(dword lba)
	mov eax, [esp+2]
	push esi
	mov esi, FLOPPY_SECTORS_PER_TRACK
	push ecx
	mov ecx, esi
	imul ecx, FLOPPY_HEADS_PER_CYLINDER
	push edx
	xor edx, edx
	div ecx
	push edi
	mov ecx, eax
	mov eax, edx
	xor edx, edx
	div esi
	mov edi, ecx
	shl edi, 8
	mov esi, eax
	and esi, 0FFh
	inc edx
	or esi, edi
	mov edi, edx
	shl esi, 8
	and edi, 0FFh
	or esi, edi
	pop edi
	mov eax, esi
	pop edx
	pop ecx
	pop esi
	ret 4

%ifndef BOOTING
;;;;;;;;;;;;;;;;;;;;;ReadFromStorage;;;;;;;;;;;;;;;;;;;;
ReadFromStorage: ;dword ReadFromStorage(word lba, word segment, word offset, word how_many)
	push bp
	mov bp, sp
	
	push bx
	push cx
	push dx

	mov cx, word [bp+10]	;HowMany
	jcxz .end
	
	mov bx, word [bp+4]	;LBA
	mov dx, word [bp+8]	;Offset
		
.read_bucle:
	push word 1		;HowMany
	push dx			;Offset
	push word [bp+6]	;Segment
	push bx			;LBA
	call ReadFromStorageSector
	
	add dx, SECTOR_SIZE
	inc bx
	dec cx
	jnz .read_bucle
	
.end:
	pop dx
	pop cx
	pop bx
	
	pop bp
	ret 8
%endif

;;;;;;;;;;;;;;;;;;;;;ReadFromStorageSector;;;;;;;;;;;;;;;;;;;;
%ifdef BOOTING
ReadFromStorage:	;dword ReadFromStorage(word lba, word segment, word offset, word how_many)
%else
ReadFromStorageSector:	;dword ReadFromStorageSector(word lba, word segment, word offset, word how_many)
%endif
	fs cmp byte [LP_DISK + DISK_LOADER_DATA.boot_drive], 0x80
	jz .read_from_hd
	jmp ReadFromFloppy
.read_from_hd:
	jmp ReadFromHD

;;;;;;;;;;;;;;;;;;;;;;;;ReadFromHD;;;;;;;;;;;;;;;;;;;;;;
HDPacket:	db 0x10		;Tamanyo del paquete
		db 0x00		;Reservado
HDSectors:	db 0x00		;Numero de sectores
		db 0x00		;Reservado
HDOffset:	dw 0x0000	;Buffer Offset
HDSegment:	dw 0x0000	;Buffer Segment
HDLBA:		dd 0x00000000	;LBA_Low
		dd 0x00000000	;LBA_High

ReadFromHD: ;dword ReadFromHD(word lba, word segment, word offset, word how_many)
	push bp
	mov bp, sp

	;Salvamos
	push si
	push dx
	
	;Hacemos "setup" del paquete
	xor eax, eax
		;LBA
	mov ax, [bp+4]
	mov [HDLBA], eax
		;Segment
	mov ax, [bp+6]
	mov [HDSegment], ax
		;Offset
	mov ax, [bp+8]
	mov [HDOffset], ax
		;Sectors
	mov ax, [bp+10]
	mov [HDSectors], al
		
	;Leemos
	mov ah, 0x42
	mov si, HDPacket
	mov dl, 0x80
	int 0x13
	jc near .error_reading_HD

	;Restauramos
	mov eax, 1
	pop dx
	pop si
	pop bp
	ret 8

.error_reading_HD:
	;Restauramos
	xor eax, eax
	pop dx
	pop si
	pop bp
	ret 8

;;;;;;;;;;;;;;;;;;;;;;ReadFromFloppy;;;;;;;;;;;;;;;;;;;;
ReadFromFloppy: ;dword ReadFromFloppy(word lba, word segment, word offset, word how_many)
	push bp
	mov bp, sp
	
	;Salvamos
	push bx
	push cx
	push dx
	push es
	
	;LBA
	xor eax, eax
	mov ax, [bp+4]
	push eax
	call LBA2CHS4FLP
	
	mov cl, al	;Sector
	mov dh, ah	;Cabeza (Head)
	shr eax, 8	
	mov ch, ah	;Cilindro
	
	;Sectors
	mov ax, [bp+10]
	
	;Segment
	mov bx, [bp+6]
	mov es, bx
	
	;Offset
	mov bx, [bp+8]
	
	;Leemos de disco
	mov ah, 2	;Servicio "leer sectores de disco"
	mov dl, 0	;Unidad
	int 0x13
	jc near .error_reading_floppy

	;Restauramos
	pop es
	pop dx
	pop cx
	pop bx
	
	mov eax, 1
	pop bp
	ret 8
	
.error_reading_floppy:
	;Restauramos
	pop es
	pop dx
	pop cx
	pop bx

	xor eax, eax
	pop bp
	ret 8
