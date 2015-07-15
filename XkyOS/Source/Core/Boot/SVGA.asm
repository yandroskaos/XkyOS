;;
;; SVGA.asm -- Real Mode SVGA Init Routines
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VbeInfoBlock:
	db 'VBE2'
	times 508 db 0

%define VESA_TAG			'VESA'
%define VESA_VERSION			0x0200
%define VESA_TAG_OFFSET			0
%define VESA_VERSION_OFFSET		4
%define VESA_MODE_PTR_OFFSET_OFFSET	14
%define VESA_MODE_PTR_SEGMENT_OFFSET	16
%define VESA_MODE_SIZE			2


ModeInfoBlock:
	times 256 db 0

%define MODE_ATTRIBUTES_OFFSET	0x00
%define X_RES_OFFSET		0x12
%define Y_RES_OFFSET		0x14
%define BITS_PER_PIXEL_OFFSET	0x19
%define MEMORY_MODEL_OFFSET	0x1B
%define PHYSICAL_MEMORY_OFFSET	0x28

%define MODE_ATTRIBUTES_MASK	0x0098
%define X_RES			0x0400
%define Y_RES			0x0300
%define BITS_PER_PIXEL		0x20
%define MEMORY_MODEL		0x06

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DetectSVGA: ;bool DetectSVGA()
	pushad

	;Get Info Block
	mov ax, 0x4F00
	mov di, VbeInfoBlock
	int 0x10
	cmp ax, 0x004F
	jnz near .error
	
	;Check VBE 2.0
	mov eax, dword [VbeInfoBlock + VESA_TAG_OFFSET]
	cmp eax, VESA_TAG
	jnz near .error

	mov ax, word [VbeInfoBlock + VESA_VERSION_OFFSET]
	cmp ax, VESA_VERSION
	jnz near .error
	
	;Mostrar la lista de modos
	mov si, [VbeInfoBlock + VESA_MODE_PTR_OFFSET_OFFSET]
	mov gs, [VbeInfoBlock + VESA_MODE_PTR_SEGMENT_OFFSET]
	
	gs mov cx, word [si]
.mode_bucle:
	cmp cx, 0xFFFF
	jz near .error

	;Pedir las caracteristicas del modo	
	mov di, ModeInfoBlock
	mov ax, 0x4F01
	int 0x10
	cmp ax, 0x004F
	jnz .next_mode
	
	;Testeamos las caracteristicas... buscamos 1024x768x32 lineal true_color
	mov ax, word [di + MODE_ATTRIBUTES_OFFSET]
	and ax, MODE_ATTRIBUTES_MASK
	cmp ax, MODE_ATTRIBUTES_MASK
	jnz .next_mode
	
	mov ax, word [di + X_RES_OFFSET]
	cmp ax, X_RES
	jnz .next_mode

	mov ax, word [di + Y_RES_OFFSET]
	cmp ax, Y_RES
	jnz .next_mode
	
	mov al, byte [di + BITS_PER_PIXEL_OFFSET]
	cmp al, BITS_PER_PIXEL
	jnz .next_mode
	
	mov al, byte [di + MEMORY_MODEL_OFFSET]
	cmp al, MEMORY_MODEL
	jnz .next_mode

	;Cambiamos de modo
	mov ax, 0x4F02
	mov bx, 0x4000
	or bx, cx
	int 0x10
	cmp ax, 0x004F
	jnz .next_mode

	;Guardar la direccion del framebuffer
	mov eax, dword [di + PHYSICAL_MEMORY_OFFSET]
	fs mov dword [LP_SVGA + SVGA_LOADER_DATA.framebuffer], eax
	fs mov dword [LP_SVGA + SVGA_LOADER_DATA.x_resolution], X_RES
	fs mov dword [LP_SVGA + SVGA_LOADER_DATA.y_resolution], Y_RES

	;Fin con exito
	jmp .success
	
.next_mode:	
	add si, VESA_MODE_SIZE
	gs mov cx, word [si]
	jmp .mode_bucle

.error:
	popad
	xor eax, eax
	ret

.success:
	popad
	mov eax, 1
	ret
