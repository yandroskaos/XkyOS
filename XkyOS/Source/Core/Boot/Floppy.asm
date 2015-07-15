;;
;; Floppy.inc -- Floppy RAM Disk
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

;;;;;;;;;;;;;;;;;;;;CreateRAMDisk;;;;;;;;;;;;;;;;;;;;
%define READ_SECTORS	0x0040
%define START_SEGMENT	0x4000
%define START_OFFSET	0x0000
%define START_LBA	0x0000
%define NUMBER_OF_READS	8

CreateRAMDisk:
	fs cmp byte [LP_DISK + DISK_LOADER_DATA.boot_drive], 0x80
	jz .done

	pushad

	;Inicializamos
	mov cx, NUMBER_OF_READS	;Lecturas
	mov dx, START_LBA	;LBA
	mov di, START_OFFSET	;Desplazamiento
	mov bx, START_SEGMENT	;Segmento
	
.read_bucle:	
	push READ_SECTORS	;Cuantos sectores
	push di			;Donde lo dejamos
	push bx			;Donde lo dejamos
	push dx			;De donde leemos
	call ReadFromStorage
	test ax, ax
	jz near .error_creating_disk

	add dx, READ_SECTORS	;Siguiente LBA
	add di, 0x8000		;Siguiente desplazamiento
	test cx, 0x0001		;Indice impar?
	jz .continue_bucle
	add bx, 0x1000		;Actualizar segmento
.continue_bucle:
	dec cx			;Decrementar indice
	jnz .read_bucle
	
	;Todo OK
	popad
.done:
	mov eax, 1
	ret

.error_creating_disk:
	popad
	xor eax, eax
	ret

;CreateRAMDisk:
;	pushad
;
;	push 0x0040	;Cuantos sectores
;	push 0x0000	;Donde lo dejamos
;	push 0x4000	;Donde lo dejamos
;	push 0x0000	;De donde leemos
;	call ReadFromStorage
;	test ax, ax
;	jz near .error_loading_module
;
;	push 0x0040	;Cuantos sectores
;	push 0x8000	;Donde lo dejamos
;	push 0x4000	;Donde lo dejamos
;	push 0x0040	;De donde leemos
;	call ReadFromStorage
;	test ax, ax
;	jz near .error_loading_module
;
;	push 0x0040	;Cuantos sectores
;	push 0x0000	;Donde lo dejamos
;	push 0x5000	;Donde lo dejamos
;	push 0x0080	;De donde leemos
;	call ReadFromStorage
;	test ax, ax
;	jz near .error_loading_module
;
;	push 0x0040	;Cuantos sectores
;	push 0x8000	;Donde lo dejamos
;	push 0x5000	;Donde lo dejamos
;	push 0x00C0	;De donde leemos
;	call ReadFromStorage
;	test ax, ax
;	jz near .error_loading_module
;
;	push 0x0040	;Cuantos sectores
;	push 0x0000	;Donde lo dejamos
;	push 0x6000	;Donde lo dejamos
;	push 0x0100	;De donde leemos
;	call ReadFromStorage
;	test ax, ax
;	jz near .error_loading_module
;
;	push 0x0040	;Cuantos sectores
;	push 0x8000	;Donde lo dejamos
;	push 0x6000	;Donde lo dejamos
;	push 0x0140	;De donde leemos
;	call ReadFromStorage
;	test ax, ax
;	jz near .error_loading_module
;
;	push 0x0040	;Cuantos sectores
;	push 0x0000	;Donde lo dejamos
;	push 0x7000	;Donde lo dejamos
;	push 0x0180	;De donde leemos
;	call ReadFromStorage
;	test ax, ax
;	jz near .error_loading_module
;
;	push 0x0040	;Cuantos sectores
;	push 0x8000	;Donde lo dejamos
;	push 0x7000	;Donde lo dejamos
;	push 0x01C0	;De donde leemos
;	call ReadFromStorage
;	test ax, ax
;	jz near .error_loading_module
;	
;	;Todo OK
;	popad
;	mov eax, 1
;	ret
;
;.error_loading_module:
;	popad
;	xor eax, eax
;	ret
