#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <string>

#include "PE.h"
#include "X.h"

void Convert(std::string pe_file_name, std::string xky_file_name);

int main(int argc, char* argv[])
{
	try
	{
		//Testear entrada
		if(argc!=3)
		{
			std::cout<<"PE2X <fichero.pe> <fichero.x>"<<std::endl;
			return 0;
		}

		//Ejecutar el "linker"
		Convert(argv[1], argv[2]);
	}
	catch(std::string error)
	{
		std::cout<<error.c_str()<<std::endl;
	}
	catch(...)
	{
		std::cout<<"Unknown error"<<std::endl;
	}

	return 0;
}

#include "SectionNames.h"

dword AlignTo(dword value, dword alignment)
{
	if(value%alignment) value += (alignment - (value%alignment));
	return value;
}

dword CalculateXkyFileSize(PEFile* pe_file, dword desired_section_alignment, dword desired_header_alignment)
{
	dword size=0;
	IMAGE_SECTION_HEADER* relocs_header = pe_file->GetSectionHeaderByName(RELOC_SECTION_NAME);
	IMAGE_SECTION_HEADER* module_header = pe_file->GetSectionHeaderByName(MODULE_SECTION_NAME);
	
	//Sumamos .module
	size+=AlignTo(module_header->Misc.VirtualSize, desired_header_alignment);

	//Sumamos todas las secciones incluida la cabecera .module menos las reubicaciones y la cabecera
	for(dword i=0; i<pe_file->NumberOfSections(); i++)
	{
		IMAGE_SECTION_HEADER* section_header = pe_file->GetSectionHeaderByIndex(i);
		if(section_header != relocs_header && section_header != module_header)
			size+=AlignTo(pe_file->GetSectionHeaderByIndex(i)->Misc.VirtualSize, desired_section_alignment);
	}

	//Reubicaciones
	dword relocs_size = 0;
	if(relocs_header)
	{
		BYTE* raw_file=pe_file->Raw();
		IMAGE_BASE_RELOCATION* base_relocation = (IMAGE_BASE_RELOCATION*)(raw_file + relocs_header->PointerToRawData);
		while(base_relocation->SizeOfBlock)
		{
			dword elements = (base_relocation->SizeOfBlock - 2*sizeof(DWORD)) / sizeof(WORD);
			BYTE* last_reubication = (((BYTE*)base_relocation) + base_relocation->SizeOfBlock - sizeof(WORD));
			if(!*last_reubication) elements--;

			//Sumamos tantos elementos por su tamaño en el fichero xky
			relocs_size += elements * sizeof(dword);
			
			//Siguiente
			base_relocation = (IMAGE_BASE_RELOCATION*)(((BYTE*)base_relocation) + base_relocation->SizeOfBlock);
		}
	}
	/*
	struct IMG_RELOCATION
	{
	dword relocations_number;
	dword relocations[];
	}
	*/
	relocs_size++;
	relocs_size=AlignTo(relocs_size, desired_section_alignment);

	//Devolvemos
	return size + relocs_size;
}

#define XKY_SECTION_ALIGNMENT	16
#define XKY_HEADER_ALIGNMENT	256

void DumpHeader(XFile* xky_file, PEFile* pe_file, dword alignment)
{
	IMAGE_SECTION_HEADER* module_section_header=pe_file->GetSectionHeaderByName(MODULE_SECTION_NAME);
	DWORD module_section_size = module_section_header->Misc.VirtualSize;
	BYTE* module_section=pe_file->GetSectionByName(MODULE_SECTION_NAME);
	xky_file->SequentialWrite(module_section, module_section_size, alignment);
	xky_file->Header()->file_header.size = xky_file->Size();
}

void DumpSection(XFile* xky_file, PEFile* pe_file, const char* section_name, dword alignment)
{
	IMAGE_SECTION_HEADER* pe_section_header=pe_file->GetSectionHeaderByName(section_name);
	IMG_SECTION_HEADER* xky_section_header=xky_file->GetSectionHeaderByName(section_name);
	if(pe_section_header && xky_section_header)
	{
		BYTE* pe_section = pe_file->GetSectionByName(section_name);
		xky_section_header->offset = xky_file->CurrentOffset();
		xky_section_header->size = AlignTo(pe_section_header->Misc.VirtualSize, alignment);
		if(pe_section_header->Misc.VirtualSize<pe_section_header->SizeOfRawData)
		{
			xky_file->SequentialWrite(pe_section, pe_section_header->Misc.VirtualSize, alignment);
		}
		else
		{
			xky_file->SequentialWrite(pe_section, pe_section_header->SizeOfRawData, alignment);
			for(DWORD i=0; i<xky_section_header->size - pe_section_header->SizeOfRawData; i++)
			{
				BYTE null_byte = 0;
				xky_file->SequentialWrite(&null_byte, 1);
			}
		}
	}
}

void DumpRelocs(XFile* xky_file, PEFile* pe_file, dword alignment)
{
	IMAGE_SECTION_HEADER* pe_relocs_section_header=pe_file->GetSectionHeaderByName(RELOC_SECTION_NAME);
	IMG_SECTION_HEADER* xky_relocs_section_header=xky_file->GetSectionHeaderByName(RELOC_SECTION_NAME);
	if(pe_relocs_section_header)
	{
		xky_relocs_section_header->offset = xky_file->CurrentOffset();
		//Deberia estar alineada a "alignment" si hemos hecho bien las cuentas 
		xky_relocs_section_header->size = xky_file->Size() - xky_file->CurrentOffset();
		//Escribir el numero de reubicaciones (mas tarde lo retocamos con "pe_total_relocs")
		dword null_dword = 0;
		xky_file->SequentialWrite(&null_dword, 4);

		DWORD pe_total_relocs = 0;

		BYTE* pe_raw_file=pe_file->Raw();
		byte* xky_raw_file = xky_file->Raw();
		IMAGE_SECTION_HEADER* pe_section;
		IMG_SECTION_HEADER* xky_section;

		IMAGE_BASE_RELOCATION* pe_reloc_block = (IMAGE_BASE_RELOCATION*)(pe_raw_file + pe_relocs_section_header->PointerToRawData);
		while(pe_reloc_block->SizeOfBlock)
		{
			DWORD pe_reloc_elements = (pe_reloc_block->SizeOfBlock - 8)/2;
			WORD* pe_reloc_array = (WORD*)(((BYTE*)pe_reloc_block) + 8);
			for(DWORD i=0; i<pe_reloc_elements; i++)
			{
				if(pe_reloc_array[i])
				{
					pe_total_relocs++;

					//Obtenemos la rva del pe a reubicar
					DWORD pe_rva_to_relocate = pe_reloc_block->VirtualAddress + (pe_reloc_array[i]&0x0FFF);
					//Vemos en qué seccion cae
					pe_section = pe_file->GetSectionHeaderByRVA(pe_rva_to_relocate);
					if(!pe_section) throw std::string("Relocating... Cant find PE section for rva");
					//CAlculamos el offset dentro de la seccion
					DWORD section_offset = pe_rva_to_relocate - pe_section->VirtualAddress;

					//Ahora obtenemos la seccion del ejecutable xky correspondiente a la del pe y la escribimos
					xky_section = xky_file->GetSectionHeaderByName((const char*)pe_section->Name); //Funciona pq las secciones tienen menos de 8 caracteres y se ponen nulos... OJO!
					dword xky_relocation = 0;
					if(!xky_section)
					{
						//Puede ser en la cabecera (no verifico, asumo salida del compilador correcta y ficheros de entrada al compilador correctos)
						xky_relocation = section_offset;
						xky_file->SequentialWrite(&xky_relocation, 4);
					}
					else
					{
						xky_relocation = xky_section->offset + section_offset;
						xky_file->SequentialWrite(&xky_relocation, 4);
					}

					//Ahora reajustar el valor apuntado por la reubicacion en el fichero de xky
					//porque contiene un relativo correcto pero en el PE
					//xky_relocation es el offset dentro del xky executable que hay que modificar
					//Este valor es una RVA del PE
					dword value_pointed_to_by_relocation = *(dword*)(xky_raw_file + xky_relocation);

					//Obtenemos la seccion que apunta
					pe_section = pe_file->GetSectionHeaderByRVA(value_pointed_to_by_relocation);
					if(!pe_section)
						throw std::string("Relocating... Cant find PE section for value");
					//Y la relativa de xky executable para hacer el ajuste
					xky_section = xky_file->GetSectionHeaderByName((const char*)pe_section->Name);
					if(!xky_section)
					{
						//Suponemos cabecera
						*(dword*)(xky_raw_file + xky_relocation) = value_pointed_to_by_relocation - pe_section->VirtualAddress;
					}
					else
					{
						*(dword*)(xky_raw_file + xky_relocation) = value_pointed_to_by_relocation - pe_section->VirtualAddress + xky_section->offset;
					}
				}
			}

			//Siguiente
			pe_reloc_block = (IMAGE_BASE_RELOCATION*)(((BYTE*)pe_reloc_block) + pe_reloc_block->SizeOfBlock);
		}

		//Guardamos el numero total de reubicaciones
		IMG_RELOCATION* xky_reloc_section = (IMG_RELOCATION*)xky_file->GetSectionByName(RELOC_SECTION_NAME);
		xky_reloc_section->number_of_relocations = pe_total_relocs;
		//Rellenamos a cero las que sobran
		for(dword i=pe_total_relocs; i<(xky_relocs_section_header->size/4 - 1); i++)
			xky_reloc_section->relocations[i] = 0;
	}
}

void Convert(std::string pe_file_name, std::string xky_file_name)
{
	//Creamos el fichero PE
	PEFile pe_file(pe_file_name);
	
	//Testear que exista una seccion ".module"
	if(!pe_file.GetSectionHeaderByName(MODULE_SECTION_NAME))
		throw std::string(".module section doesn't exists");

	//Creamos el fichero linkado
	XFile xky_file(CalculateXkyFileSize(&pe_file, XKY_SECTION_ALIGNMENT, XKY_HEADER_ALIGNMENT));

	//Volcamos la cabecera
	DumpHeader(&xky_file, &pe_file, XKY_HEADER_ALIGNMENT);

	//Volcamos por este orden:
	//	.import
	//	.data
	//	.code
	//	.export
	DumpSection(&xky_file, &pe_file, IMPORT_SECTION_NAME, XKY_SECTION_ALIGNMENT);
	DumpSection(&xky_file, &pe_file, DATA_SECTION_NAME,   XKY_SECTION_ALIGNMENT);
	DumpSection(&xky_file, &pe_file, CODE_SECTION_NAME,   XKY_SECTION_ALIGNMENT);
	DumpSection(&xky_file, &pe_file, EXPORT_SECTION_NAME, XKY_SECTION_ALIGNMENT);

	//Volcamos las relocs
//	xky_module_header->relocs_section.offset = xky_file_write_pointer;
//	xky_module_header->relocs_section.size = xky_file_size - xky_file_write_pointer;
	DumpRelocs(&xky_file, &pe_file, XKY_SECTION_ALIGNMENT);

	//Volcamos a disco
	if(!xky_file.FlushToDisk(xky_file_name))
		throw std::string("Cant open dump disk file: ") + xky_file_name;
}
