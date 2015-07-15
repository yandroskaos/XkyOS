/******************************************************************************/
/**
* @file		Debug.cpp
* @brief	Debug Subsystem
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Debug.h"
#include "SVGA.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
//CONSOLE constants
#define CHARACTER_HEIGHT	8
#define CHARACTER_WIDTH		8
#define HEIGHT_MARGIN		8
#define WIDTH_MARGIN		8

//Uninitialized Constants
#define UNINIT_CHARACTER_HEIGHT		10
#define UNINIT_WIDTH_START			10
#define UNINIT_HEIGHT_START			758

//Graphic subsystem initialized
bool debug_initialized = false;

//Console constants
WINDOW	debug_window;
dword	debug_height;
dword	debug_width;
dword	debug_x = UNINIT_WIDTH_START;
dword	debug_y = UNINIT_HEIGHT_START;
ARGB	debug_color;
string	debug_title = STRING("DEBUG CONSOLE");

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
/**
* @brief This method converts hexadecimal data into a string. Size of the data string must be provided.
* @param _buffer [in] The allocated string where the data will be formatted.
* @param _data [in] The data to convert.
* @param _size [in] The resulting string size.
*/
PRIVATE void STRING_ToString(IN string* _buffer, IN dword _data, IN byte _size)
{
	if(_size > 8)
		_size = 8;

#define TO_CHAR(X) (((X)>9)?((X)+0x37):((X)+0x30))

	byte b1 = (byte)((_data & 0xFF000000) >> 24);
	byte b2 = (byte)((_data & 0x00FF0000) >> 16);
	byte b3 = (byte)((_data & 0x0000FF00) >> 8);
	byte b4 = (byte)(_data & 0x000000FF);

	byte nibble0 = ((b1&0xF0)>>4);
	byte nibble1 = (b1&0x0F);
	byte nibble2 = ((b2&0xF0)>>4);
	byte nibble3 = (b2&0x0F);
	byte nibble4 = ((b3&0xF0)>>4);
	byte nibble5 = (b3&0x0F);
	byte nibble6 = ((b4&0xF0)>>4);
	byte nibble7 = (b4&0x0F);

	_buffer->size = _size;
	if(_size == 1)
	{
		_buffer->text[0] = TO_CHAR(nibble7);
	}
	if(_size == 2)
	{
		_buffer->text[0] = TO_CHAR(nibble6);
		_buffer->text[1] = TO_CHAR(nibble7);
	}
	if(_size == 3)
	{
		_buffer->text[0] = TO_CHAR(nibble5);
		_buffer->text[1] = TO_CHAR(nibble6);
		_buffer->text[2] = TO_CHAR(nibble7);
	}
	if(_size == 4)
	{
		_buffer->text[0] = TO_CHAR(nibble4);
		_buffer->text[1] = TO_CHAR(nibble5);
		_buffer->text[2] = TO_CHAR(nibble6);
		_buffer->text[3] = TO_CHAR(nibble7);
	}
	if(_size == 5)
	{
		_buffer->text[0] = TO_CHAR(nibble3);
		_buffer->text[1] = TO_CHAR(nibble4);
		_buffer->text[2] = TO_CHAR(nibble5);
		_buffer->text[3] = TO_CHAR(nibble6);
		_buffer->text[4] = TO_CHAR(nibble7);
	}
	if(_size == 6)
	{
		_buffer->text[0] = TO_CHAR(nibble2);
		_buffer->text[1] = TO_CHAR(nibble3);
		_buffer->text[2] = TO_CHAR(nibble4);
		_buffer->text[3] = TO_CHAR(nibble5);
		_buffer->text[4] = TO_CHAR(nibble6);
		_buffer->text[5] = TO_CHAR(nibble7);
	}
	if(_size == 7)
	{
		_buffer->text[0] = TO_CHAR(nibble1);
		_buffer->text[1] = TO_CHAR(nibble2);
		_buffer->text[2] = TO_CHAR(nibble3);
		_buffer->text[3] = TO_CHAR(nibble4);
		_buffer->text[4] = TO_CHAR(nibble5);
		_buffer->text[5] = TO_CHAR(nibble6);
		_buffer->text[6] = TO_CHAR(nibble7);
	}
	if(_size == 8)
	{
		_buffer->text[0] = TO_CHAR(nibble0);
		_buffer->text[1] = TO_CHAR(nibble1);
		_buffer->text[2] = TO_CHAR(nibble2);
		_buffer->text[3] = TO_CHAR(nibble3);
		_buffer->text[4] = TO_CHAR(nibble4);
		_buffer->text[5] = TO_CHAR(nibble5);
		_buffer->text[6] = TO_CHAR(nibble6);
		_buffer->text[7] = TO_CHAR(nibble7);
	}
}

/**
* @brief Clears the debug console screeen
* @param _color [in] Color to fill the console with.
*/
PUBLIC void DEBUG_Clear(IN ARGB _color)
{
	for(dword x = WIDTH_MARGIN; x <= (debug_width - WIDTH_MARGIN); x++)
	{
		for(dword y = HEIGHT_MARGIN; y <= debug_height - HEIGHT_MARGIN; y++)
		{
			WINDOW_SetPixel(debug_window, x, y, _color);
		}
	}

	debug_x = WIDTH_MARGIN;
	debug_y = debug_height - HEIGHT_MARGIN;

	//Colocamos el nombre
	WINDOW_PrintText(debug_window, 0, debug_height, SRGB(255, 0, 0), &debug_title);
}

/**
* @brief Writes a message in the debug console.
* @param _text [in] The message to show.
* @param _color [in] The color of the message.
*/
PRIVATE void DEBUG_Write(IN string* _text, IN ARGB _color)
{
	if(_text->size && (debug_x + _text->size * CHARACTER_WIDTH < debug_width - WIDTH_MARGIN))
	{
		WINDOW_PrintText(debug_window, debug_x, debug_y, _color, _text);
		debug_x += _text->size * CHARACTER_WIDTH;
	}
}

/**
* @brief Writes a number in hexadecimal in the debug console.
* @param _data [in] The data to show.
* @param _size [in] The number of nibbles to show, starting from the left.
* @param _color [in] The color of the data.
*/
PRIVATE void DEBUG_WriteNumber(IN dword _data, IN byte _size, IN ARGB _color)
{
	if(_size)
	{
		string buffer = STRING("00000000");
		STRING_ToString(&buffer, _data, _size);
		DEBUG_Write(&buffer, _color);
	}
}

/**
* @brief Writes a new line in the debug console.
*/
PRIVATE void DEBUG_NewLine()
{
	debug_x = WIDTH_MARGIN;

	debug_y -= CHARACTER_HEIGHT;
	if(debug_y <= HEIGHT_MARGIN)
		debug_y = debug_height - HEIGHT_MARGIN;
}

/**
* @brief Starts the debug console.
* The debug subsystem is always active, but when the windows subsystem is initialized, the
* debug subsystem asks for for a window and redirects its output to the window.
*
* @param _color [in] The color of the console window.
* @return True if everything is ok.
*/
PUBLIC bool DEBUG_Init(IN ARGB _color)
{
#ifdef _ENABLE_DEBUG_
	debug_color = _color;

	debug_window = WINDOW_GetWindow();
	if(!debug_window) return false;

	debug_height = WINDOW_GetHeight(debug_window);
	debug_width = WINDOW_GetWidth(debug_window);

	//Colocamos la consola
	DEBUG_Clear(debug_color);

	debug_initialized = true;
#endif
	return true;
}

/**
* @brief Resets the debug window and debug subsystem backs to screen dump.
* @return True if debug output can be seprated form window subsystem.
*/
PUBLIC bool DEBUG_Uninit()
{
	debug_initialized = false;
	debug_x = UNINIT_WIDTH_START;
	debug_y = UNINIT_HEIGHT_START;
	return true;
}

/**
* @brief Outputs a message in the debug subsystem.
* @param _message [in] The message to show.
* @param _color [in] The color of the message.
*/
PUBLIC void	DEBUG_Message(IN string* _message, IN ARGB _color)
{
#ifdef _ENABLE_DEBUG_
	if(debug_initialized)
	{
		DEBUG_Write(_message, _color);
		DEBUG_NewLine();
	}
	else
	{
		SVGA_PrintText(UNINIT_WIDTH_START, debug_y, _color, _message);
		debug_y -= UNINIT_CHARACTER_HEIGHT;
	}
#endif
}

/**
* @brief Outputs a message and a data number in the debug subsystem.
* @param _message [in] The message to show.
* @param _data [in] The data.
* @param _color [in] The color of the message.
*/
PUBLIC void	DEBUG_Data(IN string* _message, IN dword _data, IN ARGB _color)
{
#ifdef _ENABLE_DEBUG_
	if(debug_initialized)
	{
		DEBUG_Write(_message, _color);
		DEBUG_WriteNumber(_data, 8, _color);
		DEBUG_NewLine();
	}
	else
	{
		string buffer = STRING("00000000");
		STRING_ToString(&buffer, _data, 8);
		SVGA_PrintText(UNINIT_WIDTH_START, debug_y, _color, _message);
		SVGA_PrintText(UNINIT_WIDTH_START + _message->size*CHARACTER_WIDTH + 1, debug_y, _color, &buffer);
		debug_y -= UNINIT_CHARACTER_HEIGHT;
		if(debug_y < UNINIT_CHARACTER_HEIGHT)
			debug_y = UNINIT_HEIGHT_START;
	}
#endif
}

/**
* @brief Obtains the EIP at which it is invoked.
* @return The current EIP.
*/
PUBLIC NAKED dword DEBUG_EIP()
{
	__asm
	{
		mov eax, [esp]
		ret
	}
}

/**
* @brief Empties the debug console, or prepares the output to be displayed from the begining.
*/
PUBLIC void DEBUG_Reset()
{
#ifdef _ENABLE_DEBUG_
	if(debug_initialized)
	{
		DEBUG_Clear(debug_color);
	}
	else
	{
		debug_x = UNINIT_WIDTH_START;
		debug_y = UNINIT_HEIGHT_START;
	}
#endif
}
