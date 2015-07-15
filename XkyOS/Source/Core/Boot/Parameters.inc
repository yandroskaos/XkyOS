;;
;; Parameters.inc -- Loader parameters configuration
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

%ifndef __PARAMETERS_INCLUDES__
%define __PARAMETERS_INCLUDES__

	;Parameters at 1000:0000
%define LP_DIRECTION_SEGMENT	0x1000
%define LP_DIRECTION_OFFSET	0x0000
%define LP_DIRECTION_FLAT	0x00010000

	;Parameters
		;DISK
%define LP_DISK	0x0000
%macro DISK_LOADER_DATA  1-3 1, 0
_struct %1, %2, %3
	_dword xft		;XFS_ENTRY* xft
	_dword krnl_directory	;XFS_ENTRY* krnl_directory
	_dword boot_drive	;boot drive
	_dword boot_drive_size	;size
_end
%endmacro
DISK_LOADER_DATA DISK_LOADER_DATA

%define LP_XFT			0x0200
%define LP_XFT_KRNL_DIRECTORY	0x1000

		;MEMORY
%define LP_MEMORY	0x2000
%macro MEMORY_LOADER_DATA  1-3 1, 0
_struct %1, %2, %3
	_dword size_in_megas
	_dword descriptors_number
_end
%endmacro
MEMORY_LOADER_DATA MEMORY_LOADER_DATA

%macro MEMORY_DESCRIPTOR  1-3 1, 0
_struct %1, %2, %3
	_dword base_address_low
	_dword base_address_high
	_dword base_length_low
	_dword base_length_high
	_dword type
_end
%endmacro
MEMORY_DESCRIPTOR MEMORY_DESCRIPTOR

%define MEMORY_TYPE_AVAILABLE		0x00000001
%define MEMORY_TYPE_SYSTEM_RESERVED	0x00000002

		;SVGA
%define LP_SVGA	0x3000	;SVGA
%macro SVGA_LOADER_DATA  1-3 1, 0
_struct %1, %2, %3
	_dword framebuffer	;SVGA Framebuffer
	_dword x_resolution	;SVGA X Resolution
	_dword y_resolution	;SVGA Y Resolution
_end
%endmacro
SVGA_LOADER_DATA SVGA_LOADER_DATA

		;LOADER
;struct LOADER_DATA
;{
;	DISK_LOADER_DATA*	disk;
;	MEMORY_LOADER_DATA*	memory;
;	SVGA_LOADER_DATA*	svga;
;}
%define LP_LOADER	0x00017000
%macro LOADER_DATA  1-3 1, 0
_struct %1, %2, %3
	_dword disk	;DISK_LOADER_DATA*
	_dword memory	;MEMORY_LOADER_DATA*
	_dword svga	;SVGA_LOADER_DATA*
_end
%endmacro
LOADER_DATA LOADER_DATA

%endif ; __PARAMETERS_INCLUDES__
