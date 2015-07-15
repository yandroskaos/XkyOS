;;
;; Image.inc -- Executables file format structures
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

%ifndef __IMAGE_INCLUDES__
%define __IMAGE_INCLUDES__

%macro XKY_SECTION_HEADER 1-3 1, 0
_struct %1, %2, %3
	_byte name, 4
	_dword offset
	_dword size
	_dword flags
_end
%endmacro
XKY_SECTION_HEADER XKY_SECTION_HEADER

%macro XKY_FILE_HEADER 1-3 1, 0
_struct %1, %2, %3
	_dword size
	_dword version
	_dword kind
	_dword mode
	_dword init_ep
	_dword code_ep
	_dword finish_ep
_end
%endmacro
XKY_FILE_HEADER XKY_FILE_HEADER

%macro XKY_MODULE_HEADER 1-3 1, 0
_struct %1, %2, %3
	_dword signature
	XKY_FILE_HEADER file_header
	XKY_SECTION_HEADER imports_section
	XKY_SECTION_HEADER data_section
	XKY_SECTION_HEADER code_section
	XKY_SECTION_HEADER exports_section
	XKY_SECTION_HEADER relocs_section
_end
%endmacro
XKY_MODULE_HEADER XKY_MODULE_HEADER

%define MAX_EXTERN_NAME_SIZE 59

%macro XKY_IMPORT  1-3 1, 0
_struct %1, %2, %3
	_byte	name_size
	_byte	name_text, MAX_EXTERN_NAME_SIZE
	_dword	direction
_end
%endmacro
XKY_IMPORT XKY_IMPORT

%macro XKY_EXPORT  1-3 1, 0
_struct %1, %2, %3
	_byte	name_size
	_byte	name_text, MAX_EXTERN_NAME_SIZE
	_dword	direction
_end
%endmacro
XKY_EXPORT XKY_EXPORT

%macro XKY_RELOCATION  1-3 1, 0
_struct %1, %2, %3
	_dword number_of_relocations;
	_dword relocations;
_end
%endmacro
XKY_RELOCATION XKY_RELOCATION

%endif ; __IMAGE_INCLUDES__
