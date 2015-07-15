/******************************************************************************/
/**
* @file		PCI.h
* @brief	XkyOS Hardware PCI Library
* Definitions of peripheral component interconnect operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __PCI_H__
#define __PCI_H__

	/**
	* @brief PCI addresses type.
	*/
	typedef dword PCI;

	bool	PCI_Init();

	dword	PCI_GetNumberOfDevices	();
	PCI		PCI_GetDevice			(IN dword _index);

	/**
	* @brief Get PCI Bus.
	*/
	#define PCI_GetBusFromDirection(X)		((byte)(((X) & 0x00FF0000) >> 16))	//byte PCI_GetBusFromDirection(IN PCI _address)
	/**
	* @brief Get PCI Device.
	*/
	#define PCI_GetDeviceFromDirection(X)	((byte)(((X) & 0x0000F800) >> 11))	//byte PCI_GetDeviceFromDirection(IN PCI _address)
	/**
	* @brief  Get PCI Function.
	*/
	#define PCI_GetFunctionFromDirection(X)	((byte)(((X) & 0x00000700) >> 8))	//byte PCI_GetFunctionFromDirection(IN PCI _address)

	byte	PCI_ReadByte	(IN PCI _device, IN dword _offset);
	void	PCI_WriteByte	(IN PCI _device, IN dword _offset, IN byte _data);
	word	PCI_ReadWord	(IN PCI _device, IN dword _offset);
	void	PCI_WriteWord	(IN PCI _device, IN dword _offset, IN word _data);
	dword	PCI_ReadDword	(IN PCI _device, IN dword _offset);
	void	PCI_WriteDword	(IN PCI _device, IN dword _offset, IN dword _data);

#endif //__PCI_H__
