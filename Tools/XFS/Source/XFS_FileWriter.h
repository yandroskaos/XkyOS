#ifndef __XFS_FILE_WRITER_H__
#define __XFS_FILE_WRITER_H__

#include <string>

#include "XkyAdapter.h"
#include "Disk.h"

class XFS_FileWriter
{
public:
	static u32 Write(Disk* disk, std::string directory_name, std::string file_name, OFFSET entry, LBA where);
};


#endif
