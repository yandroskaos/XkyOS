#ifndef __DIRECTORY_H__
#define __DIRECTORY_H__

#include <string>

class DirectoryFileVisitor
{
public:
	virtual void VisitFile(const std::string& directory_name, const std::string& file_name, bool is_directory, bool reversing_files)=0;
};

class Directory
{
public:
	static void Visit(std::string directory_name,  bool reverse_files, DirectoryFileVisitor* visitor);
	static unsigned int GetItemNumber(std::string directory_name, bool count_directories);
	static unsigned int GetItemNumberRecursive(std::string directory_name, bool count_directories);
};

#endif
