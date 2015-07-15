;;
;; Modules.asm -- Real Mode Module Routines
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

;;;;;;;;;;;;;;;;;;;;;ToFlatPointer:;;;;;;;;;;;;;;;;;;;;;
ToFlatPointer:
	push bp
	mov bp, sp
	
	xor eax, eax
	;eax=SEGMENT
	mov ax, [bp+4]
	;eax=SEGMENT*16
	shl eax, 4
	;eax=SEGMENT*16 + OFFSET
	add ax, [bp+6]

	pop bp
	ret 4

;;;;;;;;;;;;;;;;;;;ToSegmentedPointer:;;;;;;;;;;;;;;;;;;
ToSegmentedPointer:
	push bp
	mov bp, sp
	
	;eax = Direccion lineal
	mov eax, dword [bp+4]
	;eax = Segmento : 0000
	mov ax, 0
	shl eax, 12
	;eax = Segment : Offset
	mov ax, [bp+4]

	pop bp
	ret 4

;;;;;;;;;;;;;;;;;;;;LoadKernelModules;;;;;;;;;;;;;;;;;;;
LoadKernelModules:
	;Ponemos un puntero al segundo elemento de la XFT (KRNL)
	mov bx, LP_XFT
	add bx, XFS_ENTRY_size

	;Leemos el directorio KRNL
 	push XFS_DIRECTORY_SECTORS		;Cuantos sectores
	push LP_XFT_KRNL_DIRECTORY		;Donde lo dejamos
	push LP_DIRECTION_SEGMENT		;Donde lo dejamos
	fs push dword [bx + XFS_ENTRY.direction];De donde leemos
	call TO_LBA
	push ax
	call ReadFromStorage
	test ax, ax
	jz near .error_loading_modules
	
	;Ponemos un puntero en la tabla del kernel
	mov si, LP_XFT_KRNL_DIRECTORY
	
	;Ponemos un puntero en la direccion de carga
	mov di, KERNEL_START_ADDRESS_SEGMENT
	mov gs, di
	mov di, KERNEL_START_ADDRESS_OFFSET
	
	;Carga de modulos
.load_module:
	;Miramos que la entrada sea valida (used!=0)
	fs cmp byte [si + XFS_ENTRY.used], 0
	jz .end_loading_modules
	;Miramos que la entrada no sea un directory (is_directory!=1)
	fs cmp byte [si + XFS_ENTRY.is_directory], 1
	jz .next_module
	
	;Cargamos el modulo
	call LoadKernelModule
	test eax, eax
	jz .error_loading_modules

	;Actualizamos la direccion de carga gs:di
	fs mov eax, dword [si + XFS_ENTRY.direction]	;eax = Direccion Lineal
	fs add eax, dword [si + XFS_ENTRY.size]	;eax = Direccion lineal + Size

	mov bx, ax
	and ax, 0xF000	;eax = Direccion final alineada a pagina
	and bx, 0x0FFF	;ebx = bytes de exceso en la ultima pagina
	test bx, bx	;Si hay bytes hay que sumar una pagina a la direccion de fin
	jz .lineal_to_segmented
	add eax, PAGE_SIZE	;Sumamos una pagina
.lineal_to_segmented:
	push eax
	call ToSegmentedPointer	;COnvertimos la direccion lineal a segmentada
	mov di, ax	;Actualizamos DI
	shr eax, 16
	mov gs, ax	;Actualizamos GS

.next_module:
	;Actualizamos la entrada en la tabla de ficheros del kernel
	add si, XFS_ENTRY_size
	
	;Cargamos el siguiente
	jmp .load_module

	;Se acabo
.end_loading_modules:
	mov eax, 1
	ret

.error_loading_modules:
	xor eax, eax
	ret

;;;;;;;;;;;;;;;;;;;;LoadKernelModule;;;;;;;;;;;;;;;;;;;;
;%define MAX_SECTORS_PER_SEGMENT	0x00000080
%define MAX_SECTORS_PER_SEGMENT	0x00000048

LoadKernelModule:
	;Parametros:
	;	fs:si -> ptr to XFS_ENTRY
	;	gs:di -> ptr to memory
	pushad
	
	;Leemos el tamaño del fichero
	fs push dword [si + XFS_ENTRY.size]
	call TO_SECTORS
	mov ecx, eax
	
	;Chequear si tenemos que alinear el segmento
	push edi
	call TO_SECTORS
	mov edx, MAX_SECTORS_PER_SEGMENT
	sub edx, eax
	cmp ecx, edx
	jna .continue_loading_module

	;Alineamos
	mov di, gs
	add di, PAGE_SIZE
	mov gs, di
	xor di, di
	
.continue_loading_module:
	;Leemos la direccion del fichero
	fs push dword [si + XFS_ENTRY.direction]
	call TO_LBA
	mov ebx, eax

	;Guardamos la direccion virtual
	push di
	push gs
	call ToFlatPointer
	fs mov dword [si + XFS_ENTRY.direction], eax

	;Guardamos en edx el numero de sectores para ir decrementando
	mov edx, ecx	
	jmp .read_file

.next_reading_iteration:	
	;Actualizamos el puntero
	;mov di, gs
	;add di, PAGE_SIZE
	;mov gs, di
	;xor di, di
	mov di, gs
	add di, 0x900
	mov gs, di
	xor di, di
	
	;Hay que leerlo en trozos de 64KB (128 sectores)
.read_file:
	;Ajustamos a 64KB (128 sectores)
	cmp edx, MAX_SECTORS_PER_SEGMENT
	jle .do_read
	mov edx, MAX_SECTORS_PER_SEGMENT
.do_read:	
	push dx		;Cuantos sectores
	push di		;Donde lo dejamos
	push gs		;Donde lo dejamos
	push bx		;De donde leemos
	call ReadFromStorage
	test ax, ax
	jz near .error_loading_module

	add ebx, edx
	sub ecx, edx
	mov edx, ecx
	jnz .next_reading_iteration
	
	;Todo OK
	popad
	mov eax, 1
	ret

.error_loading_module:
	popad
	xor eax, eax
	ret
