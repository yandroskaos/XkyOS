;;
;; Flat.asm -- Flat Protected Mode Related Routines
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

;;;;;;;;;;;;;;;;;;;;;;;;;SetLDT;;;;;;;;;;;;;;;;;;;;;;;;;
SetLDT:
	mov ax, NULL_SELECTOR
	lldt ax
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;SetTSS;;;;;;;;;;;;;;;;;;;;;;;;;
SetTSS:
	;Ponemos a cero la TSS
	mov edi, TSS_DIRECTION
	xor al, al
	mov cx, TSS_SIZE
	rep stosb
	
	;Escribimos en la TSS los valores para la pila en RING 0
	mov edi, TSS_DIRECTION
		;ESP en R0
	mov dword [edi + TSS_ESP0], KERNEL_STACK
		;SS en R0
	mov word [edi + TSS_SS0], DATA_R0_SELECTOR

	;Escribir en la GDT el descriptor de la TSS
	mov esi, GDT_DIRECTION
		;Sexta entrada TSS
	mov dword [esi+40], TSS_DESCRIPTOR_LOW
	mov dword [esi+44], TSS_DESCRIPTOR_HIGH

	;Cargamos
	mov ax, TSS_SELECTOR
	ltr ax	
	ret

;;;;;;;;;;;;;;;;;;;;;;GetRelocs;;;;;;;;;;;;;;;;;;;;;;;;;
;;Output --> eax=Hay relocs, esi=puntero al comienzo de las relocs, edi=numero de relocs
_proc GetRelocs, 1 ;bool GetRelocs(XFS_ENTRY* module)
	%arg module:dword
	_enter
	
	;Por si no hay imports, restaurar sus valores
	push esi
	push edi
	
	;Descriptor del modulo en la tabla
	mov eax, dword [module]
	
	;Acceder a las relocs del modulo
	mov eax, dword [eax + XFS_ENTRY.direction]
	lea esi, [eax + XKY_MODULE_HEADER.relocs_section]
	mov edi, dword [esi + XKY_SECTION_HEADER.size]
	_if z, {test edi, edi}
		pop edi
		pop esi
		_return 0
	_end
	
	;esi = comienzo relocs
	mov esi, dword [esi + XKY_SECTION_HEADER.offset]
	add esi, eax
	mov edi, dword [esi]
	add esi, 4
	
	
	add esp, 8	;Debido a los valores salvados de edi y esi
	_return 1
_end

;;;;;;;;;;;;;;;;;;ReubicateKernelModule;;;;;;;;;;;;;;;;;
_proc ReubicateKernelModule, 1 ;bool ReubicateKernelModule(XFS_ENTRY* module)
	%arg module:dword
	_enter ebx, ecx, esi, edi

	;Obtenemos los relocs del modulo
	_call GetRelocs, dword [module]
	_if nz, {test eax, eax}
		;eax = direccion base de carga del modulo
		mov eax, dword [module]
		mov eax, [eax + XFS_ENTRY.direction]
		_for ecx, 0, edi, 1
			;ebx = offset que debe ser reubicado
			mov ebx, [esi]
			;reubicamos
			add [eax+ebx], eax
			;next
			add esi, 4
		_next
	_end
	_return 1
_end

;;;;;;;;;;;;;;;;;;;;ReubicateModules;;;;;;;;;;;;;;;;;;;;
_proc ReubicateModules	;bool ReubicateModules()
	_enter esi
	
	;Obtener la tabla de modulos, esi apunta a la primera entrada, edi a la tabla
	mov esi, LP_DIRECTION_FLAT
	add esi, LP_XFT_KRNL_DIRECTORY
	
	;Miramos que la entrada sea valida (used!=0)
	_while ne, {cmp byte [esi + XFS_ENTRY.used], 0}
		;Miramos que la entrada no sea un directory (is_directory!=1)
		_if ne, {cmp byte [esi + XFS_ENTRY.is_directory], 1}
			;Resolvemos las imports del modulo
			_call ReubicateKernelModule, esi
			_if z, {test eax, eax}
				_return 0
			_end
		_end
	
		;Pasamos al siguiente modulo
		add esi, XFS_ENTRY_size
	_end
	_return 1
_end

;;;;;;;;;;;;;;;;;;;CompareString;;;;;;;;;;;;;;;;;;;;;;;;
_proc CompareString, 2 ;bool CompareString(string s1, string s2)
	%arg s1:dword, s2:dword
	_enter ecx, edx, esi, edi

	;Recogemos los parametros
	mov esi, [s1]
	mov edi, [s2]

	;Comparamos la longitud, si es distinta salimos
	mov ah, byte [esi]
	mov al, byte [edi]
	_if e, {cmp ah, al}

	;Comparamos byte a byte
		xor edx, edx
		mov dl, al
		inc esi
		inc edi

		_for ecx, 0, edx, 1
			mov al, byte [esi]
			mov ah, byte [edi]
			_if ne, {cmp ah, al}
				_return 0
			_end
			
			inc esi
			inc edi
		_next

		_return 1
	_end
	_return 0
_end

;;;;;;;;;;;;;;;;;;;;;;GetImports;;;;;;;;;;;;;;;;;;;;;;;;
;;Output --> eax=Hay imports, esi=Puntero al comienzo de las imports, edi=puntero al final de las imports
_proc GetImports, 1 ;bool GetImports(XFS_ENTRY* module)
	%arg module:dword
	_enter
	
	;Por si no hay imports, restaurar sus valores
	push esi
	push edi
	
	;Descriptor del modulo en la tabla
	mov eax, [module]
	
	;Acceder a las imports del modulo
	mov eax, [eax + XFS_ENTRY.direction]
	lea esi, [eax + XKY_MODULE_HEADER.imports_section]
	mov edi, [esi + XKY_SECTION_HEADER.size]
	_if z, {test edi, edi}
		pop edi
		pop esi
		_return 0
	_end
	
	;esi = comienzo imports  edi = final imports
	mov esi, [esi + XKY_SECTION_HEADER.offset]
	add esi, eax
	add edi, esi
	
	
	add esp, 8	;Debido a los valores salvados de edi y esi
	_return 1
_end

;;;;;;;;;;;;;;;;;;;;;;GetExports;;;;;;;;;;;;;;;;;;;;;;;;
;;Output --> eax=Hay exports, esi=Puntero al comienzo de las exports, edi=puntero al final de las exports
_proc GetExports, 1 ;bool GetExports(XFS_ENTRY* module)
	%arg module:dword
	_enter

	push esi
	push edi
	
	;Descriptor del modulo en la tabla
	mov eax, [module]
	
	;Acceder a las exports del modulo
	mov eax, [eax + XFS_ENTRY.direction]
	lea esi, [eax + XKY_MODULE_HEADER.exports_section]
	mov edi, [esi + XKY_SECTION_HEADER.size]
	_if z, {test edi, edi}
		pop edi
		pop esi
		_return 0
	_end
	
	;esi = comienzo exports  edi = final exports
	mov esi, [esi + XKY_SECTION_HEADER.offset]
	add esi, eax
	add edi, esi

	add esp, 8	;Debido a los valores salvados de edi y esi
	_return 1
_end

;;;;;;;;;;;;;;ResolveImportWithinModule;;;;;;;;;;;;;;;;;
_proc ResolveImportWithinModule, 2 ;bool ResolveImportWithinModule(XKY_IMPORT* import, XFS_ENTRY* module)
	%arg module_import:dword, module:dword
	_enter ecx, esi, edi
	
	;Obtenemos los exports del modulo
	_call GetExports, dword [module]
	_if nz, {test eax, eax}
		xor ecx, ecx
		;En ESI esta un puntero al comienzo de las exports y en EDI uno al final
		_while ne, {cmp edi, esi}
			;Miramos si coincide el nombre de la import con la export
			_call CompareString, dword [module_import], esi
			_if nz, {test eax, eax}
				;Escribir la direccion de la Export en la Import
				mov edi, dword [module_import]
	
				;Copiamos
				mov ecx, dword [esi + XKY_EXPORT.direction]
				mov dword [edi + XKY_IMPORT.direction], ecx

				_return 1
			_end

			;Pasamos a la siguiente export
			add esi, XKY_EXPORT_size
		_end
	_end
	_return 0
_end


;;;;;;;;;;;;;;;;;;;;ResolveImport;;;;;;;;;;;;;;;;;;;;;;;
_proc ResolveImport, 2 ;bool ResolveImport(XKY_IMPORT* import, XFS_ENTRY* table)
	%arg module_import:dword, table:dword
	_enter esi
	
	;Tabla de modulos
	mov esi, dword [table]

	;Miramos que la entrada sea valida (used!=0)
	_while ne, {cmp byte [esi + XFS_ENTRY.used], 0}
		;Miramos que la entrada no sea un directory (is_directory!=1)
		_if ne, {cmp byte [esi + XFS_ENTRY.is_directory], 1}
			_call ResolveImportWithinModule, dword [ebp+8], esi
			_if nz, {test eax, eax}
				_return 1
			_end
		_end

		;Pasamos al siguiente descriptor
		add esi, XFS_ENTRY_size
	_end

	_return 0
_end

;;;;;;;;;;;;;;;;;ResolveModuleImports;;;;;;;;;;;;;;;;;;;
_proc ResolveModuleImports, 2 ;bool ResolveModuleImports(XFS_ENTRY* module, XFS_ENTRY* krnl_table)
	%arg module:dword, krnl_table:dword
	_enter ecx, esi, edi

	;Acceder a las imports del modulo
	_call GetImports, dword [module]
	_if nz, {test eax, eax}
		;EDI puntero al final de las imports y ESI puntero al comienzo
		xor ecx, ecx
	
		;Miramos si hemos acabado
		_while ne, {cmp edi, esi}
			;Resolvemos una import
			_call ResolveImport, esi, dword [krnl_table]
			_if z, {test eax, eax}
				_return 0
			_end
	
			;Pasamos a la siguiente import
			add esi, XKY_IMPORT_size
		_end
	_end	
	_return 1
_end
	
;;;;;;;;;;;;;;;;;;;;ResolveImports;;;;;;;;;;;;;;;;;;;;;;
_proc ResolveImports ;bool ResolveImports()
	_enter esi, edi
	
	;Obtener la tabla de modulos, esi apunta a la primera entrada, edi a la tabla
	mov esi, LP_DIRECTION_FLAT
	add esi, LP_XFT_KRNL_DIRECTORY
	mov edi, esi
	
	;Miramos que la entrada sea valida (used!=0)
	_while ne, {cmp byte [esi + XFS_ENTRY.used], 0}
		;Miramos que la entrada no sea un directory (is_directory!=1)
		_if ne, {cmp byte [esi + XFS_ENTRY.is_directory], 1}
			;Resolvemos las imports del modulo
			_call ResolveModuleImports, esi, edi
			;Si hubo un error, salir
			_if z, {test eax, eax}
				_return 0
			_end
		_end
		;Pasamos al siguiente modulo
		add esi, XFS_ENTRY_size
	_end
	_return 1
_end

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
_proc PrintScreen, 1 ;void PrintScreen(dword color)
	%arg color:dword
	_enter eax, ebx, ecx, edx

	;Obtener el framebuffer de la SVGA
	mov eax, LP_DIRECTION_FLAT
	add eax, LP_SVGA
	mov ebx, dword [eax + SVGA_LOADER_DATA.framebuffer]
	
	mov eax, [color]
	
	_for ecx, 0, Y_RES*X_RES*4, 4
		mov [ebx+ecx], eax
	_next
	_return
_end
