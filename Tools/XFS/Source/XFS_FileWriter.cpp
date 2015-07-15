#include "XFS_FileWriter.h"
#include "XFS_EntryWriter.h"
#include "MappedFile.h"

u32 XFS_FileWriter::Write(Disk* disk, std::string directory_name, std::string file_name, OFFSET entry, LBA where)
{
	//Abrimos el fichero
	MappedFile file(directory_name+"\\"+file_name);

	//Escribimos la entry
	XFS_EntryWriter::Write(disk, entry, false, 0, file_name, TO_OFFSET(where), file.GetSize());

	//Escribimos el fichero
	disk->Write(TO_OFFSET(where), file.GetBasePointer(), file.GetSize());

	//Devolvemos el numero de sectores ocupados
	return TO_SECTORS(file.GetSize());
}