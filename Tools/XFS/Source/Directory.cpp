#include "Directory.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>

void Directory::Visit(std::string directory_name,  bool reverse_files, DirectoryFileVisitor* visitor)
{
	WIN32_FIND_DATA find_file_data;
	HANDLE file=INVALID_HANDLE_VALUE;
	std::string seek_directory=directory_name+"\\*";
	std::vector<std::pair<std::string, bool> > files;

	file=FindFirstFile(seek_directory.c_str(), &find_file_data);
	if(file!=INVALID_HANDLE_VALUE)
	{
		do
		{
			if(!(find_file_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
			{
				//visitor->VisitFile(directory_name, string(find_file_data.cFileName), false);
				files.push_back(std::pair<std::string, bool>(find_file_data.cFileName, false));
			}
			else
			{
				if(strcmp(find_file_data.cFileName, "..") && strcmp(find_file_data.cFileName, "."))
				{
					//visitor->VisitFile(directory_name, string(find_file_data.cFileName), true);
					files.push_back(std::pair<std::string, bool>(find_file_data.cFileName, true));
				}
			}
		}
		while(FindNextFile(file, &find_file_data)!=0);
		FindClose(file);
	}

	//Visita
	if(reverse_files)
	{
		for(std::vector<std::pair<std::string, bool> >::reverse_iterator i=files.rbegin(); i!=files.rend(); i++)
			visitor->VisitFile(directory_name, i->first, i->second, true);
	}
	else
	{
		for(std::vector<std::pair<std::string, bool> >::iterator i=files.begin(); i!=files.end(); i++)
			visitor->VisitFile(directory_name, i->first, i->second, false);
	}
}


//Utility
class CountFileVisitor : public DirectoryFileVisitor
{
	unsigned int items;
	bool count_directories;

public:
	CountFileVisitor(bool _count_directories) : items(0), count_directories(_count_directories){}
	
	virtual void VisitFile(const std::string& directory_name, const std::string& file_name, bool is_directory, bool reversing_files)
	{
		if(!is_directory || count_directories)
			items++;
	}
	
	unsigned int Items()
	{
		return items;
	}
};

class CountFileVisitorRecursive : public DirectoryFileVisitor
{
	unsigned int items;
	bool count_directories;

public:
	CountFileVisitorRecursive(bool _count_directories) : items(0), count_directories(_count_directories){}
	
	virtual void VisitFile(const std::string& directory_name, const std::string& file_name, bool is_directory, bool reversing_files)
	{
		if(!is_directory || count_directories)
			items++;

		if(is_directory)
		{
			CountFileVisitorRecursive cfv(count_directories);
			Directory::Visit(directory_name+"\\"+file_name, reversing_files, &cfv);
			items+=cfv.Items();
		}
	}
	
	unsigned int Items()
	{
		return items;
	}
};

unsigned int Directory::GetItemNumber(std::string directory_name, bool count_directories)
{
	CountFileVisitor cfv(count_directories);
	Directory::Visit(directory_name, false, &cfv);
	return cfv.Items();
}

unsigned int Directory::GetItemNumberRecursive(std::string directory_name, bool count_directories)
{
	CountFileVisitorRecursive cfv(count_directories);
	Directory::Visit(directory_name, false, &cfv);
	return cfv.Items();
}
