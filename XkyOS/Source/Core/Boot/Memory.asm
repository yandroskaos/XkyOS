;;
;; Memory.asm -- Real Mode Memory Detection Routines
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

;;;;;;;;;;;;;;;;;;;;;;WalkMemory;;;;;;;;;;;;;;;;;;;;;;;;
WalkMemory:
	;Ponemos a 0 el numero de descriptores de memoria
	fs mov dword [LP_MEMORY + MEMORY_LOADER_DATA.descriptors_number], 0

	;Recorremos los descriptores
	push es
	mov di, LP_DIRECTION_SEGMENT
	mov es, di
	mov di, LP_MEMORY + MEMORY_LOADER_DATA_size	;Direccion del descriptor en es:di
	mov ebx, 0	;Continuacion
	jmp .start_walking

.walk_memory:
	add di, MEMORY_DESCRIPTOR_size
	fs inc dword [LP_MEMORY + MEMORY_LOADER_DATA.descriptors_number]

.start_walking:
	mov ax, 0xE820			;Servicio "Query System Address Map"
	mov cx, MEMORY_DESCRIPTOR_size	;Tamaño del buffer
	mov edx, 'PAMS'			;Comprobacion
	int 15h
	jc .error_walking_memory
	cmp eax, 'PAMS'
	jnz .error_walking_memory
	cmp ebx, 0
	jnz .walk_memory
	
.end_walking_memory:
	pop es
	mov eax, 1
	ret
	
.error_walking_memory:
	pop es
	xor eax, eax
	ret

;;;;;;;;;;;;;;;;;DetectAvailableMemory;;;;;;;;;;;;;;;;;;
DetectAvailableMemory:
	pushad

	;Preguntamos por el tamanyo de la memoria
	xor eax, eax
	xor ebx, ebx
	mov ax, 0xE801
	int 15h
	jc .error_detecting_memory
	
	shr ax, 10	;KB to MB
	shr bx, 4	;64KB to MB
	add eax, ebx
	inc eax		;Add first MB

	;En eax el tamaño en megas de la memoria
	fs mov [LP_MEMORY + MEMORY_LOADER_DATA.size_in_megas], eax
	
	call WalkMemory
	test ax, ax
	jz .error_detecting_memory

	popad
	mov eax, 1
	ret
	
.error_detecting_memory:
	popad
	xor eax, eax
	ret	


;;;;;;;;;;;;;;;;;;;;;EnableA20Gate;;;;;;;;;;;;;;;;;;;;;;
;;Comments:
;;	puerto 60h - data
;;	puerto 64h - command/status
CommandWait:
	xor ax, ax
	in al, 0x64
	bt ax, 1
	jc CommandWait
	ret

DataWait:
	xor ax, ax
	in al, 64h
	bt ax, 0
	jnc DataWait
	ret
	
EnableA20Gate:
	;;;;;;;;;;;;;;;;;;;;
	;;Leemos el status;;
	;;;;;;;;;;;;;;;;;;;;
	;Esperamos a que el controlador pueda recibir comandos
	call CommandWait
	
	;Mandamos el comando "leer status"
	mov al, 0xD0
	out 0x64, al

	;Esperamos a por el controlador
	call DataWait

	;Leemos el status 0x60
	xor ax, ax
	in al, 0x60

	;Lo guardamos
	push ax

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;Escribimos el status + A20;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;Esperamos a que el controlador pueda recibir comandos
	call CommandWait

	;Mandamos el comando "escribir status"
	mov al, 0xD1
	out 0x64, al

	;Esperamos a por el controlador
	call CommandWait

	;Escribimos el nuevo valor de status activando la linea A20
	pop ax
	or al, 0x02
	out 0x60, al

	;;;;;;;;;;;;;;;
	;;Comprobamos;;
	;;;;;;;;;;;;;;;
	;Esperamos a por el controlador
	call CommandWait

	;Mandamos el comando "leer status"
	mov al, 0xD0
	out 0x64, al

	;Esperamos a por el controlador
	call DataWait

	;Leemos el status 0x60
	xor ax, ax
	in al, 0x60
	
	;Comprobamos que este activada la linea A20
	bt ax, 1
	jc .success

	;No esta activada	
	xor eax, eax
	ret

	;Esta activada
.success:
	mov eax, 1
	ret
