/******************************************************************************/
/**
* @file		Test.cpp
* @brief	XkyOS User mode test application
* Example of a working XkyOS application
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Types.h"
#include "Executable.h"

//=================================IMPORTS====================================//
#pragma data_seg(".imports")
//============================================================================//
#include "API.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
string console = STRING("CONSOLE");
#define CHARACTER_HEIGHT	8
#define CHARACTER_WIDTH		8
#define HEIGHT_MARGIN		8
#define WIDTH_MARGIN		8

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

class Console
{
	WINDOW window;

	dword height;
	dword width;

	dword x;
	dword y;

	void ToString32(IN string* buffer, IN dword data, IN dword size)
	{
		if(size > 8)
			size = 8;

	#define TO_CHAR(X) (((X)>9)?((X)+0x37):((X)+0x30))

		byte b1 = (byte)((data & 0xFF000000) >> 24);
		byte b2 = (byte)((data & 0x00FF0000) >> 16);
		byte b3 = (byte)((data & 0x0000FF00) >> 8);
		byte b4 = (byte)(data & 0x000000FF);

		byte nibble0 = ((b1&0xF0)>>4);
		byte nibble1 = (b1&0x0F);
		byte nibble2 = ((b2&0xF0)>>4);
		byte nibble3 = (b2&0x0F);
		byte nibble4 = ((b3&0xF0)>>4);
		byte nibble5 = (b3&0x0F);
		byte nibble6 = ((b4&0xF0)>>4);
		byte nibble7 = (b4&0x0F);

		buffer->size = size;
		if(size == 1)
		{
			buffer->text[0] = TO_CHAR(nibble7);
		}
		if(size == 2)
		{
			buffer->text[0] = TO_CHAR(nibble6);
			buffer->text[1] = TO_CHAR(nibble7);
		}
		if(size == 3)
		{
			buffer->text[0] = TO_CHAR(nibble5);
			buffer->text[1] = TO_CHAR(nibble6);
			buffer->text[2] = TO_CHAR(nibble7);
		}
		if(size == 4)
		{
			buffer->text[0] = TO_CHAR(nibble4);
			buffer->text[1] = TO_CHAR(nibble5);
			buffer->text[2] = TO_CHAR(nibble6);
			buffer->text[3] = TO_CHAR(nibble7);
		}
		if(size == 5)
		{
			buffer->text[0] = TO_CHAR(nibble3);
			buffer->text[1] = TO_CHAR(nibble4);
			buffer->text[2] = TO_CHAR(nibble5);
			buffer->text[3] = TO_CHAR(nibble6);
			buffer->text[4] = TO_CHAR(nibble7);
		}
		if(size == 6)
		{
			buffer->text[0] = TO_CHAR(nibble2);
			buffer->text[1] = TO_CHAR(nibble3);
			buffer->text[2] = TO_CHAR(nibble4);
			buffer->text[3] = TO_CHAR(nibble5);
			buffer->text[4] = TO_CHAR(nibble6);
			buffer->text[5] = TO_CHAR(nibble7);
		}
		if(size == 7)
		{
			buffer->text[0] = TO_CHAR(nibble1);
			buffer->text[1] = TO_CHAR(nibble2);
			buffer->text[2] = TO_CHAR(nibble3);
			buffer->text[3] = TO_CHAR(nibble4);
			buffer->text[4] = TO_CHAR(nibble5);
			buffer->text[5] = TO_CHAR(nibble6);
			buffer->text[6] = TO_CHAR(nibble7);
		}
		if(size == 8)
		{
			buffer->text[0] = TO_CHAR(nibble0);
			buffer->text[1] = TO_CHAR(nibble1);
			buffer->text[2] = TO_CHAR(nibble2);
			buffer->text[3] = TO_CHAR(nibble3);
			buffer->text[4] = TO_CHAR(nibble4);
			buffer->text[5] = TO_CHAR(nibble5);
			buffer->text[6] = TO_CHAR(nibble6);
			buffer->text[7] = TO_CHAR(nibble7);
		}
	}

	/*
	void DrawLineHorizontal(IN dword x_inf, IN dword x_sup, IN dword y, IN ARGB color)
	{
		for(dword i = x_inf; i <= x_sup; i++)
		{
			WINDOW_SetPixel(window, i, y, color);
		}
	}

	void DrawLineVertical(IN dword y_inf, IN dword y_sup, IN dword x, IN ARGB color)
	{
		for(dword i = y_inf; i <= y_sup; i++)
		{
			WINDOW_SetPixel(window, x, i, color);
		}
	}
	*/

	void Clear(IN ARGB color)
	{
		//Rellenar el rectangulo
		for(dword x = WIDTH_MARGIN; x <= (width - WIDTH_MARGIN); x++)
		{
			for(dword y = HEIGHT_MARGIN; y <= height - HEIGHT_MARGIN; y++)
			{
				XKY_WINDOW_SetPixel(window, x, y, color);
			}
		}

		/*
		DrawLineHorizontal(WIDTH_MARGIN, width - WIDTH_MARGIN, height - HEIGHT_MARGIN, color);
		DrawLineVertical(HEIGHT_MARGIN, height - HEIGHT_MARGIN, WIDTH_MARGIN, color);
		DrawLineHorizontal(WIDTH_MARGIN, width - WIDTH_MARGIN, HEIGHT_MARGIN, color);
		DrawLineVertical(HEIGHT_MARGIN, height - HEIGHT_MARGIN, width - WIDTH_MARGIN, color);
		*/

	}

public:
	Console(){}

	bool Init(IN ARGB color)
	{
		window = XKY_WINDOW_Alloc();
		if(window)
		{
			height = XKY_WINDOW_GetHeight(window);
			width = XKY_WINDOW_GetWidth(window);

			x = WIDTH_MARGIN;
			y = height - HEIGHT_MARGIN;
			
			//Colocamos la consola
			Clear(color);

			//Colocamos el nombre
			XKY_WINDOW_PrintText(window, 0, height, SRGB(255, 0, 0), &console);
			return true;
		}
		return false;
	}

	void Write(IN string* s, IN ARGB color)
	{
		if(s->size && (x + s->size* CHARACTER_WIDTH < width - WIDTH_MARGIN))
		{
			XKY_WINDOW_PrintText(window, x, y, color, s);
			x += s->size* CHARACTER_WIDTH;
		}
	}

	void WriteNumber(IN dword data, IN dword size, IN ARGB color)
	{
		if(size)
		{
			string buffer = STRING("00000000");
			ToString32(&buffer, data, size);
			Write(&buffer, color);
		}
	}

	void NewLine()
	{
		x = WIDTH_MARGIN;

		y -= CHARACTER_HEIGHT;
		if(y <= HEIGHT_MARGIN)
			y = height - HEIGHT_MARGIN;
	}

	void WriteLn(IN string* s, IN ARGB color)
	{
		Write(s, color);
		NewLine();
	}

	void SetCursor(IN dword new_x, IN dword new_y)
	{
		if((x <= width - WIDTH_MARGIN) && (y <= height - HEIGHT_MARGIN) && (x >= WIDTH_MARGIN) && (y >= HEIGHT_MARGIN))
		{
			x = new_x;
			y = new_y;
		}
	}

	dword GetX()
	{
		return x;
	}

	dword GetY()
	{
		return y;
	}
};

PUBLIC void Main()
{
	Console c;
	ARGB red = SRGB(255, 0, 0);
	ARGB blue = SRGB(255, 255, 255);
	ARGB gray = TRGB(128, 128, 128);

	if(c.Init(gray))
	{
		//PCI
		string PCI_HEADER = STRING("Enumerating PCI");
		c.WriteLn(&PCI_HEADER, red);

		dword pci_number_of_devices = XKY_PCI_GetNumberOfDevices();
		for(dword i = 0; i < pci_number_of_devices; i++)
		{
			PCI pci_address = XKY_PCI_GetDevice(i);
			c.WriteNumber(pci_address, 8, blue);
			c.NewLine();
		}


		string sok = STRING("Ok!");
		string sfail = STRING("Failed!");

		
		//Memory
		string MEM_HEADER = STRING("Asking For Memory");
		c.WriteLn(&MEM_HEADER, red);

		byte* pages = (byte*)XKY_PAGE_Alloc(XKY_ADDRESS_SPACE_GetCurrent(), 0x10000000, 2);
		if(pages)
		{
			c.WriteLn(&sok, blue);
		}
		else
		{
			c.WriteLn(&sfail, blue);
		}
		pages[0] = 5;
		pages[4096] = 12;
				
		//Disk
		string DSK_HEADER = STRING("Asking For Disk");
		c.WriteLn(&DSK_HEADER, red);

		LBA disk_sector = XKY_DISK_Alloc(16384, 2);
		if(disk_sector)
		{
			c.WriteLn(&sok, blue);
		}
		else
		{
			c.WriteLn(&sfail, blue);
		}

		if(XKY_DISK_Read(disk_sector, (VIRTUAL)pages, 2))
		{
			c.WriteLn(&sok, blue);
			for(dword i = 0; i < 2*SECTOR_SIZE; i++)
			{
				pages[i] = 0xAA;
			}
			if(XKY_DISK_Write(disk_sector, (VIRTUAL)pages, 2))
			{
				c.WriteLn(&sok, blue);
			}
			else
			{
				c.WriteLn(&sfail, blue);
			}
		}
		else
		{
			c.WriteLn(&sfail, blue);
		}
		XKY_DISK_Free(disk_sector, 2);
		
		XKY_PAGE_Free(XKY_ADDRESS_SPACE_GetCurrent(), (VIRTUAL)pages, 2);
	}

	//Done
	XKY_OS_Finish();
}

//=================================EXPORTS====================================//
#pragma data_seg(".exports")
//============================================================================//

//=================================MODULE=====================================//
#pragma data_seg(".module")
//============================================================================//
	MODULE(IMAGE_MODE_USER, IMAGE_KIND_MODULE, IMAGE_VERSION(1,0,0,0), 0, Main, 0);
