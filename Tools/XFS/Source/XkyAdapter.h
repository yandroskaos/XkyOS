#ifndef __XKY_ADAPTER_H__
#define __XKY_ADAPTER_H__

#include "Types.h"
#include "XFS.h"

#define SECTOR_SIZE 0x200

//Basic Types
typedef byte	u8;
typedef word	u16;
typedef dword	u32;

//Disk types
typedef u32 LBA;
typedef u32 OFFSET;

typedef XFS_ENTRY XFS_Entry;
#define TO_SECTORS(X)	( ((X)%SECTOR_SIZE) ? (((X)/SECTOR_SIZE)+1) : ((X)/SECTOR_SIZE))
#define TO_OFFSET(X)	((X)*SECTOR_SIZE)
#define XFS_ENTRY_SIZE	sizeof(XFS_ENTRY)

#define XFS_ENTRY_USED_OFFSET			0
#define XFS_ENTRY_IS_DIRECTORY_OFFSET		1
#define XFS_ENTRY_ITEMS_OFFSET			2
#define XFS_ENTRY_NAME_SIZE_OFFSET		3
#define XFS_ENTRY_NAME_OFFSET			4
#define XFS_ENTRY_DIRECTION_OFFSET		56
#define XFS_ENTRY_SIZE_OFFSET			60

#endif
