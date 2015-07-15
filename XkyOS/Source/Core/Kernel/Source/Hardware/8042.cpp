/******************************************************************************/
/**
* @file		8042.cpp
* @brief	XkyOS Hardware MicroChip 8042 Library
* Implementation of 8042 controller operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Types.h"
#include "IO.h"
#include "8042.h"

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

#define CONTROLLER_DATA_REGISTER	0x60 /**< 8042 data register*/
#define CONTROLLER_COMMAND_REGISTER	0x64 /**< 8042 command register*/
#define CONTROLLER_STATUS_REGISTER	0x64 /**< 8042 status register*/

/**
* @brief This function waits until the controller is prepared to write.
*/
PRIVATE void MC8042_WaitForControllerReadyToWrite()
{
	while(IO_InPortByte(CONTROLLER_STATUS_REGISTER) & 0x02);
}

/**
* @brief This function waits until the controller is prepared to read.
*/
PRIVATE void MC8042_WaitForControllerReadyToRead()
{
	while(!(IO_InPortByte(CONTROLLER_STATUS_REGISTER) & 0x01));
}

/**
* @brief This function writes a byte in 8042 command register.
*
* @param _command [in]	Command to send to 8042 controller.
*/
PUBLIC void MC8042_WriteToCommandRegister(IN byte _command)
{
	MC8042_WaitForControllerReadyToWrite();
	IO_OutPortByte(CONTROLLER_COMMAND_REGISTER, _command);
}

/**
* @brief This function reads a byte from 8042 data register.
*
* @return The data byte in the 8042.
*/
PUBLIC byte MC8042_ReadFromDataRegister()
{
	MC8042_WaitForControllerReadyToRead();
	return IO_InPortByte(CONTROLLER_DATA_REGISTER);
}

/**
* @brief This function writes a byte in 8042 data register.
*
* @param _data [in]	Data to send to 8042 controller.
*/
PUBLIC void MC8042_WriteToDataRegister(IN byte _data)
{
	MC8042_WaitForControllerReadyToWrite();
	IO_OutPortByte(CONTROLLER_DATA_REGISTER, _data);
}

/**
* @brief This function reads the command from the 8042 controller.
*
* @return The data byte in the 8042.
*/
PUBLIC byte MC8042_ReadControllerCommandByte()
{
	MC8042_WriteToCommandRegister(0x20);
	return MC8042_ReadFromDataRegister();
}

/**
* @brief This function sends a command to the 8042 controller.
*
* @param _command [in] The command for the 8042.
*/
PUBLIC void MC8042_WriteControllerCommandByte(IN byte _command)
{
	MC8042_WriteToCommandRegister(0x60);
	MC8042_WriteToDataRegister(_command);
}

#define MOUSE_CMD_ENABLE	0x000000A8 /**< 8042 command for mouse enabling*/
#define MOUSE_CMD_DISABLE	0x000000A7 /**< 8042 command for mouse disabling*/
#define MOUSE_CMD_PREFIX	0x000000D4 /**< 8042 command prefix for mouse commands*/

#define KBD_CMD_ENABLE	0x000000AE /**< 8042 command for keyboard enabling*/
#define KBD_CMD_DISABLE	0x000000AD /**< 8042 command for keyboard disabling*/

/**
* @brief This function enables the mouse.
*/
PUBLIC void MC8042_EnableMouse()
{
	MC8042_WriteToCommandRegister(MOUSE_CMD_ENABLE);
}

/**
* @brief This function disables the mouse.
*/
PUBLIC void MC8042_DisableMouse()
{
	MC8042_WriteToCommandRegister(MOUSE_CMD_DISABLE);
}

/**
* @brief This function send a command to the mouse.
*
* @param _command [in] The command for the mouse.
*/
PUBLIC bool MC8042_SendMouseCommand(IN byte _command)
{
	MC8042_WriteToCommandRegister(MOUSE_CMD_PREFIX);
	MC8042_WriteToDataRegister(_command);

	//Verify the ack
	return MC8042_ReadFromDataRegister() == 0xFA;
}

/**
* @brief This function enables the keyboard.
*/
PUBLIC void MC8042_EnableKeyboard()
{
	MC8042_WriteToCommandRegister(KBD_CMD_ENABLE);
}

/**
* @brief This function disables the keyboard.
*/
PUBLIC void MC8042_DisableKeyboard()
{
	MC8042_WriteToCommandRegister(KBD_CMD_DISABLE);
}

/**
* @brief This function send a command to the keyboard.
*
* @param _command [in] The command for the keyboard.
*/
PUBLIC bool MC8042_SendKeyboardCommand(IN byte _command)
{
	MC8042_WriteToDataRegister(_command);

	//Verify the ack
	return MC8042_ReadFromDataRegister() == 0xFA;
}
