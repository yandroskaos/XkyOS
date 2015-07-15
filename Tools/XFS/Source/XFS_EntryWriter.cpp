#include "XFS_EntryWriter.h"

#define MIN(X, Y) (((X)<(Y)) ? (X) : (Y))

u32 XFS_EntryWriter::Write(Disk* disk, OFFSET entry, bool is_directory, u8 items, std::string name, OFFSET direction, u32 size)
{
	XFS_Entry xfs_entry;
	xfs_entry.used			= 1;
	xfs_entry.is_directory	= is_directory?1:0;
	xfs_entry.items			= items;
	xfs_entry.name_size		= (u8)MIN(strlen(name.c_str()), XFS_ENTRY_NAME_SIZE);
	memset(xfs_entry.name_text, 0, XFS_ENTRY_NAME_SIZE);
	memcpy(xfs_entry.name_text, name.c_str(), xfs_entry.name_size);
	xfs_entry.direction		= direction;
	xfs_entry.size			= size;

	disk->Write(entry, (u8*)&xfs_entry, XFS_ENTRY_SIZE);
	
	return XFS_ENTRY_SIZE;
}
