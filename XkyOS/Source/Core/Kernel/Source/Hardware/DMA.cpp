/******************************************************************************/
/**
* @file		DMA.cpp
* @brief	XkyOS Hardware DMA Library
* Implementation of DMA operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Types.h"
#include "IO.h"
#include "DMA.h"
#include "Interrupts.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//

PRIVATE struct {
	int status_register;
	int command_register;
	int request_register;
	int mask_register;
	int mode_register;
	int clear_register;
	int temp_register;
	int disable_register;
	int clear_mask_register;
	int write_mask_register;
} controller_ports[] = {
  { 0x08, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0D, 0x0E, 0x0F },   
  { 0xD0, 0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDA, 0xDC, 0xDE }
};

PRIVATE struct {
	int base_current_address_register;
	int base_current_count_register;
	int page_register;
} channel_ports[] = {
  { 0x00, 0x01, 0x87 },
  { 0x02, 0x03, 0x83 },
  { 0x04, 0x05, 0x81 },
  { 0x06, 0x07, 0x82 },
  { 0xC0, 0xC2, 0x8F },
  { 0xC4, 0xC6, 0x8B },
  { 0xC8, 0xCA, 0x89 },
  { 0xCC, 0xCE, 0x8A }
};

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

PUBLIC void EnableController(int controller)
{
	// Enables the selected DMA controller.  Disabling is recommended before
	// setting other registers.

	// Bit 2 is cleared
	IO_OutPortByte(controller_ports[controller].command_register, 0x00);
	//kernelProcessorDelay();
}	


PUBLIC void DisableController(int controller)
{
	// Disables the selected DMA controller, which is recommended before
	// setting other registers.

	// Bit 2 is set
	IO_OutPortByte(controller_ports[controller].command_register, 0x04);
	//kernelProcessorDelay();
}


PRIVATE void WriteWordPort(int port, int value)
{
	// Does sequential 2-write port outputs for a couple of the registers

	byte data;

	// Set the controller register.  Start with the low byte.
	data = (byte) (value & 0xFF);
	IO_OutPortByte(port, data);
	//kernelProcessorDelay();

	// Now the high byte
	data = (byte) ((value >> 8) & 0xFF);
	IO_OutPortByte(port, data);
	//kernelProcessorDelay();
}


PUBLIC int OpenChannel(int channel, void* address, int count, int mode)
{
	// This routine prepares the registers of the specified DMA channel for
	// a data transfer.   This routine calls a series of other routines that
	// set individual registers.

	int status = 0;
	int controller = 0;
	int interrupts = 0;
	unsigned segment = 0;
	unsigned offset = 0 ;
	unsigned char data;

	if (channel > 7)
		//return (status = ERR_NOSUCHENTRY);
		return (status = 0);

	if (channel >= 4)
		controller = 1;

	// Convert the "address" argument we were passed into a base address
	// and page register
	segment = ((unsigned) address >> 16);
	offset = ((unsigned) address - (segment << 16));

	// Disable the controller while setting registers
	DisableController(controller);

	// Clear interrupts while setting DMA controller registers
	interrupts = INT_DisableInterrupts();

	// 1. Disable the channel.  Mask out all but the bottom two bits of the
	// channel number, then turn on the disable 'mask' bit
	data = (byte) ((channel & 0x03) | 0x04);
	IO_OutPortByte(controller_ports[controller].mask_register, data);
	//kernelProcessorDelay();

	// 2. Set the channel and mode.  "or" the channel with the mode
	data = (byte) ((mode | channel) & 0xFF);
	IO_OutPortByte(controller_ports[controller].mode_register, data);
	//kernelProcessorDelay();

	// 3. Do channel setup.
	// Reset the byte flip-flop before the following actions, as they each
	// require two consecutive port writes.  Value is unimportant.
	IO_OutPortByte(controller_ports[controller].clear_register, 0x01);
	//kernelProcessorDelay();

	// Set the base and current address register
	WriteWordPort(channel_ports[channel].base_current_address_register, offset);

	// Set the base and current count register, but subtract 1 first
	count--;
	WriteWordPort(channel_ports[channel].base_current_count_register, count);

	// Set the page register
	data = (byte) (segment & 0xFF);
	IO_OutPortByte(channel_ports[channel].page_register, data);
	//kernelProcessorDelay();

	// 4. Enable the channel.  Mask out all but the bottom two bits of the
	// channel number.
	data = (byte) (channel & 0x03);
	IO_OutPortByte(controller_ports[controller].mask_register, data);
	//kernelProcessorDelay();

	INT_EnableInterrupts(interrupts);

	// Re-enable the appropriate controller
	EnableController(controller);

	return (status = 0);
}

PUBLIC int CloseChannel(int channel)
{
	// This routine disables the selected DMA channel by setting the
	// appropriate mask bit.  

	int status = 0;
	int controller = 0;
	int interrupts = 0;
	byte data;

	if (channel >= 4)
		controller = 1;

	// Disable the controller while setting registers
	DisableController(controller);

	// Clear interrupts while setting DMA controller registers
	interrupts = INT_DisableInterrupts();

	// Mask out all but the bottom two bits of the channel number, as above,
	// then turn on the 'mask' bit
	data = (byte) ((channel & 0x03) | 0x04);
	IO_OutPortByte(controller_ports[controller].mask_register, data);
	//kernelProcessorDelay();

	INT_EnableInterrupts(interrupts);

	// Re-enable the appropriate controller
	EnableController(controller);

	return (status = 0);
}
