;;
;; Loader.asm -- XkyOS Second Stage Loader
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

;Jump to LOADER
jmp loader

;;;;;;;;;;;;;;;;;;;;;;;;INCLUDES;;;;;;;;;;;;;;;;;;;;;;;;
%include "Types.inc"
%include "System.inc"
%include "Parameters.inc"
%include "Memory.inc"
%include "Disk.inc"
%include "Image.inc"
%include "Disk.asm"
%include "Memory.asm"
%include "Modules.asm"
%include "PM.asm"
%include "Screen.asm"
%include "SVGA.asm"
%include "Floppy.asm"

;;;;;;;;;;;;;;;;;;;;;;;;DATA;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ErrorCreatingRAMDiskMsg:	db '[XKYLDR] Error creating floppy RAM disk',13,10,0
ErrorLoadingKernelModulesMsg:	db '[XKYLDR] Error loading kernel modules',13,10,0
ErrorEnablingA20Msg:		db '[XKYLDR] Error enabling A20 line',13,10,0
ErrorNoMemoryFoundMsg:		db '[XKYLDR] Error no memory found',13,10,0
SVGAEnabled: dd 0

;;;;;;;;;;;;;;;;;;;;;;;;;CODE;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;Error
error_creating_ram_disk:
	push ErrorCreatingRAMDiskMsg
	call PrintLine
	cli
	jmp end_loading
error_loading_kernel_modules:
	push ErrorLoadingKernelModulesMsg
	call PrintLine
	cli
	jmp end_loading
error_enabling_a20_line:
	push ErrorEnablingA20Msg
	call PrintLine
	cli
	jmp end_loading
error_no_memory:
	push ErrorNoMemoryFoundMsg
	call PrintLine
	cli
end_loading: jmp end_loading

loader:
	;Cargamos los descriptores
	mov ax, LDR_DIRECTION_SEGMENT
	mov ds, ax
	mov es, ax

	;Create RAM Disk si arrancamos de floppy
	call CreateRAMDisk
	test eax, eax
	jz near error_creating_ram_disk
	
	;Cargamos los modulos del nucleo en direcciones prefijadas (<1MB)
	call LoadKernelModules
	test eax, eax
	jz near error_loading_kernel_modules

	;Desactivamos las interrupciones
	cli

	;Activamos la linea A20 para obtener acceso a todo el espacio de direcciones
	call EnableA20Gate
	test eax, eax
	jz near error_enabling_a20_line

	;Check Memory
	call DetectAvailableMemory
	test eax, eax
	jz near error_no_memory

	;Podemos verificar el tamaño de la memoria y declinar arrancar si tenemos menos de un minimo

	;Check VGA
	call DetectSVGA
	mov dword [SVGAEnabled], eax
	
	;Reajustamos el salto para serializar la entrada en modo protegido
	add dword [protected_mode_jump_offset], LDR_DIRECTION_FLAT
	
	;Montamos los datos del entorno protegido (GDT)
	call SetGDT
	call SetIDT	;IDT

	;Activamos el modo protegido
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	
	;Limpiar la cola de prefetch
	db 0x67
	db 0x66
	db 0xEA			;far jump opcode
protected_mode_jump_offset:
	dd prefetch_queue_clean	;Code Offset
	dw CODE_R0_SELECTOR	;Code Selector
prefetch_queue_clean:
	;En este mismo momento estamos en 32 bits
[BITS 32]
	;Cargar los selectores de la GDT
	mov ax, DATA_R0_SELECTOR
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	;Finalizamos la inicializacion de las estructuras del procesador para modo protegido
	call SetTSS	;TSS
	call SetLDT	;LDT (Invalid)

	;En este momento tenemos los modulos de kernel cargados
	;y un modelo flat de memoria sin paginacion

	;Limpiamos la pantalla
	_call PrintScreen, 0x00000000
	
	;Reubicar los modulos
	call ReubicateModules
	
	;Resolver sus imports
	call ResolveImports
	test eax, eax
	jz error_resolving_imports

	;Escribir la direccion del nucleo
	mov eax, LP_DIRECTION_FLAT
	add eax, LP_XFT_KRNL_DIRECTORY	;eax = XFS_ENTRY de PARENT
	add eax, XFS_ENTRY_size		;eax = XFS_ENTRY de Kernel.x
	mov eax, [eax + XFS_ENTRY.direction]		;eax = comienzo del modulo
	mov eax, dword [eax + XKY_MODULE_HEADER.file_header + XKY_FILE_HEADER.code_ep]	;eax = Kernel Code Entry Point
		;Usamos ebx para escribir el entrypoint
	mov ebx, LDR_DIRECTION_FLAT
	add ebx, kernel_entrypoint
	mov [ebx], eax

	;Acabamos de montar el loader
	mov dword [LP_DIRECTION_FLAT + LP_DISK + DISK_LOADER_DATA.xft], LP_DIRECTION_FLAT + LP_XFT
	mov dword [LP_DIRECTION_FLAT + LP_DISK + DISK_LOADER_DATA.krnl_directory], LP_DIRECTION_FLAT + LP_XFT_KRNL_DIRECTORY
	mov dword [LP_DIRECTION_FLAT + LP_DISK + DISK_LOADER_DATA.boot_drive_size], 0
	
	mov dword [LP_LOADER + LOADER_DATA.disk], LP_DIRECTION_FLAT+LP_DISK
	mov dword [LP_LOADER + LOADER_DATA.memory], LP_DIRECTION_FLAT+LP_MEMORY
	mov dword [LP_LOADER + LOADER_DATA.svga], LP_DIRECTION_FLAT+LP_SVGA

	;Montamos la pila en modo protegido
	mov esp, KERNEL_STACK
	
	;Apilamos el parametro y la direccion de vuelta
	push dword LP_LOADER
	push dword 0
	
	;Saltamos al entrypoint del kernel
	db 0EAh			;far jump opcode
kernel_entrypoint:	dd 0h	;Code Offset
	dw CODE_R0_SELECTOR	;Code Selector

	;Error
error_resolving_imports:
	call ImportsError
	cli
loader_stop: jmp loader_stop

;;;;;;;;;;;;;;;;;;;;INCLUDES_FLAT;;;;;;;;;;;;;;;;;;;;;;;
%include "Flat.asm"

;;;;;;;;;;;;;;;;;;;;ImportsError;;;;;;;;;;;;;;;;;;;;;;;;
ImportsError:
	_if e, {cmp dword [SVGAEnabled], 1}
		_call PrintScreen, 0x00FF0000
		ret
	_else
		mov byte [0x000B8000], 'E'
		mov byte [0x000B8002], 'r'
		mov byte [0x000B8004], 'r'
		mov byte [0x000B8006], 'o'
		mov byte [0x000B8008], 'r'
		mov byte [0x000B800A], ' '
		mov byte [0x000B800C], 'c'
		mov byte [0x000B800E], 'a'
		mov byte [0x000B8010], 'l'
		mov byte [0x000B8012], 'c'
		mov byte [0x000B8014], 'u'
		mov byte [0x000B8016], 'l'
		mov byte [0x000B8018], 'a'
		mov byte [0x000B801A], 't'
		mov byte [0x000B801C], 'i'
		mov byte [0x000B801E], 'n'
		mov byte [0x000B8020], 'g'
		mov byte [0x000B8022], ' '
		mov byte [0x000B8024], 'I'
		mov byte [0x000B8026], 'm'
		mov byte [0x000B8028], 'p'
		mov byte [0x000B802A], 'o'
		mov byte [0x000B802C], 'r'
		mov byte [0x000B802E], 't'
		mov byte [0x000B8030], 's'
		mov byte [0x000B8001], 0x1C
		mov byte [0x000B8003], 0x1C
		mov byte [0x000B8005], 0x1C
		mov byte [0x000B8007], 0x1C
		mov byte [0x000B8009], 0x1C
		mov byte [0x000B800B], 0x1C
		mov byte [0x000B800D], 0x1C
		mov byte [0x000B800F], 0x1C
		mov byte [0x000B8011], 0x1C
		mov byte [0x000B8013], 0x1C
		mov byte [0x000B8015], 0x1C
		mov byte [0x000B8017], 0x1C
		mov byte [0x000B8019], 0x1C
		mov byte [0x000B801B], 0x1C
		mov byte [0x000B801D], 0x1C
		mov byte [0x000B801F], 0x1C
		mov byte [0x000B8021], 0x1C
		mov byte [0x000B8023], 0x1C
		mov byte [0x000B8025], 0x1C
		mov byte [0x000B8027], 0x1C
		mov byte [0x000B8029], 0x1C
		mov byte [0x000B802B], 0x1C
		mov byte [0x000B802D], 0x1C
		mov byte [0x000B802F], 0x1C
		mov byte [0x000B8031], 0x1C
	_end
	ret
