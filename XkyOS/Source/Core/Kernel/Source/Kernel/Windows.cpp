/******************************************************************************/
/**
* @file		Windows.cpp
* @brief	XkyOS Window manager
* Implementation of window subsystem
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Windows.h"
#include "Graphics.h"
#include "Interrupts.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "RTL.h"
#include "Environment.h"

#include "Debug.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//

/**
* @brief Implementation of a pointer.
*/
struct POINTER_IMPL
{
	COORDINATE position;
	ARGB color;
	ARGB background[8][8];
	dword mask;
};

#define POINTER_DEFAULT_COLOR	SRGB(255, 255, 255) /**< Pointer default color (white)*/

#define POINTER_HEIGHT	8 /**< Pointer height in pixels*/
#define POINTER_WIDTH	8 /**< Pointer width in pixels*/

/**
* @brief Pointer mask for an arrow.
*/
PRIVATE byte POINTER_ARROW_MASK[POINTER_HEIGHT*POINTER_WIDTH] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 0,
	1, 1, 1, 1, 1, 1, 0, 0,
	1, 1, 1, 1, 1, 0, 0, 0,
	1, 1, 1, 1, 1, 0, 0, 0,
	1, 1, 1, 0, 0, 1, 0, 0,
	1, 1, 0, 0, 0, 0, 1, 0,
	1, 0, 0, 0, 0, 0, 0, 1};

/**
* @brief Pointer mask for a clock.
*/
PRIVATE byte POINTER_CLOCK_MASK[POINTER_HEIGHT*POINTER_WIDTH] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 1, 1, 1, 1, 1, 1, 0,
	0, 0, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 1, 1, 0, 0, 0,
	0, 0, 0, 1, 1, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 0,
	1, 1, 1, 1, 1, 1, 1, 1};

#define MAX_MASKS 2		/**< Maximum number of masks*/
#define POINTER_ARROW 0	/**< Arrow pointer index*/
#define POINTER_CLOCK 1	/**< Clock pointer index*/

/**
* @brief Masks repository.
*/
PRIVATE byte* masks[MAX_MASKS] = {POINTER_ARROW_MASK, POINTER_CLOCK_MASK};

/**
* @brief The pointer.
*/
PRIVATE POINTER_IMPL mouse_pointer;


/**
* @brief Message from the mouse.
*/
struct MOUSE_MESSAGE
{
	COORDINATE cursor;
	bool right;
	bool left;
};

/**
* @brief Message from the keyboard.
*/
typedef dword KEYBOARD_MESSAGE;

/**
* @brief Implementation of a window.
*/
struct WINDOW_IMPL
{
	bool focus;
	
	RECTANGLE area;
	
	dword	pointer_mask;
	ARGB	pointer_color;
	
    ADDRESS_SPACE			keyboard_pdbr;
	fWindowKeyboardCallback	keyboard_callback;
	KEYBOARD_MESSAGE		keyboard_events[32];
	dword					keyboard_index;

	ADDRESS_SPACE			mouse_pdbr;
	fWindowMouseCallback	mouse_callback;
	MOUSE_MESSAGE			mouse_events[32];
	dword					mouse_index;
};

#define WINDOW_DEFAULT_COLOR	SRGB(0, 0, 0)	/**< Windows default color (black)*/
#define WINDOW_WIDTH_MARGIN		0x00000010		/**< Windows default margin in X*/
#define WINDOW_HEIGHT_MARGIN	0x0000000C		/**< Windows default margin in Y*/
#define MAX_WINDOWS	4	/**< Maximum number of windows*/

/**
* @brief Represents a block in the 'heap' of executions.
*/
struct HEAP_WINDOW_BLOCK
{
	bool used;
	WINDOW_IMPL window;
};
/**
* @brief Windows repository
*/
PRIVATE HEAP_WINDOW_BLOCK windows_heap[MAX_WINDOWS];

/**
* @brief Background image name.
*/
PRIVATE string background_image_name = STRING("XkyOS.bmp");

/**
* @brief Describes a size of a rectangle.
*/
PRIVATE SIZE screen_size;

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
#define TO_INDEX(X)	((X)-1)
#define TO_HANDLE(X)((X)+1)

/**
* @brief Loads the image background.
*/
PRIVATE void WINDOW_LoadBackground()
{
	dword size = FILE_Size(&background_image_name);
	if(size)
	{
		dword pages = RTL_BytesToPages(size);
		PHYSICAL memory = MEM_AllocPages(pages, UserMode);
		if(memory)
		{
			if(FILE_Read(&background_image_name, (byte*)memory))
			{
				//Copy BMP File to Background
				//Jump over BMP header
				byte* bmp = ((byte*)memory + 0x36);
				for(dword i = 0; i < 768; i++)
				{
					for(dword j = 0; j < 1024; j++)
					{
						byte B = *bmp;
						byte G = *(bmp+1);
						byte R = *(bmp+2);
						SVGA_SetPixel(j, i, SRGB(R,G,B));
						bmp+=3;
					}
				}
				//Release memory
				MEM_ReleasePages(memory, pages);
				//Ok
				return;
			}
			//Release memory
			MEM_ReleasePages(memory, pages);
		}
	}
	
	//No image
	SVGA_ClearScreen(SRGB(0,0,255));
}

/**
* @brief Draws an horizontal line.
* @param _x_low [in] The low x coordinate.
* @param _x_high [in] The high x coordinate.
* @param _y [in] The y coordinate.
* @param _color [in] The color of the line.
*/
PRIVATE void WINDOW_DrawLineHorizontal(IN dword _x_low, IN dword _x_high, IN dword _y, IN ARGB _color)
{
	for(dword i = _x_low; i <= _x_high; i++)
	{
		SVGA_SetPixel(i, _y, _color);
	}
}

/**
* @brief Draws a vertical line.
* @param _y_low [in] The low y coordinate.
* @param _y_high [in] The high y coordinate.
* @param _x [in] The x coordinate.
* @param _color [in] The color of the line.
*/
PRIVATE void WINDOW_DrawLineVertical(IN dword _y_low, IN dword _y_high, IN dword _x, IN ARGB _color)
{
	for(dword i = _y_low; i <= _y_high; i++)
	{
		SVGA_SetPixel(_x, i, _color);
	}
}

/**
* @brief Initializes a window.
* @param _index [in] The windows heap index.
* @param _x_low [in] The low x coordinate.
* @param _x_high [in] The high x coordinate.
* @param _y_low [in] The low y coordinate.
* @param _y_high [in] The high y coordinate.
*/
PRIVATE void WINDOW_Init(IN dword _index, IN dword _x_low, IN dword _x_high, IN dword _y_low, IN dword _y_high)
{
	windows_heap[_index].used = false;

	windows_heap[_index].window.focus = false;

	windows_heap[_index].window.area.down_left.x = _x_low + WINDOW_WIDTH_MARGIN;
	windows_heap[_index].window.area.down_left.y = _y_low + WINDOW_HEIGHT_MARGIN;
	windows_heap[_index].window.area.up_right.x = _x_high - WINDOW_WIDTH_MARGIN;
	windows_heap[_index].window.area.up_right.y = _y_high - WINDOW_HEIGHT_MARGIN;

	windows_heap[_index].window.pointer_color = POINTER_DEFAULT_COLOR;
	windows_heap[_index].window.pointer_mask = POINTER_ARROW;

	windows_heap[_index].window.keyboard_pdbr = 0;
	windows_heap[_index].window.keyboard_callback = 0;
	windows_heap[_index].window.keyboard_index = 0;

	windows_heap[_index].window.mouse_pdbr = 0;
	windows_heap[_index].window.mouse_callback = 0;
	windows_heap[_index].window.mouse_index = 0;
}

/**
* @brief Empties a windows.
* @param _index [in] The windows heap index.
* @param _color [in] The color.
*/
PRIVATE void WINDOW_Clear(IN dword _index, IN ARGB _color)
{
	//Make color transparent... should be solid?
	_color = COLOR_MakeTransparent(_color);

	//Rectangle
	dword x_low = windows_heap[_index].window.area.down_left.x;
	dword x_high = windows_heap[_index].window.area.up_right.x;
	dword y_low = windows_heap[_index].window.area.down_left.y;
	dword y_high = windows_heap[_index].window.area.up_right.y;
	
	//Fill the window rectangle
	for(dword x = x_low; x <= x_high; x++)
	{
		for(dword y = y_low; y <= y_high; y++)
		{
			SVGA_SetPixel(x, y, _color);
		}
	}

	//Draw lines
	_color = COLOR_MakeSolid(_color);

	WINDOW_DrawLineHorizontal(x_low, x_high, y_low, _color);
	WINDOW_DrawLineHorizontal(x_low, x_high, y_high, _color);
	WINDOW_DrawLineVertical(y_low, y_high, x_low, _color);
	WINDOW_DrawLineVertical(y_low, y_high, x_high, _color);
}

/**
* @brief Redraws the background where to mouse pointer was.
*/
PRIVATE void POINTER_RestoreBackground()
{
	for(byte y = 0; y < 8; y++)
	{
		for(byte x = 0; x < 8; x++)
		{
			SVGA_SetPixel(mouse_pointer.position.x + x, mouse_pointer.position.y - y, mouse_pointer.background[x][y]);
		}
	}
}

/**
* @brief Gets the background where the pointer is going to be.
*/
PRIVATE void POINTER_SaveBackground()
{
	for(byte y = 0; y < 8; y++)
	{
		for(byte x = 0; x < 8; x++)
		{
			mouse_pointer.background[x][y] = SVGA_GetPixel(mouse_pointer.position.x + x, mouse_pointer.position.y - y);
		}
	}
}

/**
* @brief Draws the mouse pointer.
*/
PRIVATE void POINTER_Draw()
{
	dword width = screen_size.width;
	byte* mask = masks[mouse_pointer.mask];
	for(byte y = 0; y < 8; y++)
	{
		for(byte x = 0; x < 8; x++)
		{
			if(mask[y*8+x] && (mouse_pointer.position.x + x < width) && (mouse_pointer.position.y - y > 0))
				SVGA_SetPixel(mouse_pointer.position.x + x, mouse_pointer.position.y - y, mouse_pointer.color);
		}
	}
}

/**
* @brief Initializes the mouse pointer.
* @param _mask [in] Indicates which mask to use.
* @param _color [in] The color of the pointer.
*/
PRIVATE void POINTER_Init(IN dword _mask, IN ARGB _color)
{
	//Set Color
	mouse_pointer.color = _color;
	
	//Set position
	mouse_pointer.position.x = MOUSE_GetX();
	mouse_pointer.position.y = MOUSE_GetY();

	//Set mask
	mouse_pointer.mask = _mask;
	
	POINTER_SaveBackground();
	POINTER_Draw();
}

/**
* @brief Change the mouse pointer mask.
* @param _mask [in] Indicates which mask to use.
*/
PRIVATE void POINTER_SetNewMask(IN dword _mask)
{
	if(_mask < MAX_MASKS)
		mouse_pointer.mask = _mask;
}

/**
* @brief Change the mouse pointer color.
* @param _color [in] Indicates which color to use.
*/
PRIVATE void POINTER_SetNewColor(IN ARGB _color)
{
	mouse_pointer.color = _color;
}

/**
* @brief Manage mouse pointer movement. Gets called whenever a mouse move happens.
*/
PRIVATE void POINTER_OnMove()
{
	//Restore background
	POINTER_RestoreBackground();

	//Get new position
	mouse_pointer.position.x = MOUSE_GetX();
	if(mouse_pointer.position.x >= screen_size.width - POINTER_WIDTH)
		mouse_pointer.position.x = screen_size.width - POINTER_WIDTH;

	mouse_pointer.position.y = MOUSE_GetY();
	if(mouse_pointer.position.y <= POINTER_HEIGHT)
		mouse_pointer.position.y = POINTER_HEIGHT;
	
	//Save
	POINTER_SaveBackground();
	
	//Draw
	POINTER_Draw();
}

/**
* @brief Test if a given absolute coordinate is contained in a window.
* @param _index [in] Window heap index.
* @param _x [in] X coordinate.
* @param _y [in] Y coordinate.
* @return True if coordinate is contained in window, false otherwise.
*/
PRIVATE bool WINDOW_TestCoordinatesInWindow(IN dword _index, IN dword _x, IN dword _y)
{
	return 
		(
			(_x <= windows_heap[_index].window.area.up_right.x)
			&&
			(_x >= windows_heap[_index].window.area.down_left.x)
		)
		&&
		(
			(_y <= windows_heap[_index].window.area.up_right.y)
			&&
			(_y >= windows_heap[_index].window.area.down_left.y)
		);
}

/**
* @brief Returns a window handle given a coordinate.
* @param _x [in] X coordinate.
* @param _y [in] Y coordinate.
* @return The handle if in any used window, zero otherwise.
*/
PRIVATE WINDOW WINDOW_GetFromPointer(IN dword _x, IN dword _y)
{
	for(dword i = 0; i < MAX_WINDOWS; i++)
	{
		if(windows_heap[i].used)
		{
			if(WINDOW_TestCoordinatesInWindow(i, _x, _y))
				return TO_HANDLE(i);
		}
	}
	return 0;
}

/**
* @brief Window keyboard interrupt service.
* @param _frame [in] Interrupt frame.
* @return True if call chain can be continued, false otherwise.
*/
PRIVATE bool INTERRUPT WindowKeyboardHandler(IN INTERRUPT_FRAME* _frame)
{
	//Get window
	WINDOW window = WINDOW_GetFromPointer(mouse_pointer.position.x, mouse_pointer.position.y);
	if(window)
	{
		WINDOW_IMPL* w = &windows_heap[TO_INDEX(window)].window;

		if(w->keyboard_callback && w->keyboard_pdbr)
		{
			 if(ENVIRONMENT_GetCurrent() && ENVIRONMENT_OwnsWINDOW(ENVIRONMENT_GetCurrent(), window))
			 {
				 ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
				 ADDRESS_SPACE_SwitchTo(w->keyboard_pdbr);

				//Call callback if any
				dword scan_code = KBD_ScanCode();
				w->keyboard_callback(scan_code);

				ADDRESS_SPACE_SwitchTo(current);
			}
			else
			{
				w->keyboard_events[w->keyboard_index++] = KBD_ScanCode();
			}
		}
	}
	return true;
}

/**
* @brief Window mouse interrupt service.
* @param _frame [in] Interrupt frame.
* @return True if call chain can be continued, false otherwise.
*/
PRIVATE bool INTERRUPT WindowMouseHandler(IN INTERRUPT_FRAME* _frame)
{
	//Get window
	dword window = WINDOW_GetFromPointer(mouse_pointer.position.x, mouse_pointer.position.y);
	if(window)
	{
		WINDOW_IMPL* w = &windows_heap[TO_INDEX(window)].window;

		POINTER_SetNewColor(w->pointer_color);
		POINTER_SetNewMask(w->pointer_mask);

		if(w->mouse_callback && w->mouse_pdbr)
		{
			dword x = mouse_pointer.position.x - w->area.down_left.x;
			dword y = mouse_pointer.position.y - w->area.down_left.y;
			bool left = MOUSE_IsLeftButtonDown();
			bool right = MOUSE_IsRightButtonDown();

			if(ENVIRONMENT_GetCurrent() && ENVIRONMENT_OwnsWINDOW(ENVIRONMENT_GetCurrent(), window))
			{
				ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
				ADDRESS_SPACE_SwitchTo(w->mouse_pdbr);

				//Call callback if any
				w->mouse_callback(x, y, left, right);

				ADDRESS_SPACE_SwitchTo(current);
			}
			else
			{
				dword index = w->mouse_index;
				w->mouse_events[index].left = left;
				w->mouse_events[index].right = right;
				w->mouse_events[index].cursor.x = x;
				w->mouse_events[index].cursor.y = y;
				w->mouse_index++;
			}
		}
	}

	POINTER_OnMove();
	return true;
}

/**
* @brief Window initialization.
* @return Returns true if everything goes well.
*/
PUBLIC bool WINDOW_Init()
{
	WINDOW_LoadBackground();
	
	dword height = SVGA_GetHeight();
	dword half_height = height/2;
	dword width = SVGA_GetWidth();
	dword half_width = width/2;
	
	screen_size.height = height;
	screen_size.width = width;

	WINDOW_Init(0, 0, half_width, 0, half_height);
	WINDOW_Init(1, 0, half_width, half_height, height);
	WINDOW_Init(2, half_width, width, half_height, height);
	WINDOW_Init(3, half_width, width, 0, half_height);

	WINDOW_Clear(0, WINDOW_DEFAULT_COLOR);
	WINDOW_Clear(1, WINDOW_DEFAULT_COLOR);
	WINDOW_Clear(2, WINDOW_DEFAULT_COLOR);
	WINDOW_Clear(3, WINDOW_DEFAULT_COLOR);

	//Start Mouse Pointer
	POINTER_Init(POINTER_ARROW, POINTER_DEFAULT_COLOR);
	
	//Register keyboard and mouse handlers
	if(!INT_SetHandler(HardwareInterrupt, 1, WindowKeyboardHandler))
		return false;

	if(!INT_SetHandler(HardwareInterrupt, 12, WindowMouseHandler))
		return false;

	return true;
}

/**
* @brief Allocates a window.
* @return Returns the window resource, or zero if unable to complete.
*/
PUBLIC WINDOW WINDOW_GetWindow()
{
	for(dword i = 0; i < MAX_WINDOWS; i++)
	{
		if(!windows_heap[i].used)
		{
			windows_heap[i].used = true;
			return TO_HANDLE(i);
		}
	}
	return 0;
}

/**
* @brief Frees a window.
* @param _window [in] Window resource.
*/
PUBLIC void WINDOW_ReleaseWindow(IN WINDOW _window)
{
	if(_window && TO_INDEX(_window)<MAX_WINDOWS && windows_heap[TO_INDEX(_window)].used)
	{
		windows_heap[TO_INDEX(_window)].used = false;

		WINDOW_IMPL* w = &windows_heap[TO_INDEX(_window)].window;
		w->focus = false;

		w->pointer_color = POINTER_DEFAULT_COLOR;
		w->pointer_mask = POINTER_ARROW;

		w->keyboard_pdbr = 0;
		w->keyboard_callback = 0;
		w->keyboard_index = 0;

		w->mouse_pdbr = 0;
		w->mouse_callback = 0;
		w->mouse_index = 0;
	}
}

/**
* @brief Consults a window height.
* @param _window [in] Window resource.
* @return The height of the window.
*/
PUBLIC dword WINDOW_GetHeight(IN WINDOW _window)
{
	if(_window && TO_INDEX(_window)<MAX_WINDOWS && windows_heap[TO_INDEX(_window)].used)
	{
		WINDOW_IMPL* w = &windows_heap[TO_INDEX(_window)].window;

		return w->area.up_right.y - w->area.down_left.y;
	}
	return 0;
}

/**
* @brief Consults a window width.
* @param _window [in] Window resource.
* @return The width of the window.
*/
PUBLIC dword WINDOW_GetWidth(IN WINDOW _window)
{
	if(_window && TO_INDEX(_window)<MAX_WINDOWS && windows_heap[TO_INDEX(_window)].used)
	{
		WINDOW_IMPL* w = &windows_heap[TO_INDEX(_window)].window;

		return w->area.up_right.x - w->area.down_left.x;
	}
	return 0;
}

/**
* @brief Consults a window pixel.
* @param _window [in] Window resource.
* @param _x [in] X relative coordinate.
* @param _y [in] Y relative coordinate.
* @return The color of the pixel, or zero if invalid coordinates.
*/
PUBLIC ARGB WINDOW_GetPixel(IN WINDOW _window, IN dword _x, IN dword _y)
{
	if(_window && TO_INDEX(_window)<MAX_WINDOWS && windows_heap[TO_INDEX(_window)].used)
	{
		WINDOW_IMPL* w = &windows_heap[TO_INDEX(_window)].window;

		dword x_size = w->area.up_right.x - w->area.down_left.x;
		dword y_size = w->area.up_right.y - w->area.down_left.y;
		if(_x < x_size && _y < y_size)
		{
			return SVGA_GetPixel(w->area.down_left.x + _x, w->area.down_left.y + _y);
		}
	}
	return 0;
}

/**
* @brief Sets a window pixel.
* @param _window [in] Window resource.
* @param _x [in] X relative coordinate.
* @param _y [in] Y relative coordinate.
* @param _color [in] Color for the pixel.
*/
PUBLIC void WINDOW_SetPixel(IN WINDOW _window, IN dword _x, IN dword _y, IN ARGB _color)
{
	if(_window && TO_INDEX(_window)<MAX_WINDOWS && windows_heap[TO_INDEX(_window)].used)
	{
		WINDOW_IMPL* w = &windows_heap[TO_INDEX(_window)].window;

		dword x_size = w->area.up_right.x - w->area.down_left.x;
		dword y_size = w->area.up_right.y - w->area.down_left.y;
		if(_x <= x_size && _y <= y_size)
		{
			SVGA_SetPixel(w->area.down_left.x + _x, w->area.down_left.y + _y, _color);
			return;
		}
	}
}

/**
* @brief Prints text in the window.
* @param _window [in] Window resource.
* @param _x [in] X relative coordinate.
* @param _y [in] Y relative coordinate.
* @param _color [in] Color for the text.
* @param _text [in] String text.
*/
PUBLIC void WINDOW_PrintText(IN WINDOW _window, IN dword _x, IN dword _y, IN ARGB _color, IN string* _text)
{
	if(_window && TO_INDEX(_window)<MAX_WINDOWS && windows_heap[TO_INDEX(_window)].used)
	{
		WINDOW_IMPL* w = &windows_heap[TO_INDEX(_window)].window;

		dword x_size = w->area.up_right.x - w->area.down_left.x;
		dword y_size = w->area.up_right.y - w->area.down_left.y;
		if(_x <= x_size && _y <= y_size)
		{
			//Test we can put the text
			if((_y >= 8) && ((_x + _text->size * 8) < x_size))
			{
				SVGA_PrintText(w->area.down_left.x + _x, w->area.down_left.y + _y, _color, _text);
			}
		}
	}
}

/**
* @brief Registers a keyboard callback for a window.
* @param _window [in] Window resource.
* @param _callback [in] The callback.
*/
PUBLIC void WINDOW_RegisterKeyboard(IN WINDOW _window, IN ADDRESS_SPACE _pdbr, IN fWindowKeyboardCallback _callback)
{
	if(_window && TO_INDEX(_window)<MAX_WINDOWS && windows_heap[TO_INDEX(_window)].used)
	{
		WINDOW_IMPL* w = &windows_heap[TO_INDEX(_window)].window;

		w->keyboard_pdbr = _pdbr;
		w->keyboard_callback = _callback;
	}
}

/**
* @brief Registers a mouse callback for a window.
* @param _window [in] Window resource.
* @param _callback [in] The callback.
*/
PUBLIC void WINDOW_RegisterMouse(IN WINDOW _window, IN ADDRESS_SPACE _pdbr, IN fWindowMouseCallback _callback)
{
	if(_window && TO_INDEX(_window)<MAX_WINDOWS && windows_heap[TO_INDEX(_window)].used)
	{
		WINDOW_IMPL* w = &windows_heap[TO_INDEX(_window)].window;

		w->mouse_pdbr = _pdbr;
		w->mouse_callback = _callback;
	}
}

/**
* @brief Gets arrow pointer resource.
* @return A pointer resource representing an arrow.
*/
PUBLIC POINTER POINTER_GetArrow()
{
	return POINTER_ARROW;
}

/**
* @brief Gets clock pointer resource.
* @return A pointer resource representing a clock.
*/
PUBLIC POINTER POINTER_GetClock()
{
	return POINTER_CLOCK;
}

/**
* @brief Sets a pointer in a window.
* @param _window [in] Window resource.
* @param _pointer [in] Pointer resource.
*/
PUBLIC void WINDOW_SetPointer(IN WINDOW _window, IN POINTER _pointer)
{
	if(_window && TO_INDEX(_window)<MAX_WINDOWS && windows_heap[TO_INDEX(_window)].used)
	{
		WINDOW_IMPL* w = &windows_heap[TO_INDEX(_window)].window;

		w->pointer_mask = _pointer;
	}
}

/**
* @brief Sets a pointer in a window.
* @param _window [in] Window resource.
* @param _color [in] Color of the pointer.
*/
PUBLIC void WINDOW_SetPointerColor(IN WINDOW _window, IN ARGB _color)
{
	if(_window && TO_INDEX(_window)<MAX_WINDOWS && windows_heap[TO_INDEX(_window)].used)
	{
		WINDOW_IMPL* w = &windows_heap[TO_INDEX(_window)].window;

		w->pointer_color = _color;
	}
}

/**
* @brief Flush keyboard events to the window handler when environment owning the window gains execution.
* @param _window [in] Window resource.
*/
PUBLIC void WINDOW_FlushKeyboardCallbacks(IN WINDOW _window)
{
	if(_window && TO_INDEX(_window)<MAX_WINDOWS && windows_heap[TO_INDEX(_window)].used)
	{
		WINDOW_IMPL* w = &windows_heap[TO_INDEX(_window)].window;

		if(w->keyboard_callback && w->keyboard_pdbr)
		{
			ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
			ADDRESS_SPACE_SwitchTo(w->keyboard_pdbr);

			for(dword i = 0; i < w->keyboard_index; i++)
			{
				w->keyboard_callback(w->keyboard_events[i]);
			}
			w->keyboard_index = 0;

			ADDRESS_SPACE_SwitchTo(current);
		}
	}
}

/**
* @brief Flush mouse events to the window handler when environment owning the window gains execution.
* @param _window [in] Window resource.
*/
PUBLIC void WINDOW_FlushMouseCallbacks(IN WINDOW  _window)
{
	if(_window && TO_INDEX(_window)<MAX_WINDOWS && windows_heap[TO_INDEX(_window)].used)
	{
		WINDOW_IMPL* w = &windows_heap[TO_INDEX(_window)].window;

		if(w->mouse_callback && w->mouse_pdbr)
		{
			ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
			ADDRESS_SPACE_SwitchTo(w->mouse_pdbr);

			for(dword i = 0; i < w->mouse_index; i++)
			{
				w->mouse_callback(w->mouse_events[i].cursor.x, w->mouse_events[i].cursor.y, w->mouse_events[i].left, w->mouse_events[i].right);
			}
			w->mouse_index = 0;

			ADDRESS_SPACE_SwitchTo(current);
		}
	}
}
