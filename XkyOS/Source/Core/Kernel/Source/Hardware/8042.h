/******************************************************************************/
/**
* @file		8042.h
* @brief	XkyOS Hardware MicroChip 8042 Library
* Definitions of 8042 controller operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __8042_H__
#define __8042_H__

	//Direct access to 8042
	void MC8042_WriteToCommandRegister	(IN byte _command);
	byte MC8042_ReadFromDataRegister	();
	void MC8042_WriteToDataRegister		(IN byte _data);

	byte MC8042_ReadControllerCommandByte	();
	void MC8042_WriteControllerCommandByte	(IN byte _command);

	//Mouse functions
	void MC8042_EnableMouse			();
	void MC8042_DisableMouse		();
	bool MC8042_SendMouseCommand	(IN byte _command);
	
	//Keyboard functions
	void MC8042_EnableKeyboard		();
	void MC8042_DisableKeyboard		();
	bool MC8042_SendKeyboardCommand	(IN byte _command);

#endif //__8042_H__
