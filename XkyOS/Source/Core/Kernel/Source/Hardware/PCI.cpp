/******************************************************************************/
/**
* @file		PCI.cpp
* @brief	XkyOS Hardware PCI Library
* Implementation of peripheral component interconnect operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/

#include "Types.h"
#include "IO.h"
#include "PCI.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//

#define PCI_ADDRESS_PORT	0x0CF8
#define PCI_DATA_PORT		0x0CFC

PRIVATE PCI pci_devices[256];	
PRIVATE dword pci_number_of_devices = 0;
//Currently unused
//PRIVATE dword* pci_bios = 0;

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
/*
* @brief Scans bios ROM memory looking for PCI bios entry point.
* @return Returns the address of the PCI bios (if found).
*/
/*
//Currently unused
PRIVATE dword* PCI_FindBios()
{
	for(dword* i=(dword*)0x000E0000; i<(dword*)0x00100000; i+=4)
	{
		if(*i == '_32_') return i;
	}
	return 0;
}
*/

/**
* @brief Prepares a PCI address from it's physical addresses.
* @param _bus [in] Bus the device is located on.
* @param _device [in] Device(ID) that represents the device on the bus.
* @param _function [in] Function in the device.
* @return Returns a PCI address.
*/
PRIVATE PCI PCI_CreateAddress(IN byte _bus, IN byte _device, IN byte _function)
{
	return (0x80000000 | (((dword) ((_bus) & 0xFF) << 16) |	(((_device) & 0x1F)  << 11) | (((_function) & 0x07) << 8)));
}

/**
* @brief Scans the PCI bus looking for logical PCI devices.
*/
PRIVATE void PCI_ScanBus()
{
	for(byte bus = 0; bus < 255; bus++)
	{
		for(byte device = 0; device < 32; device++)
		{
			for(byte function = 0; function < 8; function++)
			{
				//Create the address of the device
				PCI address = PCI_CreateAddress(bus, device, function);
				
				//Read first 2 words (vendor and device ID's)
				word vendorID =  PCI_ReadWord(address, 0);
				word deviceID = PCI_ReadWord(address, 0);

				//Check if theres a device
				if ((vendorID == 0x0000) ||	(vendorID == 0xFFFF) ||	(deviceID == 0xFFFF))
					continue;

				//Add device
				pci_devices[pci_number_of_devices++] = address;
			}
		}
	}
}

/**
* @brief PCI initialization routine.
* @return Returns true on successful initialization, false otherwise.
*/
PUBLIC bool PCI_Init()
{
	//Currently unused
	//pci_bios = PCI_FindBios();

	//Scan he bus for hardware
	PCI_ScanBus();
	
	return true;
}

/**
* @brief Indicates gow many different logical PCI devices are connected.
* @return Returns the number of PCI devices found.
*/
PUBLIC dword PCI_GetNumberOfDevices()
{
	return pci_number_of_devices;
}

/**
* @brief Returns a PCI address for a given index.
* @param _index [in] Index of the device we want.
* @return Returns the address of the device.
*/
PUBLIC PCI PCI_GetDevice(IN dword _index)
{
	return pci_devices[_index];
}

/**
* @brief Reads a byte from the configuration space of a device.
* @param _device [in] Device we want to read from.
* @param _offset [in] Byte-offset we want to read.
* @return Returns the byte at the byte-offset location in the config space of the device.
*/
PUBLIC byte PCI_ReadByte(IN PCI _device, IN dword _offset)
{
	//Send
	IO_OutPortDword(PCI_ADDRESS_PORT, _device + (((_offset/4)<<2) & 0x0000003F));

	//Read
	return IO_InPortByte(PCI_DATA_PORT + (word)(_offset%4));
}

/**
* @brief Writes a byte in the configuration space of a device.
* @param _device [in] Device we want to write to.
* @param _offset [in] Byte-offset we want to write.
* @param _data [in] Data that will replace the old one.
*/
PUBLIC void PCI_WriteByte(IN PCI _device, IN dword _offset, IN byte _data)
{
	//Send
	IO_OutPortDword(PCI_ADDRESS_PORT, _device + (((_offset/4)<<2) & 0x0000003F));

	//Write
	IO_OutPortByte(PCI_DATA_PORT + (word)(_offset%4), _data);
}

/**
* @brief Reads a word from the configuration space of a device.
* @param _device [in] Device we want to read from.
* @param _offset [in] Word-offset we want to read.
* @return Returns the word at the word-offset location in the config space of the device.
*/
PUBLIC word PCI_ReadWord(IN PCI _device, IN dword _offset)
{
	//Send
	IO_OutPortDword(PCI_ADDRESS_PORT, _device + (((_offset/2)<<2) & 0x0000003F));

	//Read
	return IO_InPortWord(PCI_DATA_PORT + (word)(_offset%2));
}

/**
* @brief Writes a word in the configuration space of a device.
* @param _device [in] Device we want to write to.
* @param _offset [in] Word-offset we want to write.
* @param _data [in] Data that will replace the old one.
*/
PUBLIC void PCI_WriteWord(IN PCI _device, IN dword _offset, IN word _data)
{
	//Send
	IO_OutPortDword(PCI_ADDRESS_PORT, _device + (((_offset/2)<<2) & 0x0000003F));

	//Write
	IO_OutPortWord(PCI_DATA_PORT + (word)(_offset%2), _data);
}

/**
* @brief Reads a dword from the configuration space of a device.
* @param _device [in] Device we want to read from.
* @param _offset [in] Dword-offset we want to read.
* @return Returns the dword at the dword-offset location in the config space of the device.
*/
PUBLIC dword PCI_ReadDword(IN PCI _device, IN dword _offset)
{
	//Send
	IO_OutPortDword(PCI_ADDRESS_PORT, _device + ((_offset<<2) & 0x0000003F));

	//Read
	return IO_InPortDword(PCI_DATA_PORT);
}

/**
* @brief Writes a dword in the configuration space of a device.
* @param _device [in] Device we want to write to.
* @param _offset [in] Dword-offset we want to write.
* @param _data [in] Data that will replace the old one.
*/
PUBLIC void PCI_WriteDword(IN PCI _device, IN dword _offset, IN dword _data)
{
	//Send
	IO_OutPortDword(PCI_ADDRESS_PORT, _device + ((_offset<<2) & 0x0000003F));
	
	//Write
	IO_OutPortDword(PCI_DATA_PORT, _data);
}
