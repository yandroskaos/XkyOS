#include "XFS_DirectoryWriter.h"
#include "XFS_FileWriter.h"
#include "XFS_EntryWriter.h"
#include "MappedFile.h"
#include "Directory.h"

#define PARENT_NAME "PARENT"

class XFS_DirectoryWriterVisitor : public DirectoryFileVisitor
{
	Disk* disk;
	u32 sectors_written;
	unsigned int items;

	LBA directory_table;
	OFFSET xfs_entry;
	LBA data_sector;

public:
	XFS_DirectoryWriterVisitor(Disk* _disk, LBA _where, LBA _parent)
	: disk(_disk), sectors_written(XFS_DIRECTORY_SECTORS), items(0)
	{
		directory_table=_where;
		xfs_entry=TO_OFFSET(directory_table);
		data_sector=directory_table + XFS_DIRECTORY_SECTORS;

		//Escribimos la parent entry
		xfs_entry+=XFS_EntryWriter::Write(disk, xfs_entry, true, 0, PARENT_NAME, TO_OFFSET(_parent), XFS_DIRECTORY_SIZE);
	}
	~XFS_DirectoryWriterVisitor()
	{
		OFFSET xfs_entry=TO_OFFSET(directory_table)+XFS_ENTRY_SIZE; //Nos saltamos la primera entrada que es el PARENT DIRECTORY
		for(unsigned int i=0; i<items; i++, xfs_entry+=XFS_ENTRY_SIZE)
		{
			//Leemos la entry entera
			XFS_Entry entry;
			disk->Read(xfs_entry, (u8*)&entry, XFS_ENTRY_SIZE);

			//Si es un directorio hijo
			if(entry.is_directory)
			{
				//Actualizamos los items de su primera entrada
				disk->Write(entry.direction + XFS_ENTRY_ITEMS_OFFSET, (u8*)&items, 1);
			}
		}
	}

	void VisitFile(const std::string& directory_name, const std::string& file_name, bool is_directory, bool reversing_files)
	{
		//Un elemento mas
		items++;

		//Escribir
		u32 sectors=0;
		if(is_directory)
		{
			sectors=XFS_DirectoryWriter::Write(disk, directory_name, file_name, reversing_files, xfs_entry, data_sector, directory_table);
		}
		else
		{
			sectors=XFS_FileWriter::Write(disk, directory_name, file_name, xfs_entry, data_sector);
		}

		//Actualizamos los sectores escritos
		sectors_written+=sectors;
		
		//Actualizamos los "punteros"
		data_sector+=sectors;
		xfs_entry+=XFS_ENTRY_SIZE;
	}

	unsigned int Items()
	{
		return items;
	}

	u32 SectorsWritten()
	{
		return sectors_written;
	}
};

u32 XFS_DirectoryWriter::Write(Disk* disk, std::string path, std::string directory_name,  bool reverse_files, OFFSET entry, LBA where, LBA parent)
{
	XFS_DirectoryWriterVisitor xfs_directory_writer(disk, where, parent);
	Directory::Visit(path+"\\"+directory_name, reverse_files, &xfs_directory_writer);

	//Escribimos la entry
	XFS_EntryWriter::Write(disk, entry, true, (u8)xfs_directory_writer.Items(), directory_name, TO_OFFSET(where), XFS_DIRECTORY_SIZE);

	//Devolvemos el numero de sectores ocupados
	return xfs_directory_writer.SectorsWritten();
}