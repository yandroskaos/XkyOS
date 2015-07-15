/******************************************************************************/
/**
* @file		CRT.cpp
* @brief	XkyOS Hardware Text Screen Library
* Implementation of text-bases screeen operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Types.h"
#include "Graphics.h"
#include "CRT.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
typedef word CRT_CHARACTER;

#define CRT_VIDEO_MEMORY 0x000B8000
#define CRT_COLUMNS	 80
#define CRT_LINES	 25

PRIVATE CRT_CHARACTER* screen = (CRT_CHARACTER*)CRT_VIDEO_MEMORY;
PRIVATE COORDINATE cursor = {0, 0};
PRIVATE string buffer = STRING("00000000");
	
//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

PUBLIC bool CRT_Init()
{
	return true;
}

PUBLIC void CRT_Print(IN string* s, IN byte color, IN byte background)
{
	for(byte i = 0; i < s->size; i++)
	{
		//Make character
		CRT_CHARACTER character = s->text[i]<<8;
		character |= ((color & 0x0F) << 4);
		character |= (background & 0x0F);

		//Print
		screen[cursor.y * CRT_COLUMNS + cursor.x] = character;

		//Update columns and lines
		cursor.x++;
		if(cursor.x >= CRT_COLUMNS)
		{
			cursor.x = 0;
			cursor.y++;
			if(cursor.y >= CRT_LINES)
			{
				cursor.y = 0;
			}
		}
	}
}

PUBLIC void CRT_NewLine()
{
	cursor.x = 0;

	cursor.y++;
	if(cursor.y >= CRT_LINES)
		cursor.y = 0;
}

PUBLIC void CRT_SetCursor(IN dword x, IN dword y)
{
	if((x < CRT_COLUMNS) && (y < CRT_LINES))
	{
		cursor.x = x;
		cursor.y = y;
	}
}

PUBLIC dword CRT_X()
{
	return cursor.x;
}

PUBLIC dword CRT_Y()
{
	return cursor.y;
}

PRIVATE void CRT_ToString32(IN dword data)
{
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

	buffer.text[0] = TO_CHAR(nibble0);
	buffer.text[1] = TO_CHAR(nibble1); 
	buffer.text[2] = TO_CHAR(nibble2);
	buffer.text[3] = TO_CHAR(nibble3);
	buffer.text[4] = TO_CHAR(nibble4);
	buffer.text[5] = TO_CHAR(nibble5);
	buffer.text[6] = TO_CHAR(nibble6);
	buffer.text[7] = TO_CHAR(nibble7);
}

PUBLIC void CRT_Print32(IN dword data)
{
	CRT_ToString32(data);
	CRT_Print(&buffer, 0x01, 0x00);
}
