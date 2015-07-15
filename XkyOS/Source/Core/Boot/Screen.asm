;;
;; Screen.asm -- Real Mode Screen Routines
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

;;;;;;;;;;;;;;;;;;;;;;;;;NewLine;;;;;;;;;;;;;;;;;;;;;;;;;;
NewLine: db 13,10,0

;;;;;;;;;;;;;;;;;;;;;;;;;Print;;;;;;;;;;;;;;;;;;;;;;;;;;
Print:
	push bp
	mov bp, sp
	
	push si
	push ax
	push bx
	
	mov si, [bp+4]
.message:
	lodsb
	or al, al
	jz .done
	mov ah, 0x0E
	mov bx, 7
	int 0x10
	jmp .message

.done:
	pop bx
	pop ax
	pop si
	pop bp
	ret 2

;;;;;;;;;;;;;;;;;;;;;PrintNewLine;;;;;;;;;;;;;;;;;;;;;;;
PrintNewLine:
	push NewLine
	call Print
	ret

;;;;;;;;;;;;;;;;;;;;;;;PrintLine;;;;;;;;;;;;;;;;;;;;;;;;
PrintLine: ;void PrintLine(word string)
	push bp
	mov bp, sp
	
	push word [bp+4]
	call Print
	push NewLine
	call Print
	
	pop bp
	ret 2


;;;;;;;;;;;;;;;;;;;;;;;Format16;;;;;;;;;;;;;;;;;;;;;;;;;
Format16: ;void Format16(word buffer, word data)
	push bp
	mov bp, sp
	pusha
	
	mov ax, [bp+6]
	mov bx, [bp+4]
	mov byte [bx+4], 0
	mov si, 0x0030
	mov di, 0x0037

	mov cx, ax
	and cx, 0x000F
	cmp cx, 9
	cmovbe dx, si
	cmova dx, di
	add dx, cx
	mov [bx+3], dl

	mov cx, ax
	and cx, 0x00F0
	shr cx, 4
	cmp cx, 9
	cmovbe dx, si
	cmova dx, di
	add dx, cx
	mov [bx+2], dl

	mov cx, ax
	and cx, 0x0F00
	shr cx, 8
	cmp cx, 9
	cmovbe dx, si
	cmova dx, di
	add dx, cx
	mov [bx+1], dl

	mov cx, ax
	and cx, 0xF000
	shr cx, 12
	cmp cx, 9
	cmovbe dx, si
	cmova dx, di
	add dx, cx
	mov [bx+0], dl

	popa
	pop bp
	ret 4

;;;;;;;;;;;;;;;;;;;;;;;Format32;;;;;;;;;;;;;;;;;;;;;;;;;
Format32: ;void Format32(word buffer, dword data)
	push bp
	mov bp, sp
	
	push word [bp+8]
	push word [bp+4]
	call Format16
	
	push word [bp+6]
	add word [bp+4], 4
	push word [bp+4]
	call Format16
	
	pop bp
	ret 6


;;;;;;;;;;;;;;;;;;;;;;;Print16;;;;;;;;;;;;;;;;;;;;;;;;;;
Data16: times 5 db 0
Print16: ;void Print16(word data)
	push bp
	mov bp, sp
	
	push word [bp+4]
	push Data16
	call Format16
	
	push Data16
	call Print
	
	pop bp
	ret 2

;;;;;;;;;;;;;;;;;;;;;;Print32;;;;;;;;;;;;;;;;;;;;;;;;;;;
Data32: times 9 db 0
Print32: ;void Print32(dword data)
	push bp
	mov bp, sp
	
	push dword [bp+4]
	push Data32
	call Format32
	
	push Data32
	call Print
	
	pop bp
	ret 4
