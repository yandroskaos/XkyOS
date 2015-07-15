;;
;; Types.inc -- Basic types and language support
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

%ifndef __TYPES_INCLUDES__
%define __TYPES_INCLUDES__

; =============================================================
; LANGUAGE SUPPORT
; =============================================================
;Procedure ease-of-use support
%assign pusheax 0
%assign pushebx 0
%assign pushecx 0
%assign pushedx 0
%assign pushesi 0
%assign pushedi 0
%assign pushebp 0
%assign pushesp 0

%macro _proc 1-2.nolist 0
	%push proc_ctx
	align 4
%1:
	%stacksize flat
	%assign %$localsize 0
	%assign %$argsize %2
	%assign %$prologue 0
	
	%assign pusheax 0
	%assign pushebx 0
	%assign pushecx 0
	%assign pushedx 0
	%assign pushesi 0
	%assign pushedi 0
	%assign pushebp 0
	%assign pushesp 0
%endmacro	

%macro _enter 0-*.nolist
	%ifctx proc_ctx
		%assign %$prologue 1
		push ebp
		mov ebp, esp
		%if %$localsize!=0
			sub esp, %$localsize
		%endif

		%rep %0
			%ifidn %1, eax
				%assign pusheax 1
			%elifidn %1, ebx
				%assign pushebx 1
			%elifidn %1, ecx
				%assign pushecx 1
			%elifidn %1, edx
				%assign pushedx 1
			%elifidn %1, esi
				%assign pushesi 1
			%elifidn %1, edi
				%assign pushedi 1
			%elifidn %1, ebp
				%assign pushebp 1
			%elifidn %1, esp
				%assign pushesp 1
			%else
				%error 'unknown register' %1
			%endif
			%rotate -1
		%endrep
		
		%if pusheax = 1
			push eax
		%endif
		%if pushebx = 1
			push ebx
		%endif
		%if pushecx = 1
			push ecx
		%endif
		%if pushedx = 1
			push edx
		%endif
		%if pushesi = 1
			push esi
		%endif
		%if pushedi = 1
			push edi
		%endif
		%if pushebp = 1
			push ebp
		%endif
		%if pushesp = 1
			push esp
		%endif
	%else
		%error "_enter used out of _proc definition"
	%endif
%endmacro

%macro _return 0-1
	%if %0 = 1
		%ifnidn eax, %1
			mov eax, %1
		%endif
	%endif

	%if pushesp = 1
		pop esp
	%endif
	%if pushebp = 1
		pop ebp
	%endif
	%if pushedi = 1
		pop edi
	%endif
	%if pushesi = 1
		pop esi
	%endif
	%if pushedx = 1
		pop edx
	%endif
	%if pushecx = 1
		pop ecx
	%endif
	%if pushebx = 1
		pop ebx
	%endif
	%if pusheax = 1
		pop eax
	%endif
	%if %$prologue = 1
		%if %$localsize != 0
			add esp, %$localsize
		%endif
		pop ebp
	%endif

	%if (%$argsize) = 0
		ret
	%else
		ret (%$argsize)*4
	%endif
%endmacro

;Language constructs ease-of-use support
%macro _call 1-*.nolist
	%rep %0 - 1
		%rotate -1
		push %1
	%endrep
	%rotate -1
	call %1
%endmacro	

%macro _repeat 0.nolist
	%push repeat_ctx
	%$repeatstart:
%endmacro

%macro _until 2+.nolist
	%ifctx repeat_ctx
		%2
		j%-1 near %$repeatstart
		%pop
	%else
		%error "expected _repeat before _until"
	%endif
%endmacro

%macro _for 4.nolist
	%push for_ctx
	%define %$forcounter %1
	%define %$forincrement %4
	;push %4
	mov %1, %2
	%$forstart:
	cmp %1, %3
	jae near %$forend
%endmacro

%macro _next 0.nolist
	%ifctx for_ctx
		add %$forcounter, %$forincrement
		jmp %$forstart
		%$forend:
		;pop %$forcounter
		%pop
	%else
		%error "expected _for before _next"
	%endif
%endmacro

%macro _while 2+.nolist
	%push while_ctx
	%$whilestart:
	%2
	j%-1 near %$whileend
%endmacro

%macro _if 2+.nolist
	%push if_ctx
	%assign %$ifnot_index 1
	%assign %$old_index %$ifnot_index
	%2
	j%-1 near %$ifnot %+ %$ifnot_index
%endmacro

%macro _elif 2+.nolist
	%ifctx if_ctx
		%repl elif_ctx
		jmp %$ifend
		%$ifnot %+ %$ifnot_index:
		%assign %$ifnot_index %$ifnot_index + 1 
		%2
		j%-1 near %$ifnot %+ %$ifnot_index
	%elifctx elif_ctx
		jmp %$ifend
		%$ifnot %+ %$ifnot_index:
		%assign %$ifnot_index %$ifnot_index + 1 
		%2
		j%-1 near %$ifnot %+ %$ifnot_index
	%else
		%error "expected _if before _elif"
	%endif
%endmacro

%macro _else 0.nolist
	%ifctx if_ctx
		%repl else_ctx
		jmp %$ifend
		%$ifnot %+ %$ifnot_index:
	%elifctx elif_ctx
		%repl else_ctx
		jmp %$ifend
		%$ifnot %+ %$ifnot_index:
	%else
		%error "expected _if before _else"
	%endif
%endmacro

; =============================================================
; TYPES
; =============================================================
%macro _typedef 2
	%define %1 %2
%endmacro

%macro _string 2
	%strlen string_length %2 
	%1:	db string_length
		db %2
%endmacro

%macro _string 1
	%strlen string_length %1 
	db string_length
	db %1
%endmacro

%define dword_size	4
%define word_size	2
%define byte_size	1

%macro _array 1-2 1
	times %1_size * %2 db 0
%endmacro

%macro _index 3 ;type* _index type, var, index = a + type_size*index
	push ebx
	push edx
	mov eax, %3
	mov ebx, %1_size
	mul ebx
	add eax, %2
	pop edx
	pop ebx
%endmacro

%imacro _struct 1-4.nolist 1, 0, struct_ctx
	%ifctx struct_ctx
		%define %%toplevel 0
	%elifctx union_ctx
		%define %%toplevel 0
	%else
		%define %%toplevel 1
	%endif
	%ifidni %1, "-"
		%define %%noname 1
	%else
		%define %%noname 0
	%endif
	%if (%%toplevel == 1)
		%push %4
		%define %$name %1
		%assign %$size %3
		%define %$nameless %%noname
	%else
		%if (%%noname == 1)
			%xdefine %%tname %{$name}
		%else
			%xdefine %%tname %{$name}.%1
		%endif
		%assign %%tsize %$size
		%push %4
		%define %$name %%tname
		%assign %$size %%tsize
		%define %$nameless %%noname
		%if (%%noname == 0)
			%$name equ %$size
		%endif
	%endif
	%assign %$osize %$size
	%assign %$usize 0
	%assign %$times %2
%endmacro

%imacro _union 1-3.nolist 1, 0
	_struct %1, %2, %3, union_ctx
%endmacro

%imacro _struct 0.nolist
	_struct "-", 1, 0, struct_ctx
%endmacro

%imacro _union 0.nolist
	_struct "-", 1, 0, union_ctx
%endmacro

%imacro _byte 1-2.nolist 1
	%{$name}.%1 equ %$size
	%assign %$size %$size + (%2)
	%ifctx union_ctx
		%assign %%tsize %$size-%$osize
		%if (%%tsize > %$usize)
			%assign %$usize %%tsize
		%endif
		%if (%$nameless == 0)
			%{$name}.%{1}_size equ %%tsize
		%endif
		%assign %$size %$osize
	%endif
%endmacro

%imacro _word 1-2.nolist 1
	_byte %1, 2*(%2)
%endmacro

%imacro _dword 1-2.nolist 1
	_byte %1, 4*(%2)
%endmacro

%imacro _qword 1-2.nolist 1
	_byte %1, 8*(%2)
%endmacro

%imacro _space 1.nolist
	%assign %$size %$size + (%1)
%endmacro

%imacro _align 1.nolist
	%assign %$size %$size + (((%$size + %1) / %1) * %1)
%endmacro

%imacro _istruct 1.nolist
	%ifctx istruct_ctx
		%xdefine %%tname %{$name}.%1
		times %%tname-($-%$istructstart) db 0
		%push istruct_ctx
		%define %$name %%tname
		%define %$sname %$name
	%else
		%push istruct_ctx
		%define %$name %1
		%define %$sname 0
	%endif
	%$istructstart:
%endmacro

%idefine _iunion _istruct

%imacro _atoffs 1-2+.nolist
	times (%1)-($-%$istructstart) db 0
	%2
%endmacro

%imacro _at 1-2+.nolist
	_atoffs %{$name}.%1-%$sname, %2
%endmacro

%define _field(type, var, size, name) size [var + type %+ . %+ name]

%macro _set 2 ;(x = y)
	mov eax, %2
	mov %1, eax
%endmacro

;Class and interfaces ease-of-use support
%define _class _struct
%define _iclass _istruct
%define _interface _struct
%macro _object 1
	times %1_size db 0
%endmacro
%define _method _dword
%define _attribute _dword

%macro _methods 1
	%ifctx methods_ctx
		%push methods_ctx
		%xdefine %$classname %$classname %+ _ %+ %1
	%else
		%push methods_ctx
		%xdefine %$classname %1
	%endif
%endmacro

%macro _body 1-2.nolist 0
	align 4
%$classname %+ _ %+ %1:
	%push proc_ctx
	%stacksize flat
	%assign %$localsize 0
	%assign %$argsize %2
	%assign %$prologue 0
	
	%assign pusheax 0
	%assign pushebx 0
	%assign pushecx 0
	%assign pushedx 0
	%assign pushesi 0
	%assign pushedi 0
	%assign pushebp 0
	%assign pushesp 0
%endmacro

%macro _new 2 ;void _new(void* object, type t)
	push ecx
	push esi
	push edi
	mov edi, %1
	mov esi, %2_type
	mov ecx, %2_size
	rep movsb
	mov eax, edi
	pop edi
	pop esi
	pop ecx
%endmacro

%macro _invoke 2-*.nolist
	%rep %0 - 2
		%rotate -1
		push %1
	%endrep
	
	%rotate -1
	%rotate -1
	push %1
	call [%1 + %2]
%endmacro	

%macro _end 0.nolist
	%ifctx proc_ctx
		%pop
	%elifctx methods_ctx
		%pop
	%elifctx if_ctx
		%$ifnot %+ %$ifnot_index:
		%pop
	%elifctx elif_ctx
		%$ifnot %+ %$ifnot_index:	
		%$ifend:
		%pop
	%elifctx else_ctx
		%$ifend:
		%pop
	%elifctx while_ctx
		jmp %$whilestart
		%$whileend:
		%pop
	%elifctx struct_ctx
		%if ((%$size-%$osize) > %$usize)
			%assign %$usize %$size-%$osize
		%endif
		%assign %$usize %$times*%$usize
		%if (%$nameless == 0)
			%{$name}_size equ %$usize
		%endif
		%assign %%tsize %$usize
		%pop
		%ifctx struct_ctx
			%assign %$size %$size+%%tsize
		%elifctx union_ctx
			%assign %$size %$size+%%tsize
			%if ((%$size-%$osize) > %$usize)
				%assign %$usize %$size-%$osize
			%endif
			%assign %$size %$osize
		%endif
	%elifctx union_ctx
		%if ((%$size-%$osize) > %$usize)
			%assign %$usize %$size-%$osize
		%endif
		%assign %$usize %$times*%$usize
		%if (%$nameless == 0)
			%{$name}_size equ %$usize
		%endif
		%assign %%tsize %$usize
		%pop
		%ifctx struct_ctx
			%assign %$size %$size+%%tsize
		%elifctx union_ctx
			%assign %$size %$size+%%tsize
			%if ((%$size-%$osize) > %$usize)
				%assign %$usize %$size-%$osize
			%endif
			%assign %$size %$osize
		%endif
	%elifctx istruct_ctx
		_atoffs %{$name}_size
		%pop
	%elifctx iunion_ctx
		_atoffs %{$name}_size
		%pop
	%else
		%error "_end closing without match"
	%endif
%endmacro

; =============================================================
; FILE ON-DISK FORMAT
; =============================================================
%define XKY_IMAGE_SECTION_CODE	0x00000001
%define XKY_IMAGE_SECTION_DATA	0x00000002
%define XKY_IMAGE_MODE_DYNAMIC	0x00000001
%define XKY_IMAGE_MODE_SYSTEM	0x00000002
%define XKY_IMAGE_KIND_CLASS	0x00000001
%define XKY_IMAGE_KIND_MODULE	0x00000002
%define XKY_IMAGE_VERSION(w,x,y,z) ((w<<24)|(x<<16)|(y<<8)|(z))

%macro _section 2
	db %1	;Name
	dd 0	;Offset
	dd 0	;Size
	dd %2	;Flags
%endmacro

%macro _file 6
	dd 0	;Size
	dd %3	;Version
	dd %2	;Kind
	dd %1	;Mode
	dd %4	;InitEP
	dd %5	;CodeEP
	dd %6	;FinishEP
%endmacro

%macro _module 3-6 0, 0, 0
	dd 'XKYX'
	_file %1, %2, %3, %4, %5, %6
	_section 'imps', XKY_IMAGE_SECTION_DATA
	_section 'data', XKY_IMAGE_SECTION_DATA
	_section 'code', XKY_IMAGE_SECTION_CODE
	_section 'exps', XKY_IMAGE_SECTION_DATA
	_section 'relc', XKY_IMAGE_SECTION_DATA
%endmacro

%macro _import 2
	%$import_start:
	_string %2
	times 64-($-%$import_start) db 0
	%1 :dd 0
%endmacro

%macro _export 2
	%$export_start:
	_string %2
	times 64-($-%$import_start) db 0
	dd %1
%endmacro
%endif ; __TYPES_INCLUDES__


