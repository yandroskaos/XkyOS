#include <iostream>
#include <string>
using namespace std;

#include "MappedFile.h"
#include "FloppyWriter.h"

int main(int argc, char* argv[])
{
	try
	{
		if(argc != 2)
		{
			cout<<"Uso: FloppyWriter <Imagen de Disco>"<<endl;
			return 0;
		}

		MappedFile disk_image(argv[1]);
		FloppyWriter floppy_writer;

		floppy_writer.Write(disk_image.GetBasePointer(), disk_image.GetSize());

	}
	catch(DWORD _last_error)
	{
		cout<<"Last error: "<<_last_error<<endl;
	}
	catch(...)
	{
		cout<<"Unknown error"<<endl;
	}
	return 0;
}
