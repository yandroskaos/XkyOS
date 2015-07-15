#ifndef __DISK_H__
#define __DISK_H__

#include <string>

#include "XkyAdapter.h"

#define FLOPPY_DISK_NAME	"FLOPPY"
#define FLOPPY_SECTORS		2880
#define FLOPPY_SIZE			FLOPPY_SECTORS*SECTOR_SIZE

class Disk
{
public:
	virtual ~Disk(){}

	virtual void	Write	(OFFSET offset, u8* data, u32 size)=0;
	virtual void	Read	(OFFSET offset, u8* data, u32 size)=0;

	static Disk* GetDisk(std::string disk_name, u32 desired_size=0);
};

#endif
