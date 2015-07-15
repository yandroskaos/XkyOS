#ifndef __XFS_DIRECTORY_WRITER_H__
#define __XFS_DIRECTORY_WRITER_H__

#include <string>

#include "XkyAdapter.h"
#include "Disk.h"

class XFS_DirectoryWriter
{
public:
	static u32 Write(Disk* disk, std::string path, std::string directory_name, bool reverse_files, OFFSET entry, LBA where, LBA parent);
};


#endif
