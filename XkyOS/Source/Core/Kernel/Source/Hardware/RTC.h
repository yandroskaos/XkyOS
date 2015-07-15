/******************************************************************************/
/**
* @file		RTC.h
* @brief	XkyOS Hardware Real Time Clock Library
* Definitions of real time clock operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __RTC_H__
#define __RTC_H__

	/**
	* @brief The days of the week :).
	*/
	enum DaysOfWeek
	{
		Sunday = 1,
		Monday,
		Tuesday,
		Wednesday,
		Thursday,
		Friday,
		Saturday
	};

	bool RTC_Init();

	byte		RTC_Seconds();
	byte		RTC_Minutes();
	byte		RTC_Hour();
	DaysOfWeek	RTC_DayOfWeek();
	byte		RTC_DayOfMonth();
	byte		RTC_Month();
	dword		RTC_Year();

#endif //__RTC_H__
