;;
;; Disk.inc -- On-disk binary structures
;;
;; Copyright (c) 2007 by Pablo Bravo
;; All Rights Reserved
;;

%ifndef __DISK_INCLUDES__
%define __DISK_INCLUDES__

%define SECTOR_SIZE	0x0200

%define XFT_LBA		1
%define XFT_SECTORS	1
%define XFT_SIZE	(SECTOR_SIZE*XFT_SECTORS)

%define XFS_DIRECTORY_SECTORS		8
%define XFS_DIRECTORY_SIZE		(SECTOR_SIZE*XFS_DIRECTORY_SECTORS)

%define XFS_ENTRY_NAME_SIZE		52

%macro XFS_ENTRY  1-3 1, 0
_struct %1, %2, %3
	_byte	used
	_byte	is_directory
	_byte	items
	_byte	name_size
	_byte	name_text, XFS_ENTRY_NAME_SIZE
	_dword	direction
	_dword	size
_end
%endmacro
XFS_ENTRY XFS_ENTRY

%endif ;__DISK_INCLUDES__
