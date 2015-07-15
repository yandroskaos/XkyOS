/******************************************************************************/
/**
* @file		RTC.cpp
* @brief	XkyOS Hardware Real Time Clock Library
* Implementation of real time clock operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/

#include "Types.h"
#include "IO.h"
#include "Interrupts.h"
#include "RTC.h"

// Register numbers
#define RTC_SECONDS_REGISTER	0
#define RTC_MINUTES_REGISTER	2
#define RTC_HOUR_REGISTER		4
#define RTC_DAY_WEEK_REGISTER	6
#define RTC_DAY_MONTH_REGISTER	7
#define RTC_MONTH_REGISTER		8
#define RTC_YEAR_REGISTER		9

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
/**
* @brief Real time clock initialization.
* @return Returns true if everything goes well.
*/
PUBLIC bool RTC_Init()
{
	return true;
}

/**
* @brief Waits until it's safe to use the real time clock.
*/
PRIVATE void RTC_WaitClockReady()
{
	do
	{
		IO_OutPortByte(0x70, 0x0A);
	}
	while(IO_InPortByte(0x71) & 0x80);
}

/**
* @brief Read a register from the real time clock.
* @param _rtc_register [in] The real time register we want to read.
* @return the value of the register.
*/
PRIVATE byte RTC_ReadRegister(IN byte _rtc_register)
{
	//Suspend interrupts
	dword state = INT_DisableInterrupts();

	//Wait until the clock is stable
	RTC_WaitClockReady();

	//We have 244 us to read the data we want. Disable NMI at the same time.
	IO_OutPortByte(0x70, _rtc_register | 0x80);

	//Read the data
	byte data = IO_InPortByte(0x71);

	//Reenable NMI
	IO_OutPortByte(0x70, 0x00);

	//Restore interrupts
	INT_EnableInterrupts(state);

	//The data is in BCD format. Convert it to binary.
	return ((byte) ((((data & 0xF0) >> 4) * 10) + (data & 0x0F)));
}

/**
* @brief Reads the seconds.
* @return Current seconds.
*/
PUBLIC byte RTC_Seconds()
{
	return RTC_ReadRegister(RTC_SECONDS_REGISTER);
}

/**
* @brief Reads the minutes.
* @return Current minute.
*/
PUBLIC byte RTC_Minutes()
{
	return RTC_ReadRegister(RTC_MINUTES_REGISTER);
}

/**
* @brief Reads the hour.
* @return Current hour.
*/
PUBLIC byte RTC_Hour()
{
	return RTC_ReadRegister(RTC_HOUR_REGISTER);
}

/**
* @brief Reads the day of the week.
* @return Current day of the week.
*/
PUBLIC DaysOfWeek RTC_DayOfWeek()
{
	return (DaysOfWeek)RTC_ReadRegister(RTC_DAY_WEEK_REGISTER);
}

/**
* @brief Reads the day of the month.
* @return Current day of the month.
*/
PUBLIC byte RTC_DayOfMonth()
{
	return RTC_ReadRegister(RTC_DAY_MONTH_REGISTER);
}

/**
* @brief Reads the month.
* @return Current month.
*/
PUBLIC byte RTC_Month()
{
	return RTC_ReadRegister(RTC_MONTH_REGISTER);
}

/**
* @brief Reads the year.
* @return Current year.
*/
PUBLIC dword RTC_Year()
{
	return 2000 + RTC_ReadRegister(RTC_YEAR_REGISTER);
}
