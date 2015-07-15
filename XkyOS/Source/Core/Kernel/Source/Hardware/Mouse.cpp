/******************************************************************************/
/**
* @file		Mouse.cpp
* @brief	XkyOS Hardware Mouse Library
* Implementation of mouse operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Types.h"
#include "Graphics.h"
#include "Interrupts.h"
#include "SVGA.h"
#include "8042.h"
#include "Mouse.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//

/**
* @brief Data provided by the mouse controller when a mouse interrupt happens.
*/
struct MOUSE_CONTROLLER_DATA
{
	byte info;
	byte dx;
	byte dy;
};

/**
* @brief Indicates the state of the mouse, it's coordinates and the state of the buttons.
*/
struct MOUSE_STATE
{
	COORDINATE cursor;
	dword right;
	dword left;
};

#define MOUSE_ERROR_NODATA 0xFFFFFFFF

ALIGN(4)
PRIVATE MOUSE_CONTROLLER_DATA mouse_controller_data = {0, 0, 0};
	
ALIGN(4)
PRIVATE MOUSE_STATE mouse_state = {{0, 0}, 0, 0};

ALIGN(4)
PRIVATE SIZE mouse_screen_size = {0, 0};
	
//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
/**
* @brief Retrieves the data packet from the controller.
*/
PRIVATE void MOUSE_GetDataPacket()
{
	//First byte has information about buttons state and sign information for the next 2 bytes
	mouse_controller_data.info = MC8042_ReadFromDataRegister();

	//Second byte is X delta change
	mouse_controller_data.dx = MC8042_ReadFromDataRegister();

	//Third byte is Y delta change
	mouse_controller_data.dy = MC8042_ReadFromDataRegister();
}

/**
* @brief Updates mouse coordinates after a mouse event has happened.
*/
PRIVATE void MOUSE_MoveUpdate()
{
	int difference;

	//Actualize X position
	if(mouse_controller_data.info & 0x10)
		difference = (256 - mouse_controller_data.dx)*(-1);
	else
		difference = mouse_controller_data.dx;

	mouse_state.cursor.x += difference;
	if(mouse_state.cursor.x >= 0x80000000)
		mouse_state.cursor.x = 0;
	else
	{
		if(mouse_state.cursor.x >= mouse_screen_size.width)
			mouse_state.cursor.x = mouse_screen_size.width - 1;
	}

	//Actualize Y position
	if(mouse_controller_data.info & 0x20)
		difference = (256 - mouse_controller_data.dy)*(-1);
	else
		difference = mouse_controller_data.dy;

	mouse_state.cursor.y += difference;
	if(mouse_state.cursor.y >= 0x80000000)
		mouse_state.cursor.y = 0;
	else
	{
		if(mouse_state.cursor.y >= mouse_screen_size.height)
			mouse_state.cursor.y = mouse_screen_size.height - 1;
	}
}

/**
* @brief Updates mouse buttons state after a mouse event has happened.
*/
PRIVATE void MOUSE_ButtonsUpdate()
{
	//Actualize left button state
	if(mouse_controller_data.info & 0x01)
		mouse_state.left = 1;
	else
		mouse_state.left = 0;

	//Actualize right button state
	if(mouse_controller_data.info & 0x02)
		mouse_state.right = 1;
	else
		mouse_state.right = 0;
}

/**
* @brief Mouse service interrupt.
* @param _frame [in] Interrupt frame.
* @return True if call chain can be continued, false otherwise.
*/
PRIVATE bool INTERRUPT MouseInterrupt(IN INTERRUPT_FRAME* _frame)
{
	//Obtain data from the peripheral
	MOUSE_GetDataPacket();

	//Actualize data
	MOUSE_ButtonsUpdate();
	MOUSE_MoveUpdate();

	//Continue chain
	return true;
}

/**
* @brief Mouse initialization.
* @return Returns true if everything goes well.
*/
PUBLIC bool MOUSE_Init()
{
	//Width
	mouse_screen_size.width = SVGA_GetWidth();
	mouse_state.cursor.x = mouse_screen_size.width/2;
		
	//Height
	mouse_screen_size.height = SVGA_GetHeight();
	mouse_state.cursor.y = mouse_screen_size.height/2;

	//Activate the mouse
	MC8042_EnableMouse();

	//Reset the mouse
	if(!MC8042_SendMouseCommand(0xFF))
		return false;

	//Check reset was ok, 0xAA = 'self test passed'
	if(MC8042_ReadFromDataRegister() != 0xAA)
		return false;

	//Get ID. 0x00 for PS/2 mouse
	if(MC8042_ReadFromDataRegister() != 0x00)
		return false;

	//Set 'scaling' to 2:1
	if(!MC8042_SendMouseCommand(0xE7))
		return false;

	//Activate data send (stream mode)
	if(!MC8042_SendMouseCommand(0xF4))
		return false;

	//Tell controller to issue mouse interrupts
	MC8042_WriteControllerCommandByte(MC8042_ReadControllerCommandByte() | 0x02);

	//Set interrupt handler
	return INT_SetHandler(HardwareInterrupt, 12, MouseInterrupt);
}
	
/**
* @brief Return X position of the mouse.
* @return Mouse X position.
*/
PUBLIC dword MOUSE_GetX()
{
	return mouse_state.cursor.x;
}

/**
* @brief Return Y position of the mouse.
* @return Mouse Y position.
*/
PUBLIC dword MOUSE_GetY()
{
	return mouse_state.cursor.y;
}

/**
* @brief Indicates if the left button is pressed.
* @return Mouse left button pressed.
*/
PUBLIC bool MOUSE_IsLeftButtonDown()
{
	return mouse_state.left == 1;
}

/**
* @brief Indicates if the right button is pressed.
* @return Mouse right button pressed.
*/
PUBLIC bool MOUSE_IsRightButtonDown()
{
	return mouse_state.right == 1;
}
