#ifndef __MAPPED_FILE_H__
#define __MAPPED_FILE_H__

#include <string>

class MappedFile
{
private:
	void* file;
	void* mapped_file;
	void* file_base_pointer;
	unsigned int file_size;

public:
	MappedFile(const std::string& file_name);
	~MappedFile();

	unsigned char* GetBasePointer();
	unsigned int GetSize();
};


#endif
