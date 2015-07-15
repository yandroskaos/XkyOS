/******************************************************************************/
/**
* @file		Keyboard.cpp
* @brief	XkyOS Hardware Keyboard Library
* Implementation of keyboard operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Types.h"
#include "Interrupts.h"
#include "IO.h"
#include "8042.h"
#include "Keyboard.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
PRIVATE dword kbd_scan_code = 0;
	
//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
/**
* @brief Keyboard interrupt service.
* @param _frame [in] Interrupt frame.
* @return True if call chain can be continued, false otherwise.
*/
PRIVATE bool INTERRUPT KeyboardInterrupt(IN INTERRUPT_FRAME* _frame)
{
	//It doesn't work... Controller stops comunicating
	//MC8254_ReadFromDataRegister();
	
	//Get scan_code
	kbd_scan_code = (dword)IO_InPortByte(0x60);

	//We could test scan_code to see if it's an extended one, inhibit the interrupt chain
	//and compose when the next interrupt is issued. By now we allow suscriptors to see
	//things "as is"
	return true;
}

/**
* @brief Keyboard initialization.
* @return Returns true if everything goes well.
*/
PUBLIC bool KBD_Init()
{
	//Activate keyboard
	MC8042_EnableKeyboard();

	//Tell keyboard to issue interrupts
	MC8042_WriteControllerCommandByte(MC8042_ReadControllerCommandByte()|0x01);
	
	//Set interrupt handler
	return INT_SetHandler(HardwareInterrupt, 1, KeyboardInterrupt);
}

/**
* @brief Returns the last scan code issued by the keyboard.
* @return Returns the last scan code.
*/
PUBLIC dword KBD_ScanCode()
{
	return kbd_scan_code;
}
