/******************************************************************************/
/**
* @file		IO.h
* @brief	XkyOS Hardware IO Library
* Definitions of IO operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __IO_H__
#define __IO_H__

	//Reading
	byte	IO_InPortByte	(IN word _port);
	word	IO_InPortWord	(IN word _port);
	dword	IO_InPortDword	(IN word _port);

	//Writing
	void	IO_OutPortByte	(IN word _port, IN byte _data);
	void	IO_OutPortWord	(IN word _port, IN word _data);
	void	IO_OutPortDword	(IN word _port, IN dword _data);

#endif //__IO_H__
