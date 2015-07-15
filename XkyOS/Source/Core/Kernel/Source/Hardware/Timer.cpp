/******************************************************************************/
/**
* @file		Timer.cpp
* @brief	XkyOS Hardware Timer Library
* Implementation of timer operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/

#include "Types.h"
#include "Interrupts.h"
#include "IO.h"
#include "Timer.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//

PRIVATE byte tmr_ports[]		= { 0x40, 0x41, 0x42 };
PRIVATE byte tmr_latch_command[]= { 0x00, 0x04, 0x08 };
PRIVATE byte tmr_data_command[]	= { 0x03, 0x07, 0x0B };
PRIVATE dword tmr_ticks = 0;

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
/**
* @brief Timer interrupt service.
* @param _frame [in] Interrupt frame.
* @return True if call chain can be continued, false otherwise.
*/
PRIVATE bool INTERRUPT TimerInterrupt(IN INTERRUPT_FRAME* _frame)
{
	tmr_ticks++;
	return true;
}

/**
* @brief Timer initialization routine.
* @return Returns true on successful initialization, false otherwise.
*/
PUBLIC bool TMR_Init()
{
	//Register our interrupt handler
	return INT_SetHandler(HardwareInterrupt, 0, TimerInterrupt);
}

/**
* @brief Number of ticks.
* @return Returns the number of cpu ticks since the machine was turned on.
*/
PUBLIC dword TMR_Ticks()
{
	return tmr_ticks;
}

/*
* @brief This function is used to select and read one of the system timer counters
* @param _timer [in] The timer we want to read its count
* @return The value of the counter
*/
//Currently unused
/*
PUBLIC dword TMR_Count(IN byte _timer)
{
	//Make sure the timer number is not greater than 2.  This driver only
	//supports timers 0 through 2 (since that's all most systems will have)
	if(_timer > 2)
		return 0;

	//Before we can read the timer reliably, we must send a command
	//to cause it to latch the current value.  Calculate which latch 
	//command to use
	byte command = tmr_latch_command[_timer];
	command <<= 4;

	//Send the command to the general command port
	IO_OutPortByte(0x43, command);

	//The counter will now be expecting us to read two bytes from
	//the applicable port.

	//Read the low byte first, followed by the high byte
	dword timer_value = 0;
	byte data = IO_InPortByte(tmr_ports[_timer]);
	timer_value = data;
	data = IO_InPortByte(tmr_ports[_timer]);
	timer_value |= (data << 8);

	return timer_value;
}
*/

/*
* @brief This function is used to select, set the mode and count of one of the system timer counters.
* @param _timer [in] The timer we want to set up.
* @param _mode [in] The counting mode of the timer.
* @param _count [in] The initial count.
* @return True if the timer could be set up correctly, false otherwise.
*/
//Currently unused
/*
PUBLIC bool TMR_Setup(IN byte _timer, IN byte _mode, IN word _count)
{
	//Make sure the timer number is not greater than 2.  This driver only
	//supports timers 0 through 2 (since that's all most systems will have)
	if((_timer > 2) || (_mode > 5))
		return false;

	//Calculate the data command to use
	byte command = tmr_data_command[_timer];
	command <<= 4;

	//Or the command with the mode (shifted left by one).  The
	//result is the formatted command byte we'll send to the timer
	command |= (mode << 1);

	//We can send the command to the general command port
	IO_OutPortByte(0x43, command);

	//The timer is now expecting us to send two bytes which represent
	//the initial count of the timer.  We will get this value from
	//the parameters.

	// Send low byte first, followed by the high byte to the data
	byte data = (byte) (_count & 0x00FF);
	IO_OutPortByte(tmr_ports[_timer], data);
	data = (byte) ((_count >> 8) & 0x00FF);
	IO_OutPortByte(tmr_ports[_timer], data);

	return true;
}
*/
