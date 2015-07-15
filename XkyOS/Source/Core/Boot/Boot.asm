;;
;; Boot.asm -- OS Boot Loader
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

%define BOOTING

;Jump to BOOT CODE
jmp boot

;;;;;;;;;;;;;;;;;;;;;;;;INCLUDES;;;;;;;;;;;;;;;;;;;;;;;;
%include "Types.inc"
%include "System.inc"
%include "Parameters.inc"
%include "Disk.inc"
%include "Disk.asm"

;;;;;;;;;;;;;;;;;;;;;;;FUNCTIONS;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;PrintMessage;;;;;;;;;;;;;;;;;;;;;;;
;;Input  --> SI = Pointer to buffer
PrintMessage:
	lodsb
	or al, al
	jz .done
	mov ah, 0x0E
	mov bx, 7
	int 0x10
	jmp PrintMessage
.done:
	ret

;;;;;;;;;;;;;;;;;;;;;;;;DATA;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ErrorBootingMsg: db '[XKYBOOT] Error Booting',0

;;;;;;;;;;;;;;;;;;;;;;;;;CODE;;;;;;;;;;;;;;;;;;;;;;;;;;;
boot:
	;Colocamos los descriptores de memoria
	mov ax, BOOT_SEGMENT
	mov ds, ax
	mov es, ax
	mov ax, LP_DIRECTION_SEGMENT
	mov fs, ax
	
	;Y la pila
	mov ax, BOOT_STACK_SEGMENT
	mov ss, ax
	mov sp, BOOT_STACK_OFFSET
	
	;Guardamos de donde arrancamos
	fs mov [LP_DISK + DISK_LOADER_DATA.boot_drive], edx
	
	;Vemos si arrancamos de disquete o de disco duro
	test dl, 0x80
	jz near .continue_boot

.hard_disk_boot:
	;Miramos si el HD soporta las extensiones
	mov ah, 0x41
	mov bx, 0x55AA
	int 0x13
	jc near .error_booting

.continue_boot:
	;Leemos la XFT
 	push XFT_SECTORS		;Cuantos sectores
	push LP_XFT			;Donde lo dejamos
	push LP_DIRECTION_SEGMENT	;Donde lo dejamos
	push XFT_LBA			;De donde leemos
	call ReadFromStorage
	test ax, ax
	jz near .error_booting
	
	;Ponemos un puntero al primer elemento de la XFT
	mov bx, LP_XFT

	;Leemos el LBA del fichero
	fs push dword [bx + XFS_ENTRY.direction]
	call TO_LBA
	mov edx, eax

	;Leemos el tamaño del fichero y lo traducimos a sectores
	fs push dword [bx + XFS_ENTRY.size]
	call TO_SECTORS
	
	;Leemos el Loader
	push ax
	push LDR_DIRECTION_OFFSET
	push LDR_DIRECTION_SEGMENT
	push dx
	call ReadFromStorage
	test ax, ax
	jz near .error_booting

	;Y saltamos
	jmp LDR_DIRECTION_SEGMENT:LDR_DIRECTION_OFFSET

	;En caso de error...
.error_booting:
	mov si, ErrorBootingMsg
	call PrintMessage
.end: jmp .end

;Bootable Sector Sign
times 510-($-$$) db 0
dw 0xAA55
