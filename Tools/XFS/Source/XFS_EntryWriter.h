#ifndef __XFS_ENTRY_WRITER_H__
#define __XFS_ENTRY_WRITER_H__

#include <string>

#include "XkyAdapter.h"
#include "Disk.h"

class XFS_EntryWriter
{
public:
	static u32 Write(Disk* disk, OFFSET entry, bool is_directory, u8 items, std::string name, OFFSET direction, u32 size);
};


#endif
